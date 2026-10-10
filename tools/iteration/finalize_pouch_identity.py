"""Freeze exact production inputs and match a sealed validation build to them."""
from pathlib import Path
import argparse,hashlib,json

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(__doc__);ap.add_argument('--production',type=Path,required=True);ap.add_argument('--native',type=Path,required=True);a=ap.parse_args()
 prod=json.loads((a.production/'provenance.json').read_text(encoding="utf-8"));native=json.loads((a.native/'provenance.json').read_text(encoding="utf-8"))
 assert sha(a.production/'d3d9.dll')==prod['runtimeSHA256']
 assert sha(a.native/'d3d9.dll')==native['runtimeSHA256']
 assert all(native['production'][k]==prod[k] for k in ['sourceCommit','baseCommit','sourceOverlays'])
 cpp='source/src/runtime/d3d9_proxy.cpp';text=(a.native/cpp).read_text(encoding="utf-8")
 for before,after in reversed(native['driverOnlyChanges']):
  assert text.count(after)==1
  text=text.replace(after,before,1)
 assert text==(a.production/cpp).read_text(encoding="utf-8")
 assert 'MaleModPrivateFactory' not in text and 'sealed_d3d9.hpp' not in text and 'sandbox_world_origin.hpp' not in text
 entries=[]
 for folder in ['source/src/runtime','source/third-party','base/include']:
  for p in sorted((a.production/folder).rglob('*')):
   if not p.is_file() or p.suffix.lower() in ['.obj','.pdb','.lib','.exp','.res','.exe','.dll']:continue
   relative=p.relative_to(a.production).as_posix();digest=sha(p)
   if relative!=cpp:assert sha(a.native/relative)==digest,relative
   entries.append(dict(path=relative,sha256=digest))
 identity=dict(sourceCommit=prod['sourceCommit'],baseCommit=prod['baseCommit'],sourceOverlays=prod['sourceOverlays'],sourceFiles=entries)
 digest=hashlib.sha256(json.dumps(identity,sort_keys=True,separators=(',',':')).encode()).hexdigest()
 prod.update(schema='wolverine.production-build/2',sourceMode='commit-plus-hashed-overlays',dirtySourceIncluded=True,privateFactoryIncluded=False,sandboxInputHooksIncluded=False,sandboxWorldOriginIncluded=False,sourceIdentity=digest,sourceFiles=entries)
 native.update(sourceIdentity=digest,productionSourceParityVerified=True)
 (a.production/'provenance.json').write_text(json.dumps(prod,indent=2),encoding='utf-8');(a.native/'provenance.json').write_text(json.dumps(native,indent=2),encoding='utf-8')
 print('Verified and froze',len(entries),'source/input files; identity',digest)
if __name__=='__main__':main()
