"""Prepare a normal uninstalled pouch build from HEAD plus named source changes.

The working Base lock supplies an immutable local revision. Unrelated dirty
files and the sealed sandbox factory are deliberately excluded.
"""
import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import zipfile

OVERLAYS=['meridian_adapter.h','meridian_attempt.h','compliant_dynamics.h',
 'anterior_body_adapter.h','anterior_envelope_adapter.h','pouch_contact.h',
 'anatomy_surface.h','performance_metrics.h','d3d9_proxy.cpp',
 'meridian_material.h','fabric_material.inl']

def main():
    ap=argparse.ArgumentParser(__doc__)
    ap.add_argument('--base',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--stock-maps',type=Path,required=True,help='Hash-checked licensed DDS export directory')
    ap.add_argument('--vcvars',type=Path,default=Path('C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat'))
    ap.add_argument('--directx-sdk',type=Path,default=Path('C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)'))
    args=ap.parse_args();repo=Path(__file__).resolve().parents[2];root=args.output.resolve()
    root.mkdir(parents=True,exist_ok=False)
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo).decode().strip()
    lock=json.loads((repo/'dependencies/base.lock.json').read_text());pin=lock['commit']
    for name,where,ref,paths in [('source',repo,head,['src/runtime','third-party','tools/costumes']),('base',args.base,pin,['include'])]:
        data=subprocess.check_output(['git','archive','--format=zip',ref,*paths],cwd=where)
        with zipfile.ZipFile(io.BytesIO(data)) as archive:archive.extractall(root/name)
    hashes={}
    for name in OVERLAYS:
        data=(repo/'src/runtime'/name).read_bytes();(root/'source/src/runtime'/name).write_bytes(data)
        hashes[name]=hashlib.sha256(data).hexdigest()
    if b'MaleModPrivateFactory' in (root/'source/src/runtime/d3d9_proxy.cpp').read_bytes():
        raise SystemExit('Private factory must never enter the normal runtime')
    environment=dict(os.environ,MALEMOD_STOCK_MAPS=str(args.stock_maps.resolve()))
    subprocess.run([sys.executable,str(root/'source/tools/costumes/prepare_stock_materials.py'),'--runtime',str(root/'source/src/runtime')],check=True,env=environment)
    command=f'''@echo off
call "{args.vcvars}" >nul
if errorlevel 1 exit /b 1
cd /d "{root}/source/src/runtime"
rc /nologo /fo splat_bakes.res splat_bakes.rc
if errorlevel 1 exit /b 1
cl /nologo /LD /O2 /MT /EHsc /std:c++17 /I"{root}/base/include" /I"../../third-party" /I"{args.directx_sdk}/Include" d3d9_proxy.cpp splat_bakes.res /link /DEF:d3d9_proxy.def /OUT:"{root}/d3d9.dll" /LIBPATH:"{args.directx_sdk}/Lib/x86" d3d9.lib d3dx9.lib d3d11.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib winmm.lib
'''
    (root/'build.cmd').write_text(command)
    (root/'provenance-pending.json').write_text(json.dumps(dict(sourceCommit=head,baseCommit=pin,sourceOverlays=hashes,unrelatedDirtySourceIncluded=False,privateFactory=False,installed=False,builderSHA256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()),indent=2)+'\n')
    print(root/'build.cmd')

if __name__=='__main__':main()
