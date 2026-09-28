#define main ThreadReferenceMain
#include "thread_test.cpp"
#undef main
#include <d3d9.h>
#include <d3dx9.h>
static bool FluidProjectSplat(const volumeFluid::FluidImpact& receiver,V3 p,volumeFluid::FluidImpact& out){out=receiver;out.p={p.x,p.y,0};out.n={0,0,1};return p.x<80;}
static bool FluidResolveSplat(const volumeFluid::FluidImpact& a,V3& p,V3& n){p=a.p;n=a.n;return true;}
static bool ImpactPlane(V3 from,V3 to,float radius,volumeFluid::FluidImpact& h){if(from.z>=0&&to.z<0){h={};h.p=from+(to-from)*(from.z/(from.z-to.z));h.n={0,0,1};return true;}return false;}
#include "../../src/runtime/fluid_stains.h"
static double Mass(const std::vector<float>& field){double sum=0;for(float f:field){if(!std::isfinite(f)||f<0)return -1;sum+=f;}return sum*volumeFluid::splatTexel*volumeFluid::splatTexel;}
static bool ModelTest(){
 CHECK(volumeFluid::splatBakes.Open());CHECK(volumeFluid::splatBakes.frames[0]==26&&volumeFluid::splatBakes.frames[1]==64);
 volumeFluid::FluidImpact hit{};hit.n={0,0,1};hit.velocity={85,0,-45};hit.volume=6;hit.sourceId=17;
 volumeFluid::SplatModel m;CHECK(m.Add({hit},1000));m.Update(1000);CHECK(m.marks.size()==1);CHECK(fabs(Mass(m.marks[0].density)-6)<.0001);
 auto initial=m.marks[0].density;auto active=m.marks[0].active[0];
 for(DWORD t=1016;t<1600;t+=16){m.Update(t);CHECK(m.projectionBudget>=0&&m.lastShapeUpdates<=4);CHECK(fabs(Mass(m.marks[0].density)-6)<.0001);}
 double difference=0;for(size_t i=0;i<initial.size();i++)difference+=fabs(initial[i]-m.marks[0].density[i]);CHECK(difference>1);
 CHECK(m.marks[0].active.empty());auto settled=m.marks[0].density;unsigned revision=m.marks[0].revision;
 m.Update(3000);CHECK(m.marks[0].density==settled&&m.marks[0].revision==revision);
 auto anchors=m.marks[0].samples;hit.p={3,0,0};hit.volume=12;CHECK(m.Add({hit},3010));for(DWORD t=3016;t<4000;t+=16)m.Update(t);
 CHECK(m.marks.size()==1);CHECK(fabs(Mass(m.marks[0].density)-18)<.001);
 for(size_t i=0;i<anchors.size();i++)if(anchors[i].state==1)CHECK(Length(anchors[i].anchor.p-m.marks[0].samples[i].anchor.p)<1e-6f);
 printf("PASS embedded bakes, per-contact volume, animated changing footprint, fixed settled texture, overlapping accumulation and retained anchors\n");
 volumeFluid::SplatModel ledge;ledge.project=[](const volumeFluid::FluidImpact& a,V3 p,volumeFluid::FluidImpact& out){out=a;out.p=p;return p.x<3;};hit.p={};ledge.Add({hit},1000);for(DWORD t=1000;t<4000;t+=16)ledge.Update(t);
 int blocked=0;for(auto& sample:ledge.marks[0].samples){if(sample.state==1)CHECK(sample.anchor.p.x<3);if(sample.state==2)blocked++;}CHECK(blocked>0);
 volumeFluid::SplatModel stacked;stacked.Add({hit},1000);hit.p.z=5;stacked.Add({hit},1000);CHECK(stacked.marks.size()==2);
 volumeFluid::SplatModel bounded;for(int i=0;i<40;i++){hit.p={float(i*60),0,0};bounded.Add({hit},1000);}
 CHECK(bounded.marks.size()==volumeFluid::splatMaxMarks);bounded.Update(1016);CHECK(bounded.lastShapeUpdates==4&&bounded.projectionBudget>=0);
 for(int i=0;i<20;i++)bounded.Update(1032+i*16);for(const auto& mark:bounded.marks)CHECK(mark.revision>0);
 m.Update(25000);CHECK(m.marks.empty());
 volumeFluid::Settings cfg;auto drop=Make();for(auto& node:drop.nodes){node.p.z=1;node.v={5,0,-10};}volumeFluid::F4 empty[14]{};volumeFluid::collisionSweep=ImpactPlane;double contactVolume=0;
 for(int frame=0;frame<240;frame++){std::vector<volumeFluid::FluidImpact> hits;drop.Step(1.f/120,frame/120.f,cfg,empty,{},14,&hits);for(const auto& h:hits){CHECK(h.volume>0&&h.velocity.z<0);contactVolume+=h.volume;}}
 volumeFluid::collisionSweep=nullptr;CHECK(contactVolume>28.9&&contactVolume<=29.0001);
 printf("PASS ledge rejection, separate stacked receivers, 32-patch/4-update/96-projection bounds, expiry and once-only contact volume %.6f\n",contactVolume);
 return true;
}
static LRESULT CALLBACK Proc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
static bool Render(float volume,V3 velocity,unsigned seed,const char* name,DWORD showTime=5000){
 WNDCLASSA wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandle(nullptr);wc.lpszClassName="SplatTest";RegisterClassA(&wc);HWND window=CreateWindowA(wc.lpszClassName,"Splat tests",0,0,0,800,600,nullptr,nullptr,wc.hInstance,nullptr);
 char sys[MAX_PATH];GetSystemDirectoryA(sys,MAX_PATH);strcat_s(sys,"\\d3d9.dll");HMODULE lib=LoadLibraryA(sys);CHECK(lib);auto create=(IDirect3D9*(WINAPI*)(UINT))GetProcAddress(lib,"Direct3DCreate9");CHECK(create);IDirect3D9* d9=create(D3D_SDK_VERSION);CHECK(d9);
 D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.BackBufferWidth=800;pp.BackBufferHeight=600;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.EnableAutoDepthStencil=TRUE;pp.AutoDepthStencilFormat=D3DFMT_D24S8;IDirect3DDevice9* d=nullptr;CHECK(SUCCEEDED(d9->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d)));
 volumeFluid::StainSurface splat;auto coldStart=std::chrono::steady_clock::now();CHECK(splat.Initialize(d));
 double coldMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-coldStart).count();
 printf("PRECOMPILED cold shader/device setup %.3f ms\n",coldMs);CHECK(coldMs<250);volumeFluid::FluidImpact hit{};hit.n={0,0,1};hit.velocity=velocity;hit.volume=volume;hit.sourceId=seed;splat.Add({hit},1000);for(DWORD t=1000;t<showTime;t+=16)splat.Update(t);
 D3DXMATRIX view,projection,vp;D3DXVECTOR3 eye(5,-18,30),aim(5,0,0),up(0,0,1);D3DXMatrixLookAtLH(&view,&eye,&aim,&up);D3DXMatrixOrthoLH(&projection,58,43.5,1,100);D3DXMatrixMultiply(&vp,&view,&projection);
 IDirect3DSurface9 *back=nullptr,*read=nullptr;CHECK(SUCCEEDED(d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back)));CHECK(SUCCEEDED(d->CreateOffscreenPlainSurface(800,600,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&read,nullptr)));
 float lit[2]{};for(int scene=0;scene<2;scene++){
  int value=scene?135:20;CHECK(SUCCEEDED(d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_ARGB(77,value,value,value),1,0)));CHECK(SUCCEEDED(d->BeginScene()));CHECK(splat.CaptureScene(d));
  d->SetRenderState(D3DRS_STENCILENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);d->SetSamplerState(1,D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR);d->SetTexture(1,nullptr);auto drawStart=std::chrono::steady_clock::now();CHECK(SUCCEEDED(splat.Draw(d,(float*)&vp,showTime)));
  if(scene==0){double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-drawStart).count();printf("PRECOMPILED first impact draw submission %.3f ms\n",ms);CHECK(ms<250);}DWORD state;d->GetRenderState(D3DRS_STENCILENABLE,&state);CHECK(state==TRUE);d->GetRenderState(D3DRS_ZWRITEENABLE,&state);CHECK(state==TRUE);d->GetSamplerState(1,D3DSAMP_ADDRESSU,&state);CHECK(state==D3DTADDRESS_MIRROR);IDirect3DBaseTexture9* restored=nullptr;CHECK(SUCCEEDED(d->GetTexture(1,&restored)));CHECK(restored==nullptr);d->SetRenderState(D3DRS_STENCILENABLE,FALSE);CHECK(SUCCEEDED(d->EndScene()));
  char path[128];sprintf_s(path,"%s-%s.png",name,scene?"light":"dark");CHECK(SUCCEEDED(D3DXSaveSurfaceToFileA(path,D3DXIFF_PNG,back,nullptr,nullptr)));CHECK(SUCCEEDED(d->GetRenderTargetData(back,read)));D3DLOCKED_RECT lock{};CHECK(SUCCEEDED(read->LockRect(&lock,nullptr,D3DLOCK_READONLY)));unsigned total=0,count=0;
  for(int y=0;y<600;y++)for(int x=0;x<800;x++){DWORD pixel=*(DWORD*)((char*)lock.pBits+y*lock.Pitch+x*4);CHECK((pixel>>24)==77);unsigned r=(pixel>>16)&255;if(r>unsigned(value+4)){total+=r;count++;}}
  read->UnlockRect();CHECK(count>100);lit[scene]=float(total)/count;
 }
 CHECK(lit[1]>lit[0]*1.4f);printf("PASS GPU splat scene copy, bevel draw, depth-alpha/state preservation, light response %.1f -> %.1f\n",lit[0],lit[1]);
 if(strcmp(name,"branch-grazing")==0){IDirect3DQuery9* fence=nullptr;CHECK(SUCCEEDED(d->CreateQuery(D3DQUERYTYPE_EVENT,&fence)));std::vector<double> elapsed;
  for(int frame=0;frame<40;frame++){auto begin=std::chrono::steady_clock::now();d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(135,135,135),1,0);d->BeginScene();CHECK(SUCCEEDED(splat.Draw(d,(float*)&vp,showTime)));d->EndScene();fence->Issue(D3DISSUE_END);while(fence->GetData(nullptr,0,D3DGETDATA_FLUSH)==S_FALSE)Sleep(0);elapsed.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());}
  std::sort(elapsed.begin(),elapsed.end());printf("OFFLINE 800x600 one-deposit CPU+GPU-completion median %.3f ms / p95 %.3f ms\n",elapsed[20],elapsed[38]);fence->Release();
 }
 if(strcmp(name,"motion")==0){CreateDirectoryA("branch-motion",nullptr);splat.marks.clear();splat.Add({hit},1000);
  for(int frame=0;frame<50;frame++){DWORD t=1000+frame*20;d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(75,82,89),1,0);d->BeginScene();splat.Draw(d,(float*)&vp,t);d->EndScene();char file[128];sprintf_s(file,"branch-motion/frame-%03d.png",frame);CHECK(SUCCEEDED(D3DXSaveSurfaceToFileA(file,D3DXIFF_PNG,back,nullptr,nullptr)));}
 }
 if(strcmp(name,"coupled")==0){
  splat.marks.clear();volumeFluid::config={};volumeFluid::config.viscosity=24;volumeFluid::collisionSweep=ImpactPlane;
  teaching::Fluid fluid,reference;fluid.SetVariationSeed(0x13579bdf);reference.SetVariationSeed(0x13579bdf);CHECK(fluid.Begin({0,0,50})&&reference.Begin({0,0,50}));fluid.surfaceDeposits=true;
  double contacted=0;unsigned contacts=0;size_t peakMarks=0;bool handedOff=false;std::vector<double> times;
  for(int frame=0;frame<660;frame++){
   float t=(frame+1.f)/30,yaw=fluid.MainLateralYaw(t);V3 direction=Unit({cosf(yaw),sinf(yaw),.4f});
   reference.Advance(1.f/30,{0,0,50},direction,{});
   auto start=std::chrono::steady_clock::now();fluid.Advance(1.f/30,{0,0,50},direction,{});
   for(const auto& hit:fluid.impacts){contacted+=hit.volume;++contacts;}
   CHECK(splat.Add(fluid.impacts,1000+(DWORD)(t*1000)));
   d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(100,100,100),1,0);d->BeginScene();CHECK(SUCCEEDED(splat.Draw(d,(float*)&vp,1000+(DWORD)(t*1000))));d->EndScene();
   double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();if(t>=7&&t<14)times.push_back(ms);
   peakMarks=max(peakMarks,splat.marks.size());CHECK(splat.projectionBudget>=0&&splat.lastShapeUpdates<=4);
   CHECK(fluid.emittedVolume==reference.emittedVolume);for(int i=0;i<4;i++){
    CHECK(fluid.streams[i].nodes.size()==reference.streams[i].nodes.size());
    for(size_t j=0;j<fluid.streams[i].nodes.size();j++)CHECK(Length(fluid.streams[i].nodes[j].p-reference.streams[i].nodes[j].p)<1e-6f);
   }
   if(fluid.mesh.opaqueIndices<reference.mesh.opaqueIndices)handedOff=true;
  }
  double retained=0;for(const auto& mark:splat.marks)retained+=Mass(mark.density);
  CHECK(handedOff&&fluid.Live()==0&&contacts>300&&fabs(contacted-fluid.emittedVolume)<.001&&fabs(retained-contacted)<.01);
  std::sort(times.begin(),times.end());printf("PASS full22s coupled sequence: %u contacts, emitted %.6f / deposited %.6f / retained %.6f, peak patches %u, ownership leaves dynamics identical\n",contacts,fluid.emittedVolume,contacted,retained,(unsigned)peakMarks);
  printf("OFFLINE active CPU update+mesh+splat+draw submission median %.3fms p95 %.3fms max %.3fms (not gameplay FPS)\n",times[times.size()/2],times[times.size()*95/100],times.back());CHECK(times.back()<250);
  volumeFluid::collisionSweep=nullptr;
 }
 splat.Release();back->Release();read->Release();CHECK(SUCCEEDED(d->Reset(&pp)));CHECK(splat.Initialize(d));splat.Release();d->Release();d9->Release();DestroyWindow(window);FreeLibrary(lib);return true;
}
int main(int argc,char** argv){setvbuf(stdout,nullptr,_IONBF,0);if(argc>1&&strcmp(argv[1],"--model")==0)return ModelTest()?0:1;if(argc>1&&strcmp(argv[1],"--sequence")==0)return Render(6,{85,0,-45},17,"coupled")?0:1;if(argc>1)return ModelTest()&&Render(24,{85,0,-45},17,"motion")?0:1;return ModelTest()&&Render(24,{85,0,-45},17,"branch-grazing")&&Render(30,{15,0,-110},98,"branch-steep")&&Render(6,{60,0,-35},44,"branch-small")&&Render(24,{85,0,-45},17,"growth-080ms",1080)&&Render(24,{85,0,-45},17,"growth-240ms",1240)&&Render(24,{85,0,-45},17,"growth-600ms",1600)?0:1;}
