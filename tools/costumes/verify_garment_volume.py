"""Check authored source lineage and full-waist coverage without game SDKs."""
import argparse,hashlib,json
from pathlib import Path
import numpy as np
from verify_jeans_motion import values


def audit(header,stock):
 text=header.read_text();s=json.loads(stock.read_text(encoding='utf-8-sig'))
 original={v['id']:v for v in s['vertices']}
 import struct
 counts=[]
 for variant in (0,1):
  suffix=str(variant);p=values(text,'vertices'+suffix,11)[:,:3]
  donors=values(text,'sourceDonors'+suffix,3).astype(int);weights=values(text,'sourceWeights'+suffix,3)
  resources=values(text,'sourceResource'+suffix,1)[:,0];uv=values(text,'uv'+suffix,2)
  t=values(text,'triangles'+suffix,3).astype(int);roles=values(text,'materialRoles'+suffix,1)[:,0]
  assert np.isfinite(p).all() and t.min()>=0 and t.max()<len(p)
  assert np.allclose(weights.sum(1),1,atol=2e-8) and weights.min()>-1e-8
  maximumUVError=0
  for i in np.where(resources==0)[0]:
   expected=sum(w*np.array([struct.unpack('<e',struct.pack('<H',x))[0] for x in original[d]['uv']]) for d,w in zip(donors[i],weights[i]))
   maximumUVError=max(maximumUVError,float(abs(expected-uv[i]).max()))
  assert maximumUVError<2e-8,'A volume corner lost its original texture lineage'
  belt=t[np.all(roles[t]==5,axis=1)]
  if not variant:
   samples=np.arange(0,360,.125)*np.pi/180;covered=np.zeros(len(samples),bool)
   for f in belt:
    a=np.arctan2(p[f,1],p[f,0]);center=np.arctan2(np.sin(a).mean(),np.cos(a).mean())
    a=np.angle(np.exp(1j*(a-center)));b=np.angle(np.exp(1j*(samples-center)))
    covered|=(b>=a.min()-1e-8)&(b<=a.max()+1e-8)
   assert covered.all(),'Original belt band has an angular gap around the waist'
  counts.append(dict(vertices=len(p),beltTriangles=len(belt),maximumOriginalUVError=maximumUVError))
 return dict(schema='wolverine.garment-volume-audit/1',headerSHA256=hashlib.sha256(header.read_bytes()).hexdigest(),stockExportSHA256=hashlib.sha256(stock.read_bytes()).hexdigest(),counts=counts,closedBeltAngularCoverageSamples=2880,closedBeltCompleteAngularCoverage=True,nativeVerified=False,bodyCollisionVerified=False,fullAttachmentGatePassed=False)


if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('--header',type=Path,required=True);ap.add_argument('--stock',type=Path,required=True);ap.add_argument('--output',type=Path);a=ap.parse_args();r=audit(a.header,a.stock);print(json.dumps(r,indent=2))
 if a.output:a.output.write_text(json.dumps(r,indent=2)+'\n')
