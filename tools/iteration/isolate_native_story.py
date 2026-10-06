from pathlib import Path
import runpy,sys,struct,shutil,stat,re,json,hashlib,os
p=Path(os.environ['MALEMOD_SANDBOX_WORKSPACE']).resolve();script=Path(__file__).resolve().parent;name='jungle1_kismet01';target=p/'owned-game/WGame/CookedPC'/(name+'.xxx');backup=p/'native-stream-room'/(name+'.stock.xxx')
if not backup.exists():shutil.copy2(target,backup)
sys.argv=['p',str(backup)];m=runpy.run_path(str(script/'native_package.py'));data=bytearray(m['data']);raw=m['raw'];assert hashlib.sha256(raw).hexdigest()=='8e46ec738c37ca6c9ff0ad25bb3037c0d0684e2607260e69b9467c9230e099ea', 'Checkpoint carrier binding source changed';names=m['names'];exp=m['exports'];disabled=0;actorReceipt=None
key=struct.pack('<2I',names.index('bDisabled'),0)+struct.pack('<2I',names.index('BoolProperty'),0)+bytes(8)
for i,e in enumerate(exp):
 cls=m['imports'][-e[1]-1] if e[1]<0 else str(e[1]);payload=bytearray(data[e[3]:e[3]+e[2]])
 if 'SeqAct' in cls:
  at=0
  while True:
   k=payload.find(key,at)
   if k<0:break
   struct.pack_into('<I',payload,k+24,1);disabled+=1;at=k+28
  data[e[3]:e[3]+e[2]]=payload
 elif cls=='Level':
  pr,end=m['props'](e[3],e[3]+e[2]);owner,count=struct.unpack_from('<2i',data,end);refs=struct.unpack_from('<'+str(count)+'i',data,end+8);keep=[]
  for ref in refs:
   if not ref:continue
   ae=exp[ref-1];ac=m['imports'][-ae[1]-1] if ae[1]<0 else str(ae[1])
   if ac in ['WorldInfo','RCheckpoint']:keep.append(ref)
  replacement=bytes(data[e[3]:end])+struct.pack('<2i',owner,len(keep))+struct.pack('<'+str(len(keep))+'i',*keep)+bytes(data[end+8+count*4:e[3]+e[2]])
  data[e[3]:e[3]+len(replacement)]=replacement;o=m['u'](raw,37)
  for j in range(i):o+=68+4*m['u'](data,o+44)
  struct.pack_into('<I',data,o+32,len(replacement));actorReceipt={'before':count,'after':len(keep),'keptClasses':['WorldInfo','RCheckpoint']}
uc=m['chunks'][0][0];body=bytes(data[uc:]);bs=131072;parts=[]
for b in [body[i:i+bs] for i in range(0,len(body),bs)]:
 q,r=divmod(len(b)-18,255)
 if not r:q-=1;r=255
 parts.append((bytes([0])+bytes(q)+bytes([r])+b+bytes([17,0,0]),len(b)))
chunk=struct.pack('<4I',0x9e2a83c1,bs,sum(len(z) for z,n in parts),len(body))+b''.join(struct.pack('<2I',len(z),n) for z,n in parts)+b''.join(z for z,n in parts);h=bytearray(raw[:101]);struct.pack_into('<2I',h,93,2,1);cc=max(uc,125);h+=struct.pack('<4I',uc,len(body),cc,len(chunk))+raw[101+16*m['u'](raw,97):109+16*m['u'](raw,97)];h+=bytes(cc-len(h));out=bytes(h)+chunk;out+=bytes((-len(out))%32768);(p/'native-stream-room'/(name+'.quiet.xxx')).write_bytes(out);target.chmod(stat.S_IWRITE|stat.S_IREAD);shutil.copyfile(p/'native-stream-room'/(name+'.quiet.xxx'),target)
f=p/'owned-game/WGame/PCTOC.txt';f.chmod(stat.S_IWRITE|stat.S_IREAD);s=f.read_text();s=re.sub(r'(?m)^\d+( .*?'+re.escape(name)+r'\.xxx.*)$',str(len(out))+r'\1',s);f.write_text(s)
r={'disabledStoryInputLinks':disabled,'actors':actorReceipt,'size':len(out),'sourceSHA256':hashlib.sha256(raw).hexdigest(),'outputSHA256':hashlib.sha256(out).hexdigest()};(p/'native-stream-room/kismet-quiet-receipt.json').write_text(json.dumps(r,indent=2));print(json.dumps(r))
