"""Build an isolated runtime derivative of canonical HEAD and its exact Base pin.

No dirty garment prototype is incorporated and no installed file is replaced.
The only deliberate derivative is the guarded D3D9Ex factory header in this
folder. All source is exported from Git into owned build output with provenance.
"""
import argparse, hashlib, io, json, os, pathlib, subprocess, zipfile

def run(*args, cwd=None):
    return subprocess.check_output(args, cwd=cwd)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--output',type=pathlib.Path,required=True)
    ap.add_argument('--base',type=pathlib.Path,required=True)
    ap.add_argument('--source-ref',default='HEAD')
    ap.add_argument('--vcvars',type=pathlib.Path,default=pathlib.Path('C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat'))
    ap.add_argument('--directx-sdk',type=pathlib.Path,default=pathlib.Path('C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)'))
    args=ap.parse_args(); repo=pathlib.Path(__file__).resolve().parents[2]
    target=args.output.resolve()
    if target.exists():raise SystemExit('Refusing existing output; choose fresh owned path')
    target.mkdir(parents=True)
    head=run('git','rev-parse',args.source_ref+'^{commit}',cwd=repo).decode().strip()
    lock=json.loads(run('git','show',head+':dependencies/base.lock.json',cwd=repo))
    with zipfile.ZipFile(io.BytesIO(run('git','archive','--format=zip',head,'src/runtime','third-party',cwd=repo))) as z:z.extractall(target/'source')
    with zipfile.ZipFile(io.BytesIO(run('git','archive','--format=zip',lock['commit'],'include',cwd=args.base))) as z:z.extractall(target/'base')
    helper=pathlib.Path(__file__).with_name('sealed_d3d9.hpp').read_bytes()
    native=target/'source/src/runtime';(native/'sealed_d3d9.hpp').write_bytes(helper)
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
    cmd=target/'build.cmd'
    cmd.write_text('@echo off\ncall "'+str(args.vcvars)+'" >nul\nif errorlevel 1 exit /b 1\n'
      +f'cd /d "{native}"\n'
      +f'rc /nologo /fo splat_bakes.res splat_bakes.rc\nif errorlevel 1 exit /b 1\n'
      +f'cl /nologo /LD /O2 /MT /EHsc /std:c++17 /I"{target / "base/include"}" /I"../../third-party" /I"{args.directx_sdk / "Include"}" d3d9_proxy.cpp splat_bakes.res /link /DEF:d3d9_proxy.def /OUT:"{target / "d3d9.dll"}" /LIBPATH:"{args.directx_sdk / "Lib/x86"}" d3d9.lib d3dx9.lib d3d11.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib winmm.lib\n')
    compiler_temp=target/'compiler-temp';compiler_temp.mkdir()
    compiler_env=os.environ.copy();compiler_env.update(TEMP=str(compiler_temp),TMP=str(compiler_temp))
    subprocess.run(['cmd','/c',str(cmd)],check=True,env=compiler_env)
    record={'sourceCommit':head,'baseCommit':lock['commit'],'dirtySourceIncluded':False,
      'privateFactorySHA256':hashlib.sha256(helper).hexdigest(),
      'generatedRuntimeSourceSHA256':hashlib.sha256(cpp.read_bytes()).hexdigest(),
      'builderSHA256':hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest(),
      'runtimeSHA256':hashlib.sha256((target/'d3d9.dll').read_bytes()).hexdigest(),
      'installed':False,'observedGameplay':False}
    (target/'provenance.json').write_text(json.dumps(record,indent=2)+'\n')
    print(json.dumps(record))
if __name__=='__main__':main()
