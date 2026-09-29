"""Compile every Release source with the verified RXDK LLVM recipe in parallel."""
from pathlib import Path
import argparse, concurrent.futures, hashlib, json, os, re, subprocess, sys, time

W=Path(__file__).resolve().parent
ROOT=W.parent.parent
SDK=W/'toolchain/sdk'
LLVM=W/'toolchain/llvm/xbox-windows-x64'
CLANG=LLVM/'bin/clang.exe'
WARNINGS=['-Wno-c++11-narrowing','-Wno-address-of-temporary',
          '-Wno-ignored-pragma-intrinsic','-Wno-multichar',
          '-Wno-unused-command-line-argument','-Wno-deprecated-enum-enum-conversion']

def units(names):
    result=[]
    for name in names:
        project=W/'projects'/name
        doc=json.loads((project/'rxdk.project.json').read_text(encoding='utf8'))
        config=dict(doc)
        config.update(doc['configurations']['Release'])
        out=project/config.get('outputDir','out')
        out.mkdir(exist_ok=True)
        seen=set()
        for rel in config['sources']:
            if Path(rel).is_absolute() or ':' in rel:
                raise ValueError('A relative source path is required: '+rel)
            source=project/rel
            if not source.is_file():
                raise FileNotFoundError(source)
            obj=out/Path(rel.replace('/','_').replace('\\','_')).with_suffix('.obj')
            key=str(obj).lower()
            if key in seen:
                raise ValueError('Object name collision: '+str(obj))
            seen.add(key)
            cpp=source.suffix.lower() in ('.cpp','.cxx')
            flags=[]
            if cpp:
                flags += ['-std='+config.get('cppStandard','c++23'),'-nostdinc++',
                          '-fexceptions' if config.get('exceptions',True) else '-fno-exceptions','-frtti',
                          '-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR','-I'+str(SDK/'include/c++/v1'),
                          '-fms-compatibility-version=19.20','-U_WIN32','-U__MINGW32__','-D_GNU_SOURCE']
            else:
                flags += ['-std=c23']
            flags += ['-target','i686-pc-windows-gnu','-isystem',str(LLVM/'lib/clang/24/include'),
                      '-O3','-fno-sanitize=undefined','-ffreestanding','-fno-stack-protector',
                      '-fms-extensions','-fms-compatibility','-nostdinc','-include','picolibc.h',
                      '-march=pentium3','-D_XBOX','-DXBOX','-fno-builtin','-U_DEBUG',
                      '-D__ASSERT_VERBOSE','-femulated-tls','-gline-tables-only','-I'+str(SDK/'include')]
            for inc in config.get('includePaths',[]):
                p=(project/inc).resolve()
                if not p.is_dir():
                    raise FileNotFoundError(p)
                flags.append('-I'+str(p))
            flags += ['-D'+d for d in config.get('defines',[]) if d.strip()]
            flags += WARNINGS + config.get('compileFlags',[])
            flags += ['-x','c++' if cpp else 'c','-c',str(source),'-o'+str(obj),'-MD','-MF',str(obj)+'.d']
            result.append((name,source,obj,[str(CLANG)]+flags))
    return result

def main():
    sys.stdout.reconfigure(encoding='utf8',errors='replace')
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--label',required=True)
    parser.add_argument('--jobs',type=int,default=8,choices=range(1,9))
    parser.add_argument('--projects',nargs='+',default=['libsmb','MAME','MAMEoX','MAMEoXLauncher'])
    parser.add_argument('--match',help='Only source paths containing this text; not a full-build claim')
    options=parser.parse_args()
    if not CLANG.is_file():
        raise FileNotFoundError('RXDK LLVM missing; run setup_rxdk.py')
    jobs=units(options.projects)
    if options.match:
        jobs=[j for j in jobs if options.match.lower() in str(j[1]).lower()]
    if not jobs:
        raise ValueError('No matching compile units')
    logdir=W/'logs'/('parallel-'+options.label)
    logdir.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy()
    for key in ('CL','_CL_','INCLUDE','LIB','LIBPATH','CPATH','C_INCLUDE_PATH','CPLUS_INCLUDE_PATH','OBJC_INCLUDE_PATH'):
        env.pop(key,None)
    started=time.time()
    print('RXDK_LLVM_COMPILER',CLANG,flush=True)
    print('COMPILER_SHA256',hashlib.sha256(CLANG.read_bytes()).hexdigest(),flush=True)
    print('COMPILE_UNITS',len(jobs),'WORKERS',options.jobs,flush=True)
    def compile_one(unit):
        name,source,obj,command=unit
        key=hashlib.sha256((name+'\0'+str(source)).encode()).hexdigest()[:16]
        log=logdir/(name+'-'+source.stem+'-'+key+'.log')
        argsfile=log.with_suffix('.command.json')
        argsfile.write_text(json.dumps(command,indent=2)+'\n',encoding='utf8')
        source_hash=hashlib.sha256(source.read_bytes()).hexdigest()
        with log.open('xb') as f:
            proc=subprocess.run(command,stdin=subprocess.DEVNULL,stdout=f,stderr=subprocess.STDOUT,env=env,cwd=ROOT)
        changed=hashlib.sha256(source.read_bytes()).hexdigest()!=source_hash
        text=log.read_text(encoding='utf8',errors='replace')
        errors=[line for line in text.splitlines() if ': error:' in line or ': fatal error:' in line or line.startswith('error:')]
        ok=proc.returncode==0 and obj.is_file() and Path(str(obj)+'.d').is_file() and not changed
        if not ok:
            print('FAILED',name,source.name,'exit',proc.returncode,flush=True)
        return {'project':name,'source':str(source),'source_sha256':source_hash,
                'object':str(obj),'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest() if ok else None,
                'command_file':str(argsfile),'exit_code':proc.returncode,'ok':ok,
                'source_changed_during_compile':changed,'warnings':len(re.findall(r': warning:',text)),
                'errors':errors,'log':str(log)}
    results=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=options.jobs) as pool:
        pending=[pool.submit(compile_one,u) for u in jobs]
        for future in concurrent.futures.as_completed(pending):
            results.append(future.result())
            if len(results)%100==0 or len(results)==len(jobs):
                print('PROGRESS',len(results),'/',len(jobs),'FAILED',sum(not r['ok'] for r in results),flush=True)
    summary={'compiler':str(CLANG),'started_unix':started,'duration_seconds':time.time()-started,
             'units':len(results),'passed':sum(r['ok'] for r in results),'failed':sum(not r['ok'] for r in results),
             'full_sweep':not bool(options.match),'results':sorted(results,key=lambda r:(r['project'],r['source']))}
    (logdir/'summary.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf8')
    for r in summary['results']:
        if not r['ok']:
            print('ERROR_SUMMARY',r['project'],r['source'],flush=True)
            print('\n'.join(r['errors'][:3]),flush=True)
    print('COMPLETE',summary['passed'],'passed',summary['failed'],'failed',round(summary['duration_seconds'],1),'seconds',flush=True)
    print('SUMMARY',logdir/'summary.json',flush=True)
    return 1 if summary['failed'] else 0

if __name__=='__main__':
    sys.exit(main())
