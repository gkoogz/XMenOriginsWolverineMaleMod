#pragma once
#include <algorithm>
#include <string>
namespace volumeFluid {
template<class T> void Drop(T*& p){if(p){p->Release();p=nullptr;}}
struct Particle {V3 p;float live;V3 v;float age;};
struct F4 {float x,y,z,w;};
struct Settings {
 float spacing=.32f,volume=180.f,feed=1.1f,nozzle=1.25f,viscosity=16.f,tension=12.f,lifetime=5.f;
 float gravity=98.f,speedLimit=350.f;int capacity=32768,iterations=4,viscIterations=24;
 float dropVolume=.35f,flowVariation=.3f;bool catchPlane=false;float catchDepth=45.f;
 float dropDuration=1.6f,dropHold=1.3f,dropLength=5.f;
 bool cacheViscosity=true; // Legacy particle solver, retained only for reference tools.
 float threadSpacing=.65f,breakup=.65f;int meshSides=12;
};
static Settings config;
}
