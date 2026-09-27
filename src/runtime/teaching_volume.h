#pragma once
#include "fluid_types.h"
#include "clear_strand.h"
#include "thread_mesh.h"
#include "viscous_thread.h"
namespace teaching {
struct Fluid {
 volumeFluid::Settings settings;std::string error;
 explicit Fluid(bool =true){}
 bool ready=false;double accumulator=0,clock=0;V3 origin{},lastTip{},lastDir{1,0,0};
 unsigned emitted[6]{},steps=0;double emittedVolume=0;float viscosity=2,cohesion=1.8f,spread=1;
 volumeFluid::F4 collision[8]{};double flowCDF[513]{};float flowValues[513]{};double flowArea=1;
 ClearStrand clear[2];ViscousThread streams[4];LiquidMesh mesh;
 std::vector<volumeFluid::Particle> surface;double delivered[4]{};
 void Clear(){ready=false;accumulator=clock=0;steps=0;emittedVolume=0;memset(emitted,0,sizeof(emitted));memset(delivered,0,sizeof(delivered));surface.clear();mesh.Clear();for(auto& s:clear)s.Reset();for(auto& s:streams)s.Reset();}
 bool Prepare(){settings=volumeFluid::config;
  if(!std::isfinite(settings.volume)||settings.volume<=0||settings.feed<.15f||settings.feed>10||settings.nozzle<.15f||settings.viscosity<0||settings.meshSides<8||settings.meshSides>20){error="Invalid stream configuration.";return false;}
  float speed=settings.volume/settings.feed/(3.14159265f*settings.nozzle*settings.nozzle*.88f)*(1+settings.flowVariation);
  if(speed*min(4,(int)ceilf(settings.feed/1.5f))>250){error="Flow speed exceeds 250 game units/s. Increase nozzle radius or duration.";return false;}
  sequenceEnd=max(max(20.f,11.5f+settings.feed+settings.lifetime+1.f),4.5f+settings.dropDuration+settings.dropHold+settings.lifetime+1.f);return true;
 }
 bool Begin(V3 tip){Clear();if(!Prepare())return false;origin=lastTip=tip;ready=true;error.clear();
  flowCDF[0]=0;for(int i=0;i<=512;i++){float x=i/512.f;flowValues[i]=FlowRate(x)*(1+settings.flowVariation*sinf(x*6.2831853f*3));if(i)flowCDF[i]=flowCDF[i-1]+(flowValues[i-1]+flowValues[i])*.5/512;}flowArea=flowCDF[512];for(double& f:flowCDF)f/=flowArea;return true;
 }
 static float EventTime(int e){return e==0?2.5f:e==1?4.5f:7.f+1.5f*(e-2);}
 // Cumulative smooth-ramp flow, normalized to exactly one requested event volume.
 static double FlowIntegral(double x){x=x<0?0:x>1?1:x;const double r=.12,area=1-r;if(x<r){double u=x/r;return r*(u*u*u-.5*u*u*u*u)/area;}if(x>1-r)return 1-FlowIntegral(1-x);return (x-r*.5)/area;}
 static float FlowRate(float x){if(x<=0||x>=1)return 0;float u=min(1.f,min(x,1-x)/.12f);return u*u*(3-2*u)/.88f;}
 double Cumulative(double x) const {x=x<0?0:x>1?1:x;double q=x*512;int i=min(511,(int)q);return flowCDF[i]+(flowCDF[i+1]-flowCDF[i])*(q-i);}
 float Rate(float x) const {float q=Clamp(x,0,1)*512;int i=min(511,(int)q);return float((flowValues[i]+(flowValues[i+1]-flowValues[i])*(q-i))/flowArea);}

 bool Tick(float dt,V3 tip,V3 dir,V3 inherited){
  double next=clock+dt;
  for(int e=0;e<2;e++){
   double fraction=FlowIntegral((next-EventTime(e))/settings.dropDuration);
   clear[e].Step(dt,float(next-EventTime(e)),tip,dir,settings,collision,(float)fraction,settings.dropVolume,origin);
   emittedVolume+=settings.dropVolume*(fraction-FlowIntegral((clock-EventTime(e))/settings.dropDuration));emitted[e]=fraction>=1?1:0;
  }
  double targets[4]{},amount[4]{};float speeds[4]{};bool feed[4]{};
  float radius=min(settings.nozzle,powf(settings.volume/18.f,1.f/3));
  int overlap=min(4,(int)ceilf(settings.feed/1.5f));
  float peak=settings.volume/settings.feed/(3.14159265f*radius*radius*.88f)*(1+settings.flowVariation)*overlap;
  unsigned stride=peak>settings.threadSpacing*60?1:2;bool terminal=false;
  for(int e=0;e<4;e++){
   double fraction=Cumulative((next-EventTime(e+2))/settings.feed);targets[e]=settings.volume*fraction;
   int channel=settings.feed>=1.5f?0:e;
   amount[channel]+=max(0.,targets[e]-delivered[e]);
   float mid=(float)((clock+next)*.5-EventTime(e+2))/settings.feed;
   if(mid>0&&mid<1)speeds[channel]+=settings.volume/settings.feed*(.92f+.16f*Rate(mid))/(3.14159265f*radius*radius);
   feed[channel]=feed[channel]||(fraction>0&&fraction<1);
   if(fraction>=1&&targets[e]>delivered[e]+1e-9)terminal=true;
  }
  bool birth=(steps%stride)==stride-1||terminal;
  for(int e=0;e<4;e++){
   auto& stream=streams[e];stream.Step(dt,(float)next,settings,collision,origin);
   if(birth&&amount[e]>1e-9)stream.Feed((float)amount[e],tip,dir,inherited+dir*min(settings.speedLimit,speeds[e]),(float)next,dt*stride);
   stream.feeding=feed[e];
  }
  if(birth)for(int e=0;e<4;e++){if(targets[e]>delivered[e]+1e-9)++emitted[e+2];emittedVolume+=targets[e]-delivered[e];delivered[e]=targets[e];}
  clock=next;++steps;return true;
 }
 void BuildMesh(){
  mesh.Clear();surface.clear();for(const auto& stream:streams)stream.Append(mesh,settings.meshSides,surface);mesh.opaqueIndices=(unsigned)mesh.indices.size();
  for(const auto& strand:clear)if(strand.live&&strand.volume>1e-7f){
   std::vector<V3> p(strand.p,strand.p+strand.count);std::vector<float> r;float integral=0;
   for(int i=0;i<strand.count;i++){float t=float(i)/(strand.count-1);r.push_back(.7f+.9f*powf(t,6));if(i)integral+=Length(p[i]-p[i-1])*(r[i]*r[i]+r[i-1]*r[i-1])*.5f;}
   float scale=sqrtf(strand.volume/(3.14159265f*max(.0001f,integral)));for(float& a:r)a*=scale;
   mesh.Tube(p,r,settings.meshSides,strand.volume);for(size_t i=0;i<p.size();i++)surface.push_back({p[i],3,{},r[i]});
  }
 }
 void Advance(float dt,V3 tip,V3 dir,V3){
  if(!ready||!std::isfinite(dt)||dt<=0)return;if(dt>.25f||Length(tip-lastTip)>100){Clear();return;}
  V3 velocity=(tip-lastTip)/dt;float speed=Length(velocity);if(speed>settings.speedLimit)velocity=velocity*(settings.speedLimit/speed);
  accumulator+=dt;const double fixed=1.0/120.0;int n=(int)((accumulator+1e-8)/fixed);
  for(int k=0;k<n;k++){float t=(k+1.f)/max(1.f,float(n));if(!Tick((float)fixed,lastTip+(tip-lastTip)*t,Unit(lastDir+(dir-lastDir)*t),velocity))return;}
  accumulator-=n*fixed;lastTip=tip;lastDir=dir;if(n)BuildMesh();
 }
 int Live()const {int n=0;for(const auto& s:streams)if(s.live)n+=(int)s.nodes.size();return ready?n+(clear[0].live?25:0)+(clear[1].live?25:0):0;}
};
}
