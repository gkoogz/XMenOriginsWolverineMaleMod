"""Bounded offline canonical walking-sheet evaluation; never installs a game."""
import argparse, hashlib, json, subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--base', type=Path, required=True)
    p.add_argument('--evaluator', type=Path, required=True)
    p.add_argument('--sheet-test', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--extremes', action='store_true')
    p.add_argument('--closure-only', action='store_true')
    a = p.parse_args()
    output = a.out.resolve()
    if not output.is_relative_to(ROOT/'build') or output.exists():
        raise ValueError('Choose a new owned canonical build directory')
    base = a.base.resolve()
    accepted = json.loads((base/'provenance/source-surface.json').read_text())
    if sha(a.evaluator) != accepted['currentProcessRuntime']['runtimeExecutableSHA256']:
        raise ValueError('Evaluator is not the recorded complete Win32 source runtime')
    output.mkdir(parents=True)
    cases = [(2, size, 50) for size in (25, 50, 75, 100)]
    if a.extremes:
        cases += [(state, size, angle) for state in (0, 1, 2)
                  for size in (25, 100) for angle in (1, 100)]
    rows = []
    for state, size, angle in cases:
        label = f'state{state}-overall{size}-angle{angle}'
        folder = output/label
        folder.mkdir()
        values = [state]+[50]*17
        values[1], values[5] = size, angle  # documented oracle-file control order
        controls = folder/'controls.txt'
        controls.write_text(' '.join(map(str, values))+'\n')
        prefix = folder/'surface.xyz'
        evaluated = subprocess.run([str(a.evaluator.resolve()), str(controls), str(prefix), '120'],
                                   capture_output=True, text=True, timeout=120)
        row = dict(case=label, state=state, overall=size, restAngle=angle,
                   controlsSHA256=sha(controls), sourceSteps=120,
                   sourceReturnCode=evaluated.returncode)
        (folder/'source.log').write_text(evaluated.stdout+evaluated.stderr)
        if evaluated.returncode == 0:
            command = [str(a.sheet_test.resolve()), str(prefix), str(folder/'sheet')]
            if a.closure_only:
                command.append('--closure-only')
            checked = subprocess.run(command,
                                     capture_output=True, text=True, timeout=120)
            (folder/'sheet.log').write_text(checked.stdout+checked.stderr)
            row.update(sheetReturnCode=checked.returncode,
                       sourceHashes={f.name: sha(f) for f in folder.glob('surface.xyz*')},
                       outputLogSHA256=sha(folder/'sheet.log'))
            proof = folder/'sheet'/('closure-proof.json' if a.closure_only else 'sheet-proof.json')
            if proof.exists():
                row['proof'] = json.loads(proof.read_text())
                row['proofSHA256'] = sha(proof)
            else:
                row['failure'] = checked.stderr.strip()
            body_proof = folder/'sheet/body-closure-proof.json'
            if body_proof.exists():
                row['bodyClosure'] = json.loads(body_proof.read_text())
        rows.append(row)
        print(label, row.get('sheetReturnCode', 'source-failure'), flush=True)
        receipt = dict(evaluatorSHA256=sha(a.evaluator), sheetExecutableSHA256=sha(a.sheet_test),
                       baseHead=subprocess.check_output(['git', '-C', str(base), 'rev-parse', 'HEAD'], text=True).strip(),
                       sourceBankSHA256=sha(base/'assets/wolverine-reference/geometry.npz'),
                       semanticsSHA256=sha(ROOT/'src/runtime/jockstrap_regions_data.h'),
                       measuredRoutesSHA256=sha(ROOT/'src/runtime/jockstrap_measured_data.h'),
                       fullAnatomyVertices=17528, fullAnatomyTriangles=35000,
                       scope='separate native anatomical/body closure' if a.closure_only else 'shared static sheet fit',
                       simulate=False, observedGameplay=False, rows=rows)
        (output/'receipt.json').write_text(json.dumps(receipt, indent=2)+'\n')
    return 0 if all(r.get('sheetReturnCode') == 0 for r in rows) else 1

if __name__ == '__main__':
    raise SystemExit(main())
