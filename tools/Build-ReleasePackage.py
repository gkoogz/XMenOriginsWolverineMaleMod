"""Build curated release assets; developer rooms stay available only in Git."""
import argparse,hashlib,io,json,pathlib,shutil,subprocess,zipfile

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--output',type=pathlib.Path,required=True);args=ap.parse_args()
    repo=pathlib.Path(__file__).resolve().parents[1];m=json.loads((repo/'manifest.json').read_text())
    out=args.output.resolve();out.mkdir(parents=True,exist_ok=True);prefix='WolverineAnatomyTool-v'+m['version']
    stage=out/prefix
    if stage.exists():raise SystemExit('Choose fresh output; retained staging is never overwritten')
    stage.mkdir()
    for name in ['README.md','Install.cmd','Upgrade-Beta-2.0.cmd','Rollback-Beta-2.0.cmd','Uninstall.cmd','Install.ps1','manifest.json']:
        shutil.copy2(repo/name,stage/name)
    (stage/'tools').mkdir();(stage/'releases').mkdir()
    for name in ['PatchCodec.cs','Test-ReleaseBeta20.ps1','Assert-ReleaseExclusions.ps1','Get-ReleasePayload.ps1']:
        shutil.copy2(repo/'tools'/name,stage/'tools'/name)
    shutil.copy2(repo/'releases'/(m['version']+'.md'),stage/'releases'/(m['version']+'.md'))
    for name,h in m['payload'].items():
        p=repo/'payload'/name
        if digest(p)!=h.lower():raise SystemExit('Payload hash mismatch: '+name)
        d=stage/'payload'/name;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,d)
    for name,h in m['idleClips'].items():
        p=repo/'payload/WolverineIdle'/name
        if digest(p)!=h.lower():raise SystemExit('Default audio hash mismatch: '+name)
        d=stage/'payload/WolverineIdle'/name;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,d)
    checksum=''.join(digest(p)+'  '+p.relative_to(stage).as_posix()+'\n' for p in sorted(stage.rglob('*')) if p.is_file())
    (stage/'SHA256SUMS.txt').write_text(checksum)
    subprocess.run(['powershell','-NoProfile','-ExecutionPolicy','Bypass','-File',str(repo/'tools/Assert-ReleaseExclusions.ps1'),'-StagingDirectory',str(stage)],check=True)
    seven=shutil.which('7z') or 'C:/Program Files/7-Zip/7z.exe'
    install=out/(prefix+'-Install.7z');subprocess.run([seven,'a','-t7z','-mx=5',str(install),prefix],cwd=out,check=True)
    source=out/(prefix+'-Source.zip');ref=m['runtimeSourceCommit']
    raw=subprocess.check_output(['git','archive','--format=zip',ref],cwd=repo)
    blocked=('tools/iteration/','dev/sandbox/','docs/ITERATION-SANDBOX.md')
    with zipfile.ZipFile(io.BytesIO(raw)) as original,zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as archive:
        overlays=['README.md','manifest.json','Install.ps1','releases/'+m['version']+'.md']
        for item in original.infolist():
            if any(item.filename.startswith(p) for p in blocked) or item.filename in overlays or item.is_dir():continue
            archive.writestr(prefix+'-Source/'+item.filename,original.read(item))
        for name in overlays:archive.writestr(prefix+'-Source/'+name,(repo/name).read_bytes())
        archive.writestr(prefix+'-Source/RELEASE-SOURCE.txt','Runtime source '+ref+'\nBase '+m['runtimeBaseCommit']+'\nThis archive is the approved release implementation, not the unfinished development HEAD.\nNo sandbox, private capture/profile or game asset is included.\n')
    checks=out/'SHA256SUMS.txt';checks.write_text(''.join(digest(p)+'  '+p.name+'\n' for p in (install,source)))
    proof={'version':m['version'],'runtimeSourceCommit':ref,'runtimeBaseCommit':m['runtimeBaseCommit'],'runtimeSHA256':m['runtimeSHA256'],'assets':[{'path':str(p),'bytes':p.stat().st_size,'sha256':digest(p)} for p in (install,source,checks)],'sandboxExcluded':True,'gameLaunched':False}
    (out/'package-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(proof))
if __name__=='__main__':main()
