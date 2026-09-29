#pragma once
#include "clinical_body_data.h"
// Anatomical surface study. Baseline 1.9 positions remain the reference so a
// difficult proximal triangle cannot attenuate the unchanged distal surface.
static V3 clinicalDelta[rsCount],clinicalBodyDelta[paBodyNormalCount];
static V3 clinicalReference[rsCount];
static float clinicalMaterial[rsCount],clinicalMask[rsCount];
static float clinicalFraction=1.f;
static unsigned char* clinicalBodyBuffer=nullptr;
static float ClinicalProximal(float t){return 1.f-Smoother01((t-.04f)/.34f);}
static float ClinicalTubeRadius(float t){return RapheRadius(t)*(1.f+1.2f*ClinicalProximal(t));}
static V3 ClinicalPelvicTarget(V3 p){
 V3 q=p-rapheCollarRoot;float radius=logicalShaftBodyRadius;
 float axial=Dot(q,rapheCollarAxis),t=axial/max(8.f,constraintRestLength);
 V3 radial=q-rapheCollarAxis*axial;float rho=Length(radial);if(rho<1e-5f||t>=.38f)return {};
 V3 direction=radial/rho;float down=Dot(direction,rapheCollarDown),upper=(1.f-down)*.5f;
 float growth=Smoother01((radius-2.9f)/4.72f);
 float reach=2.5f+3.f*growth+(2.f+4.f*growth)*upper;
 float support=Smoother01((axial+reach)/max(1.f,reach*.6f))*ClinicalProximal(t);
 support*=Smoother01((p.x-3.f)/3.f);
 support*=1.f-Smoother01((rho-(radius+2.f+(2.f+3.f*growth)*upper))/2.f);
 float u=max(0.f,min(1.f,(axial+reach)/(reach+4.5f+2.5f*growth)));
 float barrel=radius*1.025f+(.25f+(.9f+1.5f*growth)*upper)*(1-u)*(1-u)*(1-u);
 float center=radius*.985f,r=ClinicalTubeRadius(max(0.f,t))+RapheCover();
 float projection=center*down,disc=r*r-center*center+projection*projection;
 float lower=disc>=0.f&&projection>0.f?projection+sqrtf(max(0.f,disc)):0.f;
 return direction*min(radius*.95f,max(0.f,max(barrel,lower)-rho)*support);
}
static void ApplyClinicalCollar(unsigned char* body){
 if(!constraintSolverReady||!shaftRestFrameReady)return;
 clinicalBodyBuffer=body;
 static V3 before[rsCount],bodyBefore[paBodyNormalCount],next[rsCount];
 memcpy(before,rsPositions,sizeof(before));memcpy(bodyBefore,paBodyBefore,sizeof(bodyBefore));
 memcpy(clinicalReference,before,sizeof(clinicalReference));
 float radius=logicalShaftBodyRadius;
 GeometryFor(rsCount,[&](unsigned i){
  V3 p=before[i];float t=RapheClosest(p),ball=rapheSkinBall[i];
  clinicalMaterial[i]=t;clinicalMask[i]=ClinicalProximal(t);clinicalDelta[i]={};
  if(t>=.38f||ball>=.65f)return;
  auto f=RapheChainFrame(t);V3 q=p-f.p;float depth=Dot(q,f.down),y=Dot(q,f.side);
  float r=ClinicalTubeRadius(t)+RapheCover(),distance=sqrtf(y*y+(depth-RapheDepth(t))*(depth-RapheDepth(t)));
  V3 pelvic=ClinicalPelvicTarget(p);
  float proximal=Smoother01((t+.10f)/.10f),owner=1.f-Smoother01((ball-.25f)/.40f);
  V3 tube{};
  if(depth>radius*.35f&&distance>1e-5f&&distance<r){
   tube=(f.side*y+f.down*(depth-RapheDepth(t)))*(min(radius*.95f,r-distance)/distance);
   tube=tube*(proximal*ClinicalProximal(t)*owner);
  }
  float handoff=Smoother01(t/.12f);
  clinicalDelta[i]=pelvic*((1.f-handoff)*owner)+tube*handoff;
 });
 for(unsigned k=0;k<paBodyNormalCount;k++)clinicalBodyDelta[k]=ClinicalPelvicTarget(bodyBefore[k])*clinicalBodySupport[k];
 auto weld=[&](){
  V3 step[paBodyGroupCount]{};float best[paBodyGroupCount];for(auto& x:best)x=1e30f;
  for(unsigned k=0;k<paBodyNormalCount;k++){unsigned g=paBodyNormalGroup[k];float d=Length(clinicalBodyDelta[k]);if(d<best[g]){best[g]=d;step[g]=clinicalBodyDelta[k];}}
  for(unsigned k=0;k<paBodyNormalCount;k++)clinicalBodyDelta[k]=step[paBodyNormalGroup[k]];
  for(unsigned k=0;k<paSeamCount;k++)clinicalDelta[paSeamR14[k]]=clinicalBodyDelta[paSeamBody[k]];
 };
 weld();
 // The authored mesh topology never changes. Store each vertex's incident
 // edge occurrences in the original face traversal order once. Only vertices
 // with a nonzero field can move; zero-support vertices stay exact boundaries.
 static unsigned edgeOffsets[rsCount+1],edgeNeighbors[rsIndexCount*2];
 static bool edgesReady=false;
 if(!edgesReady){
  for(unsigned k=0;k<rsIndexCount;k+=3)for(unsigned j=0;j<3;j++){
   ++edgeOffsets[rsIndices[k+j]+1];++edgeOffsets[rsIndices[k+(j+1)%3]+1];
  }
  for(unsigned i=1;i<=rsCount;i++)edgeOffsets[i]+=edgeOffsets[i-1];
  unsigned cursor[rsCount];memcpy(cursor,edgeOffsets,sizeof(cursor));
  for(unsigned k=0;k<rsIndexCount;k+=3)for(unsigned j=0;j<3;j++){
   unsigned a=rsIndices[k+j],b=rsIndices[k+(j+1)%3];
   edgeNeighbors[cursor[a]++]=b;edgeNeighbors[cursor[b]++]=a;
  }
  edgesReady=true;
 }
 static unsigned fairActive[rsCount];unsigned fairCount=0;
 for(unsigned i=0;i<rsCount;i++)if(Length(clinicalDelta[i])>0.f)fairActive[fairCount++]=i;
 // The source cage is fixed for all eight relaxation passes in this frame.
 // Cache its edge weights once; only the displacement field changes.
 static float edgeWeights[rsIndexCount*2],edgeTotals[rsCount];
 for(unsigned k=0;k<fairCount;k++){
  unsigned i=fairActive[k];float total=0.f;
  for(unsigned e=edgeOffsets[i];e<edgeOffsets[i+1];e++){
   unsigned j=edgeNeighbors[e];float w=1.f/max(.15f,Length(before[i]-before[j]));
   edgeWeights[e]=w;total+=w;
  }
  edgeTotals[i]=total;
 }
 // Fair the one displacement field without moving its zero-support boundary.
 for(int pass=0;pass<8;pass++){
  for(unsigned k=0;k<fairCount;k++){
   unsigned i=fairActive[k];V3 weighted{};
   for(unsigned e=edgeOffsets[i];e<edgeOffsets[i+1];e++){
    unsigned j=edgeNeighbors[e];weighted=weighted+clinicalDelta[j]*edgeWeights[e];
   }
   float total=edgeTotals[i];
   next[i]=Length(clinicalDelta[i])>0.f&&total>0.f?clinicalDelta[i]*.8f+weighted*(.2f/total):clinicalDelta[i];
  }
  for(unsigned k=0;k<fairCount;k++){unsigned i=fairActive[k];clinicalDelta[i]=next[i];}
  weld();
 }
 static unsigned active[rsIndexCount/3];static PreparedSurfaceLimit limits[rsIndexCount/3];unsigned count=0;
 for(unsigned k=0;k<rsIndexCount;k+=3){unsigned a=rsIndices[k],b=rsIndices[k+1],c=rsIndices[k+2];
  if(Length(clinicalDelta[a])+Length(clinicalDelta[b])+Length(clinicalDelta[c])==0.f)continue;
  active[count]=k;limits[count++].Prepare(before[a],before[b],before[c]);
 }
 auto safe=[&](unsigned face,float floor){unsigned k=active[face],a=rsIndices[k],b=rsIndices[k+1],c=rsIndices[k+2];return limits[face].Evaluate(clinicalDelta[a],clinicalDelta[b],clinicalDelta[c],floor);};
 for(int pass=0;pass<32;pass++){
  bool changed=false;
  for(unsigned face=0;face<count;face++){float f=safe(face,.12f);if(f<.9999f){unsigned k=active[face];for(int j=0;j<3;j++)clinicalDelta[rsIndices[k+j]]=clinicalDelta[rsIndices[k+j]]*f;changed=true;}}
  for(unsigned k=0;k<paSeamCount;k++)clinicalBodyDelta[paSeamBody[k]]=clinicalDelta[paSeamR14[k]];
  for(unsigned k=0;k<paBodyFaceCount;k++){unsigned a=paBodyNormalFaces[k*3],b=paBodyNormalFaces[k*3+1],c=paBodyNormalFaces[k*3+2];float f=RSSafeFraction(bodyBefore[a],bodyBefore[b],bodyBefore[c],clinicalBodyDelta[a],clinicalBodyDelta[b],clinicalBodyDelta[c],.12f);if(f<.9999f){clinicalBodyDelta[a]=clinicalBodyDelta[a]*f;clinicalBodyDelta[b]=clinicalBodyDelta[b]*f;clinicalBodyDelta[c]=clinicalBodyDelta[c]*f;changed=true;}}
  weld();if(!changed)break;
 }
 float fraction=1.f;
 for(unsigned face=0;face<count;face++){fraction=min(fraction,safe(face,.05f));unsigned k=active[face];for(int j=0;j<3;j++)rapheNormalNeeded[rsIndices[k+j]]=true;}
 for(unsigned k=0;k<paBodyFaceCount;k++){unsigned a=paBodyNormalFaces[k*3],b=paBodyNormalFaces[k*3+1],c=paBodyNormalFaces[k*3+2];fraction=min(fraction,RSSafeFraction(bodyBefore[a],bodyBefore[b],bodyBefore[c],clinicalBodyDelta[a],clinicalBodyDelta[b],clinicalBodyDelta[c],.05f));}
 clinicalFraction=fraction;
 for(unsigned i=0;i<rsCount;i++){clinicalDelta[i]=clinicalDelta[i]*fraction;rsPositions[i]=before[i]+clinicalDelta[i];}
 for(unsigned k=0;k<paBodyNormalCount;k++){clinicalBodyDelta[k]=clinicalBodyDelta[k]*fraction;paBodyBefore[k]=bodyBefore[k]+clinicalBodyDelta[k];memcpy(body+paBodyNormalIDs[k]*32,&paBodyBefore[k],12);}
}
