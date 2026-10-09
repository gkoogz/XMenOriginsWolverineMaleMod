// Continuous replay of the real adapter: preserve solver history through
// measured control/state/capsule changes, then let each episode settle.
// Fixtures remain private licensed inputs. No device, game or input is opened.
#define main isolated_case_replay_main
#include "root_joint_replay.cpp"
#undef main
struct MotionCase {
 std::string name;int state;float glans,hang,controls[7],physics[8];V3 thighs[4];
};
int main(int argc,char** argv){
 if(argc!=5){puts("usage: root-motion-replay cases.txt body.bin report.csv enabled(0|1)");return 2;}
 const bool enabled=atoi(argv[4])!=0;
#if ROOT_JOINT_ADAPTER
 SetEnvironmentVariableA("MALEMOD_ROOT_CONTACTS",enabled?"1":"0");
#else
 if(enabled)return 3;
#endif
 std::ifstream input(argv[1]);unsigned count=0;input>>count;if(!input||!count||count>100)return 4;
 std::vector<MotionCase> cases(count);
 for(auto& c:cases){input>>c.name>>c.state>>c.glans>>c.hang;for(auto& x:c.controls)input>>x;for(auto& x:c.physics)input>>x;for(auto& p:c.thighs)input>>p.x>>p.y>>p.z;}
 if(!input)return 5;FILE* report=nullptr;fopen_s(&report,argv[3],"wb");if(!report)return 6;
 fprintf(report,"case,step,pitch,yaw,pitchVelocity,yawVelocity,softPitchLimit,driveDegrees,firstX,firstY,firstZ,tipX,tipY,tipZ,maxStationSpeed,rootError,firstLengthError\n");
 ResetStudyControls();if(!LoadBody(argv[2]))return 7;
 unsigned invalid=0,reversed=0;float maximumPitch=0,maximumYaw=0,maximumAngularSpeed=0,maximumStationSpeed=0;
 PDInput previous{};bool havePrevious=false;
 // Repeat default after the extreme sequence without resetting springs/poses.
 cases.push_back(cases.front());cases.back().name+="-after-extremes";
 for(const auto& c:cases){
  physicsState=c.state;glansUI=c.glans;hangUI=c.hang;
  memcpy(sliderUI,c.controls,sizeof(c.controls));memcpy(physUI,c.physics,sizeof(c.physics));ApplyControlMapping();shapeDirty=true;ApplyShape();
  if(!constraintSolverReady)InitializeConstraintSolver();
  collisionCapsuleOverride=true;overrideLeftA=c.thighs[0];overrideLeftB=c.thighs[1];overrideRightA=c.thighs[2];overrideRightB=c.thighs[3];
  const auto target=PDReadInput(0,0);if(!havePrevious)previous=target;
#if ROOT_JOINT_ADAPTER
  if(rootJointContactsReady)return 8; // An override must never certify native calibration.
  rootJointContactsReady=enabled; // Explicit measured replay fixture only.
#endif
  for(unsigned step=0;step<960;step++){
   PDSetInput(previous,target,min(1.f,float(step)/48.f));
   const float gait=step<480?sinf(step*.04f)*.3f:0.f,side=step<480?cosf(step*.035f)*.3f:0.f;
   StepConstraintSolver(1.f/240.f,gait,side);
   auto finitePoint=[](V3 p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);};
   float speed=0;bool finite=true;for(int i=0;i<shaftNodeCount;i++){finite&=finitePoint(shaftNodes[i]);speed=max(speed,Length(pdVelocity[i]));}
   for(int i=0;i<pdCount;i++)finite&=finitePoint(pdPosition[i])&&finitePoint(pdVelocity[i]);
   for(unsigned s=0;s<2;s++){finite&=finitePoint(cpOmega[s]);for(const auto& axis:cpBasis[s])finite&=finitePoint(axis);}
   const float rootError=Length(shaftNodes[0]-ShaftRoot()),lengthError=fabsf(Length(shaftNodes[1]-shaftNodes[0])-constraintRestLength/(shaftNodeCount-1));
   finite&=std::isfinite(shaftSpring.pitch)&&std::isfinite(shaftSpring.yaw)&&std::isfinite(shaftSpring.pitchVelocity)&&std::isfinite(shaftSpring.yawVelocity);
   if(!finite||rootError>1e-5f||lengthError>2e-5f)invalid++;
   // Source soft-stop onset is strictly below pi/2 in every mode. A local
   // response beyond pi/2 reverses the commanded tangent hemisphere. This
   // is a rejection diagnostic; it is never an angle clamp in the runtime.
   if(fabsf(shaftSpring.pitch)>=1.5707963268f||fabsf(shaftSpring.yaw)>=1.5707963268f)reversed++;
   maximumPitch=max(maximumPitch,fabsf(shaftSpring.pitch));maximumYaw=max(maximumYaw,fabsf(shaftSpring.yaw));
   maximumAngularSpeed=max(maximumAngularSpeed,max(fabsf(shaftSpring.pitchVelocity),fabsf(shaftSpring.yawVelocity)));maximumStationSpeed=max(maximumStationSpeed,speed);
   if(step%16==0||step==959){const auto first=shaftNodes[1],tip=shaftNodes[shaftNodeCount-1];fprintf(report,"%s,%u,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g\n",c.name.c_str(),step,shaftSpring.pitch,shaftSpring.yaw,shaftSpring.pitchVelocity,shaftSpring.yawVelocity,ModeValue(.42f,.62f,.92f),rootDriveAngle,first.x,first.y,first.z,tip.x,tip.y,tip.z,speed,rootError,lengthError);}
  }
  previous=target;havePrevious=true;
  printf("EPISODE %s pitch=%.9g yaw=%.9g pitchRate=%.9g yawRate=%.9g\n",c.name.c_str(),shaftSpring.pitch,shaftSpring.yaw,shaftSpring.pitchVelocity,shaftSpring.yawVelocity);
 }
 fclose(report);
 printf("RESULT enabled=%d invalid=%u reversedSteps=%u maxPitch=%.9g maxYaw=%.9g maxAngularSpeed=%.9g maxStationSpeed=%.9g nativeVisualGate=UNTESTED\n",enabled,invalid,reversed,maximumPitch,maximumYaw,maximumAngularSpeed,maximumStationSpeed);
 return invalid?9:reversed?10:0;
}
