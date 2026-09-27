#define main ThreadReferenceMain
#include "thread_test.cpp"
#undef main
#include <d3d9.h>
#include <d3dx9.h>
#include "../../src/runtime/fluid_surface.h"
static LRESULT CALLBACK Proc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
int main(){
 if(!Material())return 1;
 WNDCLASSA wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandle(nullptr);wc.lpszClassName="FluidCompatibilityCheck";RegisterClassA(&wc);HWND window=CreateWindowA(wc.lpszClassName,"Fluid check",0,0,0,64,64,nullptr,nullptr,wc.hInstance,nullptr);
 // Load the system D3D9 explicitly: never load a nearby proxy recursively.
 char system[MAX_PATH];GetSystemDirectoryA(system,MAX_PATH);strcat_s(system,"\\d3d9.dll");HMODULE library=LoadLibraryA(system);if(!library)return 2;auto create=(IDirect3D9*(WINAPI*)(UINT))GetProcAddress(library,"Direct3DCreate9");if(!create)return 3;IDirect3D9* d9=create(D3D_SDK_VERSION);if(!d9)return 4;
 D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.BackBufferWidth=64;pp.BackBufferHeight=64;pp.BackBufferFormat=D3DFMT_A8R8G8B8;IDirect3DDevice9* d=nullptr;HRESULT hr=d9->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d);if(FAILED(hr)){printf("D3D9 device unavailable 0x%08X\n",(unsigned)hr);return 5;}
 volumeFluid::Surface surface;bool okay=surface.Initialize(d);teaching::LiquidMesh mesh;mesh.Tube({{-.5f,0,.5f},{.5f,0,.5f}},{.1f,.1f},12);mesh.opaqueIndices=(unsigned)mesh.indices.size();float identity[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
 if(okay){hr=d->BeginScene();if(SUCCEEDED(hr)){hr=surface.Draw(d,mesh,identity);d->EndScene();}okay=SUCCEEDED(hr);}if(!okay)printf("D3D9 liquid mesh unavailable: %s 0x%08X\n",surface.error.c_str(),(unsigned)hr);
 surface.Release();if(okay)okay=SUCCEEDED(d->Reset(&pp));d->Release();d9->Release();DestroyWindow(window);FreeLibrary(library);if(!okay)return 6;printf("PASS CPU stream/material and D3D9 indexed mesh shaders/draw/reset. D3D11 is not required.\n");return 0;
}
