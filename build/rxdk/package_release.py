"""Validate and package a completed RXDK build without touching old XDK outputs."""
from pathlib import Path
import argparse, datetime, hashlib, json, shutil, subprocess, sys
from zipfile import ZipFile, ZIP_DEFLATED

W=Path(__file__).resolve().parent
ROOT=W.parent.parent
sys.path.insert(0,str(ROOT))
from verify_xbe import inspect, patch_retail

OLD={
    'dist/MAMEoXtras-2026-Raiden2-AB/default.xbe':'fed6a60b9465d442f976eeb03bea7f3af24f973943bf08794de1cba0f1532f2f',
    'dist/MAMEoXtras-2026-Raiden2-AB/MAMEoX.xbe':'1021572c84d7474a3184646b12a893767517de5d4925042844443c2332b840e1',
    'dist/MAMEoXtras-2026-Raiden2-AB-XBE.zip':'cc3bfabf069f5e089ba436ed240726c9540dd7e0207bcc2e8ad02eeb8433a445',
}

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def preserved():
    records=[]
    for rel,expected in OLD.items():
        p=ROOT/rel
        if not p.is_file():
            records.append({'path':rel,'present':False})
            continue
        actual=sha(p)
        if actual!=expected:
            raise RuntimeError('Existing original XDK output changed: '+rel)
        records.append({'path':rel,'present':True,'sha256':expected,'unchanged':True})
    return records

def main():
    sys.stdout.reconfigure(encoding='utf8',errors='replace')
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--label',required=True)
    ap.add_argument('--output-name',default='MAMEoXtras-2026-Raiden2-AB-RXDK')
    a=ap.parse_args()
    if Path(a.output_name).name!=a.output_name or a.output_name in ('','.','..'):
        raise ValueError('output-name must be one filename component')
    out=ROOT/'dist'/a.output_name
    archive=out.with_name(out.name+'-XBE.zip')
    if out.exists() or archive.exists():
        raise FileExistsError('Use a fresh output-name; existing packages are never overwritten')

    old=preserved()
    sweep=json.loads((W/'logs'/('parallel-'+a.label)/'summary.json').read_text(encoding='utf8'))
    if not sweep['full_sweep'] or sweep['failed'] or sweep['passed']!=sweep['units'] or sweep['units']!=1956:
        raise RuntimeError('Expected a clean 1956-unit full compile sweep')
    for r in sweep['results']:
        if not r['ok'] or sha(Path(r['source']))!=r['source_sha256']:
            raise RuntimeError('Compile source changed or did not pass: '+r['source'])
        if sha(Path(r['object']))!=r['object_sha256']:
            raise RuntimeError('Object differs from the successful full sweep: '+r['object'])

    link=json.loads((W/'logs'/('link-'+a.label+'-summary.json')).read_text(encoding='utf8'))
    if {r['project'] for r in link}!={'MAMEoX','MAMEoXLauncher'} or any(r['exit_code'] for r in link):
        raise RuntimeError('Both official CLI executable builds must pass')
    link_warnings=[]
    for r in link:
        text=Path(r['log']).read_text(encoding='utf8',errors='replace')
        if 'Resolving _' in text and ' by linking to ' in text:
            raise RuntimeError('Calling-convention auto-fixup remains: '+r['project'])
        link_warnings += [l for l in text.splitlines() if 'warning:' in l.lower()]
    if link_warnings:
        raise RuntimeError('Final executable link emitted warnings')

    raw=[]
    for name,target in [('MAMEoXLauncher','default.xbe'),('MAMEoX','MAMEoX.xbe')]:
        p=W/'projects'/name/'out-release'/(name+'.xbe')
        result=inspect(p)
        if p.stat().st_mtime < sweep['started_unix']:
            raise RuntimeError('XBE predates final build: '+str(p))
        if result['title_id']!='0x4D414D46':
            raise RuntimeError('Unexpected title ID')
        raw.append((p,target,result))

    names={s['name'] for s in raw[1][2]['sections']}
    for name in ('598','DRVSNIZE','CPUSNIZE','STARTUP'):
        if name not in names:
            raise RuntimeError('Required game/boot section missing: '+name)
    for s in raw[1][2]['sections']:
        if (s['name'].isdigit() or s['name'].startswith('CPU')) and not s['preload']:
            raise RuntimeError('Startup driver/CPU section is not preloaded: '+s['name'])
        if ((s['name'].startswith('CPU') and s['name'][3:].isdigit()) or
            (s['name'].isdigit() and s['name'] != '531')) and not (s['flags'] & 1):
            raise RuntimeError('Sectionized CPU/driver region is not writable: '+s['name'])

    out.mkdir(parents=True)
    packaged=[]
    for p,target,result in raw:
        dest=out/target
        shutil.copy2(p,dest)
        packaged.append(patch_retail(dest))
    for folder in ('Media','Skins','Docs'):
        shutil.copytree(W/'source'/folder,out/folder)
    for file in ('README_AUTOFIRE.txt','AutoFire-defaults.ini.example','README_RAIDEN2.txt','vm-raiden2-recommended.txt'):
        shutil.copy2(ROOT/file,out/file)
    license_dir=out/'Licenses'
    license_dir.mkdir()
    for source,name in [(W/'toolchain/sdk/LICENSE.md','RXDK-SDK-LICENSE.md'),
                        (W/'toolchain/samples/LICENSE.md','RXDK-Samples-LICENSE.md')]:
        if source.exists():
            shutil.copy2(source,license_dir/name)

    record={
        'status':'built_and_structurally_verified',
        'hardware_tested':False,
        'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
        'source_branch_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        'compile_units':sweep['units'],
        'compile_passed':sweep['passed'],
        'compile_failed':sweep['failed'],
        'compiler_warning_lines':sum(r['warnings'] for r in sweep['results']),
        'link_warning_lines':0,
        'link_warnings':[],
        'rxdk_vsix':'1.3.2',
        'compiler':'Xbox LLVM Clang 24.0.0git',
        'target':'i686-pc-windows-gnu',
        'cpu':'pentium3',
        'raw_xbes':[{k:v for k,v in r.items() if k!='sections'} for _,_,r in raw],
        'packaged_xbes':packaged,
        'original_xdk_artifacts':old,
        'native_windows_vmm_test':json.loads((W/'probes/vmm-native/result.json').read_text()),
        'native_windows_dsound_abi_test':json.loads((W/'probes/dsound-abi/result.json').read_text()),
        'limitations':[
            'No Xbox hardware, emulator, ROM or FPS test.',
            'Retail encoding is for homebrew-capable consoles, not a Microsoft retail signature.',
            'Source media package is not the complete original release; retain vm.txt and configuration.',
        ],
    }
    (out/'BUILD_VERIFICATION.json').write_text(json.dumps(record,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
    (out/'SHA256SUMS.txt').write_text(
        ''.join(sha(out/name)+'  '+name+'\n' for name in ('default.xbe','MAMEoX.xbe')),
        encoding='ascii'
    )
    (out/'README_BUILD.txt').write_text(
        """MAMEoXtras 2026 Raiden2 + AutoFire / RXDK 1.3.2 build

启动 default.xbe。MAMEoX.xbe 是模拟器核心，两者必须放在同一目录。
请先备份现有安装，再在单独的测试目录使用本包；不要混用新旧两套 XBE。
保留完整 MAMEoXtras 发行版的 vm.txt、配置及运行资源。本包包含源码所带
Media、Skins、Docs，不是完整发行版，也不包含游戏 ROM。

本次使用 Xbox LLVM/RXDK 编译、链接和生成 XBE，没有使用 XDK 5849 编译产物。
源码来自 rxdk-build 分支，并在 build/rxdk/source 隔离副本中生成资源和对象。
VMM 保留缺页续执行功能，使用 GNU ABI x86 异常注册实现。
已验证：全量编译、链接、XBE 结构/分段哈希/内核导入，以及 Windows 主机上的
VMM 缺页处理和声音数学函数调用约定测试。
未验证：Xbox 真机、模拟器启动、游戏正确性、帧率，以及真机 VMM 压力行为。
主机测试不等同于真机测试。

XBE 采用 homebrew retail 编码，不是微软官方零售签名。
完整构建信息见 BUILD_VERIFICATION.json，二进制哈希见 SHA256SUMS.txt。
""",
        encoding='utf-8-sig'
    )

    with ZipFile(archive,'x',compression=ZIP_DEFLATED,compresslevel=6) as z:
        for p in sorted(out.rglob('*')):
            if p.is_file() and p.name.lower()!='thumbs.db':
                z.write(p,Path(out.name)/p.relative_to(out))
    with ZipFile(archive) as z:
        bad=z.testzip()
        if bad:
            raise RuntimeError('ZIP CRC failure: '+bad)
        for name in ('default.xbe','MAMEoX.xbe'):
            if hashlib.sha256(z.read(out.name+'/'+name)).hexdigest()!=sha(out/name):
                raise RuntimeError('ZIP binary mismatch: '+name)

    preserved()
    record['package']={
        'directory':str(out),
        'zip':str(archive),
        'zip_bytes':archive.stat().st_size,
        'zip_sha256':sha(archive),
    }
    (W/'logs'/('package-'+a.label+'.json')).write_text(
        json.dumps(record,indent=2,ensure_ascii=False)+'\n',encoding='utf8'
    )
    print('PACKAGE_OK',out,flush=True)
    for r in packaged:
        print(Path(r['file']).name,r['bytes'],r['sha256'],r['encoding'],flush=True)
    print('ZIP',archive,archive.stat().st_size,sha(archive),flush=True)
    print('COMPILE',sweep['passed'],sweep['failed'],'LINK_WARNINGS',len(link_warnings),flush=True)
    return 0

if __name__=='__main__':
    sys.exit(main())
