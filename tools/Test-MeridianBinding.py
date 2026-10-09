"""Verify versioned measured bindings without private exports or a graphics SDK."""
import hashlib,json,re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
receipt=json.loads((root/'provenance/meridian-binding.json').read_text())
raw=(root/receipt['header']).read_bytes().replace(b'\r\n',b'\n')
assert hashlib.sha256(raw).hexdigest()==receipt['headerSHA256'],'Binding header changed without provenance'
assert receipt['geometryBindingRevision']==6 and receipt['columns']==64 and receipt['rows']==40
assert receipt['renderSeamAliases']==40 and receipt['privateInputsCommitted'] is False
header=raw.decode()
for name,value in [('contractRevision',6),('columns',64),('rows',40),('count',4279),('sampleCount',9002),('clothCount',2561),('faceCount',8368),('clothFaceCount',5056)]:
 assert re.search(r'\b'+name+'='+str(value)+r'\b',header),(name,value)
for array,count,maximum in [('faces',8368,4279),('clothFaces',5056,2561)]:
 block=header.split(array+'[]={',1)[1].split('};',1)[0]
 faces=[list(map(int,re.findall(r'\d+',row))) for row in re.findall(r'\{([^{}]+)\}',block)]
 assert len(faces)==count and all(len(f)==3 and len(set(f))==3 and min(f)>=0 and max(f)<maximum for f in faces),'Invalid triangle topology'
assert receipt['clothVertices']==receipt['columns']*receipt['rows']+1
assert receipt['referenceReconstructionError']<1e-5
print('PASS measured binding revision6, triangle topology, UV aliases, source hashes and private input omissions')
