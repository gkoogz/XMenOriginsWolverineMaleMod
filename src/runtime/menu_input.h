#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <atomic>

// Restrict interception to this process's game window and observed keyboard
// devices. Panel polling remains raw; the engine receives neutral panel keys.
namespace MenuInput {
static std::atomic<HWND> window{nullptr};
static std::atomic<WNDPROC> previousWindowProc{nullptr};
static std::atomic<bool> panelActive{false};static HHOOK queueHook=nullptr;
static HWND WINAPI NativeForeground(){return GetForegroundWindow();}
using Focus=HWND(WINAPI*)();static Focus foreground=NativeForeground;
static void Publish(bool displayed){panelActive.store(displayed,std::memory_order_release);}
static bool Active(){return panelActive.load(std::memory_order_acquire)&&window.load()&&foreground()==window.load();}
static SHORT Poll(int key){return window.load()&&foreground()==window.load()?GetAsyncKeyState(key):0;}
static bool Key(unsigned key,bool open){return key==VK_F6||(open&&(key==VK_F8||key==VK_UP||key==VK_DOWN||key==VK_LEFT||key==VK_RIGHT||key==VK_SHIFT||key==VK_LSHIFT||key==VK_RSHIFT));}
static bool Scan(unsigned code,bool open){return code==DIK_F6||(open&&(code==DIK_F8||code==DIK_UP||code==DIK_DOWN||code==DIK_LEFT||code==DIK_RIGHT||code==DIK_LSHIFT||code==DIK_RSHIFT));}
// UE3 can inspect retrieved messages before DispatchMessage, and may replace its
// window procedure after device creation. Fence the exact owned window's queue
// on its process-owned thread, never a desktop/global keyboard hook.
static LRESULT CALLBACK Queue(int code,WPARAM mode,LPARAM payload){
 if(code>=0&&payload){auto message=reinterpret_cast<MSG*>(payload);if(message->hwnd==window&&(message->message==WM_KEYDOWN||message->message==WM_KEYUP||message->message==WM_SYSKEYDOWN||message->message==WM_SYSKEYUP)&&Key(unsigned(message->wParam),Active())){message->message=WM_NULL;message->wParam=0;message->lParam=0;static LONG reported=0;if(InterlockedCompareExchange(&reported,1,0)==0)Log("Menu input exact HWND thread queue consumed");}}
 return CallNextHookEx(queueHook,code,mode,payload);
}
struct PatchRecord{void** slot=nullptr;void* original=nullptr;void* replacement=nullptr;};
static PatchRecord patches[64]{};static unsigned patchCount=0;
static bool PatchSlot(void** slot,void* replacement){if(*slot==replacement)return true;if(patchCount==64)return false;DWORD protection=0;if(!VirtualProtect(slot,sizeof(*slot),PAGE_READWRITE,&protection))return false;patches[patchCount++]={slot,*slot,replacement};*slot=replacement;DWORD ignored=0;VirtualProtect(slot,sizeof(*slot),protection,&ignored);return true;}
static LRESULT CALLBACK WindowProc(HWND hwnd,UINT message,WPARAM w,LPARAM l){
 if(hwnd==window&&(message==WM_KEYDOWN||message==WM_KEYUP||message==WM_SYSKEYDOWN||message==WM_SYSKEYUP)&&Key(unsigned(w),Active())){static LONG reported=0;if(InterlockedCompareExchange(&reported,1,0)==0)Log("Menu input exact HWND message path consumed");return 0;}
 auto old=previousWindowProc.load();
 if(message==WM_NCDESTROY&&hwnd==window){if(reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd,GWLP_WNDPROC))==WindowProc)SetWindowLongPtrW(hwnd,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(old));window=nullptr;previousWindowProc=nullptr;}
 return old?CallWindowProcW(old,hwnd,message,w,l):DefWindowProcW(hwnd,message,w,l);
}
static void Attach(HWND hwnd){
 DWORD process=0;DWORD thread=GetWindowThreadProcessId(hwnd,&process);if(!hwnd||!thread||process!=GetCurrentProcessId()||hwnd==window)return;
 if(queueHook){UnhookWindowsHookEx(queueHook);queueHook=nullptr;}
 if(window&&IsWindow(window)&&reinterpret_cast<WNDPROC>(GetWindowLongPtrW(window,GWLP_WNDPROC))==WindowProc)SetWindowLongPtrW(window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(previousWindowProc.load()));
 SetLastError(0);auto old=reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd,GWLP_WNDPROC));window=hwnd;previousWindowProc=old;auto replaced=SetWindowLongPtrW(hwnd,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(WindowProc));if(!replaced&&GetLastError()){window=nullptr;previousWindowProc=nullptr;return;}previousWindowProc=reinterpret_cast<WNDPROC>(replaced);Log("Menu input exact game HWND attached");
 HMODULE module=nullptr;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(Queue),&module);queueHook=SetWindowsHookExW(WH_GETMESSAGE,Queue,module,thread);Log("Menu input owned thread queue fence thread=%u installed=%d",thread,queueHook!=nullptr);
}
using State=HRESULT(STDMETHODCALLTYPE*)(void*,DWORD,LPVOID);
using Data=HRESULT(STDMETHODCALLTYPE*)(void*,DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD);
using Cooperative=HRESULT(STDMETHODCALLTYPE*)(void*,HWND,DWORD);
using Release=ULONG(STDMETHODCALLTYPE*)(void*);
struct Table{void** vtable=nullptr;State state=nullptr;Data data=nullptr;Cooperative cooperative=nullptr;Release release=nullptr;};
struct Keyboard{void* device=nullptr;HWND owner=nullptr;};
static Table tables[8]{};static unsigned tableCount=0;static Keyboard keyboards[8]{};
static Table* Methods(void* device){auto table=*static_cast<void***>(device);for(unsigned i=0;i<tableCount;i++)if(tables[i].vtable==table)return &tables[i];return nullptr;}
static Keyboard* Identity(void* device){for(auto& k:keyboards)if(k.device==device)return &k;return nullptr;}
static bool Owned(void* device){auto k=Identity(device);return k&&k->owner&&k->owner==window&&foreground()==window.load();}
static HRESULT STDMETHODCALLTYPE DeviceState(void* self,DWORD size,LPVOID output){auto methods=Methods(self);if(!methods)return E_FAIL;auto hr=methods->state(self,size,output);if(SUCCEEDED(hr)&&output&&size==256&&Owned(self)){bool open=Active();auto bytes=static_cast<unsigned char*>(output);bool neutralized=false;for(unsigned i=0;i<256;i++)if(Scan(i,open)){neutralized|=bytes[i]!=0;bytes[i]=0;}static LONG reported=0;if(neutralized&&InterlockedCompareExchange(&reported,1,0)==0)Log("Menu input owned DirectInput keyboard state neutralized");}return hr;}
static HRESULT STDMETHODCALLTYPE DeviceData(void* self,DWORD size,LPDIDEVICEOBJECTDATA output,LPDWORD count,DWORD flags){auto methods=Methods(self);if(!methods)return E_FAIL;auto hr=methods->data(self,size,output,count,flags);if(SUCCEEDED(hr)&&output&&count&&size>=sizeof(DIDEVICEOBJECTDATA_DX3)&&Owned(self)){bool open=Active(),neutralized=false;for(DWORD i=0;i<*count;i++){auto entry=reinterpret_cast<DIDEVICEOBJECTDATA*>(reinterpret_cast<unsigned char*>(output)+i*size);if(Scan(entry->dwOfs,open)){neutralized|=entry->dwData!=0;entry->dwData=0;}}static LONG reported=0;if(neutralized&&InterlockedCompareExchange(&reported,1,0)==0)Log("Menu input owned DirectInput keyboard buffered events neutralized flags=%u",flags);}return hr;}
static HRESULT STDMETHODCALLTYPE DeviceCooperative(void* self,HWND hwnd,DWORD flags){auto methods=Methods(self);if(!methods)return E_FAIL;auto hr=methods->cooperative(self,hwnd,flags);if(SUCCEEDED(hr)){auto k=Identity(self);if(k)k->owner=hwnd;}return hr;}
static ULONG STDMETHODCALLTYPE DeviceRelease(void* self){auto methods=Methods(self);if(!methods)return 0;auto remaining=methods->release(self);if(!remaining){auto k=Identity(self);if(k)*k={};}return remaining;}
using CreateDevice=HRESULT(STDMETHODCALLTYPE*)(void*,REFGUID,void**,LPUNKNOWN);
struct FactoryTable{void** vtable=nullptr;CreateDevice create=nullptr;};
static FactoryTable factories[4]{};static unsigned factoryCount=0;
static HRESULT STDMETHODCALLTYPE CreateKeyboard(void* self,REFGUID guid,void** output,LPUNKNOWN outer){
 auto vtable=*static_cast<void***>(self);CreateDevice original=nullptr;for(unsigned i=0;i<factoryCount;i++)if(factories[i].vtable==vtable)original=factories[i].create;if(!original)return E_FAIL;
 auto hr=original(self,guid,output,outer);if(FAILED(hr)||!output||!*output)return hr;
 auto device=*output;auto table=*static_cast<void***>(device);using Capabilities=HRESULT(STDMETHODCALLTYPE*)(void*,LPDIDEVCAPS);DIDEVCAPS caps{};caps.dwSize=sizeof(caps);
 if(FAILED(reinterpret_cast<Capabilities>(table[3])(device,&caps))||GET_DIDEVICE_TYPE(caps.dwDevType)!=DI8DEVTYPE_KEYBOARD)return hr;
 Keyboard* record=nullptr;for(auto& k:keyboards)if(!k.device){record=&k;break;}if(!record)return hr;
 auto methods=Methods(device);if(!methods){if(tableCount==8)return hr;methods=&tables[tableCount++];*methods={table,reinterpret_cast<State>(table[9]),reinterpret_cast<Data>(table[10]),reinterpret_cast<Cooperative>(table[13]),reinterpret_cast<Release>(table[2])};
  PatchSlot(table+9,reinterpret_cast<void*>(DeviceState));PatchSlot(table+10,reinterpret_cast<void*>(DeviceData));PatchSlot(table+13,reinterpret_cast<void*>(DeviceCooperative));PatchSlot(table+2,reinterpret_cast<void*>(DeviceRelease));}
 *record={device,nullptr};Log("Menu input observed DirectInput keyboard created");return hr;
}
using CreateInput=HRESULT(WINAPI*)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
static CreateInput originalCreate=nullptr;
static HRESULT WINAPI Create(HINSTANCE instance,DWORD version,REFIID iid,LPVOID* output,LPUNKNOWN outer){auto hr=originalCreate(instance,version,iid,output,outer);if(SUCCEEDED(hr)&&output&&*output){auto table=*static_cast<void***>(*output);bool known=false;for(unsigned i=0;i<factoryCount;i++)known|=factories[i].vtable==table;if(!known&&factoryCount<4){factories[factoryCount++]={table,reinterpret_cast<CreateDevice>(table[3])};PatchSlot(table+3,reinterpret_cast<void*>(CreateKeyboard));}}return hr;}
static void Detach(){
 if(queueHook){UnhookWindowsHookEx(queueHook);queueHook=nullptr;}
 if(window.load()&&IsWindow(window.load())&&reinterpret_cast<WNDPROC>(GetWindowLongPtrW(window.load(),GWLP_WNDPROC))==WindowProc)SetWindowLongPtrW(window.load(),GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(previousWindowProc.load()));window=nullptr;previousWindowProc=nullptr;Publish(false);
 for(unsigned i=patchCount;i>0;i--){auto& p=patches[i-1];if(*p.slot!=p.replacement)continue;DWORD protection=0;if(VirtualProtect(p.slot,sizeof(*p.slot),PAGE_READWRITE,&protection)){*p.slot=p.original;DWORD ignored=0;VirtualProtect(p.slot,sizeof(*p.slot),protection,&ignored);}}
 patchCount=0;
}
static void Install(){
 auto base=reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);auto directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];if(!directory.VirtualAddress)return;
 auto entry=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+directory.VirtualAddress);for(;entry->Name;entry++){if(_stricmp(reinterpret_cast<char*>(base+entry->Name),"dinput8.dll")||!entry->OriginalFirstThunk)continue;auto names=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+entry->OriginalFirstThunk);auto addresses=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+entry->FirstThunk);for(;names->u1.AddressOfData;names++,addresses++){if(IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal))continue;auto name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData);if(strcmp(reinterpret_cast<char*>(name->Name),"DirectInput8Create"))continue;auto slot=reinterpret_cast<void**>(&addresses->u1.Function);if(!originalCreate)originalCreate=reinterpret_cast<CreateInput>(*slot);if(PatchSlot(slot,reinterpret_cast<void*>(Create)))Log("Menu input exact executable DirectInput factory hooked");}}
}
}
