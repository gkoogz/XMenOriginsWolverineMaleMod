#pragma once
// Final shared render skin. The established cage/pouch/tube still owns motion;
// this locally rebuilt junction consumes its final surface, including body fields.
static V3 nrPositions[nrCount],nrNormals[nrCount],nrTangents[nrCount];
#include "clinical_neck_data.h"
#include "clinical_root_follow_data.h"
static bool clinicalNeckMoved[nrCount];
static V3 clinicalNeckReference[nrCount];
static void FairClinicalNeck(){
 memset(clinicalNeckMoved,0,sizeof(clinicalNeckMoved));
 if(!clinicalBodyBuffer)return;
 static float mask[nrCount],ballWeight[nrCount];static V3 initial[nrCount],storage[3][nrCount];static unsigned active[nrCount];unsigned count=0;
 for(unsigned i=0;i<nrCount;i++){
  float t=0,ball=0;
  for(unsigned j=0;j<3;j++){unsigned s=nrMaterialSources[i*3+j];float w=nrMaterialWeights[i*3+j];t+=clinicalMaterial[s]*w;ball+=rapheSkinBall[s]*w;}
  mask[i]=Smoother01((t+.12f)/.12f)*(1.f-Smoother01((t-.06f)/.30f))*(1.f-Smoother01((ball-.25f)/.40f));
  float support=ClinicalProximal(t)*(1.f-Smoother01((ball-.25f)/.40f));
  nrPositions[i]=clinicalNeckReference[i]+(nrPositions[i]-clinicalNeckReference[i])*support;
  ballWeight[i]=ball;
 }
 for(const auto& follow:clinicalRootFollow){
  V3 delta{};if(follow.a>=0)delta=delta+clinicalBodyDelta[follow.a]*follow.u;if(follow.b>=0)delta=delta+clinicalBodyDelta[follow.b]*follow.v;if(follow.c>=0)delta=delta+clinicalBodyDelta[follow.c]*follow.w;
  nrPositions[follow.vertex]=clinicalNeckReference[follow.vertex]+delta;mask[follow.vertex]=0;clinicalNeckMoved[follow.vertex]=true;
 }
 for(const auto& seam:clinicalNeckSeam){
  V3 a,b;memcpy(&a,clinicalBodyBuffer+seam.a*32,12);memcpy(&b,clinicalBodyBuffer+seam.b*32,12);
  nrPositions[seam.vertex]=a*(1.f-seam.u)+b*seam.u;mask[seam.vertex]=0;clinicalNeckMoved[seam.vertex]=true;
  clinicalNeckReference[seam.vertex]=nrPositions[seam.vertex];
 }
 for(unsigned i=0;i<nrCount;i++)if(mask[i]>1e-5f){active[count++]=i;clinicalNeckMoved[i]=true;}
 // Chebyshev evaluation of the same 200-pair Taubin filter. Work on
 // displacements so zero-support vertices remain bit-identical, not on large
 // world coordinates whose cancellation could introduce numerical drift.
 memset(storage,0,sizeof(storage));memset(initial,0,sizeof(initial));
 auto average=[&](const V3* data,unsigned i){unsigned start=clinicalNeckRows[i],end=clinicalNeckRows[i+1];V3 sum{};for(unsigned j=start;j<end;j++)sum=sum+data[clinicalNeckNeighbors[j]];return end>start?sum/float(end-start):data[i];};
 for(unsigned k=0;k<count;k++){unsigned i=active[k];initial[i]=(average(nrPositions,i)-nrPositions[i])*mask[i];}
 V3* b1=storage[0];V3* b2=storage[1];V3* next=storage[2];
 for(int coefficient=24;coefficient>=1;--coefficient){
  for(unsigned k=0;k<count;k++){unsigned i=active[k];V3 B=b1[i]*(1.f-mask[i])+average(b1,i)*mask[i];next[i]=B*2.f-b2[i]+initial[i]*clinicalFairCoefficients[coefficient];}
  V3* old=b2;b2=b1;b1=next;next=old;
 }
 for(unsigned k=0;k<count;k++){unsigned i=active[k];V3 B=b1[i]*(1.f-mask[i])+average(b1,i)*mask[i];nrPositions[i]=nrPositions[i]+B-b2[i]+initial[i]*clinicalFairCoefficients[0];}
 // The lobe boundary can contain narrow returning strips. Smoothing them
 // through themselves is not a valid way to remove a crease. Limit only those
 // strips against the original surface; do not reduce the whole collar.
 static bool limited[nrCount];
 static unsigned safetyFaces[nrIndexCount/3];unsigned safetyCount=0;
 for(unsigned k=0;k<nrIndexCount;k+=3){unsigned a=nrIndices[k],b=nrIndices[k+1],c=nrIndices[k+2];
  if((ballWeight[a]+ballWeight[b]+ballWeight[c])/3.f<=.4f)continue;
  if(Length(nrPositions[a]-clinicalNeckReference[a])==0.f&&
     Length(nrPositions[b]-clinicalNeckReference[b])==0.f&&
     Length(nrPositions[c]-clinicalNeckReference[c])==0.f)continue;
  safetyFaces[safetyCount++]=k;
 }
 for(int pass=0;pass<80;pass++){
  memset(limited,0,sizeof(limited));bool any=false;
  for(unsigned face=0;face<safetyCount;face++){unsigned k=safetyFaces[face],a=nrIndices[k],b=nrIndices[k+1],c=nrIndices[k+2];
   V3 n=Cross(clinicalNeckReference[b]-clinicalNeckReference[a],clinicalNeckReference[c]-clinicalNeckReference[a]);float area=Dot(n,n);if(area<1e-12f)continue;
   V3 current=Cross(nrPositions[b]-nrPositions[a],nrPositions[c]-nrPositions[a]);
   if(Dot(n,current)<area*.12f){limited[a]=limited[b]=limited[c]=true;any=true;}
  }
  if(!any)break;
  for(unsigned i=0;i<nrCount;i++)if(limited[i])nrPositions[i]=clinicalNeckReference[i]+(nrPositions[i]-clinicalNeckReference[i])*.75f;
 }
 for(unsigned i=0;i<nrCount;i++)memcpy(nrPacked+i*32,&nrPositions[i],12);
}
static void UpdateNeckRender(){PerfScope perf(13);
 static float unpack[256];static bool unpackReady=false;
 if(!unpackReady){for(unsigned i=0;i<256;i++)unpack[i]=i/255.f*2-1;unpackReady=true;}
 static bool attributesReady=false;
 if(!attributesReady){
  for(unsigned i=0;i<nrCount;i++)memcpy(nrPacked+i*32+12,nrAttributes+i*20,20);
  attributesReady=true;
 }
 static V3 source[rsCount];
 static __m128 sourceXYZ[rsCount];
 static __m128 referenceXYZ[rsCount];
 for(unsigned i=0;i<rsCount;i++){
  memcpy(&source[i],rsPacked+i*32,12);
  sourceXYZ[i]=_mm_set_ps(0.f,source[i].z,source[i].y,source[i].x);
  V3 ref=clinicalReference[i];referenceXYZ[i]=_mm_set_ps(0.f,ref.z,ref.y,ref.x);
 }
 GeometryFor(nrCount,[&](unsigned i){
  if(nrDirect[i]!=65535){memcpy(nrPacked+i*32,rsPacked+nrDirect[i]*32,32);nrPositions[i]=source[nrDirect[i]];clinicalNeckReference[i]=clinicalReference[nrDirect[i]];return;}
  // Evaluate XYZ together, retaining the exact weight accumulation order.
  __m128 sum=_mm_setzero_ps();
  for(unsigned j=nrRows[i];j<nrRows[i+1];j++)sum=_mm_add_ps(sum,_mm_mul_ps(sourceXYZ[nrSources[j]],_mm_set1_ps(nrWeights[j])));
  float xyz[4];_mm_storeu_ps(xyz,sum);V3 p{xyz[0],xyz[1],xyz[2]};
  __m128 reference=_mm_setzero_ps();for(unsigned j=nrRows[i];j<nrRows[i+1];j++)reference=_mm_add_ps(reference,_mm_mul_ps(referenceXYZ[nrSources[j]],_mm_set1_ps(nrWeights[j])));
  _mm_storeu_ps(xyz,reference);clinicalNeckReference[i]={xyz[0],xyz[1],xyz[2]};
  nrPositions[i]=p;memcpy(nrPacked+i*32,&p,12);
 });
#ifndef NO_CLINICAL_NECK
 FairClinicalNeck();
#endif
 static float uv[nrIndexCount/3][5];static bool uvReady=false;static std::vector<unsigned> changedFaces;
 if(!uvReady){for(unsigned k=0;k<nrIndexCount;k+=3){
  float tex[3][2];for(unsigned j=0;j<3;j++)D3DXFloat16To32Array(tex[j],reinterpret_cast<const D3DXFLOAT16*>(nrPacked+nrIndices[k+j]*32+28),2);
  float* d=uv[k/3];d[0]=tex[1][0]-tex[0][0];d[1]=tex[1][1]-tex[0][1];d[2]=tex[2][0]-tex[0][0];d[3]=tex[2][1]-tex[0][1];d[4]=d[0]*d[3]-d[1]*d[2];
  if(nrDirect[nrIndices[k]]==65535||nrDirect[nrIndices[k+1]]==65535||nrDirect[nrIndices[k+2]]==65535)changedFaces.push_back(k);
 }uvReady=true;}
 memset(nrNormals,0,sizeof(nrNormals));memset(nrTangents,0,sizeof(nrTangents));
 for(unsigned k=0;k<nrIndexCount;k+=3){
  unsigned a=nrIndices[k],b=nrIndices[k+1],c=nrIndices[k+2];V3 e=nrPositions[b]-nrPositions[a],g=nrPositions[c]-nrPositions[a],n=Cross(g,e);
  nrNormals[a]=nrNormals[a]+n;nrNormals[b]=nrNormals[b]+n;nrNormals[c]=nrNormals[c]+n;
  const float* d=uv[k/3];if(fabsf(d[4])>1e-10f){V3 t=(e*d[3]-g*d[1])/d[4];nrTangents[a]=nrTangents[a]+t;nrTangents[b]=nrTangents[b]+t;nrTangents[c]=nrTangents[c]+t;}
 }
 GeometryFor(nrCount,[&](unsigned i){
  if(nrDirect[i]!=65535&&!clinicalNeckMoved[i])return;
  V3 originalN{},originalT{};
  for(unsigned j=0;j<3;j++){const unsigned char* v=rsPacked+nrMaterialSources[i*3+j]*32;float w=nrMaterialWeights[i*3+j];
   originalN=originalN+V3{unpack[v[16]],unpack[v[17]],unpack[v[18]]}*w;
   originalT=originalT+V3{unpack[v[12]],unpack[v[13]],unpack[v[14]]}*w;
  }
  float blend=clinicalNeckMoved[i]?1.f:nrNormalMix[i];V3 n=Unit(Unit(originalN)*(1-blend)+Unit(nrNormals[i])*blend);
  V3 t=Unit(originalT)*(1-blend)+Unit(nrTangents[i])*blend;t=t-n*Dot(n,t);
  if(Length(t)<1e-6f)t=Cross(fabsf(n.z)<.9f?V3{0,0,1}:V3{0,1,0},n);t=Unit(t);
  unsigned char* v=nrPacked+i*32;v[12]=PackSigned(t.x);v[13]=PackSigned(t.y);v[14]=PackSigned(t.z);v[16]=PackSigned(n.x);v[17]=PackSigned(n.y);v[18]=PackSigned(n.z);
 });
}
