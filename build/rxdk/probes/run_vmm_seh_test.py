"""Build/run the shared VMM SEH scope on 32-bit Windows using RXDK LLVM only.
Host kernel32 import records are generated from the COFF short-import format.
No Microsoft XDK compiler, object or library is consumed by this test.
"""
from pathlib import Path
import json, struct, subprocess, sys
P = Path(__file__).resolve().parent
W = P.parent
TC = W/'toolchain/llvm/xbox-windows-x64/bin'
OUT = P/'vmm-native'
OUT.mkdir(exist_ok=True)

def run(args, label):
    r = subprocess.run([str(a) for a in args], cwd=W, capture_output=True, timeout=60)
    text=(r.stdout+r.stderr).decode('utf8',errors='replace')
    (OUT/(label+'.log')).write_text(text,encoding='utf8')
    print(label, 'exit', r.returncode, text[-2200:], flush=True)
    if r.returncode: raise SystemExit(r.returncode)

sys.stdout.reconfigure(encoding='utf8',errors='replace')
imports=[]
for name,nbytes in [('VirtualAlloc',16),('VirtualFree',12),('RaiseException',16),('ExitProcess',4)]:
    payload=(f'_{name}@{nbytes}\0KERNEL32.dll\0').encode('ascii')
    # IMPORT_CODE (0) | IMPORT_NAME_UNDECORATE (3 << 2)
    header=struct.pack('<HHHHIIHH',0,0xffff,0,0x14c,0,len(payload),0,12)
    obj=OUT/(name+'.obj'); obj.write_bytes(header+payload); imports.append(obj)
lib=OUT/'kernel32-host.lib'
run([TC/'llvm-ar.exe','rcs',lib]+imports,'import-lib')
obj=OUT/'test.obj'
exe=OUT/'test.exe'
run([TC/'clang.exe','-target','i686-pc-windows-gnu','-std=c++14','-O2','-march=pentium3',
     '-fno-exceptions','-fno-rtti','-fno-stack-protector','-ffreestanding','-fno-builtin',
     '-c',P/'vmm_seh_native_test.cpp','-o',obj], 'compile')
run([TC/'lld.exe','-flavor','link','/machine:x86','/subsystem:console','/entry:mainCRTStartup',
     '/nodefaultlib','/safeseh:no','/dynamicbase:no','/out:'+str(exe),obj,lib], 'link')
r=subprocess.run([str(exe)],cwd=OUT,capture_output=True,timeout=15)
result={'test':'native-win32-demand-paging-and-nested-seh','exit_code':r.returncode,
        'passed':r.returncode==0,'xbox_hardware_test':False,'cxx_unwind_runtime_test':False}
(OUT/'result.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
print(json.dumps(result),flush=True)
sys.exit(0 if r.returncode==0 else 1)
