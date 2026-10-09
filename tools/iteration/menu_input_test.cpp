#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <stdexcept>
static void Log(const char* format,...){va_list a;va_start(a,format);vprintf(format,a);puts("");va_end(a);}
#include "../../src/runtime/menu_input.h"
static HWND focus=nullptr;
static HWND WINAPI Focus(){return focus;}
static unsigned delivered=0;
static LRESULT CALLBACK Underlying(HWND w,UINT message,WPARAM key,LPARAM l){if(message==WM_KEYDOWN)delivered++;return DefWindowProcW(w,message,key,l);}
static void Check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct FakeDevice{void** vtable;};
static HRESULT STDMETHODCALLTYPE State(void*,DWORD size,LPVOID out){memset(out,0x80,size);return S_OK;}
static HRESULT STDMETHODCALLTYPE Data(void*,DWORD size,LPDIDEVICEOBJECTDATA out,LPDWORD count,DWORD){if(out&&*count>=2){auto a=out;auto b=reinterpret_cast<DIDEVICEOBJECTDATA*>(reinterpret_cast<unsigned char*>(out)+size);a->dwOfs=DIK_DOWN;a->dwData=0x80;b->dwOfs=DIK_W;b->dwData=0x80;*count=2;}return S_OK;}
static HRESULT STDMETHODCALLTYPE Cooperative(void*,HWND,DWORD){return S_OK;}
static ULONG STDMETHODCALLTYPE Release(void*){return 0;}
int main(){try{
 WNDCLASSW c{};c.lpfnWndProc=Underlying;c.hInstance=GetModuleHandleW(nullptr);c.lpszClassName=L"MaleMod.Owned.MenuInput.Test";RegisterClassW(&c);
 auto window=CreateWindowExW(0,c.lpszClassName,L"",WS_POPUP,0,0,32,32,nullptr,nullptr,c.hInstance,nullptr);Check(window!=nullptr,"Owned hidden window allocation failed");
 MenuInput::foreground=Focus;focus=window;MenuInput::Attach(window);MenuInput::Publish(false);
 SendMessageW(window,WM_KEYDOWN,VK_DOWN,0);Check(delivered==1,"Closed panel blocked underlying navigation");
 MenuInput::Publish(true);for(unsigned key:{VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_F8,VK_SHIFT})SendMessageW(window,WM_KEYDOWN,key,0);Check(delivered==1,"Open panel navigation leaked to underlying window");
 SendMessageW(window,WM_KEYDOWN,'W',0);Check(delivered==2,"Unrelated binding was blocked");
 void* methods[16]{};FakeDevice keyboard{methods},mouse{methods};MenuInput::tables[0]={methods,State,Data,Cooperative,Release};MenuInput::tableCount=1;MenuInput::keyboards[0]={&keyboard,window};
 unsigned char keys[256]{};Check(SUCCEEDED(MenuInput::DeviceState(&keyboard,256,keys)),"Keyboard state failed");Check(keys[DIK_DOWN]==0&&keys[DIK_F8]==0&&keys[DIK_LSHIFT]==0&&keys[DIK_W]==0x80,"Open state mask lost panel isolation or unrelated keys");
 MenuInput::Publish(false);MenuInput::DeviceState(&keyboard,256,keys);Check(keys[DIK_DOWN]==0x80&&keys[DIK_F8]==0x80&&keys[DIK_LSHIFT]==0x80&&keys[DIK_F6]==0,"Closed keyboard bindings changed");
 MenuInput::Publish(true);MenuInput::DeviceState(&mouse,256,keys);Check(keys[DIK_DOWN]==0x80&&keys[DIK_F6]==0x80,"Mouse/shared vtable was masked");
 DIDEVICEOBJECTDATA events[2]{};DWORD count=2;MenuInput::DeviceData(&keyboard,sizeof(events[0]),events,&count,DIGDD_PEEK);Check(count==2&&events[0].dwData==0&&events[1].dwData==0x80,"Buffered peek contract or unrelated event changed");
 focus=nullptr;MenuInput::DeviceState(&keyboard,256,keys);Check(keys[DIK_DOWN]==0x80,"Background keyboard was intercepted");Check(MenuInput::Poll(VK_DOWN)==0,"Background panel polls host keys");
 focus=window;MenuInput::Publish(false);SendMessageW(window,WM_KEYDOWN,VK_DOWN,0);Check(delivered==3,"Closed navigation was not restored");
 MenuInput::Detach();SendMessageW(window,WM_KEYDOWN,VK_DOWN,0);Check(delivered==4,"Window procedure was not restored");DestroyWindow(window);
 puts("PASS: exact hidden HWND open/closed isolation, unrelated keys, keyboard state/buffered peek, shared mouse table, background guard and detach; no host input/focus or hardware keyboard polling");return 0;
}catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}
