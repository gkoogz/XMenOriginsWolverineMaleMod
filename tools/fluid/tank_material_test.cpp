#define main ThreadReferenceMain
#include "thread_test.cpp"
#undef main
#include <d3d9.h>
#include <d3dx9.h>
#include "../../src/runtime/fluid_surface.h"
static LRESULT CALLBACK Proc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
static bool Run(){
 WNDCLASSA wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandle(nullptr);wc.lpszClassName="TankMaterialTest";RegisterClassA(&wc);
 HWND window=CreateWindowA(wc.lpszClassName,"Offline tank material check",0,0,0,64,64,nullptr,nullptr,wc.hInstance,nullptr);
 char system[MAX_PATH];GetSystemDirectoryA(system,MAX_PATH);strcat_s(system,"\\d3d9.dll");HMODULE library=LoadLibraryA(system);CHECK(library);
 auto create=(IDirect3D9*(WINAPI*)(UINT))GetProcAddress(library,"Direct3DCreate9");CHECK(create);IDirect3D9* d9=create(D3D_SDK_VERSION);CHECK(d9);
 D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.BackBufferWidth=64;pp.BackBufferHeight=64;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.EnableAutoDepthStencil=TRUE;pp.AutoDepthStencilFormat=D3DFMT_D24S8;
 IDirect3DDevice9* d=nullptr;CHECK(SUCCEEDED(d9->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d)));
 IDirect3DSurface9 *target=nullptr,*readback=nullptr,*back=nullptr;CHECK(SUCCEEDED(d->GetRenderTarget(0,&back)));
 CHECK(SUCCEEDED(d->CreateRenderTarget(64,64,D3DFMT_A16B16G16R16F,D3DMULTISAMPLE_NONE,0,FALSE,&target,nullptr)));
 CHECK(SUCCEEDED(d->CreateOffscreenPlainSurface(64,64,D3DFMT_A16B16G16R16F,D3DPOOL_SYSTEMMEM,&readback,nullptr)));
 CHECK(SUCCEEDED(d->SetRenderTarget(0,target)));volumeFluid::Surface surface;float darkest=10,brightest=0,depthError=0;
 for(float distance:{12.f,55.f,220.f})for(int angle=0;angle<25;angle++){
  D3DXMATRIX projection,view,clip;D3DXMatrixPerspectiveFovLH(&projection,.8f,1.f,1.f,500.f);
  float t=angle*6.2831853f/24;V3 eye=Unit({cosf(t),sinf(t),.25f})*distance;
  D3DXVECTOR3 camera(eye.x,eye.y,eye.z),aim(0,0,0),up(0,0,1);D3DXMatrixLookAtLH(&view,&camera,&aim,&up);D3DXMatrixMultiply(&clip,&view,&projection);
  V3 right=Unit(Cross(Unit(eye)*-1.f,{0,0,1})),vertical=Unit(Cross(right,Unit(eye)*-1.f));
  V3 n=angle==24?V3{}:V3{.8f,0,.6f};
  teaching::LiquidMesh mesh;float s=distance*.5f;
  mesh.vertices={{right*(-s)+vertical*(-s),n},{right*s+vertical*(-s),n},{right*s+vertical*s,n},{right*(-s)+vertical*s,n}};
  mesh.indices={0,1,2,0,2,3};mesh.opaqueIndices=6;
  CHECK(SUCCEEDED(d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0,1,0)));
  CHECK(SUCCEEDED(d->BeginScene()));CHECK(SUCCEEDED(surface.Draw(d,mesh,(float*)&clip,true)));CHECK(SUCCEEDED(d->EndScene()));
  CHECK(SUCCEEDED(d->GetRenderTargetData(target,readback)));D3DLOCKED_RECT lock{};CHECK(SUCCEEDED(readback->LockRect(&lock,nullptr,D3DLOCK_READONLY)));
  float rgba[4];D3DXFloat16To32Array(rgba,(D3DXFLOAT16*)((char*)lock.pBits+32*lock.Pitch+32*8),4);readback->UnlockRect();
  for(int j=0;j<3;j++){CHECK(std::isfinite(rgba[j])&&rgba[j]>.60f&&rgba[j]<.99f);darkest=min(darkest,rgba[j]);brightest=max(brightest,rgba[j]);}
  float expected=clip._43/clip._44;depthError=max(depthError,fabsf(rgba[3]-expected));CHECK(fabsf(rgba[3]-expected)<.0006f);
  // Fluid behind existing opaque geometry must remain hidden.
  CHECK(SUCCEEDED(d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_ARGB(64,20,30,40),.5f,0)));
  CHECK(SUCCEEDED(d->BeginScene()));CHECK(SUCCEEDED(surface.Draw(d,mesh,(float*)&clip,true)));CHECK(SUCCEEDED(d->EndScene()));
  CHECK(SUCCEEDED(d->GetRenderTargetData(target,readback)));CHECK(SUCCEEDED(readback->LockRect(&lock,nullptr,D3DLOCK_READONLY)));
  D3DXFloat16To32Array(rgba,(D3DXFLOAT16*)((char*)lock.pBits+32*lock.Pitch+32*8),4);readback->UnlockRect();CHECK(fabsf(rgba[0]-20.f/255)<.001f&&fabsf(rgba[3]-64.f/255)<.001f);
  // Transparent material must preserve the existing scene-depth alpha.
  mesh.opaqueIndices=0;mesh.indices={0,2,1,0,3,2,0,1,2,0,2,3};
  CHECK(SUCCEEDED(d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_ARGB(64,20,30,40),1,0)));
  CHECK(SUCCEEDED(d->BeginScene()));CHECK(SUCCEEDED(surface.Draw(d,mesh,(float*)&clip,true)));CHECK(SUCCEEDED(d->EndScene()));
  CHECK(SUCCEEDED(d->GetRenderTargetData(target,readback)));CHECK(SUCCEEDED(readback->LockRect(&lock,nullptr,D3DLOCK_READONLY)));
  D3DXFloat16To32Array(rgba,(D3DXFLOAT16*)((char*)lock.pBits+32*lock.Pitch+32*8),4);readback->UnlockRect();CHECK(std::isfinite(rgba[0])&&rgba[0]>20.f/255&&fabsf(rgba[3]-64.f/255)<.001f);
 }
 surface.Release();d->SetRenderTarget(0,back);target->Release();readback->Release();back->Release();CHECK(SUCCEEDED(d->Reset(&pp)));d->Release();d9->Release();DestroyWindow(window);FreeLibrary(library);
 printf("PASS 75 HDR tank material samples: RGB %.4f..%.4f, depth error %.6f; opaque occlusion, transparent depth preservation, device reset\n",darkest,brightest,depthError);return true;
}
int main(){return Run()?0:1;}
