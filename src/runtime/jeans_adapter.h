#pragma once
#include <unordered_set>
#include "jeans_data.h"
#include "menu_necklace_palette.h"
#define FABRIC_MATERIAL_NAMESPACE JeansMaterial
#include "fabric_material.inl"
#undef FABRIC_MATERIAL_NAMESPACE
namespace JeansAdapter {
static IDirect3DVertexBuffer9* vb=nullptr;
static IDirect3DIndexBuffer9* ib[2]{},*bodyIB[2][2][8]{};static unsigned bodyCount[2][2][8]{};
static IDirect3DVertexDeclaration9* declaration=nullptr;
static IDirect3DStateBlock9* state=nullptr;
static LONG rigFrame[2]={-3,-3},prepared=-3;static unsigned preparedStyle=0;
static NcRig rig;
using RenderVertex=TankTopAdapter::RenderVertex;
static std::vector<RenderVertex> rendered;
static bool Active(){return clothingStyle==2||clothingStyle==3;}
static void Release(){
 if(vb)vb->Release();vb=nullptr;for(auto& p:ib){if(p)p->Release();p=nullptr;}
 for(auto& a:bodyIB)for(auto& b:a)for(auto& p:b){if(p)p->Release();p=nullptr;}
 if(declaration)declaration->Release();declaration=nullptr;if(state)state->Release();state=nullptr;
 rigFrame[0]=rigFrame[1]=prepared=-3;rig=NcRig{};JeansMaterial::Release();
}
static void Capture(IDirect3DDevice9* d,unsigned section,bool title){
 if(!Active()||!IsHdrSceneColorPass(d))return;auto* layout=GetShaderLayout(d);if(!layout||!layout->valid)return;
 const unsigned char* palette=title?(section?ncMenuBodyPalette1:ncMenuBodyPalette0):(section?JeansRecipe::gameplayPalette1:JeansRecipe::gameplayPalette0);
 unsigned count=title?(section?sizeof(ncMenuBodyPalette1):sizeof(ncMenuBodyPalette0)):(section?sizeof(JeansRecipe::gameplayPalette1):sizeof(JeansRecipe::gameplayPalette0));
 if(count>75||layout->boneCount<count*3)return;float matrices[75*12];if(FAILED(d->GetVertexShaderConstantF(layout->boneRegister,matrices,count*3)))return;
 if(rigFrame[0]!=renderFrameSerial&&rigFrame[1]!=renderFrameSerial)rig=NcRig{};
 for(unsigned i=0;i<count;i++){rig.valid[palette[i]]=true;memcpy(rig.matrix[palette[i]],matrices+i*12,48);}rigFrame[section]=renderFrameSerial;
}
static HRESULT Body(IDirect3DDevice9* d,unsigned section,bool title,D3DPRIMITIVETYPE type,INT base,UINT minv,UINT nv,UINT start,UINT count){
 if(!Active()&&topStyle!=1)return DrawSharedBodySkin(d,type,base,minv,nv,start,count);
 unsigned open=clothingStyle==3,mode=clothingStyle+4*(topStyle==1);const unsigned short* data=nullptr;unsigned size=0;
 if(!Active()){data=section?menuRetargetBodyIndices1:menuRetargetBodyIndices0;size=section?sizeof(menuRetargetBodyIndices1):sizeof(menuRetargetBodyIndices0);}
 else if(section==0){data=open?&JeansRecipe::body01[0][0]:&JeansRecipe::body00[0][0];size=open?sizeof(JeansRecipe::body01):sizeof(JeansRecipe::body00);}
 else{data=open?&JeansRecipe::body11[0][0]:&JeansRecipe::body10[0][0];size=open?sizeof(JeansRecipe::body11):sizeof(JeansRecipe::body10);}
 if(!size)return D3D_OK;auto*& target=bodyIB[title?1:0][section][mode];auto& triangles=bodyCount[title?1:0][section][mode];
 if(!target){
  auto key=[](const unsigned short* f){return (unsigned long long)f[0]|((unsigned long long)f[1]<<16)|((unsigned long long)f[2]<<32);};
  static std::unordered_set<unsigned long long> hidden[2];
  if(hidden[section].empty()){auto* f=section?&TankTopRecipe::hiddenBody1[0][0]:&TankTopRecipe::hiddenBody0[0][0];unsigned bytes=section?sizeof(TankTopRecipe::hiddenBody1):sizeof(TankTopRecipe::hiddenBody0);for(unsigned i=0;i<bytes/2;i+=3)hidden[section].insert(key(f+i));}
  std::vector<unsigned short> indices;unsigned offset=title?0:(section?41435:17449);
  for(unsigned i=0;i<size/2;i+=3)if(topStyle!=1||!hidden[section].count(key(data+i)))for(unsigned k=0;k<3;k++)indices.push_back(data[i+k]+offset);
  if(indices.empty())return D3D_OK;triangles=unsigned(indices.size()/3);
  if(FAILED(d->CreateIndexBuffer(unsigned(indices.size()*2),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&target,nullptr)))return E_FAIL;void* raw=nullptr;if(FAILED(target->Lock(0,0,&raw,0))){target->Release();target=nullptr;return E_FAIL;}memcpy(raw,indices.data(),indices.size()*2);target->Unlock();
 }
 IDirect3DIndexBuffer9* original=nullptr;d->GetIndices(&original);HRESULT hr=d->SetIndices(target);if(SUCCEEDED(hr))hr=DrawSharedBodySkin(d,type,title?0:base,minv,nv,0,triangles);d->SetIndices(original);if(original)original->Release();return hr;
}
static void Draw(IDirect3DDevice9* d){
 if(!Active()||!IsHdrSceneColorPass(d)||rigFrame[0]!=renderFrameSerial||rigFrame[1]!=renderFrameSerial)return;
 unsigned open=clothingStyle==3;const auto* source=open?JeansRecipe::vertices1:JeansRecipe::vertices0;const auto* normals=open?JeansRecipe::normals1:JeansRecipe::normals0;const auto* uv=open?JeansRecipe::uv1:JeansRecipe::uv0;
 unsigned count=open?sizeof(JeansRecipe::vertices1)/sizeof(NcVertex):sizeof(JeansRecipe::vertices0)/sizeof(NcVertex),faces=open?sizeof(JeansRecipe::triangles1)/6:sizeof(JeansRecipe::triangles0)/6;
 auto* layout=GetShaderLayout(d);if(!layout||!layout->valid||!layout->viewValid)return;float local[16],view[16];if(FAILED(d->GetVertexShaderConstantF(layout->localRegister,local,4))||FAILED(d->GetVertexShaderConstantF(layout->viewRegister,view,4)))return;
 JeansMaterial::Capture(d,renderFrameSerial);if(!JeansMaterial::valid||!JeansMaterial::Ensure(d))return;
 if(!vb&&FAILED(d->CreateVertexBuffer(sizeof(JeansRecipe::vertices1)/sizeof(NcVertex)*sizeof(RenderVertex),D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&vb,nullptr)))return;
 if(!ib[open]){if(FAILED(d->CreateIndexBuffer(faces*6,D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&ib[open],nullptr)))return;void* raw=nullptr;if(FAILED(ib[open]->Lock(0,0,&raw,0)))return;memcpy(raw,open?JeansRecipe::triangles1:JeansRecipe::triangles0,faces*6);ib[open]->Unlock();}
 if(!declaration){D3DVERTEXELEMENT9 e[]={{0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},{0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0},{0,24,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},{0,32,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,1},D3DDECL_END()};if(FAILED(d->CreateVertexDeclaration(e,&declaration)))return;}
 if(!state&&FAILED(d->CreateStateBlock(D3DSBT_ALL,&state)))return;
 if(prepared!=renderFrameSerial||preparedStyle!=clothingStyle){
  rendered.resize(count);for(unsigned i=0;i<count;i++){
   for(unsigned k=0;k<4;k++)if(source[i].weight[k]&&!rig.valid[source[i].bone[k]])return;
   auto p=NcSkin(source[i],rig);auto& v=rendered[i];v.p[0]=p.x;v.p[1]=p.y;v.p[2]=p.z;
   for(unsigned a=0;a<3;a++){v.n[a]=0;for(unsigned k=0;k<4;k++)if(source[i].weight[k])for(unsigned b=0;b<3;b++)v.n[a]+=source[i].weight[k]/255.f*rig.matrix[source[i].bone[k]][a*4+b]*normals[i][b];}
   memcpy(v.uv,uv[i],8);v.color[0]=source[i].p[2]<5?5.f:4.f;v.color[1]=v.color[2]=1;v.color[3]=1;
  }
  void* raw=nullptr;if(FAILED(vb->Lock(0,0,&raw,D3DLOCK_DISCARD)))return;memcpy(raw,rendered.data(),count*sizeof(RenderVertex));vb->Unlock();prepared=renderFrameSerial;preparedStyle=clothingStyle;
 }
 auto audit=MeridianStateSnapshot(d);if(FAILED(state->Capture()))return;
 d->SetVertexShader(JeansMaterial::vs);d->SetPixelShader(JeansMaterial::ps);d->SetVertexDeclaration(declaration);d->SetStreamSource(0,vb,0,sizeof(RenderVertex));d->SetIndices(ib[open]);d->SetVertexShaderConstantF(0,local,4);d->SetVertexShaderConstantF(4,view,4);JeansMaterial::Apply(d);
 d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,JeansMaterial::additive?FALSE:TRUE);d->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);d->SetRenderState(D3DRS_STENCILENABLE,FALSE);d->SetRenderState(D3DRS_ALPHABLENDENABLE,JeansMaterial::additive);d->SetRenderState(D3DRS_COLORWRITEENABLE,JeansMaterial::additive?7:15);
 HRESULT hr=origDIP(d,D3DPT_TRIANGLELIST,0,0,count,0,faces),restore=state->Apply();if(FAILED(hr)||FAILED(restore))Log("Jeans draw failure=%08x restored=%08x",hr,restore);if(!audit.empty()&&audit!=MeridianStateSnapshot(d))Log("Jeans state audit MISMATCH frame=%ld",renderFrameSerial);
 static unsigned draws=0;if(++draws%240==1)Log("Jeans native draw frame=%ld bottom=%u vertices=%u triangles=%u",renderFrameSerial,clothingStyle,count,faces);
}
}
