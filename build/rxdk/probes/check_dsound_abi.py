"""Test the already-built production DSOUND bridge object on 32-bit Windows.
Only the host test supplies synthetic cdecl math implementations. The production
object remains byte-for-byte unchanged; no host test input enters the Xbox link.
"""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys

W=Path(__file__).resolve().parent.parent
ROOT=W.parent.parent
sys.path.insert(0,str(W))
import compile_parallel as recipe

def main():
    sys.stdout.reconfigure(encoding='utf8',errors='replace')
    unit=next(u for u in recipe.units(['MAMEoX']) if u[1].name=='RxdkDsoundMath.c')
    if not unit[2].is_file():raise FileNotFoundError('Compile the main project first')
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    original=digest(unit[2])
    out=W/'probes/dsound-abi';out.mkdir(exist_ok=True)
    def run(cmd,label):
        r=subprocess.run([str(a) for a in cmd],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=60)
        (out/(label+'.log')).write_bytes(r.stdout)
        if r.returncode:raise RuntimeError(label+': '+r.stdout.decode('utf8',errors='replace')[-3000:])
    command=list(unit[3]);command[command.index('-c')]='-S'
    command.remove('-MD');i=command.index('-MF');del command[i:i+2]
    for i,x in enumerate(command):
        if x.startswith('-o'):command[i]='-o'+str(out/'bridge.s')
    run(command,'bridge-assembly')
    text=(out/'bridge.s').read_text(errors='replace')
    for symbol,n in [('sin',8),('sinh',8),('pow',16)]:
        start=text.index('_'+symbol+'@'+str(n)+':')
        end=text.find('# -- End function',start)
        body=text[start:end]
        if not re.search(r'calll\s+_'+symbol+r'\b',body) or not re.search(r'retl\s+\$'+str(n)+r'\b',body):
            raise RuntimeError('Unexpected calling convention: '+symbol)
    obj=out/'test.obj';exe=out/'test.exe'
    run([recipe.CLANG,'-target','i686-pc-windows-gnu','-std=gnu99','-O2','-march=pentium3',
         '-fno-stack-protector','-fno-builtin','-ffreestanding','-c',W/'probes/dsound_math_abi_test.c','-o',obj],'test-compile')
    run([recipe.CLANG.parent/'lld.exe','-flavor','link','/machine:x86','/subsystem:console','/entry:mainCRTStartup',
         '/nodefaultlib','/safeseh:no','/dynamicbase:no','/out:'+str(exe),obj,unit[2],W/'probes/vmm-native/kernel32-host.lib'],'test-link')
    r=subprocess.run([str(exe)],cwd=out,capture_output=True,timeout=15)
    result={'test':'stdcall-cdecl-dsound-math-bridges','iterations':10000,'calls':30000,
            'exit_code':r.returncode,'passed':r.returncode==0,'synthetic_host_math_only':True,
            'production_object_sha256':original,'production_object_unchanged':digest(unit[2])==original,
            'xbox_hardware_test':False}
    (out/'result.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result),flush=True)
    return 0 if result['passed'] and result['production_object_unchanged'] else 1

if __name__=='__main__':sys.exit(main())
