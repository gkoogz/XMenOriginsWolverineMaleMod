from ramp_mesh_data import *
from scipy import sparse
from scipy.sparse.linalg import splu
normalgroup=arr('graft_normals.h','graftNormalGroup',int)
ng=normalgroup.max()+1
ids=[np.flatnonzero(normalgroup==g) for g in range(ng)]
gb=np.array([ball[i].max() for i in ids])
gs=np.array([shaft[i].max() for i in ids])
gp=np.array([base[i].mean(0) for i in ids])
faces=normalgroup[tri]
edges=np.unique(np.sort(np.vstack([faces[:,[0,1]],faces[:,[1,2]],faces[:,[2,0]]]),axis=1),axis=0)
edges=edges[edges[:,0]!=edges[:,1]]
neighbors=[[] for _ in ids]
for a,b in edges: neighbors[a].append(b);neighbors[b].append(a)
def smooth(x):
 x=np.clip(x,0,1);return x*x*x*(x*(6*x-15)+10)
support=smooth(gb/.035)*(1-smooth((gb-.65)/.33))
gf=np.array([flex[i].mean() for i in ids])
radial=np.maximum(np.linalg.norm(gp[:,1:]-[0,84.3],axis=1),1e-5)
ventral=(84.3-gp[:,2])/radial
under=smooth((ventral-.3)/.4)*smooth(gf/.05)*(1-smooth((gf-.16)/.22))*smooth(gs/.5)
support=np.maximum(support,under)
# Include neighboring shaft rows for a smooth shared attachment tangent.
for _ in range(2):
 support=np.maximum(support,np.array([max(support[ns],default=0)*.55 for ns in neighbors]))
for g in range(len(groupbase)):
 mm=members[offsets[g]:offsets[g+1]]
 if np.any(mm<47050):
  graft=mm[(mm>=47050)&(mm<49438)]-47050
  support[normalgroup[graft]]=0
active=np.flatnonzero(support>1e-5)
rows=[];cols=[];vals=[]
for g,ns in enumerate(neighbors):
 if not ns: continue
 w=np.ones(len(ns))/len(ns)
 rows.extend([g]*(len(ns)+1));cols.extend(ns+[g]);vals.extend(list(-w)+[1])
L=sparse.coo_matrix((vals,(rows,cols)),shape=(ng,ng)).tocsr()
fixed=np.setdiff1d(np.arange(ng),active)
Q=L.T@L
penalty=.0001+1.0*(1-support[active])**4
A=Q[active][:,active]+sparse.diags(penalty)
rhs=-Q[active][:,fixed]
solver=splu(A.tocsc())
from collections import defaultdict
edgefaces=defaultdict(list)
for fi,face in enumerate(faces):
 for aa,bb in zip(face,np.roll(face,-1)):edgefaces[tuple(sorted((aa,bb)))].append(fi)
pairs=np.array([f for f in edgefaces.values() if len(f)==2 and np.any(support[faces[f]]>0)])
K=solver.solve(sparse.hstack([rhs,sparse.diags(penalty)]).toarray());col=np.r_[fixed,active]
rows=[0];columns=[];coefficients=[]
for row in K:
 keep=np.flatnonzero(abs(row)>1e-5);value=row[keep].copy();value[np.argmax(abs(value))]+=1-value.sum()
 columns.extend(col[keep]);coefficients.extend(value);rows.append(len(columns))
def emit(name,values,kind):
 fmt=(lambda x:f'{float(x):.9e}f') if kind=='float' else (lambda x:str(int(x)))
 values=list(values);lines=[','.join(fmt(v) for v in values[i:i+12]) for i in range(0,len(values),12)]
 return f'static const {kind} {name}[{len(values)}]={{\n'+',\n'.join(lines)+'\n};\n'
text='// Generated constrained scrotal junction operator. See Generate-ScrotalJunction.py.\n#pragma once\n'
face_ids=np.unique(np.r_[np.flatnonzero(np.any(support[faces]>0,axis=1)),pairs.ravel()])
text+=f'static const unsigned neckFaceCount={len(face_ids)}u;\n'
text+=f'static const unsigned neckActiveCount={len(active)}u;\nstatic const unsigned neckPairCount={len(pairs)}u;\n'
no=[0];nn=[]
for ns in neighbors:nn.extend(ns);no.append(len(nn))
for name,data,kind in [('neckActive',active,'uint16_t'),('neckSupport',support,'float'),('neckRows',rows,'uint32_t'),('neckColumns',columns,'uint16_t'),('neckCoefficients',coefficients,'float'),('neckNeighborOffsets',no,'uint16_t'),('neckNeighbors',nn,'uint16_t'),('neckFacePairs',pairs.ravel(),'uint16_t'),('neckFaces',face_ids,'uint16_t')]:text+=emit(name,data,kind)
(ROOT/'src/runtime/scrotal_junction.h').write_text(text)
print('active',len(active),'coefficients',len(coefficients),'edge pairs',len(pairs))
