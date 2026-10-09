#pragma once
#include "tank_skin_shaders.h"
// Canonical maps exported losslessly from the installed gameplay material.
// Bind only the verified shader pair: sampler/register meanings are not assumed
// for depth, shadow, hair, electrode or other material permutations.
static IDirect3DTexture9* sharedSkinMaps[2]{};
// Map order is normal, specular. The captured native pixel shader decodes
// s4 as a tangent normal; s3 is the specular color multiplied by c13.
struct SkinMaterialLayout {int normal,specular,diffuse,subsurface,tiling;bool correctedBasis;};
static const SkinMaterialLayout skinMaterialLayouts[]={{4,3,2,10,14,true},{0,-1,1,2,-1,false},{1,3,2,-1,-1,false}};
struct SkinMaterialMatch {IDirect3DVertexShader9* vs;IDirect3DPixelShader9* ps;int layout;};
static std::vector<SkinMaterialMatch> skinMaterialMatches;
template<class T> static void TraceSkinShader(T* shader,const char* stage,unsigned index){
 char trace[8]{};if(GetEnvironmentVariableA("MALEMOD_MERIDIAN_LIGHT_TRACE",trace,8)!=1||trace[0]!='1')return;
 UINT bytes=0;if(FAILED(shader->GetFunction(nullptr,&bytes)))return;std::vector<DWORD> code((bytes+3)/4);if(FAILED(shader->GetFunction(code.data(),&bytes)))return;
 char name[80]{},path[MAX_PATH]{};sprintf_s(name,"SkinShader-%u-%s.bin",index,stage);SiblingPath(path,name);FILE* file=nullptr;if(!fopen_s(&file,path,"wb")&&file){fwrite(code.data(),1,bytes,file);fclose(file);}
 ID3DXBuffer* assembly=nullptr;if(SUCCEEDED(D3DXDisassembleShader(code.data(),FALSE,nullptr,&assembly))){sprintf_s(name,"SkinShader-%u-%s.txt",index,stage);SiblingPath(path,name);if(!fopen_s(&file,path,"wb")&&file){fwrite(assembly->GetBufferPointer(),1,assembly->GetBufferSize(),file);fclose(file);}assembly->Release();}
}
static int FindSkinMaterial(IDirect3DVertexShader9* vs,IDirect3DPixelShader9* ps){
 if(!vs||!ps)return -1;
 for(const auto& m:skinMaterialMatches)if(m.vs==vs&&m.ps==ps)return m.layout;
 int layout=-1;
 TraceSkinShader(vs,"vs",unsigned(skinMaterialMatches.size()));TraceSkinShader(ps,"ps",unsigned(skinMaterialMatches.size()));
 if(MatchLightingPair(vs,ps))layout=0;
 else if(LightingShaderMatches(vs,skinTankBaseVS)&&LightingShaderMatches(ps,skinTankBasePS))layout=1;
 else if(LightingShaderMatches(vs,skinTankSpotVS)&&LightingShaderMatches(ps,skinTankSpotPS))layout=2;
 if(skinMaterialMatches.size()<64){vs->AddRef();ps->AddRef();skinMaterialMatches.push_back({vs,ps,layout});if(layout>=0)Log("shared skin material adapter %d active",layout);}
 return layout;
}
static bool sharedSkinAttempted=false;
static void ReleaseSharedSkin(){
 for(auto& m:skinMaterialMatches){m.vs->Release();m.ps->Release();}skinMaterialMatches.clear();
 for(auto& p:sharedSkinMaps)if(p){p->Release();p=nullptr;}
 sharedSkinAttempted=false;
}
static bool LoadSharedSkin(IDirect3DDevice9* d){
 if(sharedSkinAttempted)return sharedSkinMaps[0]&&sharedSkinMaps[1];
 sharedSkinAttempted=true;
 const char* names[2]={"SharedBody-normal.dds","SharedBody-specular.dds"};
 for(int i=0;i<2;i++){
  char path[MAX_PATH];SiblingPath(path,names[i]);
  HRESULT hr=D3DXCreateTextureFromFileExA(d,path,D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,0,D3DFMT_UNKNOWN,D3DPOOL_DEFAULT,D3DX_FILTER_NONE,D3DX_FILTER_NONE,0,nullptr,nullptr,&sharedSkinMaps[i]);
  Log("shared body map %s load=%08X",names[i],hr);
 }
 return sharedSkinMaps[0]&&sharedSkinMaps[1];
}
static HRESULT DrawSharedSkin(IDirect3DDevice9* d,D3DPRIMITIVETYPE type,INT base,UINT minv,UINT nv,UINT start,UINT count,bool anatomy,IDirect3DTexture9* color=nullptr){
 IDirect3DVertexShader9* vs=nullptr;IDirect3DPixelShader9* ps=nullptr;d->GetVertexShader(&vs);d->GetPixelShader(&ps);
 int index=FindSkinMaterial(vs,ps);if(vs)vs->Release();if(ps)ps->Release();
 if(index<0)return origDIP(d,type,base,minv,nv,start,count);
 const auto& layout=skinMaterialLayouts[index];IDirect3DBaseTexture9* previous[16]{};bool saved[16]{};float tiling[4]{};bool tilingSaved=false;
 static bool traced[3]{};char trace[8]{};if(!traced[index]&&GetEnvironmentVariableA("MALEMOD_MERIDIAN_LIGHT_TRACE",trace,8)==1&&trace[0]=='1'){
  traced[index]=true;IDirect3DVertexDeclaration9* decl=nullptr;if(SUCCEEDED(d->GetVertexDeclaration(&decl))&&decl){D3DVERTEXELEMENT9 elements[MAXD3DDECLLENGTH]{};UINT n=MAXD3DDECLLENGTH;if(SUCCEEDED(decl->GetDeclaration(elements,&n)))for(UINT i=0;i<n;i++){auto e=elements[i];Log("Skin declaration layout=%d stream=%u offset=%u type=%u usage=%u index=%u",index,e.Stream,e.Offset,e.Type,e.Usage,e.UsageIndex);}decl->Release();}
 }
 auto bind=[&](int slot,IDirect3DBaseTexture9* texture){if(slot<0||!texture)return;if(FAILED(d->GetTexture(slot,&previous[slot])))return;saved[slot]=true;d->SetTexture(slot,texture);};
 if(LoadSharedSkin(d)){bind(layout.normal,sharedSkinMaps[0]);bind(layout.specular,sharedSkinMaps[1]);}
 bind(layout.diffuse,color);bind(layout.subsurface,color);
 if(layout.tiling>=0&&SUCCEEDED(d->GetPixelShaderConstantF(layout.tiling,tiling,1))){const float canonicalTiling[4]={16,16,0,1};tilingSaved=true;d->SetPixelShaderConstantF(layout.tiling,canonicalTiling,1);}
 if(anatomy&&captureTankMaterials)CaptureBoundDraw(d,type,base,minv,nv,start,count);
 HRESULT hr=anatomy&&layout.correctedBasis?DrawWithLightingDirections(d,type,base,minv,nv,start,count):origDIP(d,type,base,minv,nv,start,count);
 if(anatomy&&captureTankMaterials)CaptureSurface(d,"anatomy-after");
 for(int i=0;i<16;i++)if(saved[i]){d->SetTexture(i,previous[i]);if(previous[i])previous[i]->Release();}
 if(tilingSaved)d->SetPixelShaderConstantF(layout.tiling,tiling,1);
 return hr;
}
