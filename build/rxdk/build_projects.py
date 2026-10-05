"""Run RXDK CLI archive/link builds and retain diagnostic summaries."""
from pathlib import Path
import argparse, concurrent.futures, datetime, json, os, subprocess, sys

from fix_xbe_section_permissions import restore_sectionizer_writable

WORK=Path(__file__).resolve().parent
ROOT=WORK.parent.parent

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('projects',nargs='+',choices=['libsmb','MAME','SharedResources','MAMEoX','MAMEoXLauncher'])
    parser.add_argument('--label',required=True)
    parser.add_argument('--parallel',type=int,default=1)
    args=parser.parse_args()
    if args.parallel>1 and any(p in ('MAMEoX','MAMEoXLauncher') for p in args.projects):
        parser.error('Executable builds share library outputs and must run sequentially')
    cli=WORK/'vsix-1.3.2/tools/Rxdk.Cli.exe'
    if not cli.is_file():
        raise FileNotFoundError('RXDK CLI missing; run build/rxdk/setup_rxdk.py first')
    (WORK/'logs').mkdir(exist_ok=True)
    env=os.environ.copy()
    env.update(RXDK=str(WORK/'toolchain'),
               RXDK_STAGED_SDK=str(WORK/'toolchain/sdk'),
               RXDK_STAGED_TOOLS=str(WORK/'toolchain/tools'),
               RXDK_STAGED_SAMPLES=str(WORK/'toolchain/samples'))
    results=[]
    def build(name):
        log=WORK/'logs'/f'{name}-{args.label}.log'
        command=[str(cli),'build','--project-root',str(WORK/'projects'/name),'--configuration','Release']
        start=datetime.datetime.now(datetime.timezone.utc)
        print('START',name,start.isoformat(),flush=True)
        with log.open('xb') as f:
            proc=subprocess.run(command,cwd=ROOT,env=env,stdin=subprocess.DEVNULL,stdout=f,stderr=subprocess.STDOUT)
        if proc.returncode == 0 and name == 'MAMEoX':
            xbe=WORK/'projects'/name/'out-release'/'MAMEoX.xbe'
            fixed=restore_sectionizer_writable(xbe)
            print('SECTION_FLAGS',name,'RESTORED_WRITABLE',len(fixed),flush=True)
        lines=log.read_text(encoding='utf8',errors='replace').splitlines()
        errors=[line for line in lines if 'error:' in line or 'build failed:' in line]
        print('RESULT',name,'EXIT',proc.returncode,'ERROR_DIAGNOSTICS',len(errors),flush=True)
        for line in errors[:25]:
            print(line,flush=True)
        for line in lines[-3:]:
            print(line[:1400],flush=True)
        return {'project':name,'command':command,'exit_code':proc.returncode,
                'started_utc':start.isoformat(),'ended_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
                'errors':errors,'log':str(log)}
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1,min(2,args.parallel))) as pool:
        for result in pool.map(build,args.projects):
            results.append(result)
    summary=WORK/'logs'/f'{args.label}-summary.json'
    with summary.open('x',encoding='utf8') as f:
        json.dump(results,f,indent=2)
        f.write('\n')
    return 1 if any(r['exit_code'] for r in results) else 0

if __name__=='__main__':
    sys.exit(main())
