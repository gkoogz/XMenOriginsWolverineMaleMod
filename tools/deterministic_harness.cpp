#ifndef RUNTIME_SOURCE
#define RUNTIME_SOURCE "../src/runtime/d3d9_proxy.cpp"
#endif
#include RUNTIME_SOURCE
static LRESULT CALLBACK HarnessProc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
int main(int argc,char** argv){
  if(argc==2&&strcmp(argv[1],"--glans-controls-test")==0){
    // Run from a dedicated test directory: settings are sibling to this EXE.
    const int targets[18]={0,1,2,7,3,8,4,5,6,9,10,11,12,13,14,15,16,17};
    for(int row=0;row<18;row++){
      ResetStudyControls();AdjustStudyControl(row,1,1);
      float values[18];memcpy(values,sliderUI,sizeof(sliderUI));values[7]=glansUI;values[8]=hangUI;values[9]=(float)physicsState;memcpy(values+10,physUI,sizeof(physUI));
      for(int j=0;j<18;j++){float expected=j==9?(targets[row]==9?0.f:2.f):(j==targets[row]?51.f:50.f);if(values[j]!=expected)return 31;}
    }
    ResetStudyControls();AdjustStudyControl(3,1,100);if(glansUI!=100)return 32;
    AdjustStudyControl(3,-1,200);if(glansUI!=1)return 33;
    ResetStudyControls();glansUI=73;hangUI=65;sliderUI[2]=61;physUI[7]=44;SaveSettings();
    ResetStudyControls();settingsLoaded=false;LoadSettings();
    if(glansUI!=73||hangUI!=65||sliderUI[2]!=61||physUI[7]!=44)return 34;
    char path[MAX_PATH];SiblingPath(path,"WolverineLive.ini");WritePrivateProfileStringA("Shape","Glans Size",nullptr,path);
    ResetStudyControls();settingsLoaded=false;LoadSettings();if(glansUI!=50||hangUI!=65||sliderUI[2]!=61)return 35;
    ResetStudyControls();if(glansUI!=50||hangUI!=50||sliderUI[2]!=50)return 36;
    printf("PASS: 18 menu rows, glans bounds, persistence, missing-key default, reset\n");return 0;
  }
  if(argc==2&&strcmp(argv[1],"--settings-test")==0){LoadSettings();float loaded=hangUI;float shape[7];memcpy(shape,sliderUI,sizeof(shape));hangUI=73;SaveSettings();settingsLoaded=false;hangUI=50;LoadSettings();bool same=memcmp(shape,sliderUI,sizeof(shape))==0;printf("Hang loaded=%.0f saved/reloaded=%.0f existing_shape_preserved=%d\n",loaded,hangUI,same?1:0);return hangUI==73&&same?0:21;}

  if(argc<4){printf("usage: harness output.bin size state [angle] [scrotum] [frames]\n");return 2;}
  float size=(float)atof(argv[2]); physicsState=atoi(argv[3]);
  for(int i=0;i<7;i++)sliderUI[i]=50.f;
  sliderUI[0]=sliderUI[1]=sliderUI[2]=sliderUI[3]=size;
  if(argc>4)sliderUI[4]=(float)atof(argv[4]);if(argc>5)sliderUI[3]=(float)atof(argv[5]);
  if(argc>7)sliderUI[2]=(float)atof(argv[7]);if(argc>8)sliderUI[1]=(float)atof(argv[8]);
  if(argc>9)hangUI=(float)atof(argv[9]);
  if(argc>11)glansUI=max(1.f,min(100.f,(float)atof(argv[11])));
  ApplyControlMapping();int frames=argc>6?atoi(argv[6]):240;
  HINSTANCE hi=GetModuleHandleA(nullptr);WNDCLASSA wc{};wc.lpfnWndProc=HarnessProc;wc.hInstance=hi;wc.lpszClassName="V071Inspection";RegisterClassA(&wc);
  HWND hw=CreateWindowA(wc.lpszClassName,"V0.7.1 inspection",WS_OVERLAPPEDWINDOW,0,0,320,240,nullptr,nullptr,hi,nullptr);
  LoadReal();IDirect3D9* d9=realCreate9(D3D_SDK_VERSION);if(!d9)return 10;
  D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=hw;pp.BackBufferWidth=320;pp.BackBufferHeight=240;pp.BackBufferFormat=D3DFMT_A8R8G8B8;
  IDirect3DDevice9* dev=nullptr;if(FAILED(d9->CreateDevice(0,D3DDEVTYPE_HAL,hw,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&dev)))return 11;
  if(FAILED(dev->CreateVertexBuffer(50915*32,D3DUSAGE_DYNAMIC,0,D3DPOOL_DEFAULT,&graftBuffer,nullptr)))return 12;
  graftOffset=47050*32;void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);memset(raw,0,50915*32);
  for(UINT i=0;i<collarNormalTriangleCount*3;i++)memcpy((char*)raw+collarNormalTriangleIndices[i]*32,collarNormalTriangleBasePositions+i*3,12);
  for(UINT i=0;i<pelvisControlCount;i++)memcpy((char*)raw+pelvisControlIndices[i]*32,pelvisControlBasePositions+i*3,12);
  for(UINT i=0;i<graftCount;i++)memcpy((char*)raw+(47050+i)*32,morph_base+i*3,12);
  graftBuffer->Unlock();ApplyShape();
  std::vector<V3> trace;
  for(int frame=0;frame<frames;frame++){for(int sub=0;sub<3;sub++){float drive=argc>10?(float)atof(argv[10]):0.f;float time=(frame+sub/3.f)/60.f;StepConstraintSolver(1.f/180.f,drive*sinf(time*5.f),drive*sinf(time*7.f));}ApplyShape();
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
  sprintf_s(nodesFile,"%s.balls",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(ballNodes,sizeof(V3),2,fp);V3 anchors[2]={BallAnchor(0),BallAnchor(1)};fwrite(anchors,sizeof(V3),2,fp);fclose(fp);
  sprintf_s(nodesFile,"%s.trace",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(trace.data(),sizeof(V3),trace.size(),fp);fclose(fp);
  graftBuffer->Release();graftBuffer=nullptr;dev->Release();d9->Release();DestroyWindow(hw);return 0;
}
