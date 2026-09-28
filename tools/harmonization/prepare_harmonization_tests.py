import json,struct
from pathlib import Path
r=Path(__file__).parent
m=json.loads((r/'current-tank-mesh.json').read_text(encoding='utf-8-sig'))
g=json.loads((r/'current-gameplay-mesh.json').read_text(encoding='utf-8-sig'))
(r/'audit-tank-base.bin').write_bytes(b''.join(struct.pack('<3f4B4B4B4B2H',*v['p'],*v['t'],*v['n'],*v['bi'],*v['bw'],*v['uv']) for v in m['vertices']))
byname={b['name']:i for i,b in enumerate(g['bones'])}
lines=[]
for s in (1,4):
 bones=m['chunks'][m['sections'][s]['chunk']]['bones']
 ids=[byname.get(m['bones'][b]['name'],0) for b in bones]
 lines.append('static const unsigned tankGlobal'+str(s)+'[]={'+','.join(map(str,ids))+'};')
(r/'audit_palette_data.h').write_text('\n'.join(lines))
