#ifndef RUNTIME_SOURCE
#define RUNTIME_SOURCE "d3d9_proxy.cpp"
#endif
#include RUNTIME_SOURCE
#include "render_test.h"
static LRESULT CALLBACK HarnessProc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
int main(int argc,char** argv){
  if(argc==2&&strcmp(argv[1],"--state-profiles-test")==0){
    float bends[3]{};
    for(int state=0;state<3;state++){
      ResetStudyControls();physicsState=state;constraintRestLength=32.f;
      constraintBallRest[0]={14.4f,-3.f,69.f};constraintBallRest[1]={14.4f,3.f,69.f};InitializeConstraintSolver();
      float lo=1e9f,hi=-1e9f,maxStep=0.f;
      for(int j=0;j<3600;j++){
        V3 old=shaftNodes[shaftNodeCount-1];float t=j/240.f;
        StepConstraintSolver(1.f/240.f,j>1200?.22f*sinf(t*3.f):0.f,j>1200?.16f*sinf(t*2.1f):0.f);
        if(j>1200){lo=min(lo,shaftSpring.pitch);hi=max(hi,shaftSpring.pitch);maxStep=max(maxStep,Length(shaftNodes[shaftNodeCount-1]-old));}
        if(!std::isfinite(shaftNodes[shaftNodeCount-1].z))return 97;
      }
      V3 rootAxis=Unit(shaftNodes[1]-shaftNodes[0]);
      for(int i=2;i<shaftNodeCount;i++){V3 d=shaftNodes[i]-shaftNodes[0];bends[state]=max(bends[state],Length(d-rootAxis*Dot(d,rootAxis)));}
      printf("state=%d bend=%.4f root_swing=%.4f max_tip_step=%.4f\n",state,bends[state],hi-lo,maxStep);
      if(hi-lo<.015f||maxStep>1.f)return 98;
      physicsState=(state+1)%3;
      float transitionStep=0.f;
      for(int j=0;j<480;j++){V3 before=shaftNodes[shaftNodeCount-1];StepConstraintSolver(1.f/240.f,0.f,0.f);transitionStep=max(transitionStep,Length(shaftNodes[shaftNodeCount-1]-before));}
      printf("transition_from=%d max_tip_step=%.4f\n",state,transitionStep);
      if(!std::isfinite(transitionStep)||transitionStep>1.f)return 100;
    }
    return bends[0]<.5f&&bends[1]>bends[0]*2.f&&bends[2]>bends[1]*1.25f?0:99;
  }
  if(argc==2&&strcmp(argv[1],"--continuity-test")==0){
    float referenceSwing=0.f;
    for(int quiet=0;quiet<2;quiet++){
      ResetStudyControls();constraintRestLength=26.f;constraintBallRest[0]={14.4f,-3.f,69.f};constraintBallRest[1]={14.4f,3.f,69.f};InitializeConstraintSolver();
      motionQuietFrames=quiet?100:0;
      for(int j=0;j<2400;j++)StepConstraintSolver(1.f/240.f,0,0);
      float minY=1e9f,maxY=-1e9f,travel=0,coast=0,maxJump=0;V3 last=ballNodes[0];
      for(int j=0;j<2400;j++){
        float t=j/240.f,drive=j<1920?.045f*sinf(t*4.0f):0.f;
        StepConstraintSolver(1.f/240.f,0,drive);
        if(j>480&&j<1920){minY=min(minY,ballNodes[0].y);maxY=max(maxY,ballNodes[0].y);travel+=Length(ballNodes[0]-last);}
        if(j>=1920)coast+=Length(ballNodes[0]-last);
        maxJump=max(maxJump,Length(ballNodes[0]-last));last=ballNodes[0];
      }
      printf("quiet=%d swing=%.6f travel=%.6f coast=%.6f maxStep=%.6f\n",quiet,maxY-minY,travel,coast,maxJump);
      if(!std::isfinite(travel)||maxY-minY<.8f||coast<.3f||maxJump>.05f)return 90;
      if(!quiet)referenceSwing=maxY-minY;
      else if(fabsf(referenceSwing-(maxY-minY))>1e-5f)return 91;
      V3 beforeStateChange=ballNodes[0];physicsState=1;StepConstraintSolver(1.f/240.f,0,0);
      if(Length(ballNodes[0]-beforeStateChange)>.35f)return 92;
    }
    return 0;
  }
  if(argc==2&&strcmp(argv[1],"--physics-regression-test")==0){
    ResetStudyControls();for(int i=0;i<7;i++)sliderUI[i]=50.f;sliderUI[0]=sliderUI[1]=sliderUI[2]=sliderUI[3]=100.f;ApplyControlMapping();
    constraintRestLength=37.8f;constraintBallRest[0]={14.4f,-5.0f,68.0f};constraintBallRest[1]={14.4f,5.0f,68.0f};constraintSolverReady=false;physicsState=2;InitializeConstraintSolver();collisionCapsuleOverride=true;
    float actualTailSpeed=0.f;float maxSegmentError=0.f,maxContainment=0.f,maxSpeed=0.f,minClearance=1e9f;
    auto capsuleDistance=[](V3 p,V3 a,V3 b){V3 ab=b-a;float d=Dot(ab,ab),u=d>1e-8f?max(0.f,min(1.f,Dot(p-a,ab)/d)):0.f;return Length(p-(a+ab*u));};
    // Allow 15 seconds of coast-down for the heavier, softer floppy state.
    for(int step=0;step<8400;step++){
      float time=step/240.f,fold=step<4800?.5f+.5f*sinf(time*.83f):0.f;float reach=18.f*fold;
      if(step==4800)motionQuietFrames=100;
      overrideLeftA={2.f,-7.8f,79.f};overrideRightA={2.f,7.8f,79.f};
      overrideLeftB={1.f+reach,-8.2f,43.f+24.f*fold};overrideRightB={1.f+reach,8.2f,43.f+24.f*fold};
      float gait=step<4800?2.8f*sinf(time*5.7f):0.f,side=step<4800?2.5f*sinf(time*7.9f):0.f;
      V3 oldBall[2]={ballNodes[0],ballNodes[1]};StepConstraintSolver(1.f/240.f,gait,side);
      for(int i=0;i<shaftNodeCount;i++)if(!std::isfinite(shaftNodes[i].x)||!std::isfinite(shaftNodes[i].y)||!std::isfinite(shaftNodes[i].z))return 81;
      for(int i=0;i<2;i++){
        if(!std::isfinite(ballNodes[i].x)||!std::isfinite(nutNodes[i].x)||!std::isfinite(neckNodes[i].x))return 82;
        if(step>8160)actualTailSpeed=max(actualTailSpeed,Length(ballNodes[i]-oldBall[i])*240.f);
        maxContainment=max(maxContainment,Length(nutNodes[i]-ballNodes[i]));maxSpeed=max(maxSpeed,Length(ballNodes[i]-oldBall[i])*240.f);
        minClearance=min(minClearance,capsuleDistance(ballNodes[i],overrideLeftA,overrideLeftB));minClearance=min(minClearance,capsuleDistance(ballNodes[i],overrideRightA,overrideRightB));
      }
      float target=max(2.f,constraintRestLength/(shaftNodeCount-1));for(int i=0;i<shaftNodeCount-1;i++)maxSegmentError=max(maxSegmentError,fabsf(Length(shaftNodes[i+1]-shaftNodes[i])-target));
    }
    printf("actual_tail_speed=%.6f\n",actualTailSpeed);collisionCapsuleOverride=false;float settle=max(Length(ballNodes[0]-ballPrevious[0]),Length(ballNodes[1]-ballPrevious[1]))*240.f;
    printf("Layered physics diagnostics; segment_error=%.4f containment=%.3f max_speed=%.2f clearance=%.3f settle_speed=%.5f\n",maxSegmentError,maxContainment,maxSpeed,minClearance,settle);
    return maxSegmentError<.08f&&maxContainment<BallCollisionRadius()*.62f&&settle<.08f&&actualTailSpeed<.1f?0:83;
  }
  if(argc==2&&strcmp(argv[1],"--glans-controls-test")==0){
    // Run from a dedicated test directory: settings are sibling to this EXE.
    const int targets[18]={0,1,2,7,3,8,4,5,6,9,10,11,12,13,14,15,16,17};
    for(int row=0;row<18;row++){
      ResetStudyControls();AdjustStudyControl(row,1,1);
      float values[18];memcpy(values,sliderUI,sizeof(sliderUI));values[7]=glansUI;values[8]=hangUI;values[9]=(float)physicsState;memcpy(values+10,physUI,sizeof(physUI));
      for(int j=0;j<18;j++){float expected=j==9?(targets[row]==9?0.f:2.f):(j==targets[row]?51.f:50.f);if(values[j]!=expected)return 31;}
    }
    ResetStudyControls();AdjustStudyControl(3,1,100);if(glansUI!=100)return 32;
    AdjustStudyControl(3,-1,200);if(glansUI!=0)return 33;
    ResetStudyControls();glansUI=73;hangUI=65;sliderUI[2]=61;physUI[7]=44;SaveSettings();
    ResetStudyControls();settingsLoaded=false;LoadSettings();
    if(glansUI!=73||hangUI!=65||sliderUI[2]!=61||physUI[7]!=44)return 34;
    char path[MAX_PATH];SiblingPath(path,"WolverineLive.ini");WritePrivateProfileStringA("Shape","Glans Size",nullptr,path);
    ResetStudyControls();settingsLoaded=false;LoadSettings();if(glansUI!=50||hangUI!=65||sliderUI[2]!=61)return 35;
    ResetStudyControls();if(glansUI!=50||hangUI!=50||sliderUI[2]!=50)return 36;
    if(fabsf(R14Scale(50.f)-R16PreviousScale(0.f))>1e-6f||fabsf(R14Scale(100.f)-R16PreviousScale(100.f))>1e-6f||R14LowerFactor(50.f)!=0.f||R14LowerFactor(0.f)!=1.f)return 37;
    if(!(R14LowerFactor(0.f)>R14LowerFactor(25.f)&&R14LowerFactor(25.f)>R14LowerFactor(50.f)))return 38;
    if(fabsf(MapLength100(0.f)-.4f)>1e-6f||fabsf(MapLength100(50.f)-1.6f)>1e-6f||fabsf(MapLength100(100.f)-2.f)>1e-6f)return 40;
    if(!(Smoother01(0.f)<Smoother01(.5f)&&Smoother01(.5f)<Smoother01(1.f)))return 41;
    ResetStudyControls();sliderUI[1]=0.f;ApplyControlMapping();if(fabsf(sliderValues[1]-.4f)>1e-6f)return 42;
    WritePrivateProfileStringA("Meta","ControlScaleVersion","3",path);WritePrivateProfileStringA("Shape","Glans Size","0",path);
    ResetStudyControls();settingsLoaded=false;LoadSettings();if(glansUI!=50.f)return 39;
    printf("PASS: controls, persistence, v4 glans remap, expanded eased Length 0-50 range\n");return 0;
  }
  if(argc==2&&strcmp(argv[1],"--settings-test")==0){LoadSettings();float loaded=hangUI;float shape[7];memcpy(shape,sliderUI,sizeof(shape));hangUI=73;SaveSettings();settingsLoaded=false;hangUI=50;LoadSettings();bool same=memcmp(shape,sliderUI,sizeof(shape))==0;printf("Hang loaded=%.0f saved/reloaded=%.0f existing_shape_preserved=%d\n",loaded,hangUI,same?1:0);return hangUI==73&&same?0:21;}

  if(argc<4){printf("usage: harness output.bin size state [angle] [scrotum] [frames]\n");return 2;}
  float size=(float)atof(argv[2]); physicsState=atoi(argv[3]);
  for(int i=0;i<7;i++)sliderUI[i]=50.f;
  sliderUI[0]=sliderUI[1]=sliderUI[2]=sliderUI[3]=size;
  if(argc>4)sliderUI[4]=(float)atof(argv[4]);if(argc>5)sliderUI[3]=(float)atof(argv[5]);
  if(argc>7)sliderUI[2]=(float)atof(argv[7]);if(argc>8)sliderUI[1]=(float)atof(argv[8]);
  if(argc>9)hangUI=(float)atof(argv[9]);
  if(argc>11)glansUI=max(0.f,min(100.f,(float)atof(argv[11])));
  if(argc>12&&atoi(argv[12])){collisionCapsuleOverride=true;overrideLeftA={2.f,-7.8f,79.f};overrideRightA={2.f,7.8f,79.f};overrideLeftB={16.f,-8.2f,66.f};overrideRightB={16.f,8.2f,66.f};}
  ApplyControlMapping();int frames=argc>6?atoi(argv[6]):240;
  HINSTANCE hi=GetModuleHandleA(nullptr);WNDCLASSA wc{};wc.lpfnWndProc=HarnessProc;wc.hInstance=hi;wc.lpszClassName="V071Inspection";RegisterClassA(&wc);
  HWND hw=CreateWindowA(wc.lpszClassName,"V0.7.1 inspection",WS_OVERLAPPEDWINDOW,0,0,320,240,nullptr,nullptr,hi,nullptr);
  LoadReal();IDirect3D9* d9=realCreate9(D3D_SDK_VERSION);if(!d9)return 10;
  D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=hw;pp.BackBufferWidth=1000;pp.BackBufferHeight=750;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.EnableAutoDepthStencil=TRUE;pp.AutoDepthStencilFormat=D3DFMT_D16;
  IDirect3DDevice9* dev=nullptr;if(FAILED(d9->CreateDevice(0,D3DDEVTYPE_HAL,hw,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&dev)))return 11;
  if(FAILED(dev->CreateVertexBuffer(50915*32,D3DUSAGE_DYNAMIC,0,D3DPOOL_DEFAULT,&graftBuffer,nullptr)))return 12;
  graftOffset=47050*32;void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);memset(raw,0,50915*32);
  for(UINT i=0;i<collarNormalTriangleCount*3;i++)memcpy((char*)raw+collarNormalTriangleIndices[i]*32,collarNormalTriangleBasePositions+i*3,12);
  for(UINT i=0;i<pelvisControlCount;i++)memcpy((char*)raw+pelvisControlIndices[i]*32,pelvisControlBasePositions+i*3,12);
  for(UINT i=0;i<graftCount;i++)memcpy((char*)raw+(47050+i)*32,morph_base+i*3,12);
  graftBuffer->Unlock();ApplyShape();
  std::vector<V3> trace;
  for(int frame=0;frame<frames;frame++){for(int sub=0;sub<4;sub++){float drive=argc>10?(float)atof(argv[10]):0.f;float time=(frame+sub/4.f)/60.f;StepConstraintSolver(1.f/240.f,drive*sinf(time*5.f),drive*sinf(time*7.f));}ApplyShape();
    for(UINT i=0;i<graftCount;i++)if(suspensionWeight[i]>.9f)for(int b=0;b<2;b++){
      V3 ra=Unit(constraintBallRest[b]-RestBallAnchor(b)),la=Unit(ballNodes[b]-BallAnchor(b));
      float level=EggLevel(RotateFromTo(graftDeformedPositions[i]-ballNodes[b],la,ra),eggRadii[b]*.94f);
      if(!std::isfinite(level)||level<.985f){printf("Core intrusion frame=%d vertex=%u level=%.6f\n",frame,i,level);return 96;}
    }
    trace.push_back(ballNodes[0]);trace.push_back(ballNodes[1]);trace.push_back(BallAnchor(0));trace.push_back(BallAnchor(1));
    for(int band=0;band<4;band++){V3 sum{};int count=0;for(UINT i=0;i<graftCount;i++){float w=suspensionWeight[i];if(w>band*.25f&&w<=(band+1)*.25f){sum=sum+graftDeformedPositions[i];count++;}}trace.push_back(sum/(float)max(count,1));}
  }
  std::vector<V3> output(graftCount+pelvisControlCount);graftBuffer->Lock(0,0,&raw,0);
  for(UINT i=0;i<graftCount;i++)memcpy(&output[i],(char*)raw+(47050+i)*32,12);
  for(UINT i=0;i<pelvisControlCount;i++)memcpy(&output[graftCount+i],(char*)raw+pelvisControlIndices[i]*32,12);
  graftBuffer->Unlock();FILE* fp=nullptr;fopen_s(&fp,argv[1],"wb");if(!fp)return 13;fwrite(output.data(),sizeof(V3),output.size(),fp);fclose(fp);
  printf("captured %u points; size %.0f state %d; %d deterministic frames at 60Hz\n",(unsigned)output.size(),size,physicsState,frames);
  printf("radius %.6f length %.6f firstnodes=(%.3f %.3f %.3f) (%.3f %.3f %.3f)\n",logicalShaftBodyRadius,constraintRestLength,shaftNodes[1].x,shaftNodes[1].y,shaftNodes[1].z,shaftNodes[2].x,shaftNodes[2].y,shaftNodes[2].z);
  char nodesFile[MAX_PATH];sprintf_s(nodesFile,"%s.nodes",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(shaftNodes,sizeof(V3),shaftNodeCount,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.balls",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(ballNodes,sizeof(V3),2,fp);V3 anchors[2]={BallAnchor(0),BallAnchor(1)};fwrite(anchors,sizeof(V3),2,fp);fwrite(nutNodes,sizeof(V3),2,fp);fwrite(neckNodes,sizeof(V3),2,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.trace",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(trace.data(),sizeof(V3),trace.size(),fp);fclose(fp);
  sprintf_s(nodesFile,"%s.r14",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(r14Positions,sizeof(V3),r14Count,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.packed",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(r14Packed,1,sizeof(r14Packed),fp);fclose(fp);
  float eggConstants[16];for(int b=0;b<2;b++){V3 r=eggRadii[b];eggConstants[b*8]=ballNodes[b].x;eggConstants[b*8+1]=ballNodes[b].y;eggConstants[b*8+2]=ballNodes[b].z;eggConstants[b*8+3]=0;eggConstants[b*8+4]=1.f/r.x;eggConstants[b*8+5]=1.f/r.y;eggConstants[b*8+6]=1.f/r.z;eggConstants[b*8+7]=0;}
  sprintf_s(nodesFile,"%s.egg-material",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(eggConstants,sizeof(float),16,fp);fclose(fp);
  float minSupport=1e9f;int supportCount=0;
  for(UINT i=0;i<graftCount;i++)if(suspensionWeight[i]>.9f){
    V3 p=graftDeformedPositions[i];float best=1e9f;
    for(int b=0;b<2;b++){
      V3 ra=Unit(constraintBallRest[b]-RestBallAnchor(b)),la=Unit(ballNodes[b]-BallAnchor(b));
      float q=EggLevel(RotateFromTo(p-ballNodes[b],la,ra),eggRadii[b]*.94f);best=min(best,q);
    }
    minSupport=min(minSupport,best);supportCount++;
  }
  printf("Core surface protection: samples=%d minimum_level=%.6f (must be >=0.995)\n",supportCount,minSupport);
  if(!std::isfinite(minSupport)||minSupport<.995f)return 94;
  IDirect3DVertexShader9* materialVS=nullptr;IDirect3DPixelShader9* materialPS=nullptr;
  if(FAILED(dev->CreateVertexShader(skinOvoidVS,&materialVS))||FAILED(dev->CreatePixelShader(skinOvoidPS,&materialPS)))return 95;
  materialVS->Release();materialPS->Release();
  bool renderOK=TestR14Draw(dev,argv[1],pp);
  ReleaseR14();if(graftBuffer){graftBuffer->Release();graftBuffer=nullptr;}dev->Release();d9->Release();DestroyWindow(hw);return renderOK?0:71;
}
