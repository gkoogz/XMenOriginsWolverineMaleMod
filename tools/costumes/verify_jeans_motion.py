"""Audit authored motion bounds without graphics/game SDKs or installation."""
import argparse,hashlib,json,re
from pathlib import Path
import numpy as np

def values(text,name,width):
 m=re.search(r'\b'+name+r'\[.*?=\{(.*?)\};',text,re.S)
 if not m:raise ValueError('Missing '+name)
 return np.array([float(v) for v in re.findall(r'[-+]?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?',m[1])]).reshape(-1,width)

def audit(header):
 text=header.read_text();v=values(text,'vertices1',11);p=v[:,:3]
 t=values(text,'triangles1',3).astype(int);h=values(text,'motionHinges',3)
 fly=values(text,'motionFly',1)[:,0];belt=values(text,'motionBelt',1)[:,0];sides=values(text,'motionSides',1)[:,0]
 resource=values(text,'sourceResource1',1)[:,0];buckle=resource==1
 rest=np.cross(p[t[:,1]]-p[t[:,0]],p[t[:,2]]-p[t[:,0]])
 pinned=fly==0;worst=0;mincos=1;rigid=0
 for fa in (-3,0,3):
  for fb in (-3,0,3):
   for ba in (-2,0,2):
    for bb in (-2,0,2):
     angle=np.deg2rad(np.where(sides>0,fb,fa))*sides
     q=p-h;r=q.copy();c=np.cos(angle);sn=np.sin(angle)
     r[:,0]=q[:,0]*c-q[:,1]*sn;r[:,1]=q[:,0]*sn+q[:,1]*c
     moving=p+(r-q)*fly[:,None]
     angle=-np.deg2rad(np.where(sides>0,bb,ba))*sides
     q=moving-h;r=q.copy();c=np.cos(angle);sn=np.sin(angle)
     r[:,1]=q[:,1]*c-q[:,2]*sn;r[:,2]=q[:,1]*sn+q[:,2]*c
     moving+=(r-q)*belt[:,None]
     assert np.array_equal(moving[pinned],p[pinned]),'Pinned seam moved'
     now=np.cross(moving[t[:,1]]-moving[t[:,0]],moving[t[:,2]]-moving[t[:,0]])
     den=np.linalg.norm(rest,axis=1)*np.linalg.norm(now,axis=1);ok=den>1e-12
     cosine=np.sum(rest[ok]*now[ok],axis=1)/den[ok];mincos=min(mincos,float(cosine.min()))
     assert cosine.min()>0,'Triangle overturned under bounded motion'
     a=np.linalg.norm(moving[buckle,None]-moving[None,buckle],axis=2)
     b=np.linalg.norm(p[buckle,None]-p[None,buckle],axis=2)
     rigid=max(rigid,float(abs(a-b).max()));assert rigid<1e-7,'Buckle sheared'
     worst=max(worst,float(np.linalg.norm(moving-p,axis=1).max()))
 return dict(schema='wolverine.jeans-motion-audit/1',headerSHA256=hashlib.sha256(header.read_bytes()).hexdigest(),combinations=81,pinnedVertices=int(pinned.sum()),pinnedDisplacement=0,minimumFaceNormalCosine=mincos,maximumRestDisplacement=worst,maximumBuckleDistanceError=rigid,sourceUnits=True,bodyCollisionVerified=False,nativeVerified=False,attachmentGatePassed=False)

if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('--header',type=Path,default=Path(__file__).resolve().parents[2]/'src/runtime/jeans_data.h');ap.add_argument('--output',type=Path);a=ap.parse_args()
 result=audit(a.header);print(json.dumps(result,indent=2))
 if a.output:a.output.write_text(json.dumps(result,indent=2)+'\n')
