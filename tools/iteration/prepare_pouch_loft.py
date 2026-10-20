"""Prepare an explicit, uninstalled loft comparison from a sealed private build.

This copies the supplied sealed walking-baseline build into a new private
directory and records every intentional source overlay. The optional cage flags
validate the current production pouch through the same sealed native harness.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil


def main():
    ap=argparse.ArgumentParser(__doc__)
    ap.add_argument('--build',type=Path,required=True)
    ap.add_argument('--base',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--cage',action='store_true',help='Use the coherent pre-shaped pouch cage')
    ap.add_argument('--cached-extents',action='store_true',help='Include exact-input physics support extent caching')
    ap.add_argument('--staged-geometry',action='store_true',help='Evaluate gameplay geometry in CPU staging memory')
    ap.add_argument('--serial-geometry',action='store_true',help='Measure scheduler overhead with identical serial vertex arithmetic')
    ap.add_argument('--covered-interior',action='store_true',help='Experimental hidden-interior detail omission; requires exposed-surface comparison')
    ap.add_argument('--covered-cadence',action='store_true',help='Experimental alternate-frame covered surface evaluation; requires motion review')
    ap.add_argument('--canonical-cage',action='store_true',help='Validate the exact authoritative cage adapter after adoption')
    args=ap.parse_args();old=args.build.resolve();root=args.output.resolve()
    root.mkdir(parents=True,exist_ok=False)
    for name in ('source','base'):
        shutil.copytree(old/name,root/name)
    header='pouch_cage.hpp' if args.cage else 'pouch_loft.hpp'
    name='base/include/malemod/garments/'+header
    (root/name).write_bytes((args.base/'include/malemod/garments'/header).read_bytes())
    adapter=root/'source/src/runtime/meridian_adapter.h';text=adapter.read_text()
    start='  auto chart=M::PrepareAnchoredEnvelope('
    end='  std::vector<M::Vec> seamDelta'
    if text.count(start)!=1 or text.count(end)!=1:
        raise SystemExit('Adapter experimental splice contract changed')
    text=text.replace('#include "meridian_attempt.h"','#include "meridian_attempt.h"\n#include <malemod/garments/pouch_loft.hpp>')
    a=text.index(start);b=text.index(end,a)
    text=text[:a]+'''  const auto raw=points;M::WrapReceipt receipt{};bool wrapped=false;
  preparationTimer.stage="loft";
  auto loft=M::FitPouchLoft(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::count,MeridianRecipe::sampleCount-1,.12f,16);
  if(renderFrameSerial%120==0)Log("Experimental pouch loft sections=%u samples=%u length=%.4f radius=%.4f continuousContactCertified=0",loft.sections,loft.samples,loft.length,loft.maxRadius);
'''+text[b:]
    text='\n'.join(line for line in text.split('\n') if 'if(attempts%120==1)Log("Meridian continuity' not in line)
    if args.cage:
        text=text.replace('pouch_loft.hpp','pouch_cage.hpp').replace('M::FitPouchLoft','M::FitPouchCage').replace('MeridianRecipe::sampleCount-1,.12f,16','MeridianRecipe::sampleCount-1,apex,.12f,16').replace('Experimental pouch loft','Experimental pouch cage')
    adapter.write_text(text)
    command=(old/'build.cmd').read_text().replace(str(old).replace('/','\\'),str(root).replace('/','\\'))
    (root/'build.cmd').write_text(command)
    proof=json.loads((old/'provenance.json').read_text())
    proof['parentRuntimeSHA256']=proof.pop('runtimeSHA256')
    proof['experimentalAlgorithm']='ordered-measured-sample-elliptical-loft-16'
    if args.cage:proof['experimentalAlgorithm']='coherent-pre-shaped-cage-local-contacts-16'
    if args.cached_extents:
        budget='base/include/malemod/garments/pouch_budget.hpp'
        (root/budget).write_bytes((args.base/'include/malemod/garments/pouch_budget.hpp').read_bytes())
        proof['candidateInputSHA256'][budget]=hashlib.sha256((root/budget).read_bytes()).hexdigest()
        runtime=Path(__file__).resolve().parents[2]/'src/runtime'
        for h in ['compliant_dynamics.h','anterior_body_adapter.h','anterior_envelope_adapter.h','performance_metrics.h','anatomy_surface.h','pouch_contact.h']:
            target=root/'source/src/runtime'/h
            target.write_bytes((runtime/h).read_bytes())
            proof['candidateInputSHA256'][str(target.relative_to(root))]=hashlib.sha256(target.read_bytes()).hexdigest()
    proof['continuousContactCertified']=False
    for item in [name,'source/src/runtime/meridian_adapter.h']:
        proof['candidateInputSHA256'][item]=hashlib.sha256((root/item).read_bytes()).hexdigest()
    proof['prototypeBuilderSHA256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    if args.staged_geometry:
        authoritative=(Path(__file__).resolve().parents[2]/'src/runtime/d3d9_proxy.cpp').read_text()
        cpp=root/'source/src/runtime/d3d9_proxy.cpp';private=cpp.read_text()
        begin='static void ApplyShape(){PerfScope perf(8);';finish='#include "menu_input.h"'
        replacement=authoritative[authoritative.index(begin):authoritative.index(finish,authoritative.index(begin))]
        private=private[:private.index(begin)]+replacement+private[private.index(finish,private.index(begin)):]
        cpp.write_text(private);proof['candidateInputSHA256']['source/src/runtime/d3d9_proxy.cpp']=hashlib.sha256(cpp.read_bytes()).hexdigest()
    if args.serial_geometry:
        h=root/'source/src/runtime/geometry_pass.h';s=h.read_text()
        if s.count('if(count<512)')!=1:raise SystemExit('Geometry scheduling contract changed')
        h.write_text(s.replace('if(count<512)','if(true)'))
        proof['candidateInputSHA256']['source/src/runtime/geometry_pass.h']=hashlib.sha256(h.read_bytes()).hexdigest()
    if args.covered_interior:
        h=root/'source/src/runtime/rounded_shape.h';s=h.read_text()
        for call in ['ApplyPouchSurface();','ApplyRapheTubeSkin(body);']:
            if s.count(call)!=1:raise SystemExit('Covered-interior experiment contract changed')
            s=s.replace(call,'if(clothingStyle!=1)'+call)
        h.write_text(s);proof['candidateInputSHA256']['source/src/runtime/rounded_shape.h']=hashlib.sha256(h.read_bytes()).hexdigest()
        proof['exposedSurfaceParityRequired']=True
    if args.covered_cadence:
        cpp=root/'source/src/runtime/d3d9_proxy.cpp';s=cpp.read_text()
        oldcall='else if(graftBuffer)ApplyShape();'
        if s.count(oldcall)!=1:raise SystemExit('Covered cadence splice contract changed')
        cpp.write_text(s.replace(oldcall,'else if(graftBuffer&&(clothingStyle!=1||shapeDirty||teachingTimeline.active||(renderFrameSerial%2==0)))ApplyShape();'))
        proof['candidateInputSHA256']['source/src/runtime/d3d9_proxy.cpp']=hashlib.sha256(cpp.read_bytes()).hexdigest()
        proof['coveredSurfaceCadence']='alternate rendered frames; dirty controls and teaching bypass'
    if args.canonical_cage:
        if not args.cage:raise SystemExit('--canonical-cage requires --cage')
        source=Path(__file__).resolve().parents[2]/'src/runtime/meridian_adapter.h'
        adapter.write_bytes(source.read_bytes())
        proof['candidateInputSHA256']['source/src/runtime/meridian_adapter.h']=hashlib.sha256(adapter.read_bytes()).hexdigest()
        for name in ['meridian_material.h','fabric_material.inl']:
            target=root/'source/src/runtime'/name
            target.write_bytes((source.parent/name).read_bytes())
            proof['candidateInputSHA256'][str(target.relative_to(root))]=hashlib.sha256(target.read_bytes()).hexdigest()
    (root/'provenance-pending.json').write_text(json.dumps(proof,indent=2))
    print(root/'build.cmd')


if __name__=='__main__':
    main()
