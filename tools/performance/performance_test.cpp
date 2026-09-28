#include "../../src/runtime/d3d9_proxy.cpp"
#include <algorithm>
#include "performance_contracts.h"
static double ClockMs(){LARGE_INTEGER t,f;QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return 1000.*t.QuadPart/f.QuadPart;}
static unsigned draws=0;
static HRESULT STDMETHODCALLTYPE NoDraw(IDirect3DDevice9*,D3DPRIMITIVETYPE,INT,UINT,UINT,UINT,UINT){return D3D_OK;}
static HRESULT (STDMETHODCALLTYPE *realUP)(IDirect3DDevice9*,D3DPRIMITIVETYPE,UINT,const void*,UINT);
static HRESULT STDMETHODCALLTYPE CountUP(IDirect3DDevice9* d,D3DPRIMITIVETYPE p,UINT n,const void* v,UINT stride){++draws;return realUP(d,p,n,v,stride);}
int main(int argc,char** argv){
 WNDCLASSA wc{};wc.lpfnWndProc=DefWindowProcA;wc.hInstance=GetModuleHandleA(nullptr);wc.lpszClassName="PerfFixture";RegisterClassA(&wc);
 HWND w=CreateWindowA(wc.lpszClassName,"Offline performance fixture",0,0,0,1280,960,nullptr,nullptr,wc.hInstance,nullptr);
 IDirect3D9* api=::Direct3DCreate9(D3D_SDK_VERSION); // Export above hooks the real device creation.
 D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=w;pp.BackBufferWidth=1280;pp.BackBufferHeight=960;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
 IDirect3DDevice9* d=nullptr;HRESULT hr=api->CreateDevice(0,D3DDEVTYPE_HAL,w,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d);if(FAILED(hr))return 1;
 if(argc==2&&strcmp(argv[1],"--contracts")==0)return PerformanceContracts(d)?0:10;
 if(argc==3){double start=ClockMs();IDirect3DTexture9* tex=nullptr;
  hr=D3DXCreateTextureFromFileExA(d,argv[1],D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_FILTER_TRIANGLE,D3DX_FILTER_TRIANGLE,0,nullptr,nullptr,&tex);
  if(FAILED(hr))return 2;double loaded=ClockMs()-start;D3DSURFACE_DESC desc{};tex->GetLevelDesc(0,&desc);
  hr=D3DXSaveTextureToFileA(argv[2],D3DXIFF_DDS,tex,nullptr);if(FAILED(hr))return 3;
  IDirect3DTexture9* cached=nullptr;start=ClockMs();hr=D3DXCreateTextureFromFileExA(d,argv[2],D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_FILTER_NONE,D3DX_FILTER_NONE,0,nullptr,nullptr,&cached);if(FAILED(hr))return 4;
  double fast=ClockMs()-start;bool equal=tex->GetLevelCount()==cached->GetLevelCount();
  for(UINT level=0;level<tex->GetLevelCount()&&equal;level++){D3DSURFACE_DESC a{},b{};tex->GetLevelDesc(level,&a);cached->GetLevelDesc(level,&b);equal=!memcmp(&a,&b,sizeof(a));D3DLOCKED_RECT x{},y{};if(FAILED(tex->LockRect(level,&x,nullptr,D3DLOCK_READONLY))||FAILED(cached->LockRect(level,&y,nullptr,D3DLOCK_READONLY)))return 5;for(UINT row=0;row<a.Height&&equal;row++)equal=!memcmp((char*)x.pBits+row*x.Pitch,(char*)y.pBits+row*y.Pitch,a.Width*4);tex->UnlockRect(level);cached->UnlockRect(level);}
  printf("texture %ux%u mips=%u PNG=%.3fms DDS=%.3fms exact=%d\n",desc.Width,desc.Height,tex->GetLevelCount(),loaded,fast,equal);return equal?0:6;
 }
 void** vt=*(void***)d;Patch(&vt[83],(void*)CountUP,(void**)&realUP);
 settingsLoaded=false;menuOpen=true;shapeDirty=false;tankCameraSceneTick=0;graftBuffer=nullptr;anatomyScene=-1;double start=ClockMs();draws=0;
 for(int i=0;i<30;i++){d->BeginScene();OverlayFrame(d);origEndScene(d);}printf("loading overlay mean=%.3fms draws/frame=%.1f\n",(ClockMs()-start)/30,draws/30.);
 settingsLoaded=true;tankCameraSceneTick=GetTickCount();anatomyScene=1;shapeDirty=true;start=ClockMs();UpdateActiveAnatomySurface();printf("cold menu surface=%.3fms\n",ClockMs()-start);
 start=ClockMs();draws=0;for(int i=0;i<60;i++){tankCameraSceneTick=GetTickCount();anatomyVisibleFrame=renderFrameSerial;d->BeginScene();OverlayFrame(d);origEndScene(d);}printf("menu overlay+surface mean=%.3fms draws/frame=%.1f\n",(ClockMs()-start)/60,draws/60.);
 tankCameraSceneTick=0;anatomyScene=0;origDIP=NoDraw;IDirect3DVertexBuffer9* vb=nullptr;d->CreateVertexBuffer(96,0,0,D3DPOOL_MANAGED,&vb,nullptr);d->SetStreamSource(0,vb,0,32);
 start=ClockMs();for(int i=0;i<100000;i++)HookDIP(d,D3DPT_TRIANGLELIST,0,0,3,0,1);printf("idle draw hook mean=%.3fus\n",(ClockMs()-start)*1000/100000);
 for(auto& p:perfMetrics)if(p.count)printf("scope %s mean=%.3fms peak=%.3fms n=%u\n",p.name,p.total/p.count,p.peak,p.count);
 return 0;
}
