from pathlib import Path
import hashlib,struct,sys
capture=Path(sys.argv[1])
out=Path(__file__).resolve().parents[2]/'src/runtime/tank_skin_shaders.h'
lines=['#pragma once','// Exact native WStart base/spotlight shader variants, captured 2026-09-28.',
       '// These are identities for material adapters; native shaders remain bound.']
for name,index in [('Base',1),('Spot',8)]:
 for stage in ['vs','ps']:
  data=(capture/f'draw_{index:03}_{stage}.bin').read_bytes()
  lines.append('// SHA256 '+hashlib.sha256(data).hexdigest())
  lines.append(f'static const DWORD skinTank{name}{stage.upper()}[]={{')
  values=struct.unpack('<'+'I'*(len(data)//4),data)
  for at in range(0,len(values),8):lines.append(' '+','.join(f'0x{v:08x}' for v in values[at:at+8])+',')
  lines.append('};')
out.write_text('\n'.join(lines)+'\n')
print('Wrote exact tank shader identities:',out)
