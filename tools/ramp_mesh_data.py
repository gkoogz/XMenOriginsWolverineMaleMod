import sys,re,json
from pathlib import Path
import numpy as np
ROOT=Path(__file__).resolve().parent.parent
SRC=ROOT/'src/runtime'
def arr(file,name,dtype=float):
    text=(SRC/file).read_text(); body=re.search(r'\b'+name+r'\[[^\]]+\]\s*=\s*\{(.*?)\};',text,re.S)[1]
    return np.array([dtype(x.strip().rstrip('fu')) for x in body.split(',') if x.strip()])
base=arr('morph_targets.h','morph_base').reshape(-1,3)
tri=arr('graft_normals.h','graftTriangleIndices',int).reshape(-1,3)
pi=arr('pelvis_control.h','pelvisControlIndices',int)
pb=arr('pelvis_control.h','pelvisControlBasePositions').reshape(-1,3)
members=arr('collar_fairing.h','collarFairMembers',int)
offsets=arr('collar_fairing.h','collarFairMemberOffsets',int)
neighbors=arr('collar_fairing.h','collarFairNeighbors',int)
noff=arr('collar_fairing.h','collarFairNeighborOffsets',int)
distances=arr('collar_fairing.h','collarFairDistances')
weights=arr('collar_fairing.h','collarFairWeights')
shaft=np.maximum(arr('physics_weights.h','phys_shaft_weight'),arr('physics_weights.h','phys_attachment_weight'))
ball=arr('physics_weights.h','phys_scrotum_weight')
flex=arr('physics_weights.h','phys_flex_coordinate')
globalbase={int(i):p for i,p in zip(pi,pb)}
globalbase.update({47050+i:p for i,p in enumerate(base)})
ctri=arr('collar_fairing.h','collarNormalTriangleIndices',int).reshape(-1,3)
cp=arr('collar_fairing.h','collarNormalTriangleBasePositions').reshape(-1,3)
for i,p in zip(ctri.ravel(),cp):
    if int(i) not in globalbase: globalbase[int(i)]=p
groupbase=np.array([np.mean([globalbase[int(i)] for i in members[a:b]],axis=0) for a,b in zip(offsets[:-1],offsets[1:])])
def groups(dump):
    live=globalbase.copy(); live.update({47050+i:p for i,p in enumerate(dump[:2388])}); live.update({int(i):p for i,p in zip(pi,dump[2388:])})
    return np.array([np.mean([live[int(i)] for i in members[a:b]],axis=0) for a,b in zip(offsets[:-1],offsets[1:])])
