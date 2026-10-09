"""Reproduce the posterior trap and validate recovery in the SDK-free source solver.

Private generated source/geometry and compiler outputs stay in a fresh output.
This is numerical evidence, not native gameplay or attachment visual approval.
"""
import argparse, ast, hashlib, json, pathlib, subprocess

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--base',type=pathlib.Path,required=True)
    ap.add_argument('--output',type=pathlib.Path,required=True)
    ap.add_argument('--vcvars',default='C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat')
    a=ap.parse_args();base=a.base.resolve();out=a.output.resolve()
    if out.exists():raise SystemExit('Choose a fresh private fixture output')
    out.mkdir(parents=True);script=pathlib.Path(__file__).resolve().parent;repo=script.parents[1]
    import sys
    kernel=out/'surface-kernel.inc'
    subprocess.run([sys.executable,str(base/'tools/extract_surface_runtime.py'),'--process-isolated','--output',str(kernel)],check=True,cwd=base)
    original_hash=sha(kernel)
    # Use exactly the reviewed native builder's splice contract.
    tree=ast.parse((script/'build_private_runtime.py').read_text())
    replacements=[ast.literal_eval(n.value) for n in ast.walk(tree) if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='replacements' for t in n.targets)]
    if len(replacements)!=1:raise RuntimeError('Native physics integration contract changed')
    s=kernel.read_text()
    for before,after in replacements[0].items():
        if s.count(before)!=1:raise RuntimeError('Source fixture integration site differs: '+before)
        s=s.replace(before,after)
    kernel.write_text(s)
    header=repo/'src/runtime/anterior_envelope_adapter.h';bridge=header.read_text()
    first=bridge.index('static bool AnteriorEnvelopeEnabled()')
    last=bridge.index('static V3 PDEnvelopeRadii()',first)
    bridge=bridge[:first]+'static bool AnteriorEnvelopeEnabled(){return fixtureEnvelopeEnabled;}\n'+bridge[last:]
    (out/'anterior_envelope_adapter.h').write_text(bridge)
    (out/'runtime.cpp').write_bytes((base/'src/surface/runtime.cpp').read_bytes())
    fixture=script/'anterior_recovery_fixture.cpp'
    (out/'fixture.cpp').write_bytes(fixture.read_bytes())
    exe=out/'fixture.exe'
    command=out/'build.cmd'
    command.write_text('@echo off\ncall "'+a.vcvars+'" >nul\ncl /nologo /O2 /MT /EHsc /std:c++17 /fp:precise /DMALEMOD_SURFACE_PROCESS_ISOLATED=1 /I"'+str(base/'include')+'" /I"'+str(base/'legacy/wolverine/third-party')+'" "'+str(out/'fixture.cpp')+'" /Fe"'+str(exe)+'" /Fo"'+str(out/'fixture.obj')+'"\n')
    subprocess.run(['cmd','/c',str(command)],check=True,cwd=out)
    cases=[]
    for enabled,scenario in [('disabled','trapped'),('enabled','trapped'),('enabled','motion')]:
        result=subprocess.run([str(exe),enabled,scenario],capture_output=True,text=True,timeout=180)
        (out/(enabled+'-'+scenario+'.txt')).write_text(result.stdout+result.stderr)
        if result.returncode:raise RuntimeError('Fixture failed: '+enabled+' '+scenario+' '+result.stdout+result.stderr)
        cases.append(json.loads(result.stdout))
    record=dict(schema=1,nativeGameplay=False,attachmentVisualGate=False,cases=cases,
        originalKernelSHA256=original_hash,patchedKernelSHA256=sha(kernel),executableSHA256=sha(exe),
        inputs={str(p):sha(p) for p in [header,fixture,script/'build_private_runtime.py',pathlib.Path(__file__),base/'include/malemod/physics/anterior_envelope.hpp',base/'src/surface/runtime.cpp']})
    (out/'proof.json').write_text(json.dumps(record,indent=2)+'\n')
    print(json.dumps(record))
if __name__=='__main__':main()
