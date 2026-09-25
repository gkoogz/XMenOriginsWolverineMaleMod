#pragma once
#include "pouch_surface_data.h"
static V3 cpSurfaceBefore[rsCount],cpSurfaceNext[rsCount],cpSurfaceDelta[rsCount],cpTemplate[rsCount];
static V3 cpCenters[2],cpInverseBasis[2][3],cpWorldBasis[2][3],cpRenderRadii[2],cpSkinRadii[2];
static float cpSurfaceK=.55f;
static V3 CPLocalPoint(V3 p,int s){V3 d=p-cpCenters[s];return cpInverseBasis[s][0]*d.x+cpInverseBasis[s][1]*d.y+cpInverseBasis[s][2]*d.z;}
static V3 CPWorldPoint(V3 p,int s){return cpCenters[s]+cpWorldBasis[s][0]*p.x+cpWorldBasis[s][1]*p.y+cpWorldBasis[s][2]*p.z;}
static float CPPouchLevel(V3 local,V3 r){V3 a=CPDiv(local,r);float t=1.f-.13f*max(-1.f,min(1.f,a.z));a.x/=t;a.y/=t;return Length(a)-1.f;}
static float CPNormalizedRayLevel(V3 origin,V3 direction,float distance){
 float z=origin.z+direction.z*distance,t=1.f-.13f*max(-1.f,min(1.f,z));
 float x=(origin.x+direction.x*distance)/t,y=(origin.y+direction.y*distance)/t;
 return sqrtf(x*x+y*y+z*z)-1.f;
}
static float CPPouchField(V3 p){float a=CPPouchLevel(CPLocalPoint(p,0),cpSkinRadii[0]),b=CPPouchLevel(CPLocalPoint(p,1),cpSkinRadii[1]);float h=max(0.f,min(1.f,.5f+.5f*(b-a)/cpSurfaceK));return b*(1.f-h)+a*h-cpSurfaceK*h*(1.f-h);}
static void CPKeepSkinOutside(){
 for(unsigned k=0;k<cpActiveCount;k++){
  unsigned id=cpActive[k];V3 p=rsPositions[id];
  for(int s=0;s<2;s++){
   V3 local=CPLocalPoint(p,s);if(CPPouchLevel(local,cpRenderRadii[s])>=.018f)continue;
   for(int j=0;j<5;j++)local=local*(1.020f/max(1e-7f,1.f+CPPouchLevel(local,cpRenderRadii[s])));
   p=CPWorldPoint(local,s);
  }
  rsPositions[id]=p;
 }
}
static void ApplyPouchSurface(){
 if(!eggRestReady||!constraintSolverReady)return;CPEnsure();memcpy(cpSurfaceBefore,rsPositions,sizeof(cpSurfaceBefore));
 for(int s=0;s<2;s++){
  cpCenters[s]=CPCenter(s);cpRenderRadii[s]=CPRadii(s);V3 scale=CPDiv(cpRenderRadii[s],s?V3{5.724f,4.86f,7.81f}:V3{5.724f,4.86f,7.93f});
  cpSkinRadii[s]=cpRenderRadii[s]+CPMul({.80f,.95f,.65f},scale);
  for(int j=0;j<3;j++){V3 axis{j==0?1.f:0.f,j==1?1.f:0.f,j==2?1.f:0.f};cpInverseBasis[s][j]=CPInverse(axis,s);cpWorldBasis[s][j]=CPTransform(axis,s);}
 }
 V3 center=(cpCenters[0]+cpCenters[1])*.5f,span=cpCenters[1]-cpCenters[0];
 V3 pair[3]={RotateFromTo({1,0,0},{0,7.5f,.35f},span),RotateFromTo({0,1,0},{0,7.5f,.35f},span),RotateFromTo({0,0,1},{0,7.5f,.35f},span)};
 V3 scale=CPDiv((cpRenderRadii[0]+cpRenderRadii[1])*.5f,{5.724f,4.86f,7.87f});
 float atCenter=max(CPPouchLevel(CPLocalPoint(center,0),cpSkinRadii[0]),CPPouchLevel(CPLocalPoint(center,1),cpSkinRadii[1]));cpSurfaceK=max(.55f,4.f*max(0.f,atCenter)+.08f);
 float reach=30.f*max(scale.x,max(scale.y,scale.z));
 for(unsigned k=0;k<cpActiveCount;k++){
  unsigned id=cpActive[k];V3 d=CPMul({cpDirections[k*3],cpDirections[k*3+1],cpDirections[k*3+2]},scale);d=Unit(pair[0]*d.x+pair[1]*d.y+pair[2]*d.z);
  V3 origins[2],directions[2];
  for(int s=0;s<2;s++){
   origins[s]=CPDiv(CPLocalPoint(center,s),cpSkinRadii[s]);
   directions[s]=CPDiv(cpInverseBasis[s][0]*d.x+cpInverseBasis[s][1]*d.y+cpInverseBasis[s][2]*d.z,cpSkinRadii[s]);
  }
  float lo=0.f,hi=reach;for(int j=0;j<18;j++){
   float mid=(lo+hi)*.5f,a=CPNormalizedRayLevel(origins[0],directions[0],mid),b=CPNormalizedRayLevel(origins[1],directions[1],mid);
   float h=max(0.f,min(1.f,.5f+.5f*(b-a)/cpSurfaceK));
   if(b*(1.f-h)+a*h-cpSurfaceK*h*(1.f-h)<0.f)lo=mid;else hi=mid;
  }
  V3 target=center+d*((lo+hi)*.5f);rsPositions[id]=rsPositions[id]+(target-rsPositions[id])*cpWeights[k];
 }
 V3 anchor=(BallAnchor(0)+BallAnchor(1))*.5f;
 V3 lateral=Unit(span),up=Unit(anchor-center-lateral*Dot(anchor-center,lateral)),forward=Unit(Cross(lateral,up));
 float neckLength=max(.2f,Length(anchor-center)/19.11f);
 for(unsigned i=0;i<rsCount;i++){
  V3 local{cpReference[3*i]-18.85f,cpReference[3*i+1],cpReference[3*i+2]-64.275f};
  cpTemplate[i]=center+forward*(local.x*scale.x)+lateral*(local.y*scale.y)+up*(local.z*neckLength);
  cpSurfaceDelta[i]=rsPositions[i]-cpTemplate[i];
 }
 for(unsigned k=0;k<cpNeckCount;k++){
  unsigned id=cpNeckIDs[k];V3 target=cpTemplate[id];for(unsigned j=cpNeckRows[k];j<cpNeckRows[k+1];j++){const V3& v=cpSurfaceDelta[cpNeckSources[j]];float w=cpNeckWeights[j];target.x+=v.x*w;target.y+=v.y*w;target.z+=v.z*w;}
  rsPositions[id]=rsPositions[id]+(target-rsPositions[id])*cpNeckBlend[k];
 }
 for(int pass=0;pass<60;pass++){
  for(unsigned k=0;k<cpFairCount;k++){unsigned id=cpFair[k];V3 mean{};unsigned first=cpRows[id],last=cpRows[id+1];for(unsigned j=first;j<last;j++){const V3& v=rsPositions[cpNeighbors[j]];mean.x+=v.x;mean.y+=v.y;mean.z+=v.z;}cpSurfaceNext[id]=rsPositions[id]+(mean/float(last-first)-rsPositions[id])*(.4f*cpFairWeight[k]);}
  for(unsigned k=0;k<cpFairCount;k++)rsPositions[cpFair[k]]=cpSurfaceNext[cpFair[k]];
  if(pass%5==4)CPKeepSkinOutside();
 }
 // The collider and visible lower skin share the same pressure transform.
 CPKeepSkinOutside();
 for(unsigned i=0;i<r14Count;i++)r14Positions[i]=rsPositions[i];
}
