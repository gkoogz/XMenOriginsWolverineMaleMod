"""Author measured stock jeans and pinned Base hinged fly; no game writes."""
import argparse,hashlib,json,re,struct,sys
from pathlib import Path
import numpy as np
from prepare_tank_top import array

def smooth_normals(p,t):
 ns=np.zeros_like(p)
 for f in t:
  n=-np.cross(p[f[1]]-p[f[0]],p[f[2]]-p[f[0]])
  for j in f:ns[j]+=n
 groups={}
 for i,q in enumerate(p):groups.setdefault(tuple(np.round(q,6)),[]).append(i)
 for g in groups.values():ns[g]=ns[g].sum(0)
 ns/=np.maximum(np.linalg.norm(ns,axis=1)[:,None],1e-12)
 return ns

def add_volume(r,patch,thickness,offset,role,layer,include_outer=True):
 from malemod_base.garment_shell import thin_shell
 shell=thin_shell(patch['positions'],patch['triangles'],smooth_normals(patch['positions'],patch['triangles']),thickness,offset=offset)
 source=shell['source'];begin=len(r['positions'])
 faces=shell['triangles'] if include_outer else shell['triangles'][len(patch['triangles']):]
 for name,values in dict(positions=shell['positions'],donors=patch['donors'][source],weights=patch['weights'][source],panels=patch['panels'][source],roles=np.full(len(source),role),layers=shell['normal_layers']+layer).items():
  r[name]=np.concatenate((r[name],values))
 r['triangles']=np.concatenate((r['triangles'],faces+begin))
 return shell['boundary_edges']

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--stock',type=Path,required=True);ap.add_argument('--stock-psk',type=Path,required=True);ap.add_argument('--base',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 sys.path.insert(0,str(a.base));from malemod_base.fly_panels import folded_fly,fold_rigid_attachment,fly_motion_bindings
 repo=Path(__file__).resolve().parents[2];s=json.loads(a.stock.read_text(encoding='utf-8-sig'))
 assert s['schema']=='wolverine.stock-jeans/1' and s['first']==4995 and s['triangles']==1478
 original=s['vertices'];p=np.array([v['p'] for v in original]);ids={v['id']:i for i,v in enumerate(original)};tri=np.array([ids[i] for i in s['indices']]).reshape(-1,3)
 # Measured front is +X; fly envelope is in this stock mesh's source units.
 # Extra rest ease keeps the waistband outside the enlarged neutral pelvis.
 normals=np.array([v['n'] for v in original])/127.5-1;normals/=np.maximum(np.linalg.norm(normals,axis=1)[:,None],1e-12)
 fitted=p+normals*.55
 opened=folded_fly(fitted,tri,front_axis=0,side_axis=1,height_axis=2,front_plane=0,lower=70,upper=97,half_width=9,angle=np.deg2rad(155))
 variants=[dict(positions=fitted,triangles=tri,donors=np.repeat(np.arange(len(p))[:,None],3,axis=1),weights=np.tile([1.,0,0],(len(p),1)),panels=np.zeros(len(p))),opened]
 from malemod_base.garment_shell import clip_scalar_band
 uv=np.array([[struct.unpack('<e',struct.pack('<H',x))[0] for x in v['uv']] for v in original])
 # Observed leather strip in the original 1024px diffuse chart. These contours
 # retain its lacing, loops and worn edges; no replacement pixel assets.
 u=np.array([32,167,180,279,296,365,383,493,511,635])/1024
 top=np.array([29,33,43,62,68,68,60,50,30,23])/1024
 bottom=np.array([58,68,82,96,98,100,87,77,62,54])/1024
 low=uv[:,1]-np.interp(uv[:,0],u,top);high=np.interp(uv[:,0],u,bottom)-uv[:,1]
 high[(uv[:,0]<u[0])|(uv[:,0]>u[-1])|(p[:,2]<88)]=-1
 strip=clip_scalar_band(fitted,tri,low,high)
 beltEdges=[]
 for index,r in enumerate(variants):
  r['roles']=np.full(len(r['positions']),4);r['layers']=np.zeros(len(r['positions']),int)
  if index:
   patchFaces=r['triangles'][np.all(r['panels'][r['triangles']]!=0,axis=1)]
   used=np.unique(patchFaces);lookup=np.full(len(r['positions']),-1);lookup[used]=np.arange(len(used))
   patch={name:r[name][used].copy() for name in ('positions','donors','weights','panels')};patch['triangles']=lookup[patchFaces]
   add_volume(r,patch,.12,0.,4,10,include_outer=False)
  band=strip.copy();band['panels']=np.zeros(len(band['positions']),int)
  if index:
   folded=folded_fly(strip['positions'],strip['triangles'],front_axis=0,side_axis=1,height_axis=2,front_plane=0,lower=70,upper=97,half_width=9,angle=np.deg2rad(155))
   donors=folded['donors'];weights=folded['weights']
   assert np.all(strip['donors'][donors]==strip['donors'][donors[:,0]][:,None,:])
   band=dict(positions=folded['positions'],triangles=folded['triangles'],donors=strip['donors'][donors[:,0]],weights=np.einsum('ij,ijk->ik',weights,strip['weights'][donors]),panels=folded['panels'])
  beltEdges.append(add_volume(r,band,.22,.24,5,100000))
 angles=np.sort(np.unique(np.round(np.arctan2(strip['positions'][:,1],strip['positions'][:,0]),4)))
 gap=float(np.rad2deg(np.diff(np.r_[angles,angles[0]+2*np.pi])).max())
 if gap>35:raise ValueError('Original belt strip does not cover the complete waist')
 from stock_psk import buckle
 accessory,accessoryFaces=buckle(a.stock_psk);base=len(original)
 bp=np.array([v['p'] for v in accessory]);bp[:,0]+=.55
 for index,r in enumerate(variants):
  q=bp.copy()
  if index:
   # The separate rigid buckle belongs to the right fly flap. Retain it as
   # one piece, applying the same measured envelope transform to all vertices.
   q=fold_rigid_attachment(q,front_axis=0,side_axis=1,height_axis=2,lower=70,upper=97,half_width=9,angle=np.deg2rad(155))
  if index:
   rest=np.einsum('ij,ijk->ik',r['weights'],fitted[r['donors']])
   r['motion']=fly_motion_bindings(rest,r['panels'],lower=70,upper=97,half_width=9,belt_lower=89,belt_upper=92.5)
   pivot=bp.mean(0);pivot[1]=9*(pivot[2]-70)/27
   for name,values in dict(hinges=np.tile(pivot,(len(bp),1)),fly=np.ones(len(bp)),belt=np.ones(len(bp)),sides=np.ones(len(bp),int)).items():
    r['motion'][name]=np.concatenate((r['motion'][name],values))
  count=len(r['positions']);r['positions']=np.concatenate((r['positions'],q));r['triangles']=np.concatenate((r['triangles'],accessoryFaces+count));r['roles']=np.r_[r['roles'],np.full(len(bp),6)];r['layers']=np.r_[r['layers'],np.full(len(bp),200000)]
  r['donors']=np.concatenate((r['donors'],np.repeat((np.arange(len(bp))+base)[:,None],3,axis=1)))
  r['weights']=np.concatenate((r['weights'],np.tile([1.,0,0],(len(bp),1))));r['panels']=np.r_[r['panels'],np.ones(len(bp))*index]
 original=original+accessory;p=np.concatenate((p,np.array([v['p'] for v in accessory])))
 text=(repo/'src/runtime/menu_retarget_body_data.h').read_text();game=(repo/'src/runtime/fluid_gameplay_bones.h').read_text();pal=(repo/'src/runtime/menu_necklace_palette.h').read_text()
 packed=[array(text,f'menuRetargetBodyPacked{i}',np.uint8).reshape(-1,32) for i in range(2)];body=[np.array([struct.unpack('<3f',bytes(v[:12])) for v in q]) for q in packed]
 mappings=[];allBones=set()
 for section in range(2):
  titlePalette=array(pal,f'ncMenuBodyPalette{section}',np.int64);gameBones=array(game,f'fluidGameplayBones{section}',np.uint8).reshape(-1,4);mapping={}
  for v,g in zip(packed[section],gameBones):
   for k in range(4):
    if v[24+k]:
     slot=int(g[k]);bone=int(titlePalette[v[20+k]])
     if slot in mapping:assert mapping[slot]==bone
     mapping[slot]=bone
  mappings.append(mapping);allBones.update(mapping.values())
 for bone in s['palette']:assert bone in allBones
 rows=['#pragma once','// Measured licensed Alkali section. Recipe keeps source UV and face donors.','namespace JeansRecipe {','static constexpr unsigned revision=6;']
 for section,mapping in enumerate(mappings):rows+=['static const unsigned char gameplayPalette'+str(section)+'[]={'+','.join(str(mapping.get(i,0)) for i in range(max(mapping)+1))+'};']
 motion=variants[1]['motion']
 for name in ('hinges','fly','belt','sides'):
  values=motion[name]
  if values.ndim==2:
   rows+=['static const float motionHinges[][3]={']+['{'+','.join(f'{v:.9f}f' for v in q)+'},' for q in values]+['};']
  else:
   typ='signed char' if name=='sides' else 'float'
   vals=','.join(str(int(v)) if name=='sides' else f'{v:.9f}f' for v in values)
   rows+=['static const '+typ+' motion'+name.title()+'[]={'+vals+'};']
 # Coincident cut/source aliases share a shading normal while UVs stay separate.
 groups={};labels=[]
 for q,d,layer in zip(variants[1]['positions'],variants[1]['donors'],variants[1]['layers']):
  key=(*np.round(q,5),int(original[d[0]].get('sourceResource',0)),int(layer))
  labels.append(groups.setdefault(key,len(groups)))
 rows+=['static constexpr unsigned normalGroups='+str(len(groups))+';', 'static const unsigned short normalAliases[]={'+','.join(map(str,labels))+'};']
 rows+=['static constexpr unsigned motionBone='+str(accessory[0]['bone'][0])+';']
 assert all(v['bone'][0]==accessory[0]['bone'][0] for v in accessory)
 maxDiscard=0
 for index,r in enumerate(variants):
  q=r['positions'];faces=r['triangles'];ns=np.zeros_like(q)
  for f in faces:
   n=np.cross(q[f[1]]-q[f[0]],q[f[2]]-q[f[0]])
   for j in f:ns[j]+=n
  # Original faces have clockwise winding in the observed engine convention.
  ns*=-1;ns/=np.maximum(np.linalg.norm(ns,axis=1)[:,None],1e-12)
  rows+=['static const NcVertex vertices'+str(index)+'[]={'];uv=[]
  for point,d,w in zip(q,r['donors'],r['weights']):
   skin={}
   for donor,amount in zip(d,w):
    for bone,weight in zip(original[donor]['bone'],original[donor]['weight']):
     if weight:skin[bone]=skin.get(bone,0)+amount*weight
   selected=sorted(skin.items(),key=lambda x:(-x[1],x[0]))[:4];maxDiscard=max(maxDiscard,sum(skin.values())-sum(v for b,v in selected))
   ww=np.array([v for b,v in selected])*255/sum(v for b,v in selected);integer=np.floor(ww).astype(int)
   for k in np.argsort(-(ww-integer))[:255-sum(integer)]:integer[k]+=1
   bones=[b for b,v in selected];weights=integer.tolist()
   while len(bones)<4:bones.append(bones[-1]);weights.append(0)
   rows.append('{{'+','.join(f'{v:.9f}f' for v in point)+'},{'+','.join(map(str,bones))+'},{'+','.join(map(str,weights))+'}},')
   uv.append(sum(amount*np.array([struct.unpack('<e',struct.pack('<H',v))[0] for v in original[donor]['uv']]) for donor,amount in zip(d,w)))
  rows+=['};','static const unsigned short triangles'+str(index)+'[][3]={']+['{'+','.join(map(str,f))+'},' for f in faces]+['};','static const float normals'+str(index)+'[][3]={']+['{'+','.join(f'{v:.9f}f' for v in n)+'},' for n in ns]+['};','static const float uv'+str(index)+'[][2]={']+['{'+','.join(f'{v:.9f}f' for v in u)+'},' for u in uv]+['};','static const unsigned sourceDonors'+str(index)+'[][3]={']+['{'+','.join(str(original[j]['id']) for j in d)+'},' for d in r['donors']]+['};','static const float sourceWeights'+str(index)+'[][3]={']+['{'+','.join(f'{v:.9f}f' for v in w)+'},' for w in r['weights']]+['};']
  resources=[original[d[0]].get('sourceResource',0) for d in r['donors']]
  rows+=['static const unsigned char materialRoles'+str(index)+'[]={'+','.join(map(str,r['roles']))+'};']
  rows+=['// Resource 0: observed UPK jeans vertex IDs; 1: original PSK buckle wedge IDs.','static const unsigned char sourceResource'+str(index)+'[]={'+','.join(map(str,resources))+'};']
 for index,r in enumerate(variants):
  rest=np.einsum('ij,ijk->ik',r['weights'],p[r['donors']])
  rows+=['static const float restPositions'+str(index)+'[][3]={']+['{'+','.join(f'{v:.9f}f' for v in q)+'},' for q in rest]+['};']
 for section in range(2):
  bt=array(text,f'menuRetargetBodyIndices{section}',np.int64).reshape(-1,3);centers=body[section][bt].mean(axis=1)
  for opened in (0,1):
   keep=centers[:,2]>96
   if opened:keep|=(centers[:,0]>0)&(centers[:,2]>70)&(abs(centers[:,1])<9*(centers[:,2]-70)/27+2)
   selected=bt[keep];rows+=['static const unsigned short body'+str(section)+str(opened)+'[][3]={']+['{'+','.join(map(str,f))+'},' for f in selected]+['};']
 rows+=['}'];a.output.write_text('\n'.join(rows)+'\n',encoding='utf-8')
 receipt=dict(schema='wolverine.jeans-binding/1',revision=6,flapThickness=.12,beltThickness=.22,beltOutset=.24,beltBoundaryEdges=beltEdges,maximumBeltAngularSamplingGap=gap,beltWrapSourceCoverage=True,rigidBuckleShapePreserved=True,stockSHA256=s['packageHash'].lower(),stockPSKSHA256=hashlib.sha256(a.stock_psk.read_bytes()).hexdigest(),stockSectionFirst=4995,stockBuckleVertices=len(accessory),stockBuckleTriangles=len(accessoryFaces),counts=[dict(vertices=len(r['positions']),triangles=len(r['triangles'])) for r in variants],sourceUnits=True,foldDegrees=155,flyLower=70,flyUpper=97,flyHalfWidth=9,restEase=.55,maximumDiscardedSkinWeight=maxDiscard/255,headerSHA256=hashlib.sha256(a.output.read_bytes()).hexdigest(),motionBindingRevision=1,flyTravelDegrees=3,beltTravelDegrees=2,beltBandBlend=[89,92.5],originalBeltUVRetained=True,nativeVerified=False)
 a.output.with_suffix('.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt))
if __name__=='__main__':main()
