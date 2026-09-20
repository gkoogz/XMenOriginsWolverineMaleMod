#include "lighting_direction_shaders.h"
// Correct only the demonstrated shader pair, only while rendering R14.
// View vector and skin lobe retain all material strengths and sampled normals.
// Carry all three transformed basis vectors to the pixel shader instead of
// reconstructing a missing row using interpolated tangent handedness.
static bool lightingDirectionsEnabled=true;
static IDirect3DVertexShader9* lightingDirectionsVS=nullptr;
static IDirect3DPixelShader9* lightingDirectionsPS=nullptr;
struct LightingPairMatch {IDirect3DVertexShader9* vs;IDirect3DPixelShader9* ps;bool matched;};
static std::vector<LightingPairMatch> lightingPairs;
static UINT lightingDirectionsDraws=0;
template<class T,size_t N> static bool LightingShaderMatches(T* shader,const DWORD (&expected)[N]){
 if(!shader)return false;UINT bytes=0;
 if(FAILED(shader->GetFunction(nullptr,&bytes))||bytes!=sizeof(expected))return false;
 std::vector<DWORD> code(N);return SUCCEEDED(shader->GetFunction(code.data(),&bytes))&&memcmp(code.data(),expected,bytes)==0;
}
static void ReleaseLightingDirections(){
 for(auto& pair:lightingPairs){pair.vs->Release();pair.ps->Release();}lightingPairs.clear();
 if(lightingDirectionsVS){lightingDirectionsVS->Release();lightingDirectionsVS=nullptr;}
 if(lightingDirectionsPS){lightingDirectionsPS->Release();lightingDirectionsPS=nullptr;}
}
static bool MatchLightingPair(IDirect3DVertexShader9* vs,IDirect3DPixelShader9* ps){
 if(!vs||!ps)return false;
 for(auto& pair:lightingPairs)if(pair.vs==vs&&pair.ps==ps)return pair.matched;
 bool match=LightingShaderMatches(vs,skinOriginalVS)&&LightingShaderMatches(ps,skinOriginalPS);
 if(lightingPairs.size()<64){vs->AddRef();ps->AddRef();lightingPairs.push_back({vs,ps,match});Log("R14 lighting directions shader pair %s",match?"MATCHED":"unmatched; original retained");}
 return match;
}
static HRESULT DrawWithLightingDirections(IDirect3DDevice9* d,D3DPRIMITIVETYPE type,INT base,UINT minv,UINT nv,UINT start,UINT count){
 if(!lightingDirectionsEnabled)return origDIP(d,type,base,minv,nv,start,count);
 IDirect3DVertexShader9* vs=nullptr;IDirect3DPixelShader9* ps=nullptr;d->GetVertexShader(&vs);d->GetPixelShader(&ps);
 bool ready=MatchLightingPair(vs,ps);
 if(ready&&!lightingDirectionsVS)ready=SUCCEEDED(d->CreateVertexShader(skinFixedVS,&lightingDirectionsVS));
 if(ready&&!lightingDirectionsPS)ready=SUCCEEDED(d->CreatePixelShader(skinFixedPS,&lightingDirectionsPS));
 bool applied=false;
 if(ready){
  HRESULT a=d->SetVertexShader(lightingDirectionsVS),b=SUCCEEDED(a)?d->SetPixelShader(lightingDirectionsPS):E_FAIL;
  applied=SUCCEEDED(a)&&SUCCEEDED(b);
  if(!applied){d->SetVertexShader(vs);d->SetPixelShader(ps);}
 }
 HRESULT result=origDIP(d,type,base,minv,nv,start,count);
 if(applied){d->SetVertexShader(vs);d->SetPixelShader(ps);if(lightingDirectionsDraws++==0)Log("R14 lighting directions correction ACTIVE; original lighting constants retained");}
 if(vs)vs->Release();if(ps)ps->Release();return result;
}
