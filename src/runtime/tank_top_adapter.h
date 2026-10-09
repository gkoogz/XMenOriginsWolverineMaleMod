#pragma once
#include "tank_top_data.h"
#include "meridian_state_audit.h"
#define FABRIC_COTTON_TOP
#define FABRIC_MATERIAL_NAMESPACE TankTopMaterial
#include "fabric_material.inl"
#undef FABRIC_MATERIAL_NAMESPACE
#undef FABRIC_COTTON_TOP
namespace TankTopAdapter {
static constexpr unsigned count=sizeof(TankTopRecipe::vertices)/sizeof(NcVertex);
static constexpr unsigned faces=sizeof(TankTopRecipe::triangles)/sizeof(TankTopRecipe::triangles[0]);
static NcVertex fitted[count];
static unsigned bodyRevision=~0u;
static IDirect3DVertexBuffer9* vb=nullptr;
static IDirect3DIndexBuffer9* ib=nullptr;
static IDirect3DVertexDeclaration9* declaration=nullptr;
static IDirect3DStateBlock9* state=nullptr;
static LONG prepared=-3;
struct RenderVertex {float p[3],n[3],uv[2],color[4];};
static std::vector<RenderVertex> vertices;
static void Refit(){
 EnsureSharedBody();if(bodyRevision==sharedBodyRevision)return;
 memcpy(fitted,TankTopRecipe::vertices,sizeof(fitted));
 static std::unordered_map<unsigned,unsigned> lookup;
 if(lookup.empty())for(unsigned i=0;i<sizeof(TankTopRecipe::donorIDs)/sizeof(unsigned);i++)lookup.emplace(TankTopRecipe::donorIDs[i],i);
 for(unsigned i=0;i<count;i++)for(unsigned k=0;k<3;k++){
  const auto& b=TankTopRecipe::bindings[i];int section;UINT local;
  if(!MenuRetargetBodyVertex(b.donor[k],section,local))continue;
  float p[3];memcpy(p,sharedBodyOutput[section]+local*32,12);
  auto index=lookup.at(b.donor[k]);for(unsigned axis=0;axis<3;axis++)fitted[i].p[axis]+=b.weight[k]*(p[axis]-TankTopRecipe::donorBase[index][axis]);
 }
 bodyRevision=sharedBodyRevision;prepared=-3;
}
static void Collision(NcContactSolver& solver){
 Refit();solver.setGarment(topStyle==1?fitted:nullptr,topStyle==1?count:0,TankTopRecipe::triangles,topStyle==1?faces:0);
}
static void Release(){
 for(auto* p:{static_cast<IUnknown*>(vb),static_cast<IUnknown*>(ib),static_cast<IUnknown*>(declaration),static_cast<IUnknown*>(state)})if(p)p->Release();
 vb=nullptr;ib=nullptr;declaration=nullptr;state=nullptr;prepared=-3;bodyRevision=~0u;TankTopMaterial::Release();
}
static bool Ensure(IDirect3DDevice9* d){
 if(!vb&&FAILED(d->CreateVertexBuffer(count*sizeof(RenderVertex),D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&vb,nullptr)))return false;
 if(!ib){
  if(FAILED(d->CreateIndexBuffer(sizeof(TankTopRecipe::triangles),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&ib,nullptr)))return false;
  void* raw=nullptr;if(FAILED(ib->Lock(0,0,&raw,0))){ib->Release();ib=nullptr;return false;}memcpy(raw,TankTopRecipe::triangles,sizeof(TankTopRecipe::triangles));ib->Unlock();
 }
 if(!declaration){D3DVERTEXELEMENT9 elements[]={{0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},{0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0},{0,24,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},{0,32,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,1},D3DDECL_END()};if(FAILED(d->CreateVertexDeclaration(elements,&declaration)))return false;}
 if(!state&&FAILED(d->CreateStateBlock(D3DSBT_ALL,&state)))return false;
 return true;
}
static void Draw(IDirect3DDevice9* d){
 if(topStyle!=1||!IsHdrSceneColorPass(d))return;
 auto* layout=GetShaderLayout(d);if(!layout||!layout->valid||!layout->viewValid||layout->boneCount<102)return;
 float local[16],view[16];if(FAILED(d->GetVertexShaderConstantF(layout->localRegister,local,4))||FAILED(d->GetVertexShaderConstantF(layout->viewRegister,view,4)))return;
 TankTopMaterial::Capture(d,renderFrameSerial);if(!TankTopMaterial::valid||!TankTopMaterial::Ensure(d)||!Ensure(d))return;
 Refit();
 if(prepared!=renderFrameSerial){
  float palette[34*12];if(FAILED(d->GetVertexShaderConstantF(layout->boneRegister,palette,102)))return;
  NcRig rig;const unsigned bones[]={3,4,5,6,7,34},slots[]={31,30,32,14,13,33};
  for(unsigned i=0;i<6;i++){rig.valid[bones[i]]=true;memcpy(rig.matrix[bones[i]],palette+slots[i]*12,48);}
  vertices.resize(count);
  for(unsigned i=0;i<count;i++){
   auto p=NcSkin(fitted[i],rig);auto& v=vertices[i];v.p[0]=p.x;v.p[1]=p.y;v.p[2]=p.z;
   // Transform the refitted rest normal by the same weighted measured bones.
   for(unsigned a=0;a<3;a++){v.n[a]=0;for(unsigned k=0;k<4;k++)if(fitted[i].weight[k]){auto* m=rig.matrix[fitted[i].bone[k]];for(unsigned b=0;b<3;b++)v.n[a]+=fitted[i].weight[k]/255.f*m[a*4+b]*TankTopRecipe::normals[i][b];}}
   // D3D9 treats clockwise faces as front-facing. Recipe sign compares
   // right-handed cross products with outward normals, so negate that sign.
   memcpy(v.uv,TankTopRecipe::uv[i],8);v.color[0]=3;v.color[1]=v.color[2]=1;v.color[3]=-TankTopRecipe::faceNormalSign;
  }
  void* raw=nullptr;if(FAILED(vb->Lock(0,0,&raw,D3DLOCK_DISCARD)))return;memcpy(raw,vertices.data(),vertices.size()*sizeof(vertices[0]));if(FAILED(vb->Unlock()))return;prepared=renderFrameSerial;
 }
 auto audit=MeridianStateSnapshot(d);if(FAILED(state->Capture()))return;
 d->SetVertexShader(TankTopMaterial::vs);d->SetPixelShader(TankTopMaterial::ps);d->SetVertexDeclaration(declaration);d->SetStreamSource(0,vb,0,sizeof(vertices[0]));d->SetIndices(ib);
 d->SetVertexShaderConstantF(0,local,4);d->SetVertexShaderConstantF(4,view,4);TankTopMaterial::Apply(d);
 d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,TankTopMaterial::additive?FALSE:TRUE);d->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);d->SetRenderState(D3DRS_STENCILENABLE,FALSE);d->SetRenderState(D3DRS_ALPHABLENDENABLE,TankTopMaterial::additive);d->SetRenderState(D3DRS_COLORWRITEENABLE,TankTopMaterial::additive?7:15);
 HRESULT hr=origDIP(d,D3DPT_TRIANGLELIST,0,0,count,0,faces),restored=state->Apply();
 if(FAILED(hr)||FAILED(restored))Log("Tank top draw failure=%08x restored=%08x",hr,restored);
 if(!audit.empty()&&audit!=MeridianStateSnapshot(d))Log("Tank top state audit MISMATCH frame=%ld",renderFrameSerial);
 static unsigned draws=0;if(++draws%240==1)Log("Tank top native draw frame=%ld vertices=%u triangles=%u bodyRevision=%u top=%u bottom=%u",renderFrameSerial,count,faces,bodyRevision,topStyle,clothingStyle);
}
}
