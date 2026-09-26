#include "../../src/runtime/d3d9_proxy.cpp"
static IDirect3DSwapChain9* testSwap=nullptr;
static unsigned swaps=0;
static HRESULT STDMETHODCALLTYPE TestSwap(IDirect3DSwapChain9*,const RECT*,const RECT*,HWND,const RGNDATA*,DWORD){++swaps;return S_OK;}
static HRESULT STDMETHODCALLTYPE TestPresent(IDirect3DDevice9*,const RECT* a,const RECT* b,HWND h,const RGNDATA* r){return HookSwapPresent(testSwap,a,b,h,r,0);}
static LRESULT CALLBACK TestWindow(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
int main(){
  HINSTANCE instance=GetModuleHandle(nullptr);WNDCLASSA wc{};wc.lpfnWndProc=TestWindow;wc.hInstance=instance;wc.lpszClassName="CadenceHooks";RegisterClassA(&wc);
  HWND hwnd=CreateWindowA(wc.lpszClassName,"Cadence hook test",WS_OVERLAPPEDWINDOW,0,0,320,240,nullptr,nullptr,instance,nullptr);
  LoadReal();IDirect3D9* d9=realCreate9(D3D_SDK_VERSION);if(!d9)return 2;
  D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=hwnd;pp.BackBufferWidth=320;pp.BackBufferHeight=240;pp.BackBufferFormat=D3DFMT_A8R8G8B8;
  IDirect3DDevice9* dev=nullptr;if(FAILED(d9->CreateDevice(0,D3DDEVTYPE_HAL,hwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&dev)))return 3;
  if(FAILED(dev->GetSwapChain(0,&testSwap)))return 4;
  origEndScene=(EndSceneFn)(*(void***)dev)[42];origReset=(ResetFn)(*(void***)dev)[16];origPresent=TestPresent;origSwapPresent=TestSwap;
  menuOpen=false;frameRendered=false;LONG first=renderFrameSerial;
  // Multiple final EndScenes before a presentation must update only once.
  dev->BeginScene();HookEndScene(dev);dev->BeginScene();HookEndScene(dev);
  if(renderFrameSerial!=first+1)return 5;
  HookPresent(dev,nullptr,nullptr,nullptr,nullptr);
  if(renderFrameSerial!=first+1||frameRendered||presentInProgress||swaps!=1)return 6;
  // Device Present falls through the swap-chain hook: no duplicate update.
  HookPresent(dev,nullptr,nullptr,nullptr,nullptr);
  if(renderFrameSerial!=first+2||swaps!=2)return 7;
  HookSwapPresent(testSwap,nullptr,nullptr,nullptr,nullptr,0);
  if(renderFrameSerial!=first+3||swaps!=3||frameRendered||presentInProgress)return 8;
  testSwap->Release();testSwap=nullptr;
  frameRendered=presentInProgress=surfaceHoldNext=true;
  HRESULT hr=HookReset(dev,&pp);
  if(FAILED(hr)||frameRendered||presentInProgress||surfaceHoldNext)return 9;
  dev->Release();d9->Release();DestroyWindow(hwnd);
  printf("PASS: repeated EndScene, nested device/swap Present, standalone swap Present and reset update exactly once per presented frame\n");return 0;
}
