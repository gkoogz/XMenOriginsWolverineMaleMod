#pragma once
#include <algorithm>
#include <string>
namespace volumeFluid {
template<class T> void Drop(T*& p){if(p){p->Release();p=nullptr;}}
struct Particle {V3 p;float live;V3 v;float age;};
struct F4 {float x,y,z,w;};
enum FluidImpactKind : unsigned char { FLUID_IMPACT_WORLD=0,FLUID_IMPACT_BODY=1 };
struct FluidImpact {
 V3 p,n;
 FluidImpactKind kind=FLUID_IMPACT_WORLD;
 unsigned char section=0;
 unsigned triangle=0;
 float baryU=0,baryV=0;
 void* shape=nullptr;
};
typedef bool (*CollisionSweep)(V3 from,V3 to,float radius,FluidImpact& hit);
static CollisionSweep collisionSweep=nullptr;
static constexpr int collisionCount=14;
struct Settings {
 float spacing=.32f,volume=180.f,feed=1.1f,nozzle=1.25f,viscosity=16.f,tension=12.f,lifetime=5.f;
 float pulseVolumeVariation=.35f,pulseDurationVariation=.65f,angleVariation=.55f,pulseTaper=.55f;
 float gravity=98.f,speedLimit=350.f;int capacity=32768,iterations=4,viscIterations=24;
 float dropVolume=.42f,flowVariation=.3f;bool catchPlane=false;float catchDepth=45.f;
 float dropDuration=1.75f,dropHold=1.45f,dropLength=5.7f;
 bool cacheViscosity=true; // Legacy particle solver, retained only for reference tools.
 float threadSpacing=.65f,breakup=.65f;int meshSides=12;
};
static Settings config;
}
