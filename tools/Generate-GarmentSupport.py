"""Generate source support groups through actual final render-to-cage lineage."""
import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser();p.add_argument('--base',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args();geometry=a.base/'assets/wolverine-reference/geometry.npz';z=np.load(geometry)
ball=np.clip(z['physics_weights__phys_scrotum_weight'],0,1);shaft=np.clip(z['physics_weights__phys_shaft_weight'],0,1);source=z['r14_asset__r14Reference'].reshape(-1,3);cage=np.stack([shaft,ball*(source[:,1]<0),ball*(source[:,1]>=0)],axis=1)
ids=z['r14_asset__r14Sources'].astype(int);weights=z['r14_asset__r14Weight'];offsets=z['r14_asset__r14Offsets'].astype(int);r14=np.array([np.sum(cage[ids[offsets[i]:offsets[i+1]]]*weights[offsets[i]:offsets[i+1],None],axis=0) for i in range(len(offsets)-1)])
rs=np.concatenate([r14,np.sum(r14[z['rounded_render_data__rsFineSource'].reshape(-1,3)]*z['rounded_render_data__rsFineBary'].reshape(-1,3,1),axis=1)])
nr=np.sum(rs[z['neck_render_data__nrMaterialSources'].reshape(-1,3)]*z['neck_render_data__nrMaterialWeights'].reshape(-1,3,1),axis=1);groups=np.argmax(nr,axis=1);groups[np.max(nr,axis=1)<.05]=3
lines=['#pragma once','// Generated from exact nr->rs->r14->original motion-cage lineage.','// 0 shaft; 1 negative-Y lobe; 2 positive-Y lobe; 3 unrelated body/support.','static const unsigned char jockstrapSourceSupportGroup[]={']
lines +=[','.join(map(str,groups[i:i+128]))+',' for i in range(0,len(groups),128)];lines+=['};'];a.out.write_text('\n'.join(lines)+'\n');print(json.dumps({'sourceSha256':hashlib.sha256(geometry.read_bytes()).hexdigest(),'groupCounts':np.bincount(groups,minlength=4).tolist(),'finalVertices':len(groups)}))
