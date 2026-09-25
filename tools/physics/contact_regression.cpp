#include "../../src/runtime/d3d9_proxy.cpp"
#include <cstdio>

static bool Plane(float dt,float force,float belt,float initial,float& drift,float& speed){
 const int id=2;pdPosition[id]={};pdVelocity[id]={initial,0,0};pdInvMass[id]=1.f;
 float lastEnergy=initial*initial;
 for(int step=0;step<int(5.f/dt);step++){
  pdOldPosition[id]=pdPosition[id];pdVelocity[id]=pdVelocity[id]+V3{force,0,-10.f}*dt;pdPosition[id]=pdPosition[id]+pdVelocity[id]*dt;
  PDConstraint contact;pdContactCount=0;
  for(int iteration=0;iteration<8;iteration++){
   pdVelocityPass=iteration==7;
   PDContact(contact,id,{},-1,{},{belt*(step+1)*dt,0,0},{belt*step*dt,0,0},{0,0,1},pdPosition[id].z,0.f,dt);
  }
  pdVelocityPass=false;pdVelocity[id]=(pdPosition[id]-pdOldPosition[id])/dt;PDSolveContactVelocities(dt);
  float energy=Dot(pdVelocity[id],pdVelocity[id]);
  if(!std::isfinite(energy)||fabsf(pdPosition[id].z)>1e-5f)return false;
  if(initial!=0.f&&belt==0.f&&force==0.f&&energy>lastEnergy+1e-4f)return false;
  lastEnergy=energy;
 }
 drift=pdPosition[id].x;speed=pdVelocity[id].x;return true;
}
int main(){
 constraintSolverReady=shaftRestFrameReady=eggRestReady=false;constraintAccumulator=0;
 UpdateConstraintSolver(1.f/30.f,1.f,1.f);
 if(constraintSolverReady||constraintAccumulator!=0.f){printf("FAIL pre-geometry initialization\n");return 7;}
 constraintSolverReady=shaftRestFrameReady=true;
 for(int i=0;i<shaftRestSampleCount;i++)shaftRestCenters[i]=ShaftRoot()+V3{24.f*i/(shaftRestSampleCount-1),0,0};
 constraintBallRest[0]={16,-3,74};constraintBallRest[1]={16,3,74};logicalShaftBodyRadius=3.f;
 pdInvInertia[0]=pdInvInertia[1]=.03f;
 for(int i=0;i<pdCount;i++)pdInvMass[i]=i<2?0.f:1.f;
 for(int test=0;test<100;test++){
  for(int i=0;i<shaftNodeCount;i++)pdPosition[i]=ShaftRoot()+V3{i*2.f,sinf(i*.2f+test*.03f),-i*.1f-.04f*i*i};
  V3 n=Unit(V3{cosf(test*.3f),sinf(test*.3f),.7f}),analytic[shaftNodeCount],numeric[shaftNodeCount];
  for(bool material:{false,true}){
   PDSuspensionMass(test%2,{0,0,1},n,analytic,material);PDSuspensionMassFiniteDifference(test%2,{0,0,1},n,numeric,material);
   for(int i=2;i<shaftNodeCount;i++)if(Length(analytic[i]-numeric[i])>.012f){printf("FAIL attachment Jacobian %d %d error %g\n",test,i,Length(analytic[i]-numeric[i]));return 6;}
  }
 }
 // Verify the support query against a dense meridian search, including
 // near-pole directions that expose contact-normal precision failures.
 for(int test=0;test<1000;test++){
  V3 radii{1.f+(test%17),.5f+(test%13),2.f+(test%23)};
  V3 normal=Unit(V3{cosf(test*.71f)*.6f,sinf(test*.71f)*.6f,sinf(test*.193f)});
  if(test<2)normal={1e-7f,0,test?1.f:-1.f};
  V3 p=CPSupportLocal(normal,radii);float actual=Dot(p,normal);
  float h=sqrtf(normal.x*normal.x*radii.x*radii.x+normal.y*normal.y*radii.y*radii.y);
  for(int i=0;i<=1000;i++){
   float z=-1.f+i*.002f,value=h*(1.f-.13f*z)*sqrtf(max(0.f,1.f-z*z))+normal.z*radii.z*z;
   if(!std::isfinite(actual)||actual+1e-4f<value){printf("FAIL ovoid support\n");return 5;}
  }
 }
 for(float dt:{1.f/120.f,1.f/240.f,1.f/480.f}){
  float drift,speed;
  if(!Plane(dt,2.f,0.f,0.f,drift,speed)||fabsf(drift)>1e-4f||fabsf(speed)>1e-4f){printf("FAIL static friction dt=%g drift=%g speed=%g\n",dt,drift,speed);return 1;}
  if(!Plane(dt,0.f,0.f,3.f,drift,speed)||fabsf(speed)>1e-4f||drift<=0.f){printf("FAIL sliding dissipation\n");return 2;}
  if(!Plane(dt,0.f,2.f,0.f,drift,speed)||fabsf(speed-2.f)>.002f){printf("FAIL moving surface speed=%g\n",speed);return 3;}
 }
 // Coupled contact must conserve total linear momentum for unequal masses.
 pdInvMass[2]=1.f;pdInvMass[3]=.5f;pdPosition[2]={.8f,0,0};pdPosition[3]={};
 pdOldPosition[2]=pdPosition[2];pdOldPosition[3]=pdPosition[3];PDConstraint pair;pdContactCount=0;pdVelocityPass=true;
 PDContact(pair,2,{},3,{},{},{},{1,0,0},-.2f,0.f,1.f/240.f);pdVelocityPass=false;
 V3 center=pdPosition[2]+pdPosition[3]*2.f;if(Length(center-V3{.8f,0,0})>1e-5f)return 4;
 printf("PASS: ovoid support accuracy, static hold without drift, dissipative sliding, moving-surface friction at 120/240/480 Hz; unequal-mass momentum\n");return 0;
}
