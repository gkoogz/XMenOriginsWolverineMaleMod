#include <malemod/garments/meridian_material.hpp>
namespace FABRIC_MATERIAL_NAMESPACE {
#ifdef FABRIC_STOCK_MAPS
#include "stock_material_maps.inl"
#endif
static IDirect3DVertexShader9* vs=nullptr;
static IDirect3DPixelShader9* ps=nullptr;
static IDirect3DPixelShader9* variants[16]{};
static float light[4]{},ambient[4]{},direction[4]{},position[4]{},spotDirection[4]{},spotAngles[4]{},screen[4]{},depth[4]{};
static float flags[4]{}; // directional, SH, local light, spot light
static bool valid=false,additive=false;
static unsigned pass=0;
static LONG frame=-3;
static float incident[28]{};static IDirect3DBaseTexture9* basis[2]{},*attenuation=nullptr;
struct Registers {IUnknown* shader;UINT light,ambient,direction,incident,basis;bool vertex;UINT position=~0u,localLight=~0u,spotDirection=~0u,spotAngles=~0u,screen=~0u,attenuation=~0u,depth=~0u;};
static std::vector<Registers> registers;
static void Release(){
#ifdef FABRIC_STOCK_MAPS
ReleaseStockMaps();
#endif
if(vs)vs->Release();for(auto& p:variants){if(p)p->Release();p=nullptr;}vs=nullptr;ps=nullptr;for(auto& r:registers)r.shader->Release();registers.clear();for(auto& t:basis){if(t)t->Release();t=nullptr;}if(attenuation)attenuation->Release();attenuation=nullptr;valid=false;frame=-3;}
template<class T> static Registers Reflect(T* shader,bool vertex){
 for(auto r:registers)if(r.shader==shader)return r;
 Registers r{shader,~0u,~0u,~0u,~0u,~0u,vertex};UINT bytes=0;
 if(shader&&SUCCEEDED(shader->GetFunction(nullptr,&bytes))){std::vector<DWORD> code((bytes+3)/4);if(SUCCEEDED(shader->GetFunction(code.data(),&bytes))){
 char trace[8]{};if(GetEnvironmentVariableA("MALEMOD_MERIDIAN_LIGHT_TRACE",trace,8)==1&&trace[0]=='1'){
  ID3DXBuffer* assembly=nullptr;if(SUCCEEDED(D3DXDisassembleShader(code.data(),FALSE,nullptr,&assembly))){char name[80]{},path[MAX_PATH]{};sprintf_s(name,"FabricShader-%u-%s.txt",unsigned(registers.size()),vertex?"vs":"ps");SiblingPath(path,name);FILE* file=nullptr;if(!fopen_s(&file,path,"wb")&&file){fwrite(assembly->GetBufferPointer(),1,assembly->GetBufferSize(),file);fclose(file);}assembly->Release();}
 }
 ID3DXConstantTable* table=nullptr;if(SUCCEEDED(D3DXGetShaderConstantTable(code.data(),&table))){auto get=[&](const char* name){D3DXCONSTANT_DESC desc{};UINT n=1;auto handle=table->GetConstantByName(nullptr,name);return handle&&SUCCEEDED(table->GetConstantDesc(handle,&desc,&n))&&desc.RegisterSet==D3DXRS_FLOAT4?desc.RegisterIndex:~0u;};r.light=get("LightColor");r.ambient=get("AmbientColorAndSkyFactor");r.direction=get("LightDirection");r.incident=get("WorldIncidentLighting");D3DXCONSTANT_DESC desc{};UINT n=1;auto handle=table->GetConstantByName(nullptr,"SHBasisCubeTextures");if(handle&&SUCCEEDED(table->GetConstantDesc(handle,&desc,&n))&&desc.RegisterSet==D3DXRS_SAMPLER&&desc.RegisterCount==2)r.basis=desc.RegisterIndex;r.depth=get("MinZ_MaxZRatio");r.position=get("LightPositionAndInvRadius");r.localLight=get("LightColorAndFalloffExponent");r.spotDirection=get("SpotDirection");r.spotAngles=get("SpotAngles");r.screen=get("ScreenPositionScaleBias");
handle=table->GetConstantByName(nullptr,"LightAttenuationTexture");n=1;if(handle&&SUCCEEDED(table->GetConstantDesc(handle,&desc,&n))&&desc.RegisterSet==D3DXRS_SAMPLER)r.attenuation=desc.RegisterIndex;
table->Release();}}}
 if(shader&&registers.size()<64){shader->AddRef();registers.push_back(r);}return r;
}
static void Capture(IDirect3DDevice9* d,LONG serial){
 IDirect3DVertexShader9* v=nullptr;IDirect3DPixelShader9* p=nullptr;d->GetVertexShader(&v);d->GetPixelShader(&p);
 auto vr=Reflect(v,true),pr=Reflect(p,false);
 static unsigned traceCount=0;
 static const bool trace=[](){char value[8]{};return GetEnvironmentVariableA("MALEMOD_MERIDIAN_LIGHT_TRACE",value,8)==1&&value[0]=='1';}();
 if(trace&&traceCount<1000&&(serial<45||serial%120<2)){
  DWORD blend=0,src=0,dst=0,depth=0;d->GetRenderState(D3DRS_ALPHABLENDENABLE,&blend);d->GetRenderState(D3DRS_SRCBLEND,&src);d->GetRenderState(D3DRS_DESTBLEND,&dst);d->GetRenderState(D3DRS_ZFUNC,&depth);
  float l[4]{},a[4]{},sh[28]{},dir[4]{};
  if(pr.light!=~0u)d->GetPixelShaderConstantF(pr.light,l,1);if(pr.ambient!=~0u)d->GetPixelShaderConstantF(pr.ambient,a,1);if(pr.incident!=~0u)d->GetPixelShaderConstantF(pr.incident,sh,7);if(vr.direction!=~0u)d->GetVertexShaderConstantF(vr.direction,dir,1);
  Log("Fabric trace frame=%ld tank=%d vs=%p ps=%p regs=%u,%u,%u,%u,%u blend=%u,%u,%u depth=%u light=(%.4f %.4f %.4f) ambient=(%.4f %.4f %.4f %.4f) sh0=(%.4f %.4f %.4f) dir=(%.3f %.3f %.3f %.3f)",serial,TankCameraSceneActive(),v,p,vr.direction,pr.light,pr.ambient,pr.incident,pr.basis,blend,src,dst,depth,l[0],l[1],l[2],a[0],a[1],a[2],a[3],sh[0],sh[1],sh[2],dir[0],dir[1],dir[2],dir[3]);++traceCount;
 }

 // Each snapshot belongs to exactly one native lighting pass. Never combine
 // stale direction/color with another pass's SH or texture bindings.
 ++pass;valid=false;frame=serial;memset(light,0,sizeof(light));memset(ambient,0,sizeof(ambient));memset(direction,0,sizeof(direction));memset(incident,0,sizeof(incident));memset(flags,0,sizeof(flags));
 for(auto& t:basis){if(t)t->Release();t=nullptr;}if(attenuation)attenuation->Release();attenuation=nullptr;
 DWORD blend=0,src=0,dst=0;bool ok=SUCCEEDED(d->GetRenderState(D3DRS_ALPHABLENDENABLE,&blend))&&SUCCEEDED(d->GetRenderState(D3DRS_SRCBLEND,&src))&&SUCCEEDED(d->GetRenderState(D3DRS_DESTBLEND,&dst));
 additive=blend&&src==D3DBLEND_ONE&&dst==D3DBLEND_ONE;ok=ok&&(!blend||additive);
 auto readPS=[&](UINT reg,float* data,UINT count=1){return reg!=~0u&&SUCCEEDED(d->GetPixelShaderConstantF(reg,data,count));};
 bool recognized=false;
 if(pr.ambient!=~0u){recognized=true;ok=readPS(pr.ambient,ambient)&&ok;}
 if(vr.direction!=~0u&&pr.light!=~0u){recognized=true;flags[0]=1;ok=SUCCEEDED(d->GetVertexShaderConstantF(vr.direction,direction,1))&&readPS(pr.light,light)&&ok;}
 if(pr.incident!=~0u&&pr.basis!=~0u){recognized=true;flags[1]=1;ok=readPS(pr.incident,incident,7)&&ok;for(unsigned i=0;i<2;i++)ok=SUCCEEDED(d->GetTexture(pr.basis+i,&basis[i]))&&basis[i]&&ok;}
 if(vr.position!=~0u&&pr.localLight!=~0u){
  recognized=true;flags[2]=1;ok=SUCCEEDED(d->GetVertexShaderConstantF(vr.position,position,1))&&readPS(pr.localLight,light)&&readPS(pr.screen,screen)&&ok;
  ok=pr.attenuation!=~0u&&SUCCEEDED(d->GetTexture(pr.attenuation,&attenuation))&&attenuation&&ok;
  if(pr.spotDirection!=~0u&&pr.spotAngles!=~0u){flags[3]=1;ok=readPS(pr.spotDirection,spotDirection)&&readPS(pr.spotAngles,spotAngles)&&ok;}
 }
 if(!additive)ok=readPS(pr.depth,depth)&&ok;
 valid=ok&&recognized;
 if(v)v->Release();if(p)p->Release();
 if(trace&&traceCount<1000&&(serial<45||serial%120<2))Log("Fabric pass frame=%ld pass=%u valid=%d additive=%d directional=%g sh=%g local=%g spot=%g",serial,pass,valid,additive,flags[0],flags[1],flags[2],flags[3]);
}

static bool Ensure(IDirect3DDevice9* d){
#ifdef FABRIC_STOCK_MAPS
 if(!EnsureStockMaps(d))return false;
 const char* stock="1";
#else
 const char* stock="0";
#endif
 unsigned key=0;for(unsigned i=0;i<4;i++)if(flags[i]>.5f)key|=1u<<i;ps=variants[key];
 if(vs&&ps)return true;
 
#ifdef FABRIC_COTTON_TOP
 const char* cotton="1";
#else
 const char* cotton="0";
#endif
 D3DXMACRO defines[]={{"STOCK_MAPS",stock},{"COTTON_TOP",cotton},{"HAS_DIRECTION",flags[0]>.5f?"1":"0"},{"HAS_SH",flags[1]>.5f?"1":"0"},{"HAS_LOCAL",flags[2]>.5f?"1":"0"},{"HAS_SPOT",flags[3]>.5f?"1":"0"},{nullptr,nullptr}};
 const char* vertex=R"(
 float4 L[4]:register(c0);float4 V[4]:register(c4);
 struct I{float3 p:POSITION;float3 n:NORMAL;float2 uv:TEXCOORD0;float4 c:TEXCOORD1;};
 // Material role and winding are data, including values outside [0,1].
 // D3D9 color interpolators saturate; TEXCOORD retains these values.
 struct O{float4 p:POSITION;float3 n:TEXCOORD1;float3 w:TEXCOORD2;float4 screen:TEXCOORD3;float2 uv:TEXCOORD0;float4 c:TEXCOORD4;};
 O main(I i){O o;float4 w=i.p.x*L[0]+i.p.y*L[1]+i.p.z*L[2]+L[3];o.p=w.x*V[0]+w.y*V[1]+w.z*V[2]+w.w*V[3];o.w=w.xyz;o.screen=o.p;
 float3 a=L[0].xyz,b=L[1].xyz,c=L[2].xyz;float det=dot(a,cross(b,c));o.n=normalize((i.n.x*cross(b,c)+i.n.y*cross(c,a)+i.n.z*cross(a,b))*(det<0?-1:1));o.uv=i.uv;o.c=i.c;return o;})";
 const char* pixel=R"(
 float4 white:register(c0);float4 red:register(c1);float4 blue:register(c2);
 float4 params:register(c3);float4 stripes:register(c4);
 float4 light:register(c5);float4 ambient:register(c6);float4 direction:register(c7);float4 sh[7]:register(c8);samplerCUBE basis0:register(s0);samplerCUBE basis1:register(s1);
 float4 flags:register(c15);float4 position:register(c16);float4 spotDirection:register(c17);float4 spotAngles:register(c18);float4 screenBias:register(c19);float4 passParams:register(c20);sampler2D attenuation:register(s2);
 sampler2D stockDiffuse:register(s3);sampler2D stockNormal:register(s4);sampler2D stockSpecular:register(s5);sampler2D buckleDiffuse:register(s6);sampler2D buckleNormal:register(s7);float4 camera:register(c21);
 float4 main(float4 projected:TEXCOORD3,float3 n:TEXCOORD1,float3 w:TEXCOORD2,float2 uv:TEXCOORD0,float4 c:TEXCOORD4,float facing:VFACE):COLOR0{
 n*=rsqrt(max(dot(n,n),1e-10));n*=facing*(c.x>3.5?1:c.w)<0?-1:1;float3 color=white.rgb;float pouch=1-step(.5,c.x),band=step(.5,c.x)*(1-step(1.5,c.x));
#if COTTON_TOP
 pouch=0;band=0;
#endif
#if !STOCK_MAPS
 if(c.x>3.5){
  float grainAA=1/(1+fwidth(uv.x+uv.y)*180);
  float weave=1+.12*sin((uv.x+uv.y)*360)*grainAA+.035*sin(uv.x*95)*sin(uv.y*111);
  color=float3(.028,.065,.13)*weave;
  // Measured rest coordinates preserve waistband, fly and pocket stitching
  // through skinning and through the folded panels. No additional textures.
  float side=abs(c.y),height=c.z,front=step(0,c.w),edgeAA=max(fwidth(height),.08);
  float waist=1-smoothstep(.1,.1+edgeAA,abs(height-94.6));
  float fly=(1-smoothstep(.08,.08+max(fwidth(side),.05),abs(side-.55)))*step(76,height)*step(height,94)*front;
  float pocketCurve=90-.13*(side-7)*(side-7);
  float pocket=(1-smoothstep(.08,.08+edgeAA,abs(height-pocketCurve)))*step(5,side)*step(side,14)*front;
  float stitch=max(waist,max(fly,pocket));color=lerp(color,float3(.24,.16,.07),stitch*.5);
  if(c.x>4.5)color=float3(.035,.025,.018)*(1+.035*sin(uv.x*150));
 }
#endif
 float aa=max(fwidth(uv.y),.0001);float r=1-smoothstep(stripes.z-aa,stripes.z+aa,abs(uv.y-stripes.x));float b=1-smoothstep(stripes.z-aa,stripes.z+aa,abs(uv.y-stripes.y));color=lerp(color,red.rgb,r*band);color=lerp(color,blue.rgb,b*band);
 float phase=uv.x*params.x*6.2831853;float fade=1-smoothstep(.2,.65,fwidth(uv.x)*params.x);float rib=cos(phase)*fade;
 // Derivative tangent follows the authored weave under deformation. Fade at
 // subpixel scale to prevent temporal shimmer without extra texture traffic.
 float3 dx=ddx(w),dy=ddy(w);float2 ux=ddx(uv),uy=ddy(uv);float determinant=ux.x*uy.y-ux.y*uy.x;
 float3 tangent=dx*uy.y-dy*ux.y;tangent-=n*dot(n,tangent);tangent*=rsqrt(max(dot(tangent,tangent),1e-10));tangent*=determinant<0?-1:1;
 n=normalize(n+tangent*(sin(phase)*params.y*fade*pouch));color*=1+pouch*.035*rib;
#if STOCK_MAPS
 color=tex2D(stockDiffuse,uv).rgb;
 float3 mapped=tex2D(stockNormal,uv).rgb*2-1;
 float3 specular=tex2D(stockSpecular,uv).rgb*.18;
 if(!COTTON_TOP && c.x>5.5){color=tex2D(buckleDiffuse,uv).rgb;mapped=tex2D(buckleNormal,uv).rgb*2-1;specular=float3(.35,.35,.35);}
 float3 bitangent=dy*ux.x-dx*uy.x;bitangent-=n*dot(n,bitangent);bitangent*=rsqrt(max(dot(bitangent,bitangent),1e-10));bitangent*=determinant<0?-1:1;
 n=normalize(tangent*mapped.x+bitangent*mapped.y+n*max(mapped.z,.05));
 float3 view=normalize(camera.xyz-w);float3 shine=0;
#endif
 float3 illumination=ambient.rgb;
 // Rough cotton uses the engine's current diffuse lights and SH. The base
 // pass and additive lights retain their own blend/depth/alpha contracts.
 if(HAS_DIRECTION){float3 l=direction.xyz*rsqrt(max(dot(direction.xyz,direction.xyz),1e-10));illumination+=light.rgb*saturate(dot(n,l));
#if STOCK_MAPS
 shine+=light.rgb*specular*pow(saturate(dot(n,normalize(l+view))),24)*saturate(dot(n,l))*camera.w;
#endif
 }
 if(HAS_SH){
  float4 a=(texCUBE(basis0,n)*2-1)*float4(2.09439516,2.09439516,2.09439516,.785398185);
  float4 shB=(texCUBE(basis1,n)*2-1)*.785398185;
  float3 sky=sh[0].xyz*.886227548+float3(dot(sh[1],a)+dot(sh[2],shB),dot(sh[3],a)+dot(sh[4],shB),dot(sh[5],a)+dot(sh[6],shB));
  illumination+=max(sky,0);
 }
 if(HAS_LOCAL){
  float3 q=(position.xyz-w)*position.w;float radius=1-dot(q,q);float3 l=q*rsqrt(max(dot(q,q),1e-10));
  float falloff=pow(max(saturate(radius),.0001),light.w)*step(0,radius);
  if(HAS_SPOT){float cone=saturate((dot(l,-spotDirection.xyz)-spotAngles.x)*spotAngles.y);falloff*=cone*cone;}
  float2 screenUV=projected.xy/projected.w*screenBias.xy+screenBias.wz;
  // The native receiver map describes the bare chest. A newly added
  // cotton surface cannot reuse its fine self-shadow bands as cloth creases.
  float3 shadow=tex2D(attenuation,screenUV).rgb;
  if(c.x>2.5)shadow=1;
#if COTTON_TOP
  shadow=1;
#endif
  illumination+=light.rgb*saturate(dot(n,l))*falloff*shadow;
#if STOCK_MAPS
 shine+=light.rgb*specular*pow(saturate(dot(n,normalize(l+view))),24)*saturate(dot(n,l))*falloff*shadow*camera.w;
#endif
 }
 // UE3 scene alpha uses the captured inverse-depth coefficients, not opacity.
 // Additive RGB-only passes preserve it for native blur and translucency.
 float3 radiance=color*max(illumination,0);
#if STOCK_MAPS
 radiance+=shine;
#endif
 if(white.w<0)radiance=float3(.8,.01,.6); // sealed flat-color visibility probe
 // The tank's blue key exceeds 17 in linear HDR. A smooth, hue-preserving
 // shoulder keeps white cotton readable without a hard clip or an ambient floor.
 if(passParams.y>0){float peak=max(radiance.x,max(radiance.y,radiance.z));radiance*=passParams.y/(passParams.y+peak);}
 return float4(radiance,passParams.x*(passParams.z/projected.w+passParams.w));

 })";
 auto compile=[&](const char* text,const char* profile,ID3DXBuffer** code){ID3DXBuffer* errors=nullptr;HRESULT hr=D3DXCompileShader(text,(UINT)strlen(text),defines,nullptr,"main",profile,0,code,&errors,nullptr);if(errors){Log("Meridian fabric %s: %s",profile,(char*)errors->GetBufferPointer());errors->Release();}return SUCCEEDED(hr);};
 ID3DXBuffer* code=nullptr;if(!vs){if(!compile(vertex,"vs_3_0",&code))return false;HRESULT hr=d->CreateVertexShader((DWORD*)code->GetBufferPointer(),&vs);code->Release();if(FAILED(hr))return false;}
 if(!ps){if(!compile(pixel,"ps_3_0",&code))return false;HRESULT hr=d->CreatePixelShader((DWORD*)code->GetBufferPointer(),&ps);code->Release();if(FAILED(hr))return false;variants[key]=ps;}return true;
}
static void Apply(IDirect3DDevice9* d){auto s=malemod::garments::meridian::classicFabric;float values[8][4]={{s.white[0],s.white[1],s.white[2],1},{s.red[0],s.red[1],s.red[2],1},{s.blue[0],s.blue[1],s.blue[2],1},{s.ribCount,s.ribSlope,0,0},{s.redCenter,s.blueCenter,s.stripeHalfWidth,0}};memcpy(values[5],light,16);memcpy(values[6],ambient,16);memcpy(values[7],direction,16);static const bool probe=[](){char v[8]{};return GetEnvironmentVariableA("MALEMOD_MERIDIAN_LIGHT_TRACE",v,8)==1&&v[0]=='1';}();if(probe){char path[MAX_PATH]{};SiblingPath(path,"MeridianFlat.request");if(GetFileAttributesA(path)!=INVALID_FILE_ATTRIBUTES)values[0][3]=-1;}d->SetPixelShaderConstantF(0,values[0],8);d->SetPixelShaderConstantF(8,incident,7);d->SetPixelShaderConstantF(15,flags,1);d->SetPixelShaderConstantF(16,position,1);d->SetPixelShaderConstantF(17,spotDirection,1);d->SetPixelShaderConstantF(18,spotAngles,1);d->SetPixelShaderConstantF(19,screen,1);float passParams[4]={additive?0.f:1.f,TankCameraSceneActive()?.22f:0.f,depth[0],depth[1]};d->SetPixelShaderConstantF(20,passParams,1);
#ifdef FABRIC_STOCK_MAPS
 ApplyStockMaps(d);d->SetPixelShaderConstantF(21,stockCamera,1);
#endif
 d->SetTexture(2,attenuation);for(auto state:{D3DSAMP_MINFILTER,D3DSAMP_MAGFILTER})d->SetSamplerState(2,state,D3DTEXF_POINT);d->SetSamplerState(2,D3DSAMP_MIPFILTER,D3DTEXF_NONE);for(unsigned i=0;i<3;i++){
  // Every material draw owns its sampler contract. Native body textures can
  // otherwise leave a mip limit or bias on a shared sampler between frames.
  for(auto state:{D3DSAMP_ADDRESSU,D3DSAMP_ADDRESSV,D3DSAMP_ADDRESSW})d->SetSamplerState(i,state,D3DTADDRESS_CLAMP);
  d->SetSamplerState(i,D3DSAMP_MAXMIPLEVEL,0);d->SetSamplerState(i,D3DSAMP_MIPMAPLODBIAS,0);d->SetSamplerState(i,D3DSAMP_MAXANISOTROPY,1);d->SetSamplerState(i,D3DSAMP_SRGBTEXTURE,FALSE);
  if(i<2){d->SetTexture(i,basis[i]);d->SetSamplerState(i,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);d->SetSamplerState(i,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);d->SetSamplerState(i,D3DSAMP_MIPFILTER,D3DTEXF_NONE);}
 }}
}
