import ctypes,struct,pathlib,json,os
p=pathlib.Path(os.environ['MALEMOD_SANDBOX_WORKSPACE']).resolve()
import sys
source_path=pathlib.Path(sys.argv[1]); source_path=source_path if source_path.is_absolute() else p/'owned-game/WGame/CookedPC'/source_path
raw=source_path.read_bytes()
assert struct.unpack_from('<I',raw,0)[0]==0x9e2a83c1 and struct.unpack_from('<2H',raw,4)==(568,101), 'Unsupported Raven native package version'
u=lambda b,o:struct.unpack_from('<I',b,o)[0]
chunks=[struct.unpack_from('<4I',raw,101+16*i) for i in range(u(raw,97))]
data=bytearray(raw[:chunks[0][0]])
dll=ctypes.CDLL(str(p/'lzo_read.dll')); fn=dll.lzo1x_decompress_safe
fn.argtypes=[ctypes.c_void_p,ctypes.c_size_t,ctypes.c_void_p,ctypes.POINTER(ctypes.c_size_t),ctypes.c_void_p]
for uc,us,cc,cs in chunks:
 magic,bs,totalc,totalu=struct.unpack_from('<4I',raw,cc)
 assert magic==0x9e2a83c1 and totalu==us
 blocks=(totalu+bs-1)//bs
 desc=[struct.unpack_from('<2I',raw,cc+16+8*i) for i in range(blocks)]
 off=cc+16+8*blocks;chunk=bytearray()
 for c,n in desc:
  src=ctypes.create_string_buffer(raw[off:off+c]);out=ctypes.create_string_buffer(n);size=ctypes.c_size_t(n)
  assert fn(src,c,out,ctypes.byref(size),None)==0 and size.value==n
  chunk.extend(out.raw);off+=c
 assert len(chunk)==us
 if len(data)<uc+us:data.extend(bytes(uc+us-len(data)))
 data[uc:uc+us]=chunk
names=[];o=u(raw,29)
for i in range(u(raw,25)):
 n=struct.unpack_from('<i',data,o)[0];o+=4
 assert n>0
 names.append(bytes(data[o:o+n-1]).decode());o+=n+8
def name(o):
 idx,num=struct.unpack_from('<2I',data,o);return names[idx]+(('_'+str(num-1)) if num else '')
imports=[];o=u(raw,45)
for i in range(u(raw,41)):
 imports.append(name(o+20));o+=28
o=u(raw,37);exports=[]
for i in range(u(raw,33)):
 cls,sup,outer=struct.unpack_from('<3i',data,o)
 nm=name(o+12);size,pos=struct.unpack_from('<2I',data,o+32)
 count=u(data,o+44);end=o+48+4*count+16+4
 exports.append((nm,cls,size,pos));o=end
def props(o,end):
 ans=[];o+=4
 while o+8<=end:
  nm=name(o);o+=8
  if nm=='None':return ans,o
  typ=name(o);sz,idx=struct.unpack_from('<2I',data,o+8);o+=16
  extra=None
  if typ=='StructProperty':extra=name(o);o+=8
  if typ=='BoolProperty':extra=u(data,o);o+=4
  val=bytes(data[o:o+sz]);ans.append({'name':nm,'type':typ,'extra':extra,'size':sz,'value':val.hex()});o+=sz
 raise ValueError((o,end))
if __name__=='__main__':
 from collections import Counter
 print('CLASSES',Counter(imports[-cls-1] if cls<0 else str(cls) for nm,cls,size,pos in exports))
 for i,(nm,cls,size,pos) in enumerate(exports):
  cl=imports[-cls-1] if cls<0 else str(cls)
  if cl not in ['Level','StaticMeshActor','StaticMeshComponent','DirectionalLight','DirectionalLightComponent','SkyLight','SkyLightComponent','PlayerStart','RCheckpoint','WCheckpoint','WGameCheckpoint']:continue
  if cl=='StaticMeshActor' and i>2700:continue
  pr=None
  for delta in [0,4,26]+list(range(1,60)):
   try:
    candidate,end=props(pos+delta,pos+size)
    if all(x['type'].endswith('Property') for x in candidate):pr=candidate;break
   except Exception:pass
  print(i+1,nm,cl,'prefix',delta,'size',size,'native',pos+size-end if pr is not None else '?',pr)
