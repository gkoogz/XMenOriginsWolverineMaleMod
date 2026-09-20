// R14 render mesh. The original 2388-point cage still owns the established
// pelvis, shape and physics solver. This surface consumes its FINAL positions.
#include "r14_asset.h"
static IDirect3DVertexBuffer9* r14VB;
static IDirect3DIndexBuffer9* r14IB;
static V3 r14Positions[r14Count],r14Normals[r14Count],r14Tangents[r14Count];
static unsigned char r14Packed[r14Count*32];
static bool r14Ready=false,r14UploadPending=false;
static UINT r14SuccessfulDraws=0;
static float R14Scale(float ui){return ui<=50.f?.85f+.003f*ui:1.f+.012f*(ui-50.f);}
static void ReleaseR14(){
  if(r14VB){r14VB->Release();r14VB=nullptr;}
  if(r14IB){r14IB->Release();r14IB=nullptr;}
  r14UploadPending=true;
}
static void UpdateR14(const unsigned char* source){
  const float amount=R14Scale(glansUI)-1.f;
  static V3 delta[graftCount];
  for(UINT i=0;i<graftCount;i++)delta[i]=graftDeformedPositions[i]-V3{r14Reference[i*3],r14Reference[i*3+1],r14Reference[i*3+2]};
  for(UINT i=0;i<r14Count;i++){
    V3 p{r14Base[3*i]+amount*r14Growth[3*i],r14Base[3*i+1]+amount*r14Growth[3*i+1],r14Base[3*i+2]+amount*r14Growth[3*i+2]};
    for(UINT k=r14Offsets[i];k<r14Offsets[i+1];k++)p=p+delta[r14Sources[k]]*(r14Weight[k]+amount*r14GrowthWeight[k]);
    if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)){r14Ready=false;return;}
    r14Positions[i]=p;
  }
  memset(r14Normals,0,sizeof(r14Normals));memset(r14Tangents,0,sizeof(r14Tangents));
  for(UINT k=0;k<r14IndexCount;k+=3){
    UINT a=r14Indices[k],b=r14Indices[k+1],c=r14Indices[k+2];
    V3 e1=r14Positions[b]-r14Positions[a],e2=r14Positions[c]-r14Positions[a];
    V3 n=Cross(e2,e1);r14Normals[a]=r14Normals[a]+n;r14Normals[b]=r14Normals[b]+n;r14Normals[c]=r14Normals[c]+n;
    float du1=r14UV[b*2]-r14UV[a*2],dv1=r14UV[b*2+1]-r14UV[a*2+1];
    float du2=r14UV[c*2]-r14UV[a*2],dv2=r14UV[c*2+1]-r14UV[a*2+1],det=du1*dv2-dv1*du2;
    if(fabsf(det)>1e-10f){V3 t=(e1*dv2-e2*dv1)/det;r14Tangents[a]=r14Tangents[a]+t;r14Tangents[b]=r14Tangents[b]+t;r14Tangents[c]=r14Tangents[c]+t;}
  }
  V3 columns[3]={{1,0,0},{0,1,0},{0,0,1}};
  for(UINT j=0;j<r14FrameCount;j++)for(int a=0;a<3;a++)columns[a]=columns[a]+delta[r14FrameSources[j]]*r14FrameLinear[a*r14FrameCount+j];
  for(UINT i=0;i<r14Count;i++){
    V3 n=Unit(r14Normals[i]);
    if(r14CustomNormals[i*3]!=0.f||r14CustomNormals[i*3+1]!=0.f||r14CustomNormals[i*3+2]!=0.f){
      V3 custom{r14CustomNormals[i*3],r14CustomNormals[i*3+1],r14CustomNormals[i*3+2]};
      n=Unit(Cross(columns[1],columns[2])*custom.x+Cross(columns[2],columns[0])*custom.y+Cross(columns[0],columns[1])*custom.z);
    }
    V3 t=r14Tangents[i]-n*Dot(n,r14Tangents[i]);
    if(Length(t)<1e-6f)t=Cross(fabsf(n.z)<.9f?V3{0,0,1}:V3{0,1,0},n);
    t=Unit(t);unsigned char* v=r14Packed+i*32;memcpy(v,&r14Positions[i],12);
    v[12]=PackSigned(t.x);v[13]=PackSigned(t.y);v[14]=PackSigned(t.z);v[15]=127;
    v[16]=PackSigned(n.x);v[17]=PackSigned(n.y);v[18]=PackSigned(n.z);v[19]=0;
    // Preserve exact normals at the body attachment, including its weld locks.
    if(r14Flex[i]<.04f&&r14Offsets[i+1]-r14Offsets[i]==1&&fabsf(r14Weight[r14Offsets[i]]-1.f)<1e-6f)
      memcpy(v+12,source+r14Sources[r14Offsets[i]]*32+12,8);
    memcpy(v+20,r14Bones+i*4,4);memcpy(v+24,r14Weights+i*4,4);
    D3DXFloat32To16Array(reinterpret_cast<D3DXFLOAT16*>(v+28),r14UV+i*2,2);
  }
  r14Ready=true;r14UploadPending=true;
}
static bool EnsureR14(IDirect3DDevice9* d){
  if(!r14Ready)return false;
  if(!r14VB){
    if(FAILED(d->CreateVertexBuffer(sizeof(r14Packed),D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&r14VB,nullptr)))return false;
    r14UploadPending=true;
  }
  if(!r14IB){
    if(FAILED(d->CreateIndexBuffer(sizeof(r14Indices),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&r14IB,nullptr)))return false;
    void* raw=nullptr;if(FAILED(r14IB->Lock(0,0,&raw,0))){ReleaseR14();return false;}
    memcpy(raw,r14Indices,sizeof(r14Indices));r14IB->Unlock();
  }
  if(r14UploadPending){
    void* raw=nullptr;if(FAILED(r14VB->Lock(0,sizeof(r14Packed),&raw,D3DLOCK_DISCARD)))return false;
    memcpy(raw,r14Packed,sizeof(r14Packed));r14VB->Unlock();r14UploadPending=false;
  }
  return true;
}
static HRESULT DrawR14(IDirect3DDevice9* d,IDirect3DVertexBuffer9* original,UINT offset,UINT stride){
  IDirect3DIndexBuffer9* ib=nullptr;
  if(FAILED(d->GetIndices(&ib)))return E_FAIL;
  HRESULT hr=d->SetStreamSource(0,r14VB,0,32);
  if(SUCCEEDED(hr))hr=d->SetIndices(r14IB);
  if(SUCCEEDED(hr)){
    bool capture=captureRemaining>0&&captureDraw<16;
    if(capture){CaptureDraw(d,D3DPT_TRIANGLELIST,0,0,r14Count,0,r14IndexCount/3,r14Packed,sizeof(r14Packed),r14Indices,sizeof(r14Indices));CaptureSurface(d,"before");}
    hr=capture?origDIP(d,D3DPT_TRIANGLELIST,0,0,r14Count,0,r14IndexCount/3):DrawWithLightingDirections(d,D3DPT_TRIANGLELIST,0,0,r14Count,0,r14IndexCount/3);
    if(capture)CaptureSurface(d,"after");
  }
  d->SetStreamSource(0,original,offset,stride);d->SetIndices(ib);if(ib)ib->Release();
  if(SUCCEEDED(hr)){if(r14SuccessfulDraws++==0)Log("R14 replacement draw active: %u vertices, %u triangles",r14Count,r14IndexCount/3);}
  return hr;
}
