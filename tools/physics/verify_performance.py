"""Build and replay reference/candidate sequentially; compare every frame.

Usage: python verify_performance.py OUTPUT_DIRECTORY [--build-only] [--case N]
Requires the same x86 MSVC/DirectX SDK as build.cmd. Reference defaults to the
unchanged 1.2 commit; override --reference for future architecture changes.
"""
from pathlib import Path
import argparse, csv, json, statistics, subprocess, zipfile

parser=argparse.ArgumentParser()
parser.add_argument('output', type=Path)
parser.add_argument('--reference', default='21275c3')
parser.add_argument('--build-only', action='store_true')
parser.add_argument('--case', type=int)
parser.add_argument('--skip-build', action='store_true')
args=parser.parse_args()
repo=Path(__file__).resolve().parents[2]
out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
cases=['default-motion','maximum-motion','minimum-controls','state-erect','maximum-intense-pulse','tight-semi-gentle','control-and-reset-transitions','15fps-moderate-pulse']
if not args.skip_build:
    archive=out/'reference.zip'
    subprocess.run(['git','-c',f'safe.directory={repo.as_posix()}','-C',str(repo),'archive','--format=zip',f'--output={archive}',args.reference,'src/runtime'],check=True)
    with zipfile.ZipFile(archive) as z:z.extractall(out/'reference')
    for variant,runtime in [('reference',out/'reference/src/runtime/d3d9_proxy.cpp'),('candidate',repo/'src/runtime/d3d9_proxy.cpp')]:
        unit=out/f'{variant}.cpp'
        unit.write_text(('#define PREPARED_SHAPE_TEST\n' if variant=='candidate' else '')+f'#define RUNTIME_SOURCE "{runtime.as_posix()}"\n#include "{(repo/"tools/physics/performance_replay.cpp").as_posix()}"\n')
        script=out/f'build-{variant}.cmd'
        script.write_text('@echo off\ncall "C:\\BuildTools\\VC\\Auxiliary\\Build\\vcvars32.bat" >nul\n'+f'cl.exe /nologo /O2 /EHsc /std:c++17 /I"C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Include" "{unit}" /link /OUT:"{out/variant}.exe" /LIBPATH:"C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86" d3dx9.lib user32.lib gdi32.lib winmm.lib\n')
        p=subprocess.run(['cmd','/c',str(script)],cwd=out,capture_output=True)
        (out/f'build-{variant}.log').write_bytes(p.stdout+p.stderr)
        if p.returncode:raise RuntimeError(p.stdout.decode(errors='replace')+p.stderr.decode(errors='replace'))
if args.build_only:raise SystemExit(0)
report_path=out/'verification.json'
report=json.loads(report_path.read_text()) if report_path.exists() else {}
for case in [args.case] if args.case is not None else range(len(cases)):
    rows={};logs={}
    for variant in ['reference','candidate']:
        name=f'{case}-{cases[case]}-{variant}'
        p=subprocess.run([str(out/f'{variant}.exe'),str(out/f'{name}.csv'),str(case),'240'],cwd=out,capture_output=True)
        logs[variant]=p.stdout.decode(errors='replace');(out/f'{name}.log').write_bytes(p.stdout+p.stderr)
        if p.returncode:raise RuntimeError(f'{name}: {p.returncode}: {logs[variant]}')
        rows[variant]=list(csv.DictReader((out/f'{name}.csv').open()))
    differences=[]
    for a,b in zip(rows['reference'],rows['candidate']):
        fields=[k for k in ['mesh','body','physics'] if a[k]!=b[k]]
        if fields:differences.append(dict(frame=int(a['frame']),fields=fields))
    timings={variant:statistics.median(float(r['ms']) for r in data[30:]) for variant,data in rows.items()}
    report[cases[case]]=dict(frames=len(rows['reference']),mismatching_frames=differences,median_cpu_ms=timings,reduction_percent=100*(1-timings['candidate']/timings['reference']),logs=logs)
    report_path.write_text(json.dumps(report,indent=2))
    print(cases[case],timings,'mismatches',len(differences),flush=True)
    if differences:raise RuntimeError(f'Equivalence failed: {differences[:5]}')
