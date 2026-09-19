#ifndef RUNTIME_SOURCE
#define RUNTIME_SOURCE "../src/runtime/d3d9_proxy.cpp"
#endif
#include RUNTIME_SOURCE
static LRESULT CALLBACK HarnessProc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
int main(int argc,char** argv){
  if(argc<4){printf("usage: harness output.bin size state [angle] [scrotum] [frames]\n");return 2;}
  float size=(float)atof(argv[2]); physicsState=atoi(argv[3]);
  for(int i=0;i<7;i++)sliderUI[i]=50.f;
  sliderUI[0]=sliderUI[1]=sliderUI[2]=sliderUI[3]=size;
  if(argc>4)sliderUI[4]=(float)atof(argv[4]);if(argc>5)sliderUI[3]=(float)atof(argv[5]);
  if(argc>7)sliderUI[2]=(float)atof(argv[7]);if(argc>8)sliderUI[1]=(float)atof(argv[8]);
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
  for(int frame=0;frame<frames;frame++){for(int sub=0;sub<3;sub++)StepConstraintSolver(1.f/180.f,0.f,0.f);ApplyShape();}
  std::vector<V3> output(graftCount+pelvisControlCount);graftBuffer->Lock(0,0,&raw,0);
  for(UINT i=0;i<graftCount;i++)memcpy(&output[i],(char*)raw+(47050+i)*32,12);
  for(UINT i=0;i<pelvisControlCount;i++)memcpy(&output[graftCount+i],(char*)raw+pelvisControlIndices[i]*32,12);
  graftBuffer->Unlock();FILE* fp=nullptr;fopen_s(&fp,argv[1],"wb");if(!fp)return 13;fwrite(output.data(),sizeof(V3),output.size(),fp);fclose(fp);
  printf("captured %u points; size %.0f state %d; %d deterministic frames at 60Hz\n",(unsigned)output.size(),size,physicsState,frames);
  printf("radius %.6f length %.6f firstnodes=(%.3f %.3f %.3f) (%.3f %.3f %.3f)\n",logicalShaftBodyRadius,constraintRestLength,shaftNodes[1].x,shaftNodes[1].y,shaftNodes[1].z,shaftNodes[2].x,shaftNodes[2].y,shaftNodes[2].z);
  char nodesFile[MAX_PATH];sprintf_s(nodesFile,"%s.nodes",argv[1]);fopen_s(&fp,nodesFile,"wb");fwrite(shaftNodes,sizeof(V3),shaftNodeCount,fp);fclose(fp);
  graftBuffer->Release();graftBuffer=nullptr;dev->Release();d9->Release();DestroyWindow(hw);return 0;
}
