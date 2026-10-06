from pathlib import Path
import sys,struct,stat,re,shutil,hashlib,os,json
p=Path(os.environ['MALEMOD_SANDBOX_WORKSPACE']).resolve();name=sys.argv[1];directory=(p/sys.argv[2]).resolve();offline='--offline' in sys.argv[3:]
assert name.startswith('jungle1_') and re.fullmatch(r'[A-Za-z0-9_]+',name), 'Native layer name required'
allowed=(Path(os.environ['LOCALAPPDATA'])/'MaleMod/WolverineSandbox').resolve()
assert directory.is_relative_to(p) or (offline and directory.is_relative_to(allowed)), 'Output must stay inside owned workspace/storage'
plain=(directory/(name+'.xxx')).read_bytes();body=plain[109:];q,r=divmod(len(body)-18,255)
if not r:q-=1;r=255
z=bytes([0])+bytes(q)+bytes([r])+body+bytes([17,0,0]);chunk=struct.pack('<6I',0x9e2a83c1,131072,len(z),len(body),len(z),len(body))+z
h=bytearray(plain[:101]);struct.pack_into('<I',h,21,struct.unpack_from('<I',h,21)[0]|0x02000000);struct.pack_into('<2I',h,93,2,1);out=bytes(h)+struct.pack('<4I',109,len(body),125,len(chunk))+plain[101:109]+chunk;out+=bytes((-len(out))%32768);(directory/(name+'.cooked.xxx')).write_bytes(out)
if offline:
 receipt={'layer':name,'nativeBytes':len(out),'nativeSHA256':hashlib.sha256(out).hexdigest(),'installed':False,'originalGameModified':False}
 (directory/(name+'.packed.json')).write_text(json.dumps(receipt,indent=2));print(json.dumps(receipt));raise SystemExit(0)
target=p/'owned-game/WGame/CookedPC'/(name+'.xxx');backup=p/'native-stock-backups'/(name+'.xxx');backup.parent.mkdir(exist_ok=True)
if not backup.exists():shutil.copy2(target,backup)
assert 'owned-game' in target.parts, 'Private target required'
target.chmod(stat.S_IWRITE|stat.S_IREAD);shutil.copyfile(directory/(name+'.cooked.xxx'),target)
f=p/'owned-game/WGame/PCTOC.txt';f.chmod(stat.S_IWRITE|stat.S_IREAD);s=f.read_text();s=re.sub(r'(?m)^\d+( .*?'+re.escape(name)+r'\.xxx.*)$',str(len(out))+r'\1',s);f.write_text(s)
print('owned native package',name,len(out),hashlib.sha256(out).hexdigest())

receipt={'layer':name,'sourceNativeBytes':backup.stat().st_size,'sourceNativeSHA256':hashlib.sha256(backup.read_bytes()).hexdigest(),'nativeBytes':len(out),'nativeSHA256':hashlib.sha256(out).hexdigest(),'originalGameModified':False}
(directory/(name+'.packed.json')).write_text(json.dumps(receipt,indent=2))
