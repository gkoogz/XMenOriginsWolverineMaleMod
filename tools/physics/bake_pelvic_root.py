"""Bake fixed material-space collar bindings; no mesh or topology changes."""
from pathlib import Path
import argparse
import re
import struct

parser = argparse.ArgumentParser()
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
src = Path(__file__).resolve().parents[2] / 'src/runtime'

def values(file, name):
    text = (src / file).read_text()
    body = re.search(r'\b' + name + r'\[[^]]+\]\s*=\s*\{(.*?)\};', text, re.S)[1]
    return [float(v.strip().rstrip('f')) for v in body.split(',') if v.strip()]

def smooth(x):
    x = min(1., max(0., x))
    return x*x*x*(x*(6*x-15)+10)

def literal(x):
    x = struct.unpack('f', struct.pack('f', x))[0]
    text = format(x, '.9g')
    return text + ('f' if '.' in text or 'e' in text else '.0f')

flex = values('physics_weights.h', 'phys_flex_coordinate')
ball = values('physics_weights.h', 'phys_scrotum_weight')
base = values('morph_targets.h', 'morph_base')
follow = [smooth(t/.30) for t in flex]
mask = [smooth((base[3*i+2]-82)/3)*(1-smooth(b/.2)) for i, b in enumerate(ball)]
angle = [1+(f-1)*m for f, m in zip(follow, mask)]
text = '#pragma once\n// Baked proximal skin weights; lower pouch and distal shaft stay untouched.\n'
for name, data in [('pelvicRootFollow', follow), ('pelvicRootMask', mask), ('pelvicAngleFollow', angle)]:
    text += f'static const float {name}[{len(data)}]={{\n' + ','.join(map(literal, data)) + '\n};\n'
target = src / 'pelvic_root_binding.h'
if args.check:
    assert target.read_text() == text, 'Stale pelvic root bindings; run this baker.'
else:
    target.write_text(text)
print('Verified' if args.check else 'Baked', len(flex), 'pelvic material bindings.')
