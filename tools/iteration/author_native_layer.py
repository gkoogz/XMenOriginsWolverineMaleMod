from pathlib import Path
"""Raven v568 native room candidate. Uses actual cooked floor/collision data.
Offline asset authoring only; output stays outside repositories and games.
"""
import runpy,struct,sys,uuid,json,hashlib,os
import argparse
ap=argparse.ArgumentParser();ap.add_argument("--name",required=True);ap.add_argument("--empty",action="store_true");ap.add_argument('--studio',action='store_true');ap.add_argument('--output-directory');ap.add_argument('--guid');args=ap.parse_args()
if not args.name.startswith("jungle1_") or any(c not in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_" for c in args.name):raise SystemExit("Unsupported native layer name")
P=Path(os.environ['MALEMOD_SANDBOX_WORKSPACE']).resolve()
SCRIPT=Path(__file__).resolve().parent

def load(name):
 old=sys.argv;sys.argv=['native_package',name]
 try:return runpy.run_path(str(SCRIPT/'native_package.py'))
 finally:sys.argv=old
base=load('rgame_p.xxx');source=load(str(P/'native-stream-room/jungle1_zone01.stock.xxx'))
assert hashlib.sha256(base['raw']).hexdigest()=='740cdba270ca73616b2d409dcdea2567280c9d56503b4ed348c3b82823adec23', 'Room binding source changed'
assert hashlib.sha256(source['raw']).hexdigest()=='a14b172a60ff96cfc86d77fbddb1c58c3f84973fc90aa12b2125452b0ba7e6f4', 'Floor binding source changed'
original=base['data'];names=list(base['names']);nameflags={n:0 for n in names}
def ni(n):
 if n not in names:names.append(n)
 return names.index(n)
def fn(n,number=0):return struct.pack('<2I',ni(n),number)
def tag(n,t,value=b'',extra=None):
 b=fn(n)+fn(t)+struct.pack('<2I',len(value),0)
 if t=='StructProperty':b+=fn(extra)
 elif t=='BoolProperty':b+=struct.pack('<I',extra)
 return b+value
NONE=fn('None')
def obj(n,value):return tag(n,'ObjectProperty',struct.pack('<i',value))
def vec(n,xyz):return tag(n,'StructProperty',struct.pack('<3f',*xyz),'Vector')
def rot(n,xyz):return tag(n,'StructProperty',struct.pack('<3i',*xyz),'Rotator')
def boo(n,x):return tag(n,'BoolProperty',extra=int(x))
def arr(n,values):return tag(n,'ArrayProperty',struct.pack('<I',len(values))+b''.join(struct.pack('<i',x) for x in values))
def string(n,v):return tag(n,'StrProperty',struct.pack('<I',len(v)+1)+v.encode()+b'\0')
def classref(n,package='Engine'):return addimp('Core','Class',-14 if package=='Engine' else -15,n)
# Preserve every existing import/export index; append additions only.
imps=[]
for i in range(len(base['imports'])):imps.append(bytes(original[base['u'](base['raw'],45)+i*28:base['u'](base['raw'],45)+(i+1)*28]))
def addimp(cp,cn,outer,n):
 b=fn(cp)+fn(cn)+struct.pack('<i',outer)+fn(n)
 if b not in imps:imps.append(b)
 return -imps.index(b)-1
meshcls=classref('StaticMesh');bodycls=classref('RB_BodySetup');actorcls=classref('StaticMeshActor');componentcls=classref('StaticMeshComponent');lightcls=classref('DirectionalLight');lightcomponentcls=classref('DirectionalLightComponent');checkpointcls=classref('RCheckpoint','RavenShared')
miccls=classref('MaterialInstanceConstant')
materialpkg=addimp('Core','Package',0,'EngineMaterials');material=addimp('Engine','Material',materialpkg,'GeomMaterial')
# Original export table descriptors, payloads and variable net-object lists.
exports=[];o=base['u'](base['raw'],37)
for name,cls,size,pos in base['exports']:
 count=base['u'](original,o+44);length=68+4*count
 exports.append([bytearray(original[o:o+length]),bytes(original[pos:pos+size])]);o+=length

def replace(i,payload):exports[i-1][1]=payload
# Root package identity changes; all positive references remain stable.
exports[3][0][12:20]=fn(args.name)
# Remove Raven stream-by-URL startup action. Keep the genuine empty level/model.
pr,end=base['props'](base['exports'][8][3],sum(base['exports'][8][2:4]));old=base['exports'][8];replace(9,struct.pack('<I',0)+arr('SequenceObjects',[])+NONE+original[end:old[3]+old[2]])
# Real Level TTransArray owner/count/native tail. Keep WorldInfo + PlayerStart.
nm,cl,sz,pos=base['exports'][1];pr,end=base['props'](pos,pos+sz)
owner,count=struct.unpack_from('<2i',original,end);assert owner==2 and count==4
actors=[11] if args.empty else [11,12,14];tail=original[end+8+count*4:pos+sz]
replace(2,original[pos:end]+struct.pack('<2i',owner,len(actors))+struct.pack('<'+str(len(actors))+'i',*actors)+tail)
# Stock PlayerStart state frame and properties, moved above actual floor.
nm,cl,sz,pos=base['exports'][4];pr,end=base['props'](pos+26,pos+sz)
payload=bytearray(original[pos:pos+sz]);key=fn('Location')+fn('StructProperty')
k=payload.index(key)+24+8;payload[k:k+12]=struct.pack('<3f',0,0,110);replace(5,bytes(payload))
# Native state frame observed in PlayerStart/WorldInfo: class/class/mask/latent/stack/IP.
def state(c):return struct.pack('<iiQHiiI',c,c,0xffffffffffffffff,0,0,-1,0)
def add(n,c,outer,payload,hasstack=False,archetype=0):
 flags=0x7000100000000|(0x200000000000000 if hasstack else 0)
 descriptor=struct.pack('<3i',c,0,outer)+fn(n)+struct.pack('<iQ2I2I',archetype,flags,len(payload),0,0,0)+bytes(16)+struct.pack('<I',0)
 assert len(descriptor)==68
 exports.append([bytearray(descriptor),payload]);return len(exports)
assert add('SandboxFloor',actorcls,2,state(actorcls)+arr('Components',[13])+obj('StaticMeshComponent',13)+obj('CollisionComponent',13)+vec('Location',(0,0,2000.0))+rot('Rotation',(0,0,0))+vec('DrawScale3D',(1000,1000,.1))+boo('bWorldGeometry',True)+boo('bBlockActors',True)+NONE,True)==12
assert add('StaticMeshComponent0',componentcls,12,struct.pack('<2i',0,0)+obj('StaticMesh',18)+arr('Materials',[20])+boo('BlockActors',True)+boo('BlockZeroExtent',True)+boo('BlockNonZeroExtent',True)+boo('BlockRigidBody',True)+boo('HiddenGame',False)+NONE+struct.pack('<I',0))==13
assert add('SandboxLight',lightcls,2,state(lightcls)+arr('Components',[15])+obj('LightComponent',15)+rot('Rotation',(-8192,-8192,0))+NONE,True)==14
channels=boo('Static',True)+boo('Dynamic',True)+boo('CompositeDynamic',True)+NONE
shadowflags=(boo('CastShadows',False)+boo('CastStaticShadows',False)+boo('CastDynamicShadows',False)+boo('bCastCompositeShadow',False)) if args.studio else b''
assert add('DirectionalLightComponent0',lightcomponentcls,14,struct.pack('<2i',0,0)+tag('Brightness','FloatProperty',struct.pack('<f',.12))+tag('LightColor','StructProperty',bytes([225,225,225,255]),'Color')+tag('LightingChannels','StructProperty',channels,'LightingChannelContainer')+boo('bOnlyAffectSameAndSpecifiedLevels',False)+NONE+struct.pack('<2i',0,0))==15
assert add('RCheckpoint_2',checkpointcls,2,state(checkpointcls)+string('PersistentLevelName','MaleModSandbox_p')+string('MapName','MaleModSandbox_p')+vec('Location',(0,0,110))+rot('Rotation',(0,0,0))+obj('CylinderComponent',17)+obj('CollisionComponent',17)+arr('Overrides',[])+NONE,True)==16
nm,cl,sz,pos=base['exports'][0];assert add('CylinderComponent0',cl,16,bytes(original[pos:pos+sz]))==17
# Remap nested property names from the native floor's package. Payload arrays of
# vectors/indices stay byte-identical; nested tagged convex elements are walked.
snames=source['names'];sb=source['data']
def sfn(buf,o):
 a,num=struct.unpack_from('<2I',buf,o);return snames[a],num

def walk(buf,o=0):
 out=bytearray()
 while True:
  n,num=sfn(buf,o);o+=8;out+=fn(n,num)
  if n=='None':return bytes(out),o
  t,tn=sfn(buf,o);size,index=struct.unpack_from('<2I',buf,o+8);o+=16;extra=b''
  if t=='StructProperty':st,sn=sfn(buf,o);extra=fn(st,sn);o+=8
  if t=='BoolProperty':extra=buf[o:o+4];o+=4
  value=buf[o:o+size];o+=size
  if t=='NameProperty':nn,ns=sfn(value,0);value=fn(nn,ns)
  elif t=='StructProperty':
   if st not in ['Vector','Rotator','Guid','Box','LinearColor','Color','Matrix','Quat','Plane']:
    value,used=walk(value);assert used==size
  elif t=='ArrayProperty' and n=='ConvexElems':
   count=struct.unpack_from('<I',value,0)[0];vv=bytearray(value[:4]);q=4
   for _ in range(count):part,used=walk(value,q);vv+=part;q=used
   assert q==len(value);value=bytes(vv)
  out+=fn(t,tn)+struct.pack('<2I',len(value),index)+extra+value
# Real native mesh data: bounds, kDOP triangles, vertex/index buffers and section.
nm,cl,sz,pos=source['exports'][2476];properties,pe=source['props'](pos,pos+sz);native=bytearray(sb[pe:pos+sz]);struct.pack_into('<i',native,28,19)
o=32
for _ in range(2):es,n=struct.unpack_from('<2I',native,o);o+=8+es*n
# Flatten actual native position stream while retaining its indices/XY topology.
# Every top/bottom vertex is assigned one transverse plane; the kDOP remains conservative.
assert struct.unpack_from('<4I',native,1312)==(12,40,12,40)
for vi in range(40):
 vo=1328+12*vi;z=struct.unpack_from('<f',native,vo+8)[0];struct.pack_into('<f',native,vo+8,18.1782 if z>1 else 0)
assert struct.unpack_from('<6I',native,1808)==(2,20,40,0,20,40)
for vi in range(40):
 z=struct.unpack_from('<f',native,1328+12*vi+8)[0]
 native[1832+20*vi:1832+20*vi+4]=bytes([255,128,128,127])
 native[1832+20*vi+4:1832+20*vi+8]=bytes([128,128,255 if z>1 else 0,255])
for node in range(25):
 struct.pack_into('<f',native,40+32*node+8,0);struct.pack_into('<f',native,40+32*node+20,18.1782)
struct.pack_into('<f',native,8,9.0891);struct.pack_into('<f',native,20,9.0891)
version=struct.unpack_from('<I',native,o)[0];o+=4;assert version==18
count=struct.unpack_from('<I',native,o)[0];assert count==0;o+=4
lodcount=struct.unpack_from('<I',native,o)[0];assert lodcount==1;o+=4
flags,elems,bulksize,oldoffset=struct.unpack_from('<4i',native,o);assert elems==0 and bulksize==0
bulkoffset=o+12;o+=16
sectioncount=struct.unpack_from('<I',native,o)[0];assert sectioncount==1;o+=4
oldmaterial=struct.unpack_from('<i',native,o)[0];assert oldmaterial==847;struct.pack_into('<i',native,o,20)
meshpayload=struct.pack('<I',0)+obj('BodySetup',19)+tag('LightMapCoordinateIndex','IntProperty',struct.pack('<i',1))+tag('LightMapResolution','IntProperty',struct.pack('<i',16))+NONE+native
assert add('SandboxFloorMesh',meshcls,4,meshpayload)==18
nm,cl,sz,pos=source['exports'][2121];properties,pe=source['props'](pos,pos+sz);bodyprops,used=walk(sb,pos+4);assert used==pe
assert add('SandboxFloorCollision',bodycls,18,struct.pack('<I',0)+bodyprops+bytes(sb[pe:pos+sz]))==19
# Actual GeomMaterial exposes VectorParameter Color. Native MIC without static permutation has no tail.
vp=tag('ParameterName','NameProperty',fn('Color'))+tag('ParameterValue','StructProperty',struct.pack('<4f',.32,.32,.32,1),'LinearColor')+tag('ExpressionGUID','StructProperty',bytes(16),'Guid')+NONE
assert add('SandboxGreyMaterial',miccls,4,struct.pack('<I',0)+obj('Parent',material)+tag('VectorParameterValues','ArrayProperty',struct.pack('<I',1)+vp)+NONE)==20
if args.studio and not args.empty:
 # All names/defaults below are observed in this Raven Engine package, not
 # generic UE3 assumptions. SkyLightComponent has the same eight-byte native
 # LightComponent tail and provides supported upper/lower hemisphere fill.
 engine=load('Engine.xxx')
 required={'SkyLight','SkyLightComponent','Default__SkyLightComponent','LowerBrightness','LowerColor','CastShadows','CastStaticShadows','CastDynamicShadows','bCastCompositeShadow','bCanAffectDynamicPrimitivesOutsideDynamicChannel'}
 assert required.issubset(set(engine['names'])), 'Raven studio light contract absent'
 skycls=classref('SkyLight');skycomponentcls=classref('SkyLightComponent')
 skyactor=add('SandboxSoftFill',skycls,2,state(skycls)+arr('Components',[22])+obj('LightComponent',22)+NONE,True)
 assert skyactor==21
 skycomponent=add('SkyLightComponent0',skycomponentcls,21,struct.pack('<2i',0,0)+tag('Brightness','FloatProperty',struct.pack('<f',.35))+tag('LowerBrightness','FloatProperty',struct.pack('<f',.35))+tag('LightColor','StructProperty',bytes([235,235,235,255]),'Color')+tag('LowerColor','StructProperty',bytes([235,235,235,255]),'Color')+tag('LightingChannels','StructProperty',channels,'LightingChannelContainer')+boo('bCanAffectDynamicPrimitivesOutsideDynamicChannel',True)+boo('bOnlyAffectSameAndSpecifiedLevels',False)+shadowflags+NONE+struct.pack('<2i',0,0))
 assert skycomponent==22
 actors.append(skyactor)
 # Five cheap grey panels enclose the horizon. Reuse the observed native
 # forty-vertex floor mesh. Backdrop panels never enter pawn/rigid collision.
 panels=[('West',(-18000,0,11000),(-16384,0,0)),('East',(18000,0,11000),(16384,0,0)),('South',(0,-18000,11000),(-16384,16384,0)),('North',(0,18000,11000),(16384,16384,0)),('Ceiling',(0,0,22000),(32768,0,0))]
 for label,location,rotation in panels:
  actorindex=len(exports)+1;componentindex=actorindex+1
  add('SandboxBackdrop'+label,actorcls,2,state(actorcls)+arr('Components',[componentindex])+obj('StaticMeshComponent',componentindex)+vec('Location',location)+rot('Rotation',rotation)+vec('DrawScale3D',(1000,1000,.1))+boo('bWorldGeometry',False)+boo('bBlockActors',False)+NONE,True)
  add('StaticMeshComponent'+label,componentcls,actorindex,struct.pack('<2i',0,0)+obj('StaticMesh',18)+arr('Materials',[20])+boo('BlockActors',False)+boo('BlockZeroExtent',False)+boo('BlockNonZeroExtent',False)+boo('BlockRigidBody',False)+boo('CastShadow',False)+boo('bCastDynamicShadow',False)+boo('HiddenGame',False)+NONE+struct.pack('<I',0))
  actors.append(actorindex)
 levelname,levelcls,levelsize,levelpos=base['exports'][1]
 levelprops,levelend=base['props'](levelpos,levelpos+levelsize)
 replace(2,original[levelpos:levelend]+struct.pack('<2i',owner,len(actors))+struct.pack('<'+str(len(actors))+'i',*actors)+tail)
# Plain native package with original version/licensee, fresh GUID and tables.
header=bytearray(base['raw'][:101]+base['raw'][117:125]);assert len(header)==109
struct.pack_into('<I',header,21,base['u'](header,21)&~0x02000000)
struct.pack_into('<2I',header,93,0,0)
# Freeze the accepted studio layer's package identity for byte-identical
# regeneration. A future scene revision should use an explicit new GUID.
packageguid=uuid.UUID(args.guid) if args.guid else uuid.UUID('c558ac4e-fc9a-4380-8c26-66202506cc13') if args.studio and args.name=='jungle1_zone01a' else uuid.uuid5(uuid.NAMESPACE_URL,'https://github.com/gkoogz/XMenOriginsWolverineMaleMod/grey-studio/v1/'+args.name)
header[53:69]=packageguid.bytes
nameblob=b''.join(struct.pack('<I',len(n)+1)+n.encode()+b'\0'+struct.pack('<Q',nameflags.get(n,0)) for n in names)
nameoff=len(header);impoff=nameoff+len(nameblob);expoff=impoff+28*len(imps);depoff=expoff+sum(len(d) for d,b in exports);start=depoff+4*len(exports)
struct.pack_into('<7I',header,25,len(names),nameoff,len(exports),expoff,len(imps),impoff,depoff)
struct.pack_into('<I',header,8,start);struct.pack_into('<2I',header,73,len(exports),len(names))
content=bytearray();serial=start
for i,(desc,payload) in enumerate(exports):
 if i==17:
  payload=bytearray(payload);nativeStart=len(payload)-len(native);struct.pack_into('<I',payload,nativeStart+bulkoffset,serial+nativeStart+bulkoffset+4);payload=bytes(payload)
 struct.pack_into('<2I',desc,32,len(payload),serial);content+=payload;serial+=len(payload)
output=bytes(header)+nameblob+b''.join(imps)+b''.join(bytes(d) for d,b in exports)+bytes(4*len(exports))+content
outdir=Path(args.output_directory).resolve() if args.output_directory else P/'authored-native-layers'
allowed=Path(os.environ['LOCALAPPDATA'])/'MaleMod/WolverineSandbox'
assert outdir.is_relative_to(P) or outdir.is_relative_to(allowed.resolve()), 'Output must stay in owned sandbox storage'
out=outdir/(args.name+'.xxx');out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(output)
receipt={'nativeVersion':568,'licensee':101,'sourceRoom': 'rgame_p.xxx','sourceFloor':'jungle1_zone01.xxx:floor_stone_03_jun','floorActualTriangles':48,'sourceConvexCollisionPreserved':True,'actorRefs':actors,'actors':['WorldInfo'] if args.empty else ['WorldInfo','StaticMeshActor','DirectionalLight'],'exports':len(exports),'nativeBytes':len(output),'sha256':hashlib.sha256(output).hexdigest(),'sourceHashes':{'rgame_p.xxx':hashlib.sha256(base['raw']).hexdigest(),'jungle1_zone01.xxx':hashlib.sha256(source['raw']).hexdigest()},'installed':False,'observedGameplay':False,'stockMapActorBootstrap':True,'componentsRegistered':True,'topPlaneNativeZ':18.1782,'floorWorldLocation':[0,0,2000],'floorActorScale':[1000,1000,.1],'lightBrightness':.12,'flattenedNativePositionStream':True,'collisionContract':'original convex cache plus conservative kDOP with flattened mesh triangles'}
receipt.update({'studio':args.studio,'packageGUID':str(packageguid),'softSkyFill':.35 if args.studio else None,'greyBackdropPanels':5 if args.studio else 0,'directionalShadows':True,'fillShadows':False if args.studio else None,'engineLightSourceSHA256':hashlib.sha256(engine['raw']).hexdigest() if args.studio and not args.empty else None})
if args.studio and not args.empty:receipt['actors']=['WorldInfo','StaticMeshActor','DirectionalLight','SkyLight']+['StaticMeshActorBackdrop']*5
(out.parent/(args.name+'.authoring.json')).write_text(json.dumps(receipt,indent=2));print(json.dumps(receipt))
