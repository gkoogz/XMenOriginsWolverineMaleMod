#pragma once
#if __has_include(<malemod/physics/anterior_envelope.hpp>)
#include <malemod/physics/anterior_envelope.hpp>
// Adapter calibration only. The numerical anterior-envelope query lives in Base.
static bool AnteriorEnvelopeEnabled(){
 return true;
}
static V3 PDEnvelopeRadii(){
 V3 radius{};
 for(int s=0;s<2;s++){
  V3 extent=PDSkinExtents(s);radius.x=max(radius.x,extent.x);radius.y=max(radius.y,extent.y);radius.z=max(radius.z,extent.z);
 }return radius;
}
static bool PDAnteriorOwnsContact(V3 point,V3 rodToLobe,float radius){
 return AnteriorEnvelopeEnabled()&&rodToLobe.x>0&&malemod::physics::AnteriorEnvelope(point,pdPosition[pdBody0],pdPosition[pdBody0+1],PDEnvelopeRadii(),radius).active;
}
static void PDAnteriorEnvelope(float dt){
 if(!AnteriorEnvelopeEnabled())return;
 PDSync();
 V3 radius=PDEnvelopeRadii();
 for(int i=2;i<shaftNodeCount;i++){
  auto c=malemod::physics::AnteriorEnvelope(pdPosition[i],pdPosition[pdBody0],pdPosition[pdBody0+1],radius,logicalShaftBodyRadius*1.4f);
  if(!c.active)continue;
  // Bounded nonimpulsive recovery preserves momentum and free tangential motion.
  auto correction=malemod::physics::DistributeEnvelopeCorrection<V3>(c,min(-c.gap,12.f*dt/24.f),pdInvMass[i],pdInvMass[pdBody0],pdInvMass[pdBody0+1]);
  V3 shifts[3]={correction.point,correction.first,correction.second};
  int ids[3]={i,pdBody0,pdBody0+1};
  for(int k=0;k<3;k++){pdPosition[ids[k]]=pdPosition[ids[k]]+shifts[k];pdOldPosition[ids[k]]=pdOldPosition[ids[k]]+shifts[k];}
 }
}
static void PDAnteriorEnvelopeVelocity(){
 if(!AnteriorEnvelopeEnabled())return;
 // Only cancel velocity entering the anterior web; lateral swing is retained.
 V3 radius=PDEnvelopeRadii();
 for(int i=2;i<shaftNodeCount;i++){
  auto c=malemod::physics::AnteriorEnvelope(pdPosition[i],pdPosition[pdBody0],pdPosition[pdBody0+1],radius,logicalShaftBodyRadius*1.4f+.02f);
  if(!c.active)continue;
  auto change=malemod::physics::CancelEnvelopeInwardVelocity(c,pdVelocity[i],pdVelocity[pdBody0],pdVelocity[pdBody0+1],pdInvMass[i],pdInvMass[pdBody0],pdInvMass[pdBody0+1]);
  pdVelocity[i]=pdVelocity[i]+change.point;
  pdVelocity[pdBody0]=pdVelocity[pdBody0]+change.first;
  pdVelocity[pdBody0+1]=pdVelocity[pdBody0+1]+change.second;
 }
}

#else
// An older pinned Base retains its original numerical path. Private candidates
// explicitly overlay and hash the new shared header; ordinary builds must not
// acquire an unpinned local implementation.
static bool AnteriorEnvelopeEnabled(){return false;}
static bool PDAnteriorOwnsContact(V3,V3,float){return false;}
static void PDAnteriorEnvelope(float){}
static void PDAnteriorEnvelopeVelocity(){}
#endif
