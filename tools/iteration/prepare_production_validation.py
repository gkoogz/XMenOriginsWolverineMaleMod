"""Add only the sealed driver to an already identified production source tree."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

def main():
    ap=argparse.ArgumentParser(__doc__)
    ap.add_argument('--production',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();src=a.production.resolve();out=a.output.resolve()
    proof=json.loads((src/'provenance.json').read_text())
    assert hashlib.sha256((src/'d3d9.dll').read_bytes()).hexdigest()==proof['runtimeSHA256']
    out.mkdir(parents=True,exist_ok=False)
    for name in ['source','base']:shutil.copytree(src/name,out/name)
    native=out/'source/src/runtime'
    for name in ['sealed_d3d9.hpp','sandbox_world_origin.hpp']:
        shutil.copyfile(Path(__file__).with_name(name),native/name)
    cpp=native/'d3d9_proxy.cpp';s=cpp.read_text(encoding="utf-8")
    changes=[
      ('static bool menuOpen=true, shapeDirty=true;','static bool menuOpen=false, shapeDirty=true;'),
      ('static HRESULT STDMETHODCALLTYPE HookDIP(','static HRESULT STDMETHODCALLTYPE MaleModPrivateSourceDIP('),
      ('static HRESULT STDMETHODCALLTYPE HookCreateDevice(','#include "sealed_d3d9.hpp"\nstatic HRESULT STDMETHODCALLTYPE HookCreateDevice('),
      ('static HRESULT STDMETHODCALLTYPE HookPresent(','static void MaleModPrivateCapture(IDirect3DDevice9*,bool);\nstatic HRESULT MaleModPrivateMeasuredPresent(IDirect3DDevice9*,const RECT*,const RECT*,HWND,const RGNDATA*);\nstatic HRESULT MaleModPrivatePresentResult(IDirect3DDevice9*,HRESULT);\nstatic HRESULT STDMETHODCALLTYPE HookPresent('),
      ('menuTankDrawnThisFrame=false;frameRendered=false;\n  insidePresent=true;','MaleModPrivateCapture(d,menuTankDrawnThisFrame);menuTankDrawnThisFrame=false;frameRendered=false;\n  insidePresent=true;'),
      ('insidePresent=true;HRESULT result=origPresent(d,src,dst,wnd,dirty);insidePresent=false;return result;','insidePresent=true;HRESULT result=MaleModPrivateMeasuredPresent(d,src,dst,wnd,dirty);insidePresent=false;return MaleModPrivatePresentResult(d,result);'),
      ('if(SUCCEEDED(hr)&&out&&*out){LoadSettings();','if(SUCCEEDED(hr)&&out&&*out){MaleModPrivatePoolTranslation(*out);MaleModPrivateActivate(wnd?wnd:pp->hDeviceWindow);LoadSettings();'),
      ('IDirect3D9* WINAPI Direct3DCreate9(UINT sdk){LoadReal();IDirect3D9* d=realCreate9(sdk);','IDirect3D9* WINAPI Direct3DCreate9(UINT sdk){LoadReal();IDirect3D9* d=MaleModPrivateFactory(sdk,realDll,realCreate9);')]
    for before,after in changes:
        if s.count(before)!=1:raise ValueError('Driver insertion contract changed: '+before)
        s=s.replace(before,after,1)
    cpp.write_text(s,encoding="utf-8")
    (out/'build.cmd').write_text((src/'build.cmd').read_text().replace(str(src),str(out)))
    proof={'production':proof,'sourceCommit':proof['sourceCommit'],'baseCommit':proof['baseCommit'],
      'meridianCandidate':True,'productionSourceIntegrated':True,'driverOnlyChanges':changes,
      'generatedRuntimeSourceSHA256':hashlib.sha256(cpp.read_bytes()).hexdigest(),
      'driverSHA256':hashlib.sha256((native/'sealed_d3d9.hpp').read_bytes()).hexdigest(),
      'installed':False,'nativeGameplayObserved':False}
    (out/'provenance-pending.json').write_text(json.dumps(proof,indent=2))
    print(out/'build.cmd')

if __name__=='__main__':main()
