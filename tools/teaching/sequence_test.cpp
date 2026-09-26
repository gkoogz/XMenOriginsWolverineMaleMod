#include "../../src/runtime/d3d9_proxy.cpp"
#include "../r14/cpu_buffer.h"
#include <fstream>
#define CHECK(x) do {if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);return false;}}while(0)
static bool FluidTests(){
 for(int fps:{15,30,60,120,144}){
  teaching::Timeline t;teaching::Fluid f;t.Start();f.Begin({0,0,80});int maximum=0;
  for(int i=0;i<21*fps;i++){
   t.Advance(1.f/fps);f.Advance(1.f/fps,{0,0,80},{1,0,0},{0,0,-98});
   maximum=max(maximum,f.Live());
   for(auto& strand:f.strands)for(int n=0;n<strand.count;n++)CHECK(std::isfinite(strand.nodes[n].p.x)&&Length(strand.nodes[n].p)<1000);
  }
  CHECK(!t.active&&t.time==20);CHECK(f.emitted==6&&f.detached==6&&maximum>0&&f.Live()==0);
  printf("PASS %d FPS: six emissions, six detachments, bounded finite particles, completion/cleanup; max nodes=%d\n",fps,maximum);
 }
 teaching::Fluid a,b;a.Begin({0,0,80});b.Begin({0,0,80});
 for(int i=0;i<8*30;i++)a.Advance(1.f/30,{0,0,80},{1,0,0},{0,0,-98});
 for(int i=0;i<8*120;i++)b.Advance(1.f/120,{0,0,80},{1,0,0},{0,0,-98});
 CHECK(a.emitted==b.emitted&&a.Live()==b.Live());
 for(int s=0;s<8;s++)for(int n=0;n<a.strands[s].count;n++)CHECK(Length(a.strands[s].nodes[n].p-b.strands[s].nodes[n].p)<.0001f);
 a.Advance(.5f,{0,0,80},{1,0,0},{0,0,-98});CHECK(!a.ready&&a.Live()==0);
 b.Advance(.01f,{100,0,80},{1,0,0},{0,0,-98});CHECK(!b.ready&&b.Live()==0);
 teaching::Fluid filament;filament.Begin({0,0,80});
 for(int i=0;i<862;i++)filament.Advance(1.f/120,{0,0,80},{1,0,0},{0,0,-98});
 float arc=0;const auto& stream=filament.strands[2];
 for(int i=1;i<stream.count;i++)arc+=Length(stream.nodes[i].p-stream.nodes[i-1].p);
 CHECK(arc>2.f*teaching::tracerScale&&arc<8.f*teaching::tracerScale&&!stream.attached);
 for(float viscosity:{.25f,1.f,3.f}){
  teaching::Fluid moving;moving.Begin({0,0,80});moving.viscosity=viscosity;
  for(int i=0;i<21*120;i++){
   float t=i/120.f;V3 tip{2.f*sinf(t),0,80.f+sinf(t*2.f)};
   moving.Advance(1.f/120,tip,Unit(V3{1,.2f*sinf(t),.2f*cosf(t)}),{0,0,-98});
   for(const auto& f:moving.strands)if(f.count&&f.attached)CHECK(Length(f.nodes[f.count-1].p-tip)<.001f);
  }
  CHECK(moving.emitted==6&&moving.detached==6&&!moving.Live());
 }
 printf("PASS exact 30/120 FPS match, stalls/teleports, moving anchor at three viscosity settings, detached filament arc %.3f\n",arc);return true;
}
static void InitMesh(){
 graftBuffer=new CpuVertexBuffer(50915*32);graftOffset=47050*32;void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);memset(raw,0,50915*32);
 for(UINT i=0;i<collarNormalTriangleCount*3;i++)memcpy((char*)raw+collarNormalTriangleIndices[i]*32,collarNormalTriangleBasePositions+i*3,12);
 for(UINT i=0;i<pelvisControlCount;i++)memcpy((char*)raw+pelvisControlIndices[i]*32,pelvisControlBasePositions+i*3,12);
 for(UINT i=0;i<graftCount;i++)memcpy((char*)raw+(47050+i)*32,morph_base+i*3,12);
 graftBuffer->Unlock();shapeDirty=true;ApplyShape();
}
static bool PulseTests(){
 ResetStudyControls();throbMode=0;teachingTimeline.Start();teachingTimeline.time=7;
 ApplyControlMapping();CHECK(effectiveShapeUI[1]==54&&effectiveShapeUI[2]==58&&effectiveShapeUI[4]==42&&effectiveGlansUI==54);
 CHECK(sliderUI[1]==50&&sliderUI[2]==50&&sliderUI[4]==50&&glansUI==50);
 for(float angle:{1.f,4.f,10.f,50.f,85.f,100.f}){
  sliderUI[4]=angle;ApplyControlMapping();CHECK(effectiveShapeUI[4]>=1&&effectiveShapeUI[4]<=100);
  if(angle<=4||angle>=85)CHECK(effectiveShapeUI[2]==sliderUI[2]);
 }
 CancelTeaching();ApplyControlMapping();CHECK(effectiveShapeUI[2]==sliderUI[2]&&effectiveShapeUI[4]==sliderUI[4]);
 printf("PASS classroom pulse amplitude, stored controls, low/high-angle fold guards, cancellation\n");return true;
}
static bool IntegrationTests(){
 for(int state=0;state<3;state++){
  ResetStudyControls();physicsState=state;throbMode=2;throbSizePulse=.4f;throbTwitchPulse=.3f;
  sliderUI[0]=sliderUI[1]=100;ApplyControlMapping();InitMesh();
  float before[7];memcpy(before,sliderUI,sizeof(before));float beforePhysics[8];memcpy(beforePhysics,physUI,sizeof(beforePhysics));
  teachingTimeline.Start();float minMode=10;float identity[12]={1,0,0,0,0,1,0,0,0,0,1,0};
  V3 oldTip{},dir{};float worst=0;
  for(int i=0;i<601;i++){
   teachingTimeline.Advance(1.f/30);ApplyControlMapping();UpdateConstraintSolver(1.f/30,0,0);ApplyShape();
   V3 tip;CHECK(TeachingEmitter(identity,tip,dir));if(i)worst=max(worst,Length(tip-oldTip));oldTip=tip;
   if(i>210&&i<390)minMode=min(minMode,shaftMode);
   for(UINT v=0;v<r14Count;v++)CHECK(std::isfinite(r14Positions[v].x)&&std::isfinite(r14Positions[v].y)&&std::isfinite(r14Positions[v].z));
  }
  CHECK(!teachingTimeline.active&&minMode<.03f&&physicsState==state&&throbMode==2);
  CHECK(!memcmp(before,sliderUI,sizeof(before))&&!memcmp(beforePhysics,physUI,sizeof(beforePhysics)));
  teachingTimeline.Start();AdjustStudyControl(1,1,1);CHECK(!teachingTimeline.active);teachingTimeline.Start();ResetStudyControls();CHECK(!teachingTimeline.active);
  printf("PASS 20s integrated state=%d: finite surface, firmness reached %.6f, saved preset preserved, cancel/reset; max tip step %.3f\n",state,minMode,worst);
  graftBuffer->Release();graftBuffer=nullptr;
 }
 return true;
}
static LRESULT CALLBACK Proc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
static bool RenderTest(){
 ResetStudyControls();physicsState=0;sliderUI[4]=50;ApplyControlMapping();InitMesh();
 WNDCLASSA wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandle(nullptr);wc.lpszClassName="TeachingFluidTest";RegisterClassA(&wc);
 HWND window=CreateWindowA(wc.lpszClassName,"Teaching fluid offline test",0,0,0,1000,750,nullptr,nullptr,wc.hInstance,nullptr);
 LoadReal();CHECK(realCreate9);IDirect3D9* d9=realCreate9(D3D_SDK_VERSION);CHECK(d9);
 D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.BackBufferWidth=1000;pp.BackBufferHeight=750;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.EnableAutoDepthStencil=TRUE;pp.AutoDepthStencilFormat=D3DFMT_D24S8;
 IDirect3DDevice9* d=nullptr;CHECK(SUCCEEDED(d9->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&d)));
 // Use the real game's captured shader to verify reflection and matrix convention.
 std::ifstream in("C:/Games/X-Men Origins Wolverine/Binaries/R14Capture_20260920_023631/draw_000_vs.bin",std::ios::binary);
 std::vector<char> bytes((std::istreambuf_iterator<char>(in)),{});CHECK(!bytes.empty());
 IDirect3DVertexShader9* gameVS=nullptr;CHECK(SUCCEEDED(d->CreateVertexShader((DWORD*)bytes.data(),&gameVS)));d->SetVertexShader(gameVS);
 ShaderLayout* layout=GetShaderLayout(d);CHECK(layout&&layout->valid&&layout->viewValid);
 float bone[12]={1,0,0,0,0,1,0,0,0,0,1,0};float local[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
 float view[16]={1.f/45,0,0,0,0,0,1.f/200,0,0,1.f/33.75f,0,0,-.65f,-2.f,.5f,1};
 d->SetVertexShaderConstantF(layout->boneRegister,bone,3);d->SetVertexShaderConstantF(layout->localRegister,local,4);d->SetVertexShaderConstantF(layout->viewRegister,view,4);
 float l[16],v[16],b[12];CHECK(TeachingMatrices(d,l,v,b));CHECK(!memcmp(local,l,sizeof(l))&&!memcmp(view,v,sizeof(v))&&!memcmp(bone,b,sizeof(b)));
 V3 tip,dir;CHECK(TeachingEmitter(bone,tip,dir));
 CHECK(Dot(dir,Unit(TeachingRing(r14RingCount-6)-TeachingRing(r14CrownRing)))>.8f);
 // Validate rotated/translated tip transport independently of shader rendering.
 float moved[12]={0,-1,0,12,1,0,0,25,0,0,1,-4};V3 mt,md;CHECK(TeachingEmitter(moved,mt,md));
 CHECK(Length(mt-V3{12-tip.y,25+tip.x,tip.z-4})<.001f&&Length(md-V3{-dir.y,dir.x,dir.z})<.001f);
 CHECK(EnsureTeachingShaders(d));
 IDirect3DVertexBuffer9* source=nullptr;d->CreateVertexBuffer(64,0,D3DFVF_XYZ|D3DFVF_DIFFUSE,D3DPOOL_MANAGED,&source,nullptr);CHECK(source);
 d->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);d->SetStreamSource(0,source,0,16);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);d->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);d->SetRenderState(D3DRS_COLORWRITEENABLE,15);
 teachingTimeline.Start();teachingFluid.Begin(tip);teachingFluidTime=0;
 for(int frame=0;frame<=870;frame++){
  teachingTimeline.Advance(1.f/120);float dt=(float)(teachingTimeline.time-teachingFluidTime);teachingFluidTime=teachingTimeline.time;
  teachingFluid.Advance(dt,tip,dir,{0,0,-98});
 }
 CHECK(teachingFluid.Live()>0);
 d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(26,36,46),1,0);CHECK(SUCCEEDED(d->BeginScene()));
 // Render the actual 1.3 refined anatomy in a neutral diagram material.
 std::vector<TeachingVertex> anatomy;anatomy.reserve(r14IndexCount);
 for(UINT i=0;i<r14IndexCount;i++){UINT id=r14Indices[i];float light=.4f+.6f*fabsf(Unit(r14Normals[id]).y);int c=(int)(light*180);anatomy.push_back({r14Positions[id],D3DCOLOR_XRGB(c,c,c)});}
 d->SetVertexShader(teachingVS);d->SetPixelShader(teachingPS);d->SetVertexShaderConstantF(0,local,4);d->SetVertexShaderConstantF(4,view,4);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ZENABLE,TRUE);
 CHECK(SUCCEEDED(d->DrawPrimitiveUP(D3DPT_TRIANGLELIST,(UINT)anatomy.size()/3,anatomy.data(),sizeof(TeachingVertex))));
 d->SetVertexShader(gameVS);d->SetVertexShaderConstantF(layout->boneRegister,bone,3);d->SetVertexShaderConstantF(layout->localRegister,local,4);d->SetVertexShaderConstantF(layout->viewRegister,view,4);
 d->SetStreamSource(0,source,0,16);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
 ++renderFrameSerial;DrawTeachingFluid(d);CHECK(teachingDraws>0&&SUCCEEDED(teachingLastDraw));
 IDirect3DVertexShader9* check=nullptr;d->GetVertexShader(&check);CHECK(check==gameVS);check->Release();
 IDirect3DVertexBuffer9* checkBuffer=nullptr;UINT offset=0,stride=0;d->GetStreamSource(0,&checkBuffer,&offset,&stride);CHECK(checkBuffer==source&&offset==0&&stride==16);checkBuffer->Release();
 DWORD cull=0,write=0;d->GetRenderState(D3DRS_CULLMODE,&cull);d->GetRenderState(D3DRS_ZWRITEENABLE,&write);CHECK(cull==D3DCULL_CW&&write==TRUE);
 CHECK(TeachingMatrices(d,l,v,b));
 for(int i=0;i<16;i++){if(local[i]!=l[i])printf("local[%d] %g -> %g\n",i,local[i],l[i]);if(view[i]!=v[i])printf("view[%d] %g -> %g\n",i,view[i],v[i]);}
 for(int i=0;i<12;i++)if(bone[i]!=b[i])printf("bone[%d] %g -> %g\n",i,bone[i],b[i]);
 CHECK(!memcmp(local,l,sizeof(l))&&!memcmp(view,v,sizeof(v))&&!memcmp(bone,b,sizeof(b)));
 unsigned count=teachingDraws;DrawTeachingFluid(d);CHECK(count==teachingDraws);
 d->EndScene();IDirect3DSurface9* back=nullptr;d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back);CHECK(back);CHECK(SUCCEEDED(D3DXSaveSurfaceToFileA("teaching-fluid.png",D3DXIFF_PNG,back,nullptr,nullptr)));back->Release();
 ReleaseTeaching();CHECK(!teachingTimeline.active&&!teachingVS&&!teachingPS&&teachingFluid.Live()==0);
 CHECK(SUCCEEDED(d->Reset(&pp)));CHECK(EnsureTeachingShaders(d));ReleaseTeaching();
 source->Release();gameVS->Release();graftBuffer->Release();graftBuffer=nullptr;
 for(UINT i=0;i<shaderLayoutCount;i++)if(shaderLayouts[i].shader)shaderLayouts[i].shader->Release();shaderLayoutCount=0;
 d->Release();d9->Release();DestroyWindow(window);
 printf("PASS real shader reflection, transformed emitter, D3D draw, pipeline restoration, duplicate-pass suppression, device reset/recreation; teaching-fluid.png\n");return true;
}
int main(int argc,char** argv){
 if(argc>1&&strcmp(argv[1],"--render")==0)return RenderTest()?0:1;
 if(argc>1&&strcmp(argv[1],"--fluid")==0)return FluidTests()?0:1;
 if(!FluidTests()||!PulseTests())return 1;if(!IntegrationTests())return 2;return 0;
}
