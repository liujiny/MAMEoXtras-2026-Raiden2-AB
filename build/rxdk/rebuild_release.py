"""Full RXDK release build for the rxdk-build branch."""
from pathlib import Path
import argparse, datetime, json, os, re, subprocess, sys

W=Path(__file__).resolve().parent
ROOT=W.parent.parent

def main():
    sys.stdout.reconfigure(encoding='utf8',errors='replace')
    default=datetime.datetime.now(datetime.timezone.utc).strftime('rebuild-%Y%m%dT%H%M%S%f')
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--label',default=default)
    ap.add_argument('--jobs',type=int,default=8,choices=range(1,9))
    ap.add_argument('--output-name')
    a=ap.parse_args()
    if not re.fullmatch(r'[A-Za-z0-9_-]{1,80}',a.label):
        raise ValueError('Invalid label')
    if (W/'logs'/('parallel-'+a.label)).exists():
        raise FileExistsError('Choose a new build label')
    override=os.environ.get('RXDK_BUILD_PYTHON')
    raw=Path(override) if override else Path(sys.executable)
    py=raw if raw.is_file() else raw.parent/'python.exe'
    if not py.is_file():
        raise FileNotFoundError('Set RXDK_BUILD_PYTHON to an executable Python 3 launcher')
    output=a.output_name or 'MAMEoXtras-2026-Raiden2-AB-RXDK-'+a.label
    env=os.environ.copy()
    completed=[]
    def status(stage,state):
        value={'round':a.label,'stage':stage,'state':state,'completed_stages':completed,
               'at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
        temp=W/'BUILD_PROGRESS.json.tmp'
        temp.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')
        os.replace(temp,W/'BUILD_PROGRESS.json')
    try:
        stage='setup-check'; status(stage,'running')
        r=subprocess.run([str(py),'-u','-B',str(W/'setup_rxdk.py'),'--check'],cwd=ROOT,env=env)
        completed.append({'stage':stage,'exit_code':r.returncode})
        if r.returncode:
            raise RuntimeError('RXDK prerequisites are missing; run build/rxdk/setup_rxdk.py')

        stage='prepare-source'; status(stage,'running')
        r=subprocess.run([str(py),'-u','-B',str(W/'prepare_source.py')],cwd=ROOT,env=env)
        completed.append({'stage':stage,'exit_code':r.returncode})
        if r.returncode:
            raise RuntimeError('source mirror preparation failed')

        stage='resources'; status(stage,'running')
        (W/'logs').mkdir(exist_ok=True)
        for rdf in sorted((W/'source/SharedResources/MediaFiles').glob('*.rdf')):
            log=W/'logs'/('resource-'+a.label+'-'+rdf.stem+'.log')
            with log.open('xb') as f:
                r=subprocess.run([str(W/'toolchain/tools/bundler.exe'),rdf.name,'-q'],cwd=rdf.parent,
                                 env=env,stdin=subprocess.DEVNULL,stdout=f,stderr=subprocess.STDOUT,timeout=180)
            if r.returncode:
                raise RuntimeError('Resource failure: '+str(log))
        completed.append({'stage':stage,'exit_code':0})

        steps=[
            ('compile',[str(W/'compile_parallel.py'),'--label',a.label,'--jobs',str(a.jobs)]),
            ('archive',[str(W/'build_projects.py'),'libsmb','MAME','SharedResources','--label','archive-'+a.label,'--parallel','2']),
            ('link',[str(W/'build_projects.py'),'MAMEoX','MAMEoXLauncher','--label','link-'+a.label]),
            ('test-vmm',[str(W/'probes/run_vmm_seh_test.py')]),
            ('test-dsound',[str(W/'probes/check_dsound_abi.py')]),
            ('package',[str(W/'package_release.py'),'--label',a.label,'--output-name',output]),
        ]
        for stage,args in steps:
            status(stage,'running'); print('STAGE',stage,flush=True)
            r=subprocess.run([str(py),'-u','-B']+args,cwd=ROOT,env=env,timeout=3600)
            completed.append({'stage':stage,'exit_code':r.returncode})
            if r.returncode:
                raise RuntimeError(stage+' failed; see logs and BUILD_PROGRESS.json')
        status('done','completed')
        print('BUILD_AND_PACKAGE_COMPLETE',ROOT/'dist'/output,flush=True)
        return 0
    except Exception as error:
        status(stage,'failed')
        print('BUILD_FAILED',str(error),file=sys.stderr,flush=True)
        return 1

if __name__=='__main__':
    sys.exit(main())
