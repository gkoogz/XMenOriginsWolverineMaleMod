from pathlib import Path
import numpy as np,struct,hashlib,json,sys
root=Path(__file__).resolve().parents[2]
src=root/'src'/'runtime'
source=Path(sys.argv[1]) if len(sys.argv)>1 else root/'research'/'height-bakes.npz'
arrays=[]
with np.load(source) as data:
    for name in ['oblique','grazing']:
        a=data[name].astype(np.float64)
        peak=a.max(axis=(1,2),keepdims=True)
        arrays.append(np.uint16(np.rint(a/peak*65535)))
blob=struct.pack('<4I',0x314B5053,192,len(arrays[0]),len(arrays[1]))+b''.join(a.astype('<u2').tobytes() for a in arrays)
(src/'splat_bakes.bin').write_bytes(blob)
(src/'splat_bakes.rc').write_text('201 RCDATA "splat_bakes.bin"\n')
report={'sourceSHA256':hashlib.sha256(source.read_bytes()).hexdigest(),'packedSHA256':hashlib.sha256(blob).hexdigest(),'bytes':len(blob),'resolution':192,'frames':[len(a) for a in arrays],'encoding':'Per-frame normalized height, little-endian uint16. Original Mantaflow meshes; no third-party assets.'}
(src/'splat_bakes.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report))
