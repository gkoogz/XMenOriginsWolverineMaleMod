"""Verify versioned measured bindings without private exports or a graphics SDK."""
import hashlib,json,re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
receipt=json.loads((root/'provenance/meridian-binding.json').read_text())
raw=(root/receipt['header']).read_bytes().replace(b'\r\n',b'\n')
assert hashlib.sha256(raw).hexdigest()==receipt['headerSHA256'],'Binding header changed without provenance'
assert receipt['geometryBindingRevision']==5 and receipt['columns']==32 and receipt['rows']==24
assert receipt['renderSeamAliases']==24 and receipt['privateInputsCommitted'] is False
header=raw.decode()
for name,value in [('contractRevision',5),('columns',32),('rows',24),('count',2471),('clothCount',769),('faceCount',4816)]:
 assert re.search(r'\b'+name+'='+str(value)+r'\b',header),(name,value)
print('PASS measured binding revision5, topology, UV aliases, source hashes and private input omissions')
