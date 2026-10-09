"""Build an isolated runtime derivative of canonical HEAD and its exact Base pin.

No installed file is replaced. Explicit --meridian-recipe overlays the named
current adapter and shared headers, with separate hashes; unrelated dirty files
are excluded. The other derivative is the guarded D3D9Ex factory header. All source is exported from Git into owned build output with provenance.
"""
import argparse, hashlib, io, json, os, pathlib, subprocess, zipfile

def run(*args, cwd=None):
    return subprocess.check_output(args, cwd=cwd)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--output',type=pathlib.Path,required=True)
    ap.add_argument('--base',type=pathlib.Path,required=True)
    ap.add_argument('--source-ref',default='HEAD')
    ap.add_argument('--meridian-recipe',type=pathlib.Path,help='Explicit private experimental recipe; never an installed baseline')
    ap.add_argument('--vcvars',type=pathlib.Path,default=pathlib.Path('C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat'))
    ap.add_argument('--directx-sdk',type=pathlib.Path,default=pathlib.Path('C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)'))
    args=ap.parse_args(); repo=pathlib.Path(__file__).resolve().parents[2]
    target=args.output.resolve()
    if target.exists():raise SystemExit('Refusing existing output; choose fresh owned path')
    target.mkdir(parents=True)
    head=run('git','rev-parse',args.source_ref+'^{commit}',cwd=repo).decode().strip()
    lock=json.loads(run('git','show',head+':dependencies/base.lock.json',cwd=repo))
    with zipfile.ZipFile(io.BytesIO(run('git','archive','--format=zip',head,'src/runtime','third-party','tools/costumes',cwd=repo))) as z:z.extractall(target/'source')
    with zipfile.ZipFile(io.BytesIO(run('git','archive','--format=zip',lock['commit'],'include',cwd=args.base))) as z:z.extractall(target/'base')
    helper=pathlib.Path(__file__).with_name('sealed_d3d9.hpp').read_bytes()
    native=target/'source/src/runtime';(native/'sealed_d3d9.hpp').write_bytes(helper)
    import sys
    subprocess.run([sys.executable,str(target/'source/tools/costumes/prepare_stock_materials.py'),'--runtime',str(native)],check=True)
    (native/'sandbox_world_origin.hpp').write_bytes(pathlib.Path(__file__).with_name('sandbox_world_origin.hpp').read_bytes())
    cpp=native/'d3d9_proxy.cpp';s=cpp.read_text()
    s=s.replace('static bool menuOpen=true, shapeDirty=true;','static bool menuOpen=false, shapeDirty=true;',1)
    old='IDirect3D9* WINAPI Direct3DCreate9(UINT sdk){LoadReal();IDirect3D9* d=realCreate9(sdk);'
    s=s.replace('static HRESULT STDMETHODCALLTYPE HookDIP(', 'static HRESULT STDMETHODCALLTYPE MaleModPrivateSourceDIP(',1)
    s=s.replace('static HRESULT STDMETHODCALLTYPE HookCreateDevice(', '#include "sealed_d3d9.hpp"\nstatic HRESULT STDMETHODCALLTYPE HookCreateDevice(',1)
    s=s.replace('static HRESULT STDMETHODCALLTYPE HookPresent(', 'static void MaleModPrivateCapture(IDirect3DDevice9*,bool);\nstatic HRESULT MaleModPrivateMeasuredPresent(IDirect3DDevice9*,const RECT*,const RECT*,HWND,const RGNDATA*);\nstatic HRESULT MaleModPrivatePresentResult(IDirect3DDevice9*,HRESULT);\nstatic HRESULT STDMETHODCALLTYPE HookPresent(',1)
    s=s.replace('menuTankDrawnThisFrame=false;frameRendered=false;\n  insidePresent=true;', 'MaleModPrivateCapture(d,menuTankDrawnThisFrame);menuTankDrawnThisFrame=false;frameRendered=false;\n  insidePresent=true;',1)
    s=s.replace('insidePresent=true;HRESULT result=origPresent(d,src,dst,wnd,dirty);insidePresent=false;return result;', 'insidePresent=true;HRESULT result=MaleModPrivateMeasuredPresent(d,src,dst,wnd,dirty);insidePresent=false;static unsigned privatePresents=0;if(privatePresents++<12)Log("Private Present %u hr=%08x coop=%08x",privatePresents,result,d->TestCooperativeLevel());return MaleModPrivatePresentResult(d,result);',1)
    s=s.replace('if(SUCCEEDED(hr)&&out&&*out){LoadSettings();','if(SUCCEEDED(hr)&&out&&*out){MaleModPrivatePoolTranslation(*out);MaleModPrivateActivate(wnd?wnd:pp->hDeviceWindow);LoadSettings();',1)
    new='IDirect3D9* WINAPI Direct3DCreate9(UINT sdk){LoadReal();IDirect3D9* d=MaleModPrivateFactory(sdk,realDll,realCreate9);'
    if s.count(old)!=1:raise SystemExit('Canonical factory signature changed; refusing fuzzy source patch')
    cpp.write_text(s.replace(old,new))
    candidate_hashes={}
    if args.meridian_recipe and (native/'meridian_adapter.h').exists():
        raise SystemExit('This source already contains the versioned meridian adapter. Omit --meridian-recipe and use its exact Base pin and committed binding contract.')
    if args.meridian_recipe:
        inputs={
            'base/include/malemod/garments/meridian_continuity.hpp':args.base/'include/malemod/garments/meridian_continuity.hpp',
            'base/include/malemod/garments/meridian_follow.hpp':args.base/'include/malemod/garments/meridian_follow.hpp',
            'source/src/runtime/anterior_envelope_adapter.h':repo/'src/runtime/anterior_envelope_adapter.h',
            'base/include/malemod/physics/anterior_envelope.hpp':args.base/'include/malemod/physics/anterior_envelope.hpp',
            'source/src/runtime/meridian_adapter.h':repo/'src/runtime/meridian_adapter.h',
            'source/src/runtime/meridian_material.h':repo/'src/runtime/meridian_material.h',
            'source/src/runtime/meridian_state_audit.h':repo/'src/runtime/meridian_state_audit.h',
            'source/src/runtime/shared_skin_material.h':repo/'src/runtime/shared_skin_material.h',
            'source/src/runtime/r14_runtime.h':repo/'src/runtime/r14_runtime.h',
            'source/src/runtime/fluid_surface.h':repo/'src/runtime/fluid_surface.h',
            'base/include/malemod/garments/meridian_material.hpp':args.base/'include/malemod/garments/meridian_material.hpp',
            'source/src/runtime/meridian_recipe.h':args.meridian_recipe/'meridian_recipe.h',
            'base/include/malemod/garments/meridian_runtime.hpp':args.base/'include/malemod/garments/meridian_runtime.hpp',
            'base/include/malemod/garments/meridian_clearance.hpp':args.base/'include/malemod/garments/meridian_clearance.hpp',
            'base/include/malemod/garments/meridian_rig.hpp':args.base/'include/malemod/garments/meridian_rig.hpp'}
        for name,path in inputs.items():
            data=path.read_bytes();destination=target/name;destination.parent.mkdir(parents=True,exist_ok=True);destination.write_bytes(data)
            candidate_hashes[name]=hashlib.sha256(data).hexdigest()
        physics=native/'compliant_dynamics.h'
        body=physics.read_text()
        replacements={
            'static void PDRodBody(PDConstraint& c,int s,int j,float dt){':'static bool AnteriorEnvelopeEnabled();\nstatic bool PDAnteriorOwnsContact(V3,V3,float);\nstatic void PDRodBody(PDConstraint& c,int s,int j,float dt){',
            ' int id=pdBody0+s;float t;V3 q{},arm{},n=PDContactNormal(c,s,pdPosition[j],pdPosition[j+1],arm,q,t);':' int id=pdBody0+s;float t;V3 q{},arm{},n=PDContactNormal(c,s,pdPosition[j],pdPosition[j+1],arm,q,t);\n if(PDAnteriorOwnsContact(q,n,radius)){c.normal=0;c.tangent={};c.anchorReady=false;return;}',
            'static void StepConstraintSolver(float dt,float gait,float side){':'#include "anterior_envelope_adapter.h"\nstatic void StepConstraintSolver(float dt,float gait,float side){',
            'j<shaftNodeCount-2;j++)PDRodBody':'j<shaftNodeCount-2+int(AnteriorEnvelopeEnabled());j++)PDRodBody',
            '  PDKeepPouchVentral(ventralCorrection);':'  PDKeepPouchVentral(ventralCorrection);\n  PDAnteriorEnvelope(dt);',
            ' PDSolveContactVelocities(dt);':' PDSolveContactVelocities(dt);\n PDAnteriorEnvelopeVelocity();'}
        for before,after in replacements.items():
            if body.count(before)!=1:raise SystemExit('Physics integration site changed: '+before)
            body=body.replace(before,after)
        physics.write_text(body)
        candidate_hashes['generated/compliant_dynamics.h']=hashlib.sha256(physics.read_bytes()).hexdigest()
        source=cpp.read_text()
        needle='#include "jockstrap_adapter.h"'
        if source.count(needle)!=1:raise SystemExit('Candidate adapter insertion site changed')
        source=source.replace(needle,needle+'\n#include "meridian_adapter.h"')
        source=source.replace('static void UpdateJockstrapSource(const unsigned char* body){',
            'static void UpdateJockstrapSource(const unsigned char* body){\n if(MeridianAdapter::Enabled()){MeridianAdapter::Update(body);return;}')
        source=source.replace('JockstrapAdapter::Draw(', 'MeridianAdapter::Draw(')
        source=source.replace('JockstrapAdapter::CaptureSection(', 'MeridianAdapter::CaptureSection(')
        source=source.replace('JockstrapAdapter::Release();','MeridianAdapter::Release();JockstrapAdapter::Release();')
        declaration='static IDirect3DIndexBuffer9* MeridianAnatomyIndices(IDirect3DDevice9*);\nstatic unsigned MeridianAnatomyIndexCount();\n'
        source=source.replace('#include "r14_runtime.h"',declaration+'#include "r14_runtime.h"')
        cpp.write_text(source)
        for filename,function_name in [('r14_runtime.h','DrawR14'),('menu_tank.h','DrawMenuTank')]:
            path=native/filename;source=path.read_text();begin=source.index('static HRESULT '+function_name+'(')
            prefix,draw=source[:begin],source[begin:]
            marker='  IDirect3DIndexBuffer9* '
            draw=draw.replace(marker,'  auto* clothCoveredIB=MeridianAnatomyIndices(d);\n  unsigned anatomyIndices=clothCoveredIB?MeridianAnatomyIndexCount():nrIndexCount;\n'+marker,1)
            draw=draw.replace('SetIndices(r14IB)','SetIndices(clothCoveredIB?clothCoveredIB:r14IB)').replace('nrIndexCount/3','anatomyIndices/3')
            path.write_text(prefix+draw)
    cmd=target/'build.cmd'
    cmd.write_text('@echo off\ncall "'+str(args.vcvars)+'" >nul\nif errorlevel 1 exit /b 1\n'
      +f'cd /d "{native}"\n'
      +f'rc /nologo /fo splat_bakes.res splat_bakes.rc\nif errorlevel 1 exit /b 1\n'
      +f'cl /nologo /LD /O2 /MT /EHsc /std:c++17 /I"{target / "base/include"}" /I"../../third-party" /I"{args.directx_sdk / "Include"}" d3d9_proxy.cpp splat_bakes.res /link /DEF:d3d9_proxy.def /OUT:"{target / "d3d9.dll"}" /LIBPATH:"{args.directx_sdk / "Lib/x86"}" d3d9.lib d3dx9.lib d3d11.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib winmm.lib\n')
    compiler_temp=target/'compiler-temp';compiler_temp.mkdir()
    compiler_env=os.environ.copy();compiler_env.update(TEMP=str(compiler_temp),TMP=str(compiler_temp))
    subprocess.run(['cmd','/c',str(cmd)],check=True,env=compiler_env)
    record={'sourceCommit':head,'baseCommit':lock['commit'],'dirtySourceIncluded':bool(args.meridian_recipe),'unrelatedDirtySourceIncluded':False,
      'privateFactorySHA256':hashlib.sha256(helper).hexdigest(),
      'sandboxWorldOriginSHA256':hashlib.sha256((native/'sandbox_world_origin.hpp').read_bytes()).hexdigest(),
      'generatedRuntimeSourceSHA256':hashlib.sha256(cpp.read_bytes()).hexdigest(),
      'builderSHA256':hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest(),
      'runtimeSHA256':hashlib.sha256((target/'d3d9.dll').read_bytes()).hexdigest(),
      'stockMaterialManifestSHA256':hashlib.sha256((target/'source/tools/costumes/stock_materials.json').read_bytes()).hexdigest(),
      'installed':False,'observedGameplay':False,'meridianCandidate':(native/'meridian_adapter.h').exists(),'productionSourceIntegrated':(native/'meridian_adapter.h').exists() and not bool(args.meridian_recipe),'candidateInputSHA256':candidate_hashes}
    (target/'provenance.json').write_text(json.dumps(record,indent=2)+'\n')
    print(json.dumps(record))
if __name__=='__main__':main()
