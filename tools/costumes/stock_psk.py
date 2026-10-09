"""Read measured PSK rest geometry and skin weights without Blender or SDKs."""
import struct
from pathlib import Path
import numpy as np

def read_psk(path):
 data=Path(path).read_bytes();offset=0;chunks={}
 while offset<len(data):
  tag,_,size,count=struct.unpack_from('<20s3i',data,offset);offset+=32
  if size<0 or count<0 or offset+size*count>len(data):raise ValueError('Invalid PSK chunk')
  chunks[tag.rstrip(b'\0').decode()]=(data[offset:offset+size*count],size,count);offset+=size*count
 def rows(name,fmt):
  blob,size,count=chunks[name]
  if size!=struct.calcsize(fmt):raise ValueError('Unexpected PSK layout: '+name)
  return [struct.unpack_from(fmt,blob,i*size) for i in range(count)]
 p=np.asarray(rows('PNTS0000','<3f'));w=rows('VTXW0000','<I2f2BH')
 if len(p)<=65536:w=[(q[0]&65535,*q[1:]) for q in w]
 faces=rows('FACE0000','<3H2BI');materials=[q[0].split(b'\0')[0].decode() for q in rows('MATT0000','<64s6i')]
 bones=[q[0].split(b'\0')[0].decode() for q in rows('REFSKELT','<64s3i11f')]
 weights=[{} for _ in p]
 for weight,point,bone in rows('RAWWEIGHTS','<fii'):
  if not 0<=point<len(p) or not 0<=bone<len(bones):raise ValueError('Invalid PSK skin donor')
  weights[point][bone]=weights[point].get(bone,0)+weight
 return p,w,faces,materials,bones,weights

def torso(path):
 p,w,faces,mats,bones,weights=read_psk(path)
 # These are measured stock names, never an inferred engine skeleton.
 # The authoring adapter's observed canonical torso bone IDs are explicit.
 ids={3,4,5,6,7,34}
 membership=np.array([sum(v for k,v in q.items() if k in ids) for q in weights])
 tri=np.array([[w[i][0] for i in f[:3]] for f in faces if mats[f[3]]=='MAT_Wolverine_Body_Alkali'])
 keep=np.all((p[tri][:,:,2]>89)&(p[tri][:,:,2]<146)&(membership[tri]>.7),axis=1)
 return p,tri[keep]

def buckle(path):
 p,w,faces,mats,bones,weights=read_psk(path)
 faces=[f[:3] for f in faces if mats[f[3]]=='MAT_Wolverine_Buckle']
 ids=sorted({i for f in faces for i in f});lookup={i:j for j,i in enumerate(ids)}
 q=np.asarray([p[w[i][0]] for i in ids]);tri=np.asarray([[lookup[i] for i in f] for f in faces])
 ns=np.zeros_like(p)
 for f in faces:
  points=[w[i][0] for i in f];a,b,c=p[points];n=-np.cross(b-a,c-a)
  for i in points:ns[i]+=n
 ns/=np.maximum(np.linalg.norm(ns,axis=1)[:,None],1e-12)
 vertices=[]
 for i in ids:
  point=w[i][0];skin=sorted(weights[point].items(),key=lambda v:-v[1])
  if len(skin)!=1:raise ValueError('Expected measured rigid stock buckle')
  uv=[struct.unpack('<H',struct.pack('<e',v))[0] for v in w[i][1:3]]
  vertices.append(dict(id=i,p=p[point].tolist(),n=(ns[point]*127.5+127.5).tolist(),uv=uv,bone=[skin[0][0]]*4,weight=[255,0,0,0],sourceResource=1))
 return vertices,tri
