"""Actual source/persistent-cloth closed-loop gate. No gameplay or install.

Each size/feedback pair runs in a separate process. A failed drape or first
dynamic frame remains a failed receipt, never an accepted shortened run.
"""
import argparse
import hashlib
import json
import struct
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--base', type=Path, required=True)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--frames', type=int, default=48)
    args = parser.parse_args()
    base, library, executable, out = [p.resolve() for p in
                                      (args.base, args.library, args.executable, args.out)]
    if not library.is_relative_to(base/'build') or not executable.is_relative_to(ROOT/'build') or not out.is_relative_to(ROOT/'build'):
        raise ValueError('Use owned build binaries and output directory')
    if args.frames < 1 or args.frames > 600:
        raise ValueError('Frame count outside integration test contract')
    files = [ROOT/'tests/garment_source_coupling_test.cpp', ROOT/'tests/garment_input_json.hpp',
             ROOT/'tools/Build-GarmentSourceCoupling.cmd',
             ROOT/'src/runtime/jockstrap_adapter.h', ROOT/'src/runtime/jockstrap_measured_data.h',
             ROOT/'src/runtime/jockstrap_reaction_data.h', ROOT/'tools/Verify-GarmentSourceCoupling.py']
    files += sorted((base/'include/malemod/garments').glob('*.hpp'))
    files += [base/'include/malemod/surface/runtime.hpp',
              base/'include/malemod/surface/garment_impulse.hpp',
              base/'src/surface/runtime.cpp', base/'provenance/source-surface.json']
    inputs = {str(p):sha(p) for p in files}
    receipt = dict(schema=1, scope='Actual partitioned source/cloth offline integration',
                   sourceHz=60, clothHz=120, frames=args.frames, observedGameplay=False,
                   installedGameUnchanged=True, nonlinearSurfaceJacobian=False,
                   materialFrameOffsetProjection='analytic rendered/material-frame gradients; frame support moment explicit',
                   baseCommit=git(base, 'rev-parse', 'HEAD'),
                   canonicalCommit=git(ROOT, 'rev-parse', 'HEAD'),
                   baseDirty=git(base, 'status', '--porcelain', '--untracked-files=all'),
                   canonicalDirty=git(ROOT, 'status', '--porcelain', '--untracked-files=all'),
                   executable=dict(path=str(executable), sha256=sha(executable)),
                   library=dict(path=str(library), sha256=sha(library)), sourceHashes=inputs,
                   cases=[])
    out.mkdir(parents=True, exist_ok=True)
    for size in (25, 50, 75, 100):
        cases = []
        for feedback in (0, 1):
            directory = out/f'overall{size}-feedback{feedback}'
            if directory.exists():
                raise ValueError('Use a fresh case directory; retained outputs must not mask a failed run')
            run = subprocess.run([str(executable), str(size), str(feedback), str(args.frames), str(directory)],
                                 capture_output=True, text=True)
            (directory/'stdout.txt').write_text(run.stdout, encoding='utf-8')
            (directory/'stderr.txt').write_text(run.stderr, encoding='utf-8')
            rows = [json.loads(line) for line in (directory/'frames.jsonl').read_text().splitlines()]
            case = dict(overall=size, feedback=feedback, exitCode=run.returncode,
                        completedFrames=len(rows), files={p.name:sha(p) for p in directory.iterdir() if p.is_file()},
                        movingReactionExercised=any(r['contactRecords'] > 0 and r['reactionSeconds'] > 0 for r in rows),
                        allFrameBudgets=all(r['contactsSatisfied'] and r['materialSatisfied'] and r['fineStretchRatio'] <= 1.15 for r in rows),
                        persistentRest=all(r['materialResets'] <= 1 for r in rows),
                        exactSubsteps=all(r['clothSubsteps'] == 2 for r in rows),
                        passed=False)
            final = directory/'final-source.bin'
            case['numericalPassed'] = bool(run.returncode == 0 and len(rows) == args.frames and final.is_file()
                                  and struct.unpack_from('<I', final.read_bytes())[0] == 6
                                  and case['movingReactionExercised'] and case['allFrameBudgets']
                                  and case['persistentRest'] and case['exactSubsteps'])
            backlog = maximum_backlog = 0.
            for row in rows:
                backlog = max(0., backlog + row['stepMilliseconds'] - 1000/60)
                maximum_backlog = max(maximum_backlog, backlog)
            case['meanStepMilliseconds'] = sum(r['stepMilliseconds'] for r in rows)/len(rows) if rows else None
            case['maximumAccumulatedLagMilliseconds'] = maximum_backlog
            case['realtimeBudgetSatisfied'] = bool(rows and case['meanStepMilliseconds'] <= 1000/60
                                                   and backlog <= 100 and maximum_backlog <= 250)
            case['passed'] = case['numericalPassed'] and case['realtimeBudgetSatisfied']
            cases.append(case)
            print(size, feedback, 'PASS' if case['passed'] else 'FAIL', len(rows), flush=True)
        complete = all(c['numericalPassed'] for c in cases)
        changed = complete and cases[0]['files']['final-source.bin'] != cases[1]['files']['final-source.bin']
        receipt['cases'].append(dict(overall=size, records=cases, actualFeedbackChangesSource=bool(changed),
                                     numericalPassed=bool(complete and changed),
                                     passed=bool(all(c['passed'] for c in cases) and changed)))
        (out/'proof.json').write_text(json.dumps(receipt, indent=2)+'\n', encoding='utf-8')
    receipt['sourceUnchangedDuringRun'] = all(p.exists() and sha(p) == inputs[str(p)] for p in files)
    receipt['passed'] = receipt['sourceUnchangedDuringRun'] and all(c['passed'] for c in receipt['cases'])
    (out/'proof.json').write_text(json.dumps(receipt, indent=2)+'\n', encoding='utf-8')
    return 0 if receipt['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
