"""Fixture-test strict Base resolution without touching real repositories."""
import json,os,subprocess,tempfile
from pathlib import Path
repo=Path(__file__).resolve().parents[1]
def run(*args,**kwargs):return subprocess.run(args,capture_output=True,text=True,**kwargs)
with tempfile.TemporaryDirectory(prefix='malemod-resolver-test-') as temporary:
 root=Path(temporary);adapter=root/'adapter';base=root/'base';(adapter/'tools').mkdir(parents=True);(adapter/'dependencies').mkdir();base.mkdir()
 (adapter/'tools/Resolve-Base.ps1').write_bytes((repo/'tools/Resolve-Base.ps1').read_bytes());headers=['surface/pelvic_frame.hpp','surface/garment_support.hpp','garments/jockstrap.hpp','garments/numerical_support.hpp','controls/presentation.hpp']
 for header in headers:
  p=base/'include/malemod'/header;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('// fixture\n')
 for args in [('init',),('config','user.name','Fixture'),('config','user.email','fixture@example.invalid'),('add','.'),('commit','-m','fixture')]:
  result=run('git','-C',str(base),*args);assert result.returncode==0,result.stderr
 commit=run('git','-C',str(base),'rev-parse','HEAD').stdout.strip();lock={'schemaVersion':1,'repository':'https://github.com/gkoogz/MaleModBase.git','commit':commit};lockpath=adapter/'dependencies/base.lock.json';lockpath.write_text(json.dumps(lock));env=dict(os.environ,MALEMOD_BASE_PATH=str(base));command=['pwsh','-NoProfile','-File',str(adapter/'tools/Resolve-Base.ps1')]
 def check(success,diagnostic=False):
  result=run(*command,*(['-Diagnostic'] if diagnostic else []),env=env);assert (result.returncode==0)==success,result.stdout+result.stderr
  if success:assert Path(result.stdout.strip())==base
 check(True);(base/'include/malemod/garments/jockstrap.hpp').write_text('// dirty\n');check(False);check(True,True);run('git','-C',str(base),'restore','.');
 (base/'include/malemod/garments/new.hpp').write_text('// untracked\n');check(False);(base/'include/malemod/garments/new.hpp').unlink();lock['commit']='0'*40;lockpath.write_text(json.dumps(lock));check(False);check(True,True);lock['commit']=commit;lockpath.write_text(json.dumps(lock));(base/'include/malemod/garments/jockstrap.hpp').unlink();check(False);check(False,True)
print('PASS pinned Base path, dirty tracked/untracked rejection, wrong pin, explicit diagnostic and missing API gates')
