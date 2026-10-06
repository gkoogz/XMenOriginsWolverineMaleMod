"""Export shared measured force bindings; no adapter-local anatomy laws."""
import argparse,hashlib,json,sys
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser();p.add_argument('--base',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args();sys.path.insert(0,str(a.base))
from malemod_base.garment_reactions import reference_reaction_bindings,reaction_bindings_header
geometry=a.base/'assets/wolverine-reference/geometry.npz';values=reference_reaction_bindings(np.load(geometry));a.out.write_text(reaction_bindings_header(values))
print(json.dumps(dict(sourceSHA256=hashlib.sha256(geometry.read_bytes()).hexdigest(),exporterSHA256=hashlib.sha256((a.base/'malemod_base/garment_reactions.py').read_bytes()).hexdigest(),vertices=len(values),generalizedBodies=14,nonlinearSurfaceJacobian=False)))
