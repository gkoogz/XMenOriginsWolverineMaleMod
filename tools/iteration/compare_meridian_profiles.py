"""Alternate exact-build CPU replays and require bit-identical geometry.

Profiles must use the same private pose sequence. This is CPU evidence, not
an FPS or visual acceptance test. No installed runtime is modified.
"""
import argparse
import hashlib
import json
from pathlib import Path
import statistics
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline-build', type=Path, required=True)
    parser.add_argument('--candidate-build', type=Path, required=True)
    parser.add_argument('--baseline-profile', type=Path, required=True)
    parser.add_argument('--candidate-profile', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--repeats', type=int, default=10)
    args = parser.parse_args()
    if not 1 <= args.repeats <= 100:
        parser.error('Use repeats 1..100')
    out = args.output.resolve()
    if out.exists():
        raise SystemExit('Choose a fresh output directory')
    sources, proofs, builds, lists = {}, {}, {}, {}
    for mode in ('baseline', 'candidate'):
        build = getattr(args, mode+'_build').resolve()
        profile = getattr(args, mode+'_profile').resolve()
        proof = json.loads((profile/'profile.json').read_text())
        manifest = json.loads((build/'provenance.json').read_text())
        if digest(profile/'profile.cpp') != proof['profileSHA256'] or proof['runtimeSHA256'] != manifest['runtimeSHA256']:
            raise SystemExit('Profile/build identity differs: '+mode)
        for name, expected in manifest['candidateInputSHA256'].items():
            if name.startswith('base/') or name.startswith('source/src/runtime/meridian_'):
                if digest(build/name) != expected:
                    raise SystemExit('Build input differs: '+name)
        poses = [Path(p) for p in (profile/'poses.txt').read_text().splitlines()]
        if len(poses) != len(proof['poseInputs']):
            raise SystemExit('Pose count differs')
        for pose, expected in zip(poses, proof['poseInputs']):
            if pose.name != expected['path'] or digest(pose) != expected['sha256']:
                raise SystemExit('Pose identity differs')
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
        sources[mode], proofs[mode], builds[mode], lists[mode] = source, proof, build, profile/'poses.txt'
    if proofs['baseline']['poseInputs'] != proofs['candidate']['poseInputs']:
        raise SystemExit('Profiles use different poses')
    if proofs['baseline']['recipeSHA256'] != proofs['candidate']['recipeSHA256']:
        raise SystemExit('Profiles use different recipes')
    out.mkdir(parents=True)
    command = '@echo off\ncall "C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat" >nul\nif errorlevel 1 exit /b 1\n'
    for mode in sources:
        (out/(mode+'.cpp')).write_text(sources[mode])
        build = builds[mode]
        command += f'cl /nologo /O2 /MT /EHsc /std:c++17 /I"{build / "base/include"}" /I"{build / "source/src/runtime"}" "{out / (mode+".cpp")}" /Fo"{out / (mode+".obj")}" /Fe"{out / (mode+".exe")}"\nif errorlevel 1 exit /b 1\n'
    (out/'build.cmd').write_text(command)
    subprocess.run(['cmd', '/c', str(out/'build.cmd')], check=True)
    runs = []
    for trial in range(3):
        for mode in (('baseline', 'candidate') if trial%2 == 0 else ('candidate', 'baseline')):
            binary = out/(mode+f'-{trial}.bin')
            result = json.loads(subprocess.check_output([str(out/(mode+'.exe')), str(lists[mode]), str(args.repeats), str(binary)], text=True))
            result.update(mode=mode, trial=trial, geometrySHA256=digest(binary))
            runs.append(result)
    phases = set.intersection(*(set(r['phases']) for r in runs))
    means = {mode: {p: statistics.mean(r['phases'][p]['meanMs'] for r in runs if r['mode'] == mode) for p in sorted(phases)} for mode in sources}
    result = dict(sameOutput=len({r['geometrySHA256'] for r in runs}) == 1,
                  means=means, runs=runs, nativeFPSMeasured=False,
                  buildRuntimeSHA256={m: p['runtimeSHA256'] for m, p in proofs.items()},
                  poseInputs=proofs['baseline']['poseInputs'])
    (out/'comparison.json').write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(dict(sameOutput=result['sameOutput'], means=means), indent=2))
    if not result['sameOutput'] or any(r['outerFailuresIncludingWarmup'] for r in runs):
        raise SystemExit('REJECT: geometry changed or outer failure occurred')


if __name__ == '__main__':
    main()
