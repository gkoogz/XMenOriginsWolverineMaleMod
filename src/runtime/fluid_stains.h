#pragma once
// Small persistent impact decals for the title-tank demonstration. Marks are
// bounded, depth-tested geometry rather than a screen-space particle field.
namespace volumeFluid {
struct StainVertex {V3 p;float alpha;};
struct StainMark {V3 p,n;float radius;DWORD born;unsigned seed;FluidImpactKind kind;unsigned triangle;float baryU,baryV;void* shape;};
static const char* stainShader=R"HLSL(
float4 VP[4]:register(c0);float4 Material:register(c4);
struct O{float4 p:POSITION;float a:TEXCOORD0;};
O Vertex(float3 p:POSITION,float a:TEXCOORD0){O o;o.p=p.x*VP[0]+p.y*VP[1]+p.z*VP[2]+VP[3];o.a=a;return o;}
float4 Pixel(O o):COLOR0{return float4(Material.rgb,o.a*Material.a);}
)HLSL";
class StainSurface {
public:
 std::vector<StainMark> marks;IDirect3DVertexShader9* vs=nullptr;IDirect3DPixelShader9* ps=nullptr;IDirect3DVertexDeclaration9* decl=nullptr;IDirect3DVertexBuffer9* vb=nullptr;UINT capacity=0;
 void Release(){Drop(vs);Drop(ps);Drop(decl);Drop(vb);capacity=0;marks.clear();}
 bool Initialize(IDirect3DDevice9* d){
  if(vs&&ps&&decl)return true;
  for(int i=0;i<2;i++){ID3DXBuffer *code=nullptr,*errors=nullptr;HRESULT hr=D3DXCompileShader(stainShader,(UINT)strlen(stainShader),nullptr,nullptr,i?"Pixel":"Vertex",i?"ps_3_0":"vs_3_0",D3DXSHADER_OPTIMIZATION_LEVEL3,&code,&errors,nullptr);Drop(errors);if(FAILED(hr)){Drop(code);return false;}hr=i?d->CreatePixelShader((DWORD*)code->GetBufferPointer(),&ps):d->CreateVertexShader((DWORD*)code->GetBufferPointer(),&vs);Drop(code);if(FAILED(hr))return false;}
  D3DVERTEXELEMENT9 elements[]={{0,0,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},{0,12,D3DDECLTYPE_FLOAT1,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TEXCOORD,0},D3DDECL_END()};
  return SUCCEEDED(d->CreateVertexDeclaration(elements,&decl));
 }
 void Add(const std::vector<FluidImpact>& impacts,DWORD now){
  for(const auto& hit:impacts){
   bool nearby=false;for(auto& mark:marks)if(mark.kind==hit.kind&&Dot(mark.n,hit.n)>.55f&&Length(mark.p-hit.p)<1.15f){nearby=true;break;}if(nearby)continue;
   unsigned seed=(unsigned)(fabsf(hit.p.x)*73856093.f)^(unsigned)(fabsf(hit.p.y)*19349663.f)^(unsigned)(fabsf(hit.p.z)*83492791.f)^(unsigned)now;
   float radius=.72f+(seed&1023)/1023.f*1.15f;marks.push_back({hit.p,Unit(hit.n),radius,now,seed,hit.kind,hit.triangle,hit.baryU,hit.baryV,hit.shape});
   // Twenty seconds of sustained multi-pulse output can easily exceed the old
   // 96-mark ring. Keep a generous bounded pool so age, rather than insertion
   // pressure, controls disappearance.
   if(marks.size()>2048)marks.erase(marks.begin(),marks.begin()+(marks.size()-2048));
  }
 }
 HRESULT Draw(IDirect3DDevice9* d,const float* vp,DWORD now){
  marks.erase(std::remove_if(marks.begin(),marks.end(),[&](const StainMark& m){return DWORD(now-m.born)>=20000u;}),marks.end());
  if(marks.empty())return S_FALSE;if(!Initialize(d))return E_FAIL;
  std::vector<StainVertex> vertices;vertices.reserve(marks.size()*30);
  const int sides=10;
  for(const auto& mark:marks){
   V3 position=mark.p,normal=mark.n;if(mark.kind==FLUID_IMPACT_BODY&&!FluidResolveBodyAnchor(mark.triangle,mark.baryU,mark.baryV,position,normal))continue;
   float age=DWORD(now-mark.born)*.001f,fade=age<15.f?1.f:max(0.f,(20.f-age)/5.f);V3 n=Unit(normal),u=Unit(Cross(n,fabsf(n.z)<.82f?V3{0,0,1}:V3{0,1,0})),v=Cross(n,u);V3 center=position+n*.045f;
   for(int i=0;i<sides;i++){float a=i*6.283185307f/sides,b=(i+1)*6.283185307f/sides;float ra=mark.radius*(.78f+.22f*sinf((i*3+(mark.seed&7))*1.7f)),rb=mark.radius*(.78f+.22f*sinf(((i+1)*3+(mark.seed&7))*1.7f));vertices.push_back({center,fade});vertices.push_back({center+(u*cosf(a)+v*sinf(a))*ra,fade});vertices.push_back({center+(u*cosf(b)+v*sinf(b))*rb,fade});}
  }
  UINT count=(UINT)vertices.size();if(!vb||capacity<count){Drop(vb);capacity=count+768;if(FAILED(d->CreateVertexBuffer(capacity*sizeof(StainVertex),D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&vb,nullptr)))return E_FAIL;}
  void* raw=nullptr;HRESULT hr=vb->Lock(0,count*sizeof(StainVertex),&raw,D3DLOCK_DISCARD);if(FAILED(hr))return hr;memcpy(raw,vertices.data(),count*sizeof(StainVertex));vb->Unlock();
  IDirect3DStateBlock9* saved=nullptr;if(FAILED(d->CreateStateBlock(D3DSBT_ALL,&saved)))return E_FAIL;float bank[1024];if(FAILED(d->GetVertexShaderConstantF(0,bank,256))){Drop(saved);return E_FAIL;}
  float constants[20]{};memcpy(constants,vp,64);constants[16]=1.22f;constants[17]=1.08f;constants[18]=.91f;constants[19]=.90f;
  d->SetVertexShader(vs);d->SetPixelShader(ps);d->SetVertexDeclaration(decl);d->SetStreamSource(0,vb,0,sizeof(StainVertex));d->SetIndices(nullptr);for(int i=0;i<4;i++)d->SetStreamSourceFreq(i,1);d->SetVertexShaderConstantF(0,constants,5);d->SetPixelShaderConstantF(0,constants,5);
  d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);d->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);d->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);d->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);d->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);d->SetRenderState(D3DRS_SRGBWRITEENABLE,FALSE);d->SetRenderState(D3DRS_COLORWRITEENABLE,7);float depthBias=-.0002f;DWORD packedBias;memcpy(&packedBias,&depthBias,4);d->SetRenderState(D3DRS_DEPTHBIAS,packedBias);hr=d->DrawPrimitive(D3DPT_TRIANGLELIST,0,count/3);
  saved->Apply();d->SetVertexShaderConstantF(0,bank,256);Drop(saved);return hr;
 }
};
}
