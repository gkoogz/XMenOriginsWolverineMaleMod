#include "../../src/runtime/necklace_contact.h"
#include "../../src/runtime/tank_top_data.h"
#include <cassert>
int main(){
 NcRig rig;for(unsigned i=0;i<128;i++){rig.valid[i]=true;rig.matrix[i][0]=rig.matrix[i][5]=rig.matrix[i][10]=1;}
 NcContactSolver solver;NcContactStats a,b,c;float bare[4],dressed[4],restored[4];
 assert(solver.solve(rig,bare,a));
 float nakedFront;assert(solver.surface(0,120,nakedFront));
 solver.setGarment(TankTopRecipe::vertices,sizeof(TankTopRecipe::vertices)/sizeof(NcVertex),TankTopRecipe::triangles,sizeof(TankTopRecipe::triangles)/sizeof(TankTopRecipe::triangles[0]));
 assert(solver.solve(rig,dressed,b));float topFront;assert(solver.surface(0,120,topFront));
 assert(topFront>nakedFront+.1f);assert(b.remaining<.0001f&&!b.limited);
 for(unsigned i=0;i<4;i++)assert(dressed[i]>=bare[i]);
 // Exact same bone pose: changing only the costume must invalidate contact.
 solver.setGarment(nullptr,0,nullptr,0);assert(solver.solve(rig,restored,c));assert(memcmp(bare,restored,sizeof(bare))==0);
 rig.valid[6]=false;assert(!solver.solve(rig,restored,c));
}
