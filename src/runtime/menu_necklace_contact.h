#pragma once
#include "menu_necklace_palette.h"
// The native title scene submits tags before the torso. Defer just the HDR tag
// draw until both current body palettes have arrived, then restore its original
// material/pass state. No previous-frame body pose or static chest inflation.
namespace MenuNecklace {
static IDirect3DStateBlock9* pending=nullptr;
static LONG frame=-3;
static NcRig rig;
static NcContactSolver solver;
static D3DPRIMITIVETYPE type;static INT base;static UINT minv,nv,start,count;
static void Release(){if(pending)pending->Release();pending=nullptr;frame=-3;rig=NcRig{};}
static bool Queue(IDirect3DDevice9* d,D3DPRIMITIVETYPE t,INT b,UINT m,UINT n,UINT s,UINT c){
 if(!IsHdrSceneColorPass(d))return false;
 Release();if(FAILED(d->CreateStateBlock(D3DSBT_ALL,&pending))||!pending)return false;
 frame=renderFrameSerial;type=t;base=b;minv=m;nv=n;start=s;count=c;return true;
}
static void Capture(IDirect3DDevice9* d,int section){
 if(!pending||frame!=renderFrameSerial)return;
 auto* layout=GetShaderLayout(d);const auto* palette=section?ncMenuBodyPalette1:ncMenuBodyPalette0;
 unsigned n=section?sizeof(ncMenuBodyPalette1):sizeof(ncMenuBodyPalette0);
 if(!layout||!layout->valid||layout->boneCount<n*3)return;
 float matrices[75*12];if(FAILED(d->GetVertexShaderConstantF(layout->boneRegister,matrices,n*3)))return;
 for(unsigned i=0;i<n;i++)if(palette[i]<128){memcpy(rig.matrix[palette[i]],matrices+i*12,48);rig.valid[palette[i]]=true;}
}
static void Draw(IDirect3DDevice9* d){
 if(!pending||frame!=renderFrameSerial)return;
 IDirect3DStateBlock9* body=nullptr;if(FAILED(d->CreateStateBlock(D3DSBT_ALL,&body))||!body)return;
 // WStart's full skin palette and LocalToWorld live above c127. A deferred
 // shader/state-block switch can leave the tag palette in those registers.
 // Preserve the complete native constants explicitly, including shadow/view
 // permutations; a same-shader pointer alone does not prove restoration.
 float bodyVS[256*4],bodyPS[224*4];
 if(FAILED(d->GetVertexShaderConstantF(0,bodyVS,256))||FAILED(d->GetPixelShaderConstantF(0,bodyPS,224))){body->Release();return;}
 auto audit=MeridianStateSnapshot(d);
 HRESULT hr=pending->Apply();auto* layout=GetShaderLayout(d);
 float original[108]{};bool saved=SUCCEEDED(hr)&&layout&&layout->valid&&layout->boneCount>=27&&SUCCEEDED(d->GetVertexShaderConstantF(layout->boneRegister,original,27));
 if(saved){
  for(unsigned i=0;i<sizeof(ncPalette3);i++){memcpy(rig.matrix[ncPalette3[i]],original+i*12,48);rig.valid[ncPalette3[i]]=true;}
  TankTopAdapter::Collision(solver);float shifts[4];NcContactStats stats;
  if(solver.solve(rig,shifts,stats)){
   float corrected[108];memcpy(corrected,original,sizeof(corrected));auto* spine=rig.matrix[6];
   for(unsigned i=0;i<4;i++){float amount=shifts[ncPalette3[i]-103];for(unsigned a=0;a<3;a++)corrected[i*12+a*4+3]+=spine[a*4]*amount;}
   d->SetVertexShaderConstantF(layout->boneRegister,corrected,27);
   static unsigned samples=0;if(++samples%120==1)Log("menu necklace surface contact top=%u points=%d residual=%.5f limited=%d currentFrame=%ld",topStyle,stats.contacts,stats.remaining,stats.limited,renderFrameSerial);
  }else Log("menu necklace current-pose contact unavailable frame=%ld",renderFrameSerial);
 }
 if(SUCCEEDED(hr))hr=origDIP(d,type,base,minv,nv,start,count);
 HRESULT restored=body->Apply();HRESULT restoredVS=d->SetVertexShaderConstantF(0,bodyVS,256),restoredPS=d->SetPixelShaderConstantF(0,bodyPS,224);body->Release();
 if(FAILED(hr)||FAILED(restored)||FAILED(restoredVS)||FAILED(restoredPS))Log("menu necklace deferred draw failure=%08x restored=%08x constants=%08x/%08x",hr,restored,restoredVS,restoredPS);
 if(!audit.empty()&&audit!=MeridianStateSnapshot(d))Log("menu necklace state audit MISMATCH frame=%ld",renderFrameSerial);
 Release();
}
}
