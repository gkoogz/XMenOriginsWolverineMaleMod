#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <cmath>
#include <cstdio>
#include <chrono>
struct V3 {float x,y,z;};
#include "../../src/runtime/fluid_compute.h"
int main(){setvbuf(stdout,nullptr,_IONBF,0);volumeFluid::Solver solver;volumeFluid::Settings c;c.capacity=8192;c.catchPlane=true;c.catchDepth=5;c.gravity=20;
 if(!solver.Initialize(c.capacity)){printf("FAIL %s\n",solver.error.c_str());return 1;}solver.Reset({0,0,0});
 std::vector<volumeFluid::Particle> initial;for(int z=0;z<10;z++)for(int y=-5;y<5;y++)for(int x=-5;x<5;x++)initial.push_back({{x*c.spacing,y*c.spacing,z*c.spacing},1,{2,0,0},0});solver.Add(initial);
 auto t=std::chrono::steady_clock::now();for(int k=0;k<120;k++){solver.Step(1.f/120,c,{},{1,0,0});if(!solver.Read()){printf("FAIL %s\n",solver.error.c_str());return 2;}}
 float lo=1e9f,hi=-1e9f;double vx=0;for(auto p:solver.snapshot){lo=min(lo,p.p.z);hi=max(hi,p.p.z);vx+=p.v.x;}
 printf("PASS GPU particles=%u active=%u z=[%.4f,%.4f] meanVx=%.4f ms/step+read=%.3f\n",solver.count,solver.active,lo,hi,vx/solver.count,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count()/120);
 return lo< -5?3:0;
}
