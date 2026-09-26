#ifndef RUNTIME_SOURCE
#define RUNTIME_SOURCE "../../src/runtime/d3d9_proxy.cpp"
#endif
#include RUNTIME_SOURCE
#include "../r14/render_test.h"
#include "../r14/cpu_buffer.h"
static void ApplyReplayShape(){
#ifdef SURFACE_CADENCE_TEST
  UpdateVisibleSurface();
#else
  ApplyShape();
#endif
}
static LRESULT CALLBACK HarnessProc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
int main(int argc,char** argv){
  if(argc==2&&strcmp(argv[1],"--throb-test")==0){
    ResetStudyControls();
    for(int i=1;i<=3;i++)sliderUI[i]=100.f;
    glansUI=100.f;sliderUI[4]=50.f;
    const float expected[4]={100.f,120.f,136.f,152.f};
    const float expectedAngle[4]={50.f,42.f,36.f,30.f};
    for(int mode=0;mode<4;mode++){
      throbMode=mode;throbSizePulse=throbTwitchPulse=throbAngleSizePulse=1.f;ApplyControlMapping();
      for(int i=1;i<=3;i++)if(fabsf(effectiveShapeUI[i]-expected[mode])>1e-5f||sliderUI[i]!=100.f)return 101;
      if(fabsf(effectiveGlansUI-min(100.f,expected[mode]))>1e-5f||glansUI!=100.f)return 102;
      if(mode&&!(sliderValues[1]>sliderSpecs[1].hi&&sliderValues[2]>sliderSpecs[2].hi&&sliderValues[3]>sliderSpecs[3].hi))return 103;
      if(fabsf(effectiveShapeUI[4]-expectedAngle[mode])>1e-5f)return 104;
    }
    throbMode=3;throbSizePulse=0.f;throbAngleSizePulse=1.f;throbTwitchPulse=0.f;ApplyControlMapping();
    if(effectiveShapeUI[1]!=126.f||effectiveGlansUI!=100.f||effectiveShapeUI[4]!=50.f)return 113;
    throbSizePulse=1.f;throbAngleSizePulse=0.f;ApplyControlMapping();
    if(effectiveShapeUI[1]!=126.f||effectiveGlansUI!=100.f)return 114;
    sliderUI[4]=4.f;throbMode=3;throbSizePulse=throbTwitchPulse=throbAngleSizePulse=1.f;ApplyControlMapping();
    if(effectiveShapeUI[1]!=100.f||effectiveShapeUI[4]!=1.f||effectiveGlansUI!=100.f)return 110;
    sliderUI[4]=10.f;ApplyControlMapping();
    if(effectiveShapeUI[4]!=1.f||effectiveShapeUI[1]!=100.f||effectiveGlansUI!=100.f)return 111;
    if(ThrobEnvelope(.2f,.2f,1.05f,false)!=0.f||ThrobEnvelope(1.25f,.2f,1.05f,false)!=0.f||ThrobEnvelope(.725f,.2f,1.05f,false)<.99f)return 105;
    if(ThrobEnvelope(1.15f,1.15f,4.6f,true)!=0.f||ThrobEnvelope(1.35f,1.15f,4.6f,true)<.999f
       ||ThrobEnvelope(3.45f,1.15f,4.6f,true)<.45f||ThrobEnvelope(3.45f,1.15f,4.6f,true)>.6f
       ||ThrobEnvelope(5.75f,1.15f,4.6f,true)!=0.f)return 106;
    if(ThrobEnvelope(1.29f,1.15f,1.05f,true)<.999f||ThrobEnvelope(2.2f,1.15f,1.05f,true)!=0.f)return 115;
    if(AdvanceTwitchClock(5.74f,.02f)<1.15f||AdvanceTwitchClock(5.74f,.02f)>1.17f)return 112;
    ResetStudyControls();AdjustStudyControl(1,-1,1.f);if(throbMode!=3)return 107;
    SaveSettings();ResetStudyControls();settingsLoaded=false;LoadSettings();if(throbMode!=3)return 108;
    throbMode=0;ResetThrobClock();ApplyControlMapping();if(effectiveShapeUI[1]!=sliderUI[1]||effectiveGlansUI!=glansUI)return 109;
    printf("PASS: four modes, max-slider overdrive, independent envelopes, menu cycle, persistence, off restores base\n");return 0;
  }
  if(argc==2&&strcmp(argv[1],"--surface-limit-test")==0){
    unsigned seed=20260922u;auto random=[&](){seed=1664525u*seed+1013904223u;return float((seed>>8)&65535)/32767.5f-1.f;};
    for(int test=0;test<10000;test++){
      V3 p[3],d[3];for(int i=0;i<3;i++){p[i]={random()*10,random()*10,random()*10};d[i]={random()*20,random()*20,random()*20};}
      float floor=.20f+(test%3)*.075f,limit=SurfaceCorrectionLimit(p[0],p[1],p[2],d[0],d[1],d[2],floor);
      V3 n=Cross(p[1]-p[0],p[2]-p[0]);float area=Dot(n,n);
      if(!std::isfinite(limit)||limit<0||limit>1)return 91;
      for(int sample=0;sample<=20;sample++){
        float t=limit*sample/20.f;V3 a=p[0]+d[0]*t,b=p[1]+d[1]*t,c=p[2]+d[2]*t;
        if(Dot(n,Cross(b-a,c-a))<(floor-1e-4f)*area)return 92;
      }
    }
    // Cross a former 1 -> 1/2 backtracking boundary with a tiny change.
    V3 a{0,0,0},b{1,0,0},c{0,1,0};
    float left=SurfaceCorrectionLimit(a,b,c,V3{},V3{-.75001f,0,0},V3{},.25f);
    float right=SurfaceCorrectionLimit(a,b,c,V3{},V3{-.74999f,0,0},V3{},.25f);
    if(fabsf(left-right)>1e-4f)return 93;
    printf("PASS: 10000 surface corrections retain projected area across 21 samples; continuous former half-step boundary\n");return 0;
  }
  if(argc==2&&strcmp(argv[1],"--glans-controls-test")==0){
    // Run from a dedicated test directory: settings are sibling to this EXE.
    const int targets[20]={9,18,19,0,1,2,7,3,8,4,5,6,10,11,12,13,14,15,16,17};
    for(int row=0;row<20;row++){
      ResetStudyControls();AdjustStudyControl(row,1,1);
      float values[20];memcpy(values,sliderUI,sizeof(sliderUI));values[7]=glansUI;values[8]=hangUI;values[9]=(float)physicsState;memcpy(values+10,physUI,sizeof(physUI));values[18]=(float)throbMode;values[19]=idleChatterEnabled?1.f:0.f;
      for(int j=0;j<20;j++){float expected=j==9?(targets[row]==9?0.f:2.f):j>=18?(j==targets[row]?1.f:0.f):(j==targets[row]?51.f:50.f);if(values[j]!=expected)return 31;}
    }
    ResetStudyControls();AdjustStudyControl(6,1,100);if(glansUI!=100)return 32;
    AdjustStudyControl(6,-1,200);if(glansUI!=0)return 33;
    ResetStudyControls();glansUI=73;hangUI=65;sliderUI[2]=61;physUI[7]=44;SaveSettings();
    ResetStudyControls();settingsLoaded=false;LoadSettings();
    if(glansUI!=73||hangUI!=65||sliderUI[2]!=61||physUI[7]!=44)return 34;
    char path[MAX_PATH];SiblingPath(path,"WolverineLive.ini");WritePrivateProfileStringA("Shape","Glans Size",nullptr,path);
    ResetStudyControls();settingsLoaded=false;LoadSettings();if(glansUI!=50||hangUI!=65||sliderUI[2]!=61)return 35;
    ResetStudyControls();if(glansUI!=50||hangUI!=50||sliderUI[2]!=50)return 36;
    printf("PASS: 20 menu rows, glans bounds, persistence, missing-key default, reset\n");return 0;
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
  if(argc>12)sliderUI[6]=max(0.f,min(100.f,(float)atof(argv[12])));
  if(argc>13)physUI[0]=(float)atof(argv[13]);
  if(argc>14)sliderUI[5]=(float)atof(argv[14]);
  if(argc>15){throbMode=max(0,min(3,atoi(argv[15])));throbSizePulse=argc>16?(float)atof(argv[16]):1.f;throbTwitchPulse=argc>17?(float)atof(argv[17]):1.f;throbAngleSizePulse=argc>18?(float)atof(argv[18]):throbTwitchPulse;}
  ApplyControlMapping();int frames=argc>6?atoi(argv[6]):240;
  HINSTANCE hi=GetModuleHandleA(nullptr);WNDCLASSA wc{};wc.lpfnWndProc=HarnessProc;wc.hInstance=hi;wc.lpszClassName="V071Inspection";RegisterClassA(&wc);
  HWND hw=CreateWindowA(wc.lpszClassName,"V0.7.1 inspection",WS_OVERLAPPEDWINDOW,0,0,320,240,nullptr,nullptr,hi,nullptr);
  LoadReal();IDirect3D9* d9=realCreate9(D3D_SDK_VERSION);if(!d9)return 10;
  D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=hw;pp.BackBufferWidth=1000;pp.BackBufferHeight=750;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.EnableAutoDepthStencil=TRUE;pp.AutoDepthStencilFormat=D3DFMT_D16;
  IDirect3DDevice9* dev=nullptr;HRESULT create=d9->CreateDevice(0,D3DDEVTYPE_HAL,hw,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&dev);
  if(FAILED(create))create=d9->CreateDevice(0,D3DDEVTYPE_REF,hw,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&dev);
  if(FAILED(create))printf("D3D unavailable: %08X; CPU geometry tests only, draw/reset tests skipped\n",create);
  if(dev){if(FAILED(dev->CreateVertexBuffer(50915*32,D3DUSAGE_DYNAMIC,0,D3DPOOL_DEFAULT,&graftBuffer,nullptr)))return 12;}
  else graftBuffer=new CpuVertexBuffer(50915*32);
  graftOffset=47050*32;void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);memset(raw,0,50915*32);
  for(UINT i=0;i<collarNormalTriangleCount*3;i++)memcpy((char*)raw+collarNormalTriangleIndices[i]*32,collarNormalTriangleBasePositions+i*3,12);
  for(UINT i=0;i<pelvisControlCount;i++)memcpy((char*)raw+pelvisControlIndices[i]*32,pelvisControlBasePositions+i*3,12);
  for(UINT i=0;i<graftCount;i++)memcpy((char*)raw+(47050+i)*32,morph_base+i*3,12);
  graftBuffer->Unlock();ApplyShape();
  FILE* motion=nullptr;char motionName[MAX_PATH];
  if(GetEnvironmentVariableA("ROOT_MOTION_TRACE",motionName,MAX_PATH))fopen_s(&motion,motionName,"wb");
  std::vector<V3> trace;
  int throbLevel=1;char throbSetting[32];if(GetEnvironmentVariableA("THROB_LEVEL",throbSetting,32))throbLevel=atoi(throbSetting);
  int analysisMode=argc>19?atoi(argv[19]):1;
  float analysisFPS=argc>20?(float)atof(argv[20]):60.f;
  char seriesName[MAX_PATH];sprintf_s(seriesName,"%s.series",argv[1]);FILE* series=nullptr;fopen_s(&series,seriesName,"wb");
  float pressureMax=0.f; float minimumSurfaceZ=1e9f,maximumTether=0.f,maximumTetherRatio=0.f;
  FILE* meshTrace=nullptr;char meshPath[MAX_PATH];
  if(GetEnvironmentVariableA("MESH_TRACE",meshPath,MAX_PATH))fopen_s(&meshTrace,meshPath,"wb");
  char finePath[MAX_PATH];sprintf_s(finePath,"%s.fine-metrics",argv[1]);FILE* fineLog=nullptr;fopen_s(&fineLog,finePath,"wb");
  float pouchLowest=1e9f,pouchCompression=0.f;
  for(int frame=0;frame<frames;frame++){
    float time=frame/analysisFPS,cycle=time<5.75f?time:1.15f+fmodf(time-1.15f,4.6f);
    throbMode=analysisMode?throbLevel:0;
    throbSizePulse=analysisMode==2?0.f:ThrobEnvelope(fmodf(time,3.f),.20f,1.05f,false);
    throbTwitchPulse=analysisMode==3?0.f:ThrobEnvelope(cycle,1.15f,4.6f,true);
    throbAngleSizePulse=analysisMode==2?0.f:ThrobEnvelope(cycle,1.15f,1.05f,true);
    float driveScale=1.f;char stopValue[64];if(GetEnvironmentVariableA("DRIVE_STOP",stopValue,64)&&time>=atof(stopValue))driveScale=0.f;
    char resumeValue[64];if(GetEnvironmentVariableA("DRIVE_RESUME",resumeValue,64)&&time>=atof(resumeValue))driveScale=1.f;
    char weightShift[8];if(GetEnvironmentVariableA("WEIGHT_SHIFT",weightShift,8)){
      float phase=max(0.f,time-4.f),shift=1.2f*sinf(phase*1.1f),forward=.5f*sinf(phase*.8f);
      collisionCapsuleOverride=true;overrideLeftA={2.f+forward,-7.8f+shift,79.f};overrideRightA={2.f+forward,7.8f+shift,79.f};overrideLeftB={1.f,-8.2f-shift*.35f,43.f};overrideRightB={1.f,8.2f-shift*.35f,43.f};
    }
    ApplyControlMapping();UpdateConstraintSolver(1.f/analysisFPS,driveScale*(argc>10?(float)atof(argv[10]):0.f)*sinf(time*5.f),driveScale*(argc>10?(float)atof(argv[10]):0.f)*sinf(time*7.f));ApplyReplayShape();
    for(unsigned vi=0;vi<rsCount;vi++)minimumSurfaceZ=min(minimumSurfaceZ,rsPositions[vi].z);for(int si=0;si<2;si++)maximumTether=max(maximumTether,Length(ballNodes[si]-BallAnchor(si)));
    if(meshTrace&&time>=0.f&&time<16.f&&frame%4==0)fwrite(r14Positions,sizeof(V3),r14Count,meshTrace);
    if(frame%4==0){
      float minimum=1e9f;unsigned invalid=0,reversed=0;float seamError=0;
      for(unsigned i=0;i<rsCount;i++)if(!std::isfinite(rsPositions[i].x)||!std::isfinite(rsPositions[i].y)||!std::isfinite(rsPositions[i].z))invalid++;
      for(unsigned i=0;i<rsIndexCount;i+=3){V3 a=rsPositions[rsIndices[i]],b=rsPositions[rsIndices[i+1]],c=rsPositions[rsIndices[i+2]],normal=Cross(b-a,c-a);minimum=min(minimum,Length(normal));unsigned parent=rsFaceParent[i/3]*3;V3 x=r14Positions[r14Indices[parent]],y=r14Positions[r14Indices[parent+1]],z=r14Positions[r14Indices[parent+2]];if(Dot(normal,Cross(y-x,z-x))<0)reversed++;}
      for(unsigned i=0;i<paSeamCount;i++)seamError=max(seamError,Length(r14Positions[paSeamR14[i]]-paBodyBefore[paSeamBody[i]]));
      float row[7]={time,rsAppliedFraction,rsFineFraction,minimum,float(invalid),float(reversed),seamError};fwrite(row,sizeof(float),7,fineLog);
    }
    for(int b=0;b<2;b++){
      pressureMax=max(pressureMax,lobeCompression[b]);
      V3 x=LobePressureOffset({1,0,0},b),y=LobePressureOffset({0,1,0},b),z=LobePressureOffset({0,0,1},b);
      if(lobeCompression[b]<0.f||lobeCompression[b]>.12001f||fabsf(Dot(x,Cross(y,z))-1.f)>1e-4f)return 130;
      if(Length(LobePressureOffset(LobePressureOffset({1,2,3},b),b,true)-V3{1,2,3})>1e-4f)return 131;
    }
    for(int si=0;si<2;si++)maximumTetherRatio=max(maximumTetherRatio,Length(ballNodes[si]-BallAnchor(si))/CPTetherLimit(si));pouchLowest=min(pouchLowest,cpMinimumGap);pouchCompression=max(pouchCompression,max(cpCompression[0],cpCompression[1]));
    float info[8]={time,effectiveShapeUI[4],effectiveShapeUI[1],constraintRestLength,shaftSpring.pitch,shaftSpring.pitchVelocity,throbTwitchPulse,logicalShaftBodyRadius};
    fwrite(info,sizeof(float),8,series);fwrite(shaftNodes,sizeof(V3),shaftNodeCount,series);fwrite(ballNodes,sizeof(V3),2,series);
    for(UINT id : {355u,354u,36u,37u,351u,17u,16u,60u,67u,74u,356u,108u,318u,9000u})fwrite(&r14Positions[id],sizeof(V3),1,series);
    if(motion){
      float factors[3]={debugRampFraction,debugRapheFraction,debugSmoothFraction};fwrite(factors,sizeof(float),3,motion);
      fwrite(r14Positions,sizeof(V3),r14NewStart,motion);
      void* body=nullptr;graftBuffer->Lock(0,0,&body,0);
      for(UINT i=0;i<pelvisControlCount;i++)fwrite((char*)body+pelvisControlIndices[i]*32,12,1,motion);
      graftBuffer->Unlock();
    }
    trace.push_back(ballNodes[0]);trace.push_back(ballNodes[1]);trace.push_back(BallAnchor(0));trace.push_back(BallAnchor(1));
    for(int band=0;band<4;band++){V3 sum{};int count=0;for(UINT i=0;i<graftCount;i++){float w=suspensionWeight[i];if(w>band*.25f&&w<=(band+1)*.25f){sum=sum+graftDeformedPositions[i];count++;}}trace.push_back(sum/(float)max(count,1));}
  }
  if(motion)fclose(motion);fclose(series);if(meshTrace)fclose(meshTrace);fclose(fineLog);
  printf("SOLVER maximum_tether_ratio %.6f\n",pdMaxTetherRatio);printf("BOUNDS minimum_surface_z=%.6f maximum_tether=%.6f maximum_tether_ratio=%.6f\n",minimumSurfaceZ,maximumTether,maximumTetherRatio);printf("PRESSURE final %.6f %.6f max %.6f, determinant and inverse PASS\n",lobeCompression[0],lobeCompression[1],pressureMax);
  // Regression: moving the collision proxies without touching the surface
  // must not change shaft shape memory (the old enclosing-sphere fade did).
  {
    std::vector<V3> saved(r14Positions,r14Positions+r14Count),first(r14Count);
    V3 savedBalls[2]={ballNodes[0],ballNodes[1]};
    PreserveShaftRaphe();PreserveAxialRaphe();memcpy(first.data(),r14Positions,sizeof(r14Positions));
    memcpy(r14Positions,saved.data(),sizeof(r14Positions));
    ballNodes[0]=ballNodes[0]+V3{100,100,100};ballNodes[1]=ballNodes[1]+V3{100,100,100};
    PreserveShaftRaphe();PreserveAxialRaphe();
    bool same=memcmp(first.data(),r14Positions,sizeof(r14Positions))==0;
    ballNodes[0]=savedBalls[0];ballNodes[1]=savedBalls[1];
    memcpy(r14Positions,saved.data(),sizeof(r14Positions));
    if(!same){printf("FAIL: remote lobe proxies altered ridge shape\n");return 124;}
    printf("PASS: ridge shape independent of lobe proximity proxies\n");
  }
  std::vector<V3> output(graftCount+pelvisControlCount);graftBuffer->Lock(0,0,&raw,0);
  for(UINT i=0;i<graftCount;i++)memcpy(&output[i],(char*)raw+(47050+i)*32,12);
  for(UINT i=0;i<pelvisControlCount;i++)memcpy(&output[graftCount+i],(char*)raw+pelvisControlIndices[i]*32,12);
  char bodyFile[MAX_PATH];sprintf_s(bodyFile,"%s.body",argv[1]);FILE* bodyOut=nullptr;fopen_s(&bodyOut,bodyFile,"wb");for(unsigned k=0;k<paBodyNormalCount;k++)fwrite((char*)raw+paBodyNormalIDs[k]*32,1,32,bodyOut);fclose(bodyOut);
  graftBuffer->Unlock();FILE* fp=nullptr;fopen_s(&fp,argv[1],"wb");if(!fp)return 13;fwrite(output.data(),sizeof(V3),output.size(),fp);fclose(fp);
  printf("captured %u points; size %.0f state %d; %d deterministic frames\n",(unsigned)output.size(),size,physicsState,frames);
  printf("radius %.6f length %.6f firstnodes=(%.3f %.3f %.3f) (%.3f %.3f %.3f)\n",logicalShaftBodyRadius,constraintRestLength,shaftNodes[1].x,shaftNodes[1].y,shaftNodes[1].z,shaftNodes[2].x,shaftNodes[2].y,shaftNodes[2].z);
  char nodesFile[MAX_PATH];sprintf_s(nodesFile,"%s.nodes",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(shaftNodes,sizeof(V3),shaftNodeCount,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.balls",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(ballNodes,sizeof(V3),2,fp);V3 anchors[2]={BallAnchor(0),BallAnchor(1)};fwrite(anchors,sizeof(V3),2,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.pouch",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(cpCenters,sizeof(V3),2,fp);fwrite(cpRenderRadii,sizeof(V3),2,fp);fwrite(cpWorldBasis,sizeof(V3),6,fp);fwrite(cpOmega,sizeof(V3),2,fp);fwrite(cpCompression,sizeof(float),2,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.trace",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(trace.data(),sizeof(V3),trace.size(),fp);fclose(fp);
  sprintf_s(nodesFile,"%s.r14",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(r14Positions,sizeof(V3),r14Count,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.packed",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(r14Packed,1,sizeof(r14Packed),fp);fclose(fp);
  sprintf_s(nodesFile,"%s.render-packed",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(rsPacked,1,sizeof(rsPacked),fp);fclose(fp);
  sprintf_s(nodesFile,"%s.render-indices",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(rsIndices,1,sizeof(rsIndices),fp);fclose(fp);
  printf("ROUNDED coarse=%.8f fine=%.8f vertices=%u faces=%u\n",rsAppliedFraction,rsFineFraction,rsCount,rsIndexCount/3);
  printf("POUCH gap=%.8f compression=%.8f\n",pouchLowest,pouchCompression);
  if(pouchLowest<-.0001f||pouchCompression>.06501f)return 141;
  printf("ATTACHMENT fraction %.8f\n",paAppliedFraction);
  printf("axial t=%f radius=%f\n",axialReferenceT,axialCoreRadius);
  sprintf_s(nodesFile,"%s.axial",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(axialMoved,sizeof(float),r14Count,fp);fclose(fp);
  if(physicsState!=2){
    float worstDeficit=0.f;
    for(UINT id : {17u,16u,60u,67u,74u,356u}){
      float t=ClosestRestShaftFlex(AxialMaterialRest(id));
      if(t>.42f)continue;
      V3 center{},tangent{};SampleShaftChain(t,center,tangent);
      V3 ventral=Unit(Cross(Unit(V3{0,1,0}-tangent*tangent.y),tangent));
      worstDeficit=max(worstDeficit,axialCoreRadius-Dot(r14Positions[id]-center,ventral));
    }
    printf("CONTOUR proximal inward deficit %.8f (limit %.8f)\n",worstDeficit,axialCoreRadius*.001f);
    if(worstDeficit>axialCoreRadius*.001f)return 125;
  }
  bool renderOK=dev?TestR14Draw(dev,argv[1],pp):true;
  ReleaseR14();if(graftBuffer){graftBuffer->Release();graftBuffer=nullptr;}if(dev)dev->Release();d9->Release();DestroyWindow(hw);return renderOK?0:71;
}




