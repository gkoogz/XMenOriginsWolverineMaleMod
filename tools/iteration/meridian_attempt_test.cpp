#include "../../src/runtime/meridian_attempt.h"
#include <cstdio>
static void Require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
int main(){try{
 MeridianAttemptGate gate;std::vector<malemod::garments::meridian::Vec> p={{1,2,3},{4,5,6},{7,8,9}};
 unsigned ids[]={0,2};
 Require(gate.Begin(12,4,p,ids,2),"First pose skipped");
 Require(!gate.Begin(12,4,p,ids,2),"Failed pose repeated across hooks");
 p[1][0]=42;Require(!gate.Begin(12,4,p,ids,2),"Unused scratch invalidated gate");
 p[2][0]+=1;Require(gate.Begin(12,4,p,ids,2),"Late coherent palette/input suppressed");
 Require(gate.Begin(13,4,p,ids,2),"New frame suppressed");
 Require(gate.Begin(13,5,p,ids,2),"Control/scene epoch suppressed");
 gate.Reset();Require(gate.Begin(13,5,p,ids,2),"Device reset suppressed");
 std::puts("PASS: duplicate rejection, changed pose, frame, epoch and device reset");return 0;
 }catch(const std::exception& e){std::printf("FAIL: %s\n",e.what());return 1;}}
