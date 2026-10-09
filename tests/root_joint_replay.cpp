// Offline replay of the actual adapter CPU functions against private measured
// inputs. No game launch, input, rendering or installation occurs here.
#include "../src/runtime/d3d9_proxy.cpp"
#include "../tools/r14/cpu_buffer.h"
#include <fstream>
#include <string>
static bool LoadBody(const char* path){
 preparedShapeReady=shaftRestFrameReady=eggRestReady=constraintSolverReady=false;paBasisSaved=false;ResetCompliantDynamics();
 if(graftBuffer)graftBuffer->Release();graftBuffer=new CpuVertexBuffer(50915*32);graftOffset=47050*32;
 void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);FILE* f=nullptr;fopen_s(&f,path,"rb");
 if(!f)return false;const bool read=fread(raw,32,50915,f)==50915;fclose(f);graftBuffer->Unlock();return read;
}
static void WriteState(FILE* f){
 fwrite(shaftNodes,sizeof(shaftNodes),1,f);fwrite(pdPosition,sizeof(pdPosition),1,f);fwrite(pdVelocity,sizeof(pdVelocity),1,f);
 fwrite(&shaftSpring,sizeof(shaftSpring),1,f);const float drive[]={rootDriveAngle,rootDriveVelocity};fwrite(drive,sizeof(drive),1,f);
 fwrite(graftDeformedPositions,sizeof(graftDeformedPositions),1,f);fwrite(rsPacked,sizeof(rsPacked),1,f);
 for(unsigned s=0;s<2;s++)fwrite(sharedBodyOutput[s],32,sharedBodyCount[s],f);
}
int main(int argc,char** argv){
 if(argc!=5){puts("usage: root-joint-replay cases.txt body.bin output.bin enabled(0|1)");return 2;}
 const bool enabled=atoi(argv[4])!=0;
#if ROOT_JOINT_ADAPTER
 SetEnvironmentVariableA("MALEMOD_ROOT_CONTACTS",enabled?"1":"0");
#else
 if(enabled)return 3;
#endif
 std::ifstream cases(argv[1]);unsigned count=0;cases>>count;if(!cases||count>100)return 4;
 FILE* output=nullptr;fopen_s(&output,argv[3],"wb");if(!output)return 5;
 for(unsigned c=0;c<count;c++){
  ResetStudyControls();std::string name;cases>>name>>physicsState>>glansUI>>hangUI;
  for(auto& x:sliderUI)cases>>x;for(auto& x:physUI)cases>>x;
  V3 thighs[4];for(auto& p:thighs)cases>>p.x>>p.y>>p.z;if(!cases)return 6;
  ApplyControlMapping();if(!LoadBody(argv[2]))return 7;shapeDirty=true;ApplyShape();InitializeConstraintSolver();
  collisionCapsuleOverride=true;overrideLeftA=thighs[0];overrideLeftB=thighs[1];overrideRightA=thighs[2];overrideRightB=thighs[3];
  auto input=PDReadInput(0,0);PDSetInput(input,input,1);
#if ROOT_JOINT_ADAPTER
  if(rootJointContactsReady)return 8; // Override/fallback never claims native calibration.
  rootJointContactsReady=enabled; // Explicit measured numerical test fixture.
#endif
  for(unsigned step=0;step<120;step++){
   StepConstraintSolver(1.f/240.f,sinf(step*.04f)*.3f,cosf(step*.035f)*.3f);
   for(const auto& p:shaftNodes)if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z))return 9;
   if(Length(shaftNodes[0]-ShaftRoot())>1e-5f)return 10;
   if(fabsf(Length(shaftNodes[1]-shaftNodes[0])-constraintRestLength/(shaftNodeCount-1))>2e-5f)return 11;
   if(step%30==0||step==119){shapeDirty=true;ApplyShape();WriteState(output);}
  }
  printf("PASS measured CPU case %s enabled=%d\n",name.c_str(),enabled);
 }
 fclose(output);return 0;
}
