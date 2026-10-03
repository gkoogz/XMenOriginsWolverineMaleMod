#pragma once
// Adapter only. Include after the final nrPacked surface, shared body arrays,
// ShaderLayout and the verified gameplay bone-remap data.
#include <malemod/garments/jockstrap.hpp>
#include "jockstrap_measured_data.h"
#include "jockstrap_worker.h"
namespace JockstrapAdapter {
namespace G=malemod::garments;
// Heap lifetime avoids joining a worker inside the Windows DLL loader lock.
// Device reset/scene release explicitly stops it outside that lock.
static JockstrapCpuWorker* worker=nullptr;
static std::shared_ptr<const G::Output> snapshot;
static unsigned long long styleGeneration=1;
static G::Input input;
static G::Style style=G::Style::Naked;
static const unsigned char* currentBody=nullptr;
static unsigned long long epoch=1;
static bool ready=false;
static LONG drawnFrame=-2,paletteFrame[3]={-2,-2,-2};
static float palettes[3][75*12]{};static UINT paletteCount[3]{};
static float localMatrix[16]{},viewMatrix[16]{};
static IDirect3DVertexShader9* vertexShader=nullptr;
static IDirect3DPixelShader9* pixelShader=nullptr;
static IDirect3DVertexDeclaration9* declaration=nullptr;
static IDirect3DStateBlock9* stateBlock=nullptr;
struct RenderVertex{float p[3],n[3],uv[2],color[4];};
static std::vector<RenderVertex> rendered;
static std::vector<unsigned short> indices;
static G::Point Position(const unsigned char* p){float v[3];memcpy(v,p,12);return {v[0],v[1],v[2]};}
static G::Point Normal(const unsigned char* p){G::Point n{p[16]/127.5-1,p[17]/127.5-1,p[18]/127.5-1};return G::Length(n)>1e-9?G::Unit(n):G::Point{0,0,1};}
static G::Sample Recipe(const JockstrapMeasuredSample& r,G::Surface surface){G::Sample sample;sample.normal={};for(unsigned k=0;k<4;k++){auto d=r.donors[k];if(d.weight<=0)continue;const auto* packed=surface==G::Surface::Body?currentBody+d.vertex*32:nrPacked+d.vertex*32;sample.position=G::Add(sample.position,G::Mul(Position(packed),d.weight));sample.normal=G::Add(sample.normal,G::Mul(Normal(packed),d.weight));sample.lineage.donors[k]={surface,d.vertex,d.weight};}return sample;}
template<std::size_t N> static void Contour(const JockstrapMeasuredSample(&recipe)[N],std::vector<G::Sample>& out,G::Surface surface){out.resize(N);for(unsigned i=0;i<N;i++)out[i]=Recipe(recipe[i],surface);}
static void SetStyle(unsigned value){style=value==1?G::Style::WhiteJockstrap:G::Style::Naked;styleGeneration++;snapshot.reset();ready=false;if(worker)worker->Clear();}
static unsigned GetStyle(){return unsigned(style);}
static const G::Output* latest=nullptr;
static void Refresh(){if(!worker||style==G::Style::Naked)return;auto published=worker->Poll(epoch);if(published){snapshot=std::move(published);latest=snapshot.get();ready=!snapshot->mesh.vertices.empty();}else{snapshot.reset();latest=nullptr;ready=false;}}
static const G::Output* Latest(){Refresh();return ready?latest:nullptr;}
static bool Update(const unsigned char* body,unsigned long long characterEpoch=1,G::Point measuredGravity={0,0,-72},const std::vector<G::Capsule>& measuredContacts={}){
 currentBody=body;if(!body)return false;auto requestEpoch=characterEpoch^(styleGeneration<<32);if(epoch!=requestEpoch){snapshot.reset();latest=nullptr;ready=false;}epoch=requestEpoch;
 try{input.characterEpoch=epoch;input.topologyRevision=1;input.frame={};input.frame.lateral={0,-1,0};input.frame.forward={1,0,0};input.frame.up={0,0,1};input.gravity=measuredGravity;input.bodyContacts=measuredContacts;
  if(style==G::Style::Naked){snapshot.reset();latest=nullptr;ready=false;return true;}
  Contour(jockstrap_waist,input.waist,G::Surface::Body);Contour(jockstrap_opening,input.opening,G::Surface::Anatomy);Contour(jockstrap_strapLeft,input.rearStraps[0],G::Surface::Body);Contour(jockstrap_strapRight,input.rearStraps[1],G::Surface::Body);
  input.anatomy.resize(nrCount);for(unsigned i=0;i<nrCount;i++){input.anatomy[i]={Position(nrPacked+i*32),Normal(nrPacked+i*32),{}};input.anatomy[i].lineage.donors[0]={G::Surface::Anatomy,i,1};}
  if(input.anatomyTriangles.empty()){input.anatomyTriangles.resize(nrIndexCount/3);for(unsigned i=0;i<nrIndexCount/3;i++)input.anatomyTriangles[i]={nrIndices[i*3],nrIndices[i*3+1],nrIndices[i*3+2]};}
  if(!worker)worker=new JockstrapCpuWorker;worker->Submit(std::make_shared<G::Input>(input));Refresh();return true;
 }catch(const std::exception& e){ready=false;Log("Garment source update rejected: %s",e.what());return false;}
}
// Capture at each verified native body/anatomy draw. Frame tags prevent mixing
// previous camera/body palettes into a present-frame cloth pose.
static void CaptureSection(IDirect3DDevice9* d,int section){if(style==G::Style::Naked||section<0||section>2)return;auto* layout=GetShaderLayout(d);if(!layout||!layout->valid||!layout->viewValid)return;UINT count=(std::min)(75u,layout->boneCount/3);if(!count||FAILED(d->GetVertexShaderConstantF(layout->boneRegister,palettes[section],count*3)))return;if(FAILED(d->GetVertexShaderConstantF(layout->localRegister,localMatrix,4))||FAILED(d->GetVertexShaderConstantF(layout->viewRegister,viewMatrix,4)))return;paletteCount[section]=count;paletteFrame[section]=renderFrameSerial;}
static bool Pose(const G::Vertex& v,RenderVertex& out){G::Point point{},normal{};double total=0;bool title=TankCameraSceneActive();
 for(auto donor:v.lineage.donors){if(donor.weight<=0)continue;const unsigned char* packed=nullptr;const unsigned char* overrideBones=nullptr;unsigned section=2;
  if(donor.surface==G::Surface::Anatomy){if(donor.vertex>=nrCount)return false;packed=(title?menuTankPacked:nrPacked)+donor.vertex*32;}
  else{int s;UINT local;if(!MenuRetargetBodyVertex(donor.vertex,s,local))return false;section=unsigned(s);packed=sharedBodyBase[s]+local*32;if(!title)overrideBones=(s?fluidGameplayBones1:fluidGameplayBones0)+local*4;}
  if(paletteFrame[section]!=renderFrameSerial)return false;
  double weightTotal=0;for(unsigned k=0;k<4;k++){unsigned bone=overrideBones?overrideBones[k]:packed[20+k];double weight=packed[24+k]/255.;if(weight<=0)continue;if(bone>=paletteCount[section])return false;auto* matrix=palettes[section]+bone*12;double w=weight*donor.weight;
   G::Point p{},n{};for(unsigned r=0;r<3;r++){p[r]=matrix[r*4]*v.position[0]+matrix[r*4+1]*v.position[1]+matrix[r*4+2]*v.position[2]+matrix[r*4+3];n[r]=matrix[r*4]*v.normal[0]+matrix[r*4+1]*v.normal[1]+matrix[r*4+2]*v.normal[2];}point=G::Add(point,G::Mul(p,w));normal=G::Add(normal,G::Mul(n,w));total+=w;weightTotal+=weight;
  }if(weightTotal<.5)return false;
 }
 if(total<.5||G::Length(normal)<1e-12)return false;point=G::Mul(point,1/total);normal=G::Unit(normal);for(unsigned k=0;k<3;k++){out.p[k]=float(point[k]);out.n[k]=float(normal[k]);}out.uv[0]=float(v.uv[0]);out.uv[1]=float(v.uv[1]);return true;
}
static void Release(){ready=false;latest=nullptr;drawnFrame=-2;if(worker){worker->Stop();delete worker;worker=nullptr;}snapshot.reset();styleGeneration++;input=G::Input{};for(auto& frame:paletteFrame)frame=-2;memset(paletteCount,0,sizeof(paletteCount));if(vertexShader){vertexShader->Release();vertexShader=nullptr;}if(pixelShader){pixelShader->Release();pixelShader=nullptr;}if(declaration){declaration->Release();declaration=nullptr;}if(stateBlock){stateBlock->Release();stateBlock=nullptr;}rendered.clear();indices.clear();}
static bool EnsureShaders(IDirect3DDevice9* d){if(vertexShader&&pixelShader&&declaration&&stateBlock)return true;
 const char* vs="float4 L[4]:register(c0);float4 V[4]:register(c4);struct I{float3 p:POSITION;float3 n:NORMAL;float2 uv:TEXCOORD0;float4 c:COLOR0;};struct O{float4 p:POSITION;float3 n:TEXCOORD1;float2 uv:TEXCOORD0;float4 c:COLOR0;};O main(I i){O o;float4 w=i.p.x*L[0]+i.p.y*L[1]+i.p.z*L[2]+L[3];o.p=w.x*V[0]+w.y*V[1]+w.z*V[2]+w.w*V[3];o.n=normalize(i.n);o.uv=i.uv;o.c=i.c;return o;}";
 const char* ps="float4 tint:register(c0);float4 main(float3 n:TEXCOORD1,float2 uv:TEXCOORD0,float4 c:COLOR0):COLOR0{float knit=1+.018*cos(uv.x*804.2477)+.004*cos(uv.y*1608.4954);float lighting=.76+.24*abs(dot(normalize(n),normalize(float3(.35,.55,.76))));return float4(tint.rgb*knit*lighting,tint.a);}";
 ID3DXBuffer *code=nullptr,*errors=nullptr;HRESULT hr=D3DXCompileShader(vs,(UINT)strlen(vs),nullptr,nullptr,"main","vs_3_0",0,&code,&errors,nullptr);if(errors){Log("Garment VS: %s",(char*)errors->GetBufferPointer());errors->Release();}if(FAILED(hr))return false;hr=d->CreateVertexShader((DWORD*)code->GetBufferPointer(),&vertexShader);code->Release();if(FAILED(hr))return false;code=nullptr;errors=nullptr;hr=D3DXCompileShader(ps,(UINT)strlen(ps),nullptr,nullptr,"main","ps_3_0",0,&code,&errors,nullptr);if(errors){Log("Garment PS: %s",(char*)errors->GetBufferPointer());errors->Release();}if(FAILED(hr))return false;hr=d->CreatePixelShader((DWORD*)code->GetBufferPointer(),&pixelShader);code->Release();if(FAILED(hr))return false;
 D3DVERTEXELEMENT9 elements[]={{0,0,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},{0,12,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_NORMAL,0},{0,24,D3DDECLTYPE_FLOAT2,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TEXCOORD,0},{0,32,D3DDECLTYPE_FLOAT4,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_COLOR,0},D3DDECL_END()};return SUCCEEDED(d->CreateVertexDeclaration(elements,&declaration))&&SUCCEEDED(d->CreateStateBlock(D3DSBT_ALL,&stateBlock));
}
static void Draw(IDirect3DDevice9* d){Refresh();if(!ready||!latest||style==G::Style::Naked||drawnFrame==renderFrameSerial)return;DWORD color;d->GetRenderState(D3DRS_COLORWRITEENABLE,&color);if(!(color&7)||!EnsureShaders(d))return;const auto& mesh=latest->mesh;rendered.resize(mesh.vertices.size());indices.resize(mesh.triangles.size()*3);for(unsigned i=0;i<mesh.vertices.size();i++)if(!Pose(mesh.vertices[i],rendered[i]))return;for(auto& v:rendered){v.color[0]=v.color[1]=v.color[2]=v.color[3]=1;}
 if(FAILED(stateBlock->Capture()))return;d->SetVertexShader(vertexShader);d->SetPixelShader(pixelShader);d->SetVertexDeclaration(declaration);d->SetVertexShaderConstantF(0,localMatrix,4);d->SetVertexShaderConstantF(4,viewMatrix,4);d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);d->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);d->SetRenderState(D3DRS_STENCILENABLE,FALSE);d->SetRenderState(D3DRS_COLORWRITEENABLE,15);HRESULT hr=S_OK;for(unsigned material=0;material<4;material++){unsigned used=0;for(const auto& triangle:mesh.triangles)if(unsigned(triangle.material)==material)for(auto vertex:triangle.vertices)indices[used++]=(unsigned short)vertex;if(!used)continue;const auto& color=latest->materials[material].color;float tint[4]={float(color[0]),float(color[1]),float(color[2]),1};d->SetPixelShaderConstantF(0,tint,1);hr=d->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,(UINT)rendered.size(),used/3,indices.data(),D3DFMT_INDEX16,rendered.data(),sizeof(RenderVertex));if(FAILED(hr))break;}stateBlock->Apply();if(SUCCEEDED(hr))drawnFrame=renderFrameSerial;
}
}
