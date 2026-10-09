#pragma once
#include <malemod/physics/anterior_capsule.hpp>
static bool AnteriorBodyEnabled(){return true;}
static V3 PDAnteriorBodyExtents(int s){
 return {max(PDSkinSupport(s,{1,0,0}).x,-PDSkinSupport(s,{-1,0,0}).x),
         max(PDSkinSupport(s,{0,1,0}).y,-PDSkinSupport(s,{0,-1,0}).y),
         max(PDSkinSupport(s,{0,0,1}).z,-PDSkinSupport(s,{0,0,-1}).z)};
}
static bool PDAnteriorBodyOwnsContact(int s,V3 a,V3 b,float radius){
 return AnteriorBodyEnabled()&&malemod::physics::AnteriorCapsule(pdPosition[pdBody0+s],a,b,PDAnteriorBodyExtents(s),radius).active;
}
static void PDAnteriorBodyRecovery(float dt,bool velocityOnly=false){
 if(!AnteriorBodyEnabled())return;
 for(int s=0;s<2;s++){
  int id=pdBody0+s;V3 extent=PDAnteriorBodyExtents(s);
  // Match existing measured pelvis/thigh calibration. The shared query uses
  // +X anterior and a transverse ellipse; it never chooses a rear contact.
  for(int j=0;j<3;j++){
   V3 a=j<2?pdThigh[j*2]:V3{3,0,70},b=j<2?pdThigh[j*2+1]:V3{5.4f,0,86};
   float radius=j<2?7.2f:6.4f;
   auto contact=malemod::physics::AnteriorCapsule(pdPosition[id],a,b,extent,radius);
   if(velocityOnly){
    V3 oldA=j<2?pdOldThigh[j*2]:a,oldB=j<2?pdOldThigh[j*2+1]:b;
    V3 velocity=((a-oldA)*(1-contact.station)+(b-oldB)*contact.station)/dt;
    pdVelocity[id]=malemod::physics::RemoveAnteriorInwardVelocity(contact,pdVelocity[id],velocity);
   }else{
    // A trapped existing pose recovers continuously. Move history with it so
    // projection supplies no launch impulse or damping of unrelated motion.
    float speed=max(24.f,4.f*max(extent.x,max(extent.y,extent.z)));
    V3 shift=malemod::physics::AnteriorRecovery(contact,speed*dt/(24.f*3.f));
    pdPosition[id]=pdPosition[id]+shift;pdOldPosition[id]=pdOldPosition[id]+shift;
   }
  }
 }
}
