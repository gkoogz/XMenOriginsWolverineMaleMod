"""Verify the repository-only developer recipe, independent of retail assets."""
import hashlib,json,pathlib,sys
root=pathlib.Path(__file__).resolve().parents[2]
manifest=json.loads((root/'dev/sandbox/delta-manifest.json').read_text(encoding='utf-8'))
for item in manifest['files']:
    p=root/item['path']
    if not p.is_file():raise SystemExit('Missing recipe source: '+item['path'])
    b=p.read_bytes().replace(b'\r\n',b'\n')
    if hashlib.sha256(b).hexdigest()!=item['lfSHA256']:raise SystemExit('Recipe source changed: '+item['path'])
print('PASS source-only grey recipe: '+str(len(manifest['files']))+' LF-normalized source files; no local save/binary dependency')
