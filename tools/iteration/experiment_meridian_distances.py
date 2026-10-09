"""Offline exact-output experiment; does not edit a runtime or install a DLL.

Run profile_meridian_cpu.py first against an exact candidate. This experiment
keeps topology, iterations and update rate, and reuses three signed distances
inside each face/plane test. Both variants use the same private pose sequence.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import statistics
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--build', type=Path, required=True)
    ap.add_argument('--profile', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--vcvars', type=Path, default=Path('C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat'))
    args = ap.parse_args()
    build, profile, out = args.build.resolve(), args.profile.resolve(), args.output.resolve()
    if out.exists():
        raise SystemExit('Choose a fresh output directory')
    proof = json.loads((profile/'profile.json').read_text())
    provenance = json.loads((build/'provenance.json').read_text())
    if digest(profile/'profile.cpp') != proof['profileSHA256'] or proof['runtimeSHA256'] != provenance['runtimeSHA256']:
        raise SystemExit('Profile/build identity differs')
    for relative, expected in provenance['candidateInputSHA256'].items():
        if relative.startswith('base/') or relative.endswith('/meridian_recipe.h'):
            if digest(build/relative) != expected:
                raise SystemExit('Build input differs: '+relative)
    poses = [Path(x) for x in (profile/'poses.txt').read_text().splitlines()]
    if len(poses) != len(proof['poseInputs']):
        raise SystemExit('Pose count differs')
    for p, expected in zip(poses, proof['poseInputs']):
        if p.name != expected['path'] or digest(p) != expected['sha256']:
            raise SystemExit('Pose differs: '+str(p))
    source = (profile/'profile.cpp').read_text()
    replacements = {
        ' M::SurfaceContinuity continuity;': ' std::ofstream geometry(argv[3],std::ios::binary);\n M::SurfaceContinuity continuity;',
        'Timer total("total");auto phase=Clock::now();': 'auto totalBegin=Clock::now();auto phase=Clock::now();',
        '  }catch(const std::exception& e){++failures;}': '  }catch(const std::exception& e){++failures;}\n  Record("total",totalBegin);\n  geometry.write((const char*)points.data(),points.size()*sizeof(M::Vec));\n  geometry.write((const char*)vertices.data(),MeridianRecipe::count*sizeof(Vertex));',
    }
    for before, after in replacements.items():
        if source.count(before) != 1:
            raise SystemExit('Profile splice changed: '+before)
        source = source.replace(before, after)
    out.mkdir(parents=True)
    (out/'profile.cpp').write_text(source)
    target = out/'reuse/include/malemod/garments'
    target.mkdir(parents=True)
    for name in ('meridian_continuity.hpp','meridian_clearance.hpp','meridian_rig.hpp','meridian_runtime.hpp'):
        shutil.copyfile(build/'base/include/malemod/garments'/name, target/name)
    header = target/'meridian_clearance.hpp'
    source = header.read_text()
    changes = {
        '    for(const Plane& plane:hull){float cost=0,score=std::numeric_limits<float>::infinity();bool allowed=true;\n     for(unsigned id:ids)score=(std::min)(score,Signed(plane,work[id]));':
        '    for(const Plane& plane:hull){float cost=0,score=std::numeric_limits<float>::infinity();bool allowed=true;\n     float distances[3];for(unsigned j=0;j<3;j++){distances[j]=Signed(plane,work[ids[j]]);score=(std::min)(score,distances[j]);}',
        '     for(unsigned id:ids){float value=Signed(plane,work[id]);score=(std::min)(score,value);':
        '     for(unsigned j=0;j<3;j++){unsigned id=ids[j];float value=distances[j];score=(std::min)(score,value);',
    }
    for before, after in changes.items():
        if source.count(before) != 1:
            raise SystemExit('Numerical splice changed')
        source = source.replace(before, after)
    header.write_text(source)
    command = f'@echo off\ncall "{args.vcvars}" >nul\nif errorlevel 1 exit /b 1\n'
    for mode, inc in [('baseline',build/'base/include'),('reuse',out/'reuse/include')]:
        command += f'cl /nologo /O2 /MT /EHsc /std:c++17 /I"{inc}" /I"{build / "source/src/runtime"}" "{out / "profile.cpp"}" /Fo"{out / (mode+".obj")}" /Fe"{out / (mode+".exe")}"\nif errorlevel 1 exit /b 1\n'
    (out/'build.cmd').write_text(command)
    subprocess.run(['cmd','/c',str(out/'build.cmd')],check=True)
    rows = []
    for trial in range(3):
        for mode in (('baseline','reuse') if trial%2 == 0 else ('reuse','baseline')):
            binary = out/(mode+f'-{trial}.bin')
            result = json.loads(subprocess.check_output([str(out/(mode+'.exe')),str(profile/'poses.txt'),'10',str(binary)],text=True))
            result.update(mode=mode,trial=trial,geometrySHA256=digest(binary))
            rows.append(result)
    means = {m: {p: statistics.mean(r['phases'][p]['meanMs'] for r in rows if r['mode'] == m) for p in ('total','clearance')} for m in ('baseline','reuse')}
    result = dict(runtimeSHA256=proof['runtimeSHA256'],sameOutput=len({r['geometrySHA256'] for r in rows})==1,means=means,runs=rows,nativeFPSMeasured=False,installed=False)
    (out/'comparison.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(sameOutput=result['sameOutput'],means=means),indent=2))
    if not result['sameOutput']:
        raise SystemExit('REJECT: output changed')


if __name__ == '__main__':
    main()
