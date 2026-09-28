"""Bind the 16 existing electrode islands to the authored gameplay body."""
import json, numpy as np
from pathlib import Path
from scipy.spatial import cKDTree
root=Path(__file__).parent
menu=json.loads((root/'current-tank-mesh.json').read_text(encoding='utf-8-sig'))
game=json.loads((root/'current-gameplay-mesh.json').read_text(encoding='utf-8-sig'))
pos=np.array([v['p'] for v in menu['vertices']]); gp=np.array([v['p'] for v in game['vertices']])
s=menu['sections'][5]; tri=np.array(menu['indices'][s['first']:s['first']+s['count']*3]).reshape(-1,3)
parent={int(i):int(i) for i in np.unique(tri)}
def find(i):
 while parent[i]!=i: parent[i]=parent[parent[i]]; i=parent[i]
 return i
def union(a,b): parent[find(a)]=find(b)
same={}
for i in parent: union(i,same.setdefault(tuple(np.round(pos[i],4)),i))
for a,b,c in tri: union(int(a),int(b)); union(int(a),int(c))
groups={}
for i in parent: groups.setdefault(find(i),[]).append(i)
groups=sorted(groups.values(),key=min); assert len(groups)==16
bodytri=[]
for s in (game['sections'][4],game['sections'][6]): bodytri.extend(game['indices'][s['first']:s['first']+s['count']*3])
bodytri=np.array(bodytri).reshape(-1,3); points=gp[bodytri]; tree=cKDTree(points.mean(1))
def closest(p,a,b,c):
 ab=b-a;ac=c-a;ap=p-a;d1=ab@ap;d2=ac@ap
 if d1<=0 and d2<=0:return np.array([1.,0,0])
 bp=p-b;d3=ab@bp;d4=ac@bp
 if d3>=0 and d4<=d3:return np.array([0.,1,0])
 vc=d1*d4-d3*d2
 if vc<=0 and d1>=0 and d3<=0:
  v=d1/(d1-d3);return np.array([1-v,v,0])
 cp=p-c;d5=ab@cp;d6=ac@cp
 if d6>=0 and d5<=d6:return np.array([0.,0,1])
 vb=d5*d2-d1*d6
 if vb<=0 and d2>=0 and d6<=0:
  w=d2/(d2-d6);return np.array([1-w,0,w])
 va=d3*d6-d5*d4
 if va<=0 and d4-d3>=0 and d5-d6>=0:
  w=(d4-d3)/((d4-d3)+(d5-d6));return np.array([0.,1-w,w])
 den=1/(va+vb+vc);v=vb*den;w=vc*den;return np.array([1-v-w,v,w])
bindings=[];membership=np.full(2162,255)
for k,ids in enumerate(groups):
 center=pos[ids].mean(0); _,candidates=tree.query(center,k=256);best=None
 for t in candidates:
  a,b,c=points[t]
  if np.linalg.norm(np.cross(b-a,c-a))<1e-5:continue
  bary=closest(center,a,b,c);q=bary@points[t];dist=np.linalg.norm(center-q)
  if best is None or dist<best[0]:best=(dist,t,bary)
 dist,t,bary=best;bindings.append((bodytri[t],bary));membership[np.array(ids)-9920]=k
 print(f'electrode {k}: {len(ids)} vertices, anchor distance {dist:.4f}')
assert (membership<16).all()
def fs(v):return ','.join(f'{float(x):.9f}f' for x in v)
out=['#pragma once','// Generated from installed mesh exports; preserves native placement at rest.',
     'struct ElectrodeBinding { unsigned vertex[3]; float bary[3]; };',
     'static const ElectrodeBinding electrodeBindings[16]={']
out += [' {{'+','.join(map(str,ids))+'},{'+fs(bary)+'}},' for ids,bary in bindings]
out += ['};','static const unsigned char electrodeMembership[2162]={']
out += [','.join(map(str,membership[i:i+64]))+',' for i in range(0,2162,64)]
out += ['};']
(root/'../../src/runtime/electrode_bindings_data.h').write_text('\n'.join(out)+'\n')
