#pragma once
#include <Xinput.h>
#include "sandbox_world_origin.hpp"
static bool maleModPrivateExActive=false;
static double maleModPrivateDrawMs=0;static unsigned maleModPrivateDrawCalls=0;
static HWND maleModPrivateWindow=nullptr;
static unsigned maleModPrivateGameFrames=0;
static bool maleModPrivateHeldKeys[256]{},maleModPrivateCaptureRequested=false;
static bool maleModPrivatePaused=false;
static unsigned maleModPrivateKeyRelease[256]{},maleModPrivateControlRevision=0;
static LPARAM MaleModPrivateKeyParam(unsigned key,bool down){return LPARAM(1u|(MapVirtualKeyW(key,MAPVK_VK_TO_VSC)<<16)|((key>=VK_LEFT&&key<=VK_DOWN)?0x01000000u:0u)|(down?0u:0xc0000000u));}
static void MaleModPrivateCommands(unsigned frame,bool game){
 if(!maleModPrivateExActive||!game)return;
 for(unsigned key=1;key<256;key++)if(maleModPrivateKeyRelease[key]&&frame>=maleModPrivateKeyRelease[key]){maleModPrivateHeldKeys[key]=false;maleModPrivateKeyRelease[key]=0;PostMessageW(maleModPrivateWindow,WM_KEYUP,key,MaleModPrivateKeyParam(key,false));}
 char path[MAX_PATH]{};SiblingPath(path,"MaleModSandboxControl.ini");unsigned revision=GetPrivateProfileIntA("Control","Revision",0,path);if(!revision||revision==maleModPrivateControlRevision)return;maleModPrivateControlRevision=revision;
 char name[32]{},action[32]{};GetPrivateProfileStringA("Control","Key","",name,sizeof(name),path);GetPrivateProfileStringA("Control","Action","Press",action,sizeof(action),path);
 if(!strcmp(name,"Pause")||!strcmp(name,"Resume")){maleModPrivatePaused=!strcmp(name,"Pause");Log("Private command revision=%u key=%s",revision,name);return;}
 if(!strcmp(name,"Defaults")){ResetStudyControls();QueueSettingsSave();Log("Private command revision=%u key=Defaults",revision);return;}
 if(!strcmp(name,"Naked")||!strcmp(name,"Jockstrap")){SetJockstrapStyle(!strcmp(name,"Jockstrap")?1:0);QueueSettingsSave();shapeDirty=true;Log("Private command revision=%u key=%s",revision,name);return;}
 if(!strcmp(name,"Overall")){int value=GetPrivateProfileIntA("Control","Value",50,path);if(value>=1&&value<=100){AdjustStudyControl(3,value>=sliderUI[0]?1:-1,fabsf(value-sliderUI[0]));QueueSettingsSave();Log("Private command revision=%u key=Overall value=%d",revision,value);}return;}
 if(!strcmp(name,"Capture")){maleModPrivateCaptureRequested=true;Log("Private command capture revision=%u",revision);return;}
 struct NamedKey{const char* name;unsigned key;};const NamedKey allowed[]={{"TurnLeft",'R'},{"TurnRight",'T'},{"LookUp",'Y'},{"LookDown",'U'},{"Zoom",'V'},{"W",'W'},{"A",'A'},{"S",'S'},{"D",'D'},{"Space",VK_SPACE},{"Shift",VK_SHIFT},{"F6",VK_F6},{"F8",VK_F8},{"Up",VK_UP},{"Down",VK_DOWN},{"Left",VK_LEFT},{"Right",VK_RIGHT}};
 unsigned key=0;for(const auto& item:allowed)if(!strcmp(name,item.name)){key=item.key;break;}if(!key)return;
 if(strcmp(action,"Press")&&strcmp(action,"Hold")&&strcmp(action,"Release"))return;
 bool down=strcmp(action,"Release")!=0;maleModPrivateHeldKeys[key]=down;maleModPrivateKeyRelease[key]=!strcmp(action,"Press")?frame+2:0;
 PostMessageW(maleModPrivateWindow,down?WM_KEYDOWN:WM_KEYUP,key,MaleModPrivateKeyParam(key,down));maleModPrivateCaptureRequested=true;Log("Private command revision=%u key=%s action=%s",revision,name,action);
}
static bool MaleModPrivateControlsEnabled(){char value[8]{};return maleModPrivateExActive&&GetEnvironmentVariableA("MALEMOD_PRIVATE_TEST_CONTROLS",value,sizeof(value))==1&&value[0]=='1';}
static SHORT WINAPI MaleModPrivateKeyQuery(int key){
 if(key==VK_LSHIFT)key=VK_SHIFT;
 if(maleModPrivateExActive&&key>=0&&key<256&&maleModPrivateHeldKeys[key])return SHORT(0x8000);
 // This shim is installed only in the verified private child's own modules.
 // It never injects an event into the interactive desktop or system keyboard.
 if(MaleModPrivateControlsEnabled()){
  if(key=='W'&&maleModPrivateGameFrames>=30&&maleModPrivateGameFrames<150)return SHORT(0x8000);
  if(key==VK_F6&&((maleModPrivateGameFrames>=200&&maleModPrivateGameFrames<202)||(maleModPrivateGameFrames>=350&&maleModPrivateGameFrames<352)))return SHORT(0x8000);
  if(key==VK_DOWN&&((maleModPrivateGameFrames>=204&&maleModPrivateGameFrames<206)||(maleModPrivateGameFrames>=208&&maleModPrivateGameFrames<210)||(maleModPrivateGameFrames>=212&&maleModPrivateGameFrames<214)))return SHORT(0x8000);
  if(key==VK_RIGHT&&maleModPrivateGameFrames>=216&&maleModPrivateGameFrames<218)return SHORT(0x8000);
 }
 if(maleModPrivateExActive)return 0; // Never mirror the host keyboard into the sealed engine.
 using Fn=SHORT(WINAPI*)(int);static auto original=reinterpret_cast<Fn>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetAsyncKeyState"));return original?original(key):0;
}
static DWORD WINAPI MaleModPrivatePadState(DWORD index,XINPUT_STATE* state){
 if(!maleModPrivateExActive||!maleModPrivateGameFrames||index||!state)return ERROR_DEVICE_NOT_CONNECTED;
 ZeroMemory(state,sizeof(*state));state->dwPacketNumber=maleModPrivateGameFrames;
 auto held=[](unsigned k){return maleModPrivateHeldKeys[k]?1:0;};
 state->Gamepad.sThumbLX=SHORT((held('D')-held('A'))*20000);state->Gamepad.sThumbLY=SHORT((held('W')-held('S'))*20000);
 state->Gamepad.sThumbRX=SHORT((held('T')-held('R'))*16000);state->Gamepad.sThumbRY=SHORT((held('Y')-held('U'))*12000);
 if(held(VK_SPACE))state->Gamepad.wButtons|=XINPUT_GAMEPAD_A;
 return ERROR_SUCCESS;
}
static DWORD WINAPI MaleModPrivatePadVibration(DWORD,XINPUT_VIBRATION*){return ERROR_SUCCESS;}
static HWND WINAPI MaleModPrivateFocusQuery(){return IsWindow(maleModPrivateWindow)?maleModPrivateWindow:nullptr;}
static void MaleModPrivateFocusImports(HWND window){
 char message[256]{};sprintf_s(message,"Private focus before virtualization foreground=%p active=%p focus=%p owned=%p\n",GetForegroundWindow(),GetActiveWindow(),GetFocus(),window);OutputDebugStringA(message);
 // Substitute queries only in the exact owned UE3 executable. No host window
 // receives activation/input, and no system DLL or desktop is changed.
 maleModPrivateWindow=window;for(auto module:{GetModuleHandleW(nullptr),reinterpret_cast<HMODULE>(&__ImageBase)}){auto base=reinterpret_cast<BYTE*>(module);auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
 auto table=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
 for(auto entry=table;entry->Name;++entry){const char* library=reinterpret_cast<char*>(base+entry->Name);bool inputPad=!_stricmp(library,"xinput1_3.dll");if((!inputPad&&_stricmp(library,"user32.dll"))||!entry->OriginalFirstThunk)continue;
  auto lookup=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+entry->OriginalFirstThunk);auto address=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+entry->FirstThunk);
  for(;lookup->u1.AddressOfData;++lookup,++address){if(IMAGE_SNAP_BY_ORDINAL32(lookup->u1.Ordinal)){unsigned ordinal=IMAGE_ORDINAL32(lookup->u1.Ordinal);if(inputPad&&(ordinal==2||ordinal==3)){DWORD old=0;if(VirtualProtect(address,sizeof(*address),PAGE_READWRITE,&old)){address->u1.Function=reinterpret_cast<DWORD>(ordinal==2?reinterpret_cast<void*>(MaleModPrivatePadState):reinterpret_cast<void*>(MaleModPrivatePadVibration));DWORD ignored=0;VirtualProtect(address,sizeof(*address),old,&ignored);}}continue;}auto symbol=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+lookup->u1.AddressOfData)->Name;
   if(strcmp(reinterpret_cast<char*>(symbol),"GetForegroundWindow")&&strcmp(reinterpret_cast<char*>(symbol),"GetActiveWindow")&&strcmp(reinterpret_cast<char*>(symbol),"GetFocus")&&strcmp(reinterpret_cast<char*>(symbol),"GetAsyncKeyState"))continue;
   DWORD old=0;if(!VirtualProtect(address,sizeof(*address),PAGE_READWRITE,&old))continue;address->u1.Function=reinterpret_cast<DWORD>(!strcmp(reinterpret_cast<char*>(symbol),"GetAsyncKeyState")?reinterpret_cast<void*>(MaleModPrivateKeyQuery):reinterpret_cast<void*>(MaleModPrivateFocusQuery));DWORD ignored=0;VirtualProtect(address,sizeof(*address),old,&ignored);OutputDebugStringA(reinterpret_cast<char*>(symbol));
  }
 }
 }
}
static HRESULT STDMETHODCALLTYPE HookDIP(IDirect3DDevice9* d,D3DPRIMITIVETYPE type,INT base,UINT minv,UINT nv,UINT start,UINT count){MaleModSandboxOrigin(d,type,nv,count);LARGE_INTEGER a{},b{},f{};QueryPerformanceCounter(&a);HRESULT hr=MaleModPrivateSourceDIP(d,type,base,minv,nv,start,count);if(maleModPrivateExActive){QueryPerformanceCounter(&b);QueryPerformanceFrequency(&f);maleModPrivateDrawMs+=double(b.QuadPart-a.QuadPart)*1000/double(f.QuadPart);++maleModPrivateDrawCalls;}return hr;}
static HRESULT MaleModPrivateMeasuredPresent(IDirect3DDevice9* d,const RECT* src,const RECT* dst,HWND window,const RGNDATA* dirty){
 if(!maleModPrivateExActive)return origPresent(d,src,dst,window,dirty);
 LARGE_INTEGER a{},b{},frequency{};QueryPerformanceCounter(&a);HRESULT hr=origPresent(d,src,dst,window,dirty);QueryPerformanceCounter(&b);QueryPerformanceFrequency(&frequency);
 static double sum=0;static unsigned count=0;sum+=double(b.QuadPart-a.QuadPart)*1000/double(frequency.QuadPart);if(++count==120){Log("Private original Present mean=%.3fms samples=120",sum/count);sum=0;count=0;}return hr;
}
static HRESULT MaleModPrivatePresentResult(IDirect3DDevice9* d,HRESULT hr){
 // D3D9Ex can render on the private station while its output is occluded.
 // The classic engine incorrectly treats that success status as suspension.
 // Preserve every real device error, checking device health before translation.
 if(maleModPrivateExActive&&hr==S_PRESENT_OCCLUDED&&d->TestCooperativeLevel()==D3D_OK)return D3D_OK;
 return hr;
}
static void MaleModPrivateWriteCapture(IDirect3DDevice9* d,unsigned frame,bool command){
 char root[MAX_PATH]{};if(!GetEnvironmentVariableA("MALEMOD_PRIVATE_CAPTURE",root,sizeof(root)))return;
 char file[MAX_PATH]{};if(command)sprintf_s(file,"%s-frame-%u-command-%u.png",root,frame,maleModPrivateControlRevision);else sprintf_s(file,"%s-frame-%u.png",root,frame);
 IDirect3DSurface9* back=nullptr;if(SUCCEEDED(d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back))&&back){HRESULT hr=D3DXSaveSurfaceToFileA(file,D3DXIFF_PNG,back,nullptr,nullptr);back->Release();if(command)Log("Private native capture revision=%u result=%08x file=%s",maleModPrivateControlRevision,hr,file);}
}
static void MaleModPrivateCapture(IDirect3DDevice9* d,bool menuTank){
 if(!maleModPrivateExActive)return;
 static unsigned frame=0,menuFrames=0,gameFrames=0,releaseAt=0,releaseKey=VK_RETURN;++frame;if(menuTank)++menuFrames;const bool game=anatomyScene==0&&anatomyVisibleFrame==renderFrameSerial-1;if(game)++gameFrames;if(gameFrames==1&&game){PostMessageW(maleModPrivateWindow,WM_ACTIVATEAPP,TRUE,GetCurrentThreadId());PostMessageW(maleModPrivateWindow,WM_ACTIVATE,WA_ACTIVE,0);Log("Private native gameplay activation: exact owned HWND only");}
 if(releaseAt&&frame>=releaseAt){PostMessageW(maleModPrivateWindow,WM_KEYUP,releaseKey,0xc0000001);releaseAt=0;}
 char boot[64]{};GetEnvironmentVariableA("MALEMOD_PRIVATE_MENU",boot,sizeof(boot));
 const bool startNew=!strcmp(boot,"new-game")||!strcmp(boot,"new-game-normal");
 const bool enterTitle=menuTank&&menuFrames==80&&(!strcmp(boot,"enter-title")||startNew);
 const bool newGame=menuTank&&menuFrames==160&&startNew;
 const bool normal=menuTank&&menuFrames==320&&!strcmp(boot,"new-game-normal");
 if((enterTitle||newGame||normal)&&IsWindow(maleModPrivateWindow)){
  // Observed WStart native title screen explicitly requests Enter. Deliver
  // only this single transition; inspect the subsequent native capture.
  PostMessageW(maleModPrivateWindow,WM_KEYDOWN,VK_RETURN,0x001c0001);
  releaseAt=frame+4;releaseKey=VK_RETURN;
  OutputDebugStringA(enterTitle?"Private native title Enter delivered to owned HWND\n":normal?"Private native observed Normal difficulty selected\n":"Private native observed New Game selected\n");
 }
 LARGE_INTEGER commandA{},commandB{},commandFrequency{};QueryPerformanceCounter(&commandA);maleModPrivateGameFrames=gameFrames;MaleModPrivateCommands(frame,game);QueryPerformanceCounter(&commandB);QueryPerformanceFrequency(&commandFrequency);
 static double commandSum=0;static unsigned commandCount=0;if(game){commandSum+=double(commandB.QuadPart-commandA.QuadPart)*1000/double(commandFrequency.QuadPart);if(++commandCount==120){Log("Private command poll mean=%.3fms samples=120",commandSum/120);commandSum=0;commandCount=0;}}
 char startPaused[8]{};if(gameFrames==90&&game&&!MaleModPrivateControlsEnabled()&&GetEnvironmentVariableA("MALEMOD_PRIVATE_START_PAUSED",startPaused,sizeof(startPaused))==1&&startPaused[0]=='1'){maleModPrivatePaused=true;Log("Private command revision=0 key=Pause reason=startup-ready");}
 while(maleModPrivatePaused){Sleep(50);MaleModPrivateCommands(frame,game);if(maleModPrivateCaptureRequested){maleModPrivateCaptureRequested=false;MaleModPrivateWriteCapture(d,frame,true);}}
 static LARGE_INTEGER frequency{},last{};static double frameSeconds=0;static unsigned frameSamples=0;LARGE_INTEGER now{};QueryPerformanceCounter(&now);if(!frequency.QuadPart)QueryPerformanceFrequency(&frequency);if(last.QuadPart&&game){double dt=double(now.QuadPart-last.QuadPart)/double(frequency.QuadPart);if(dt<.5){frameSeconds+=dt;if(++frameSamples==120){Log("Private native present timing mean=%.3fms samples=120 Overall=%.0f clothing=%u menu=%d",frameSeconds*1000/120,sliderUI[0],clothingStyle,menuOpen);Log("Private inclusive draw-hook mean=%.3fms callsPerFrame=%.2f samples=120",maleModPrivateDrawMs/120,double(maleModPrivateDrawCalls)/120);maleModPrivateDrawMs=0;maleModPrivateDrawCalls=0;frameSeconds=0;frameSamples=0;}}}last=now;
 if(game&&MaleModPrivateControlsEnabled()&&(gameFrames==30||gameFrames==150)){
  const bool down=gameFrames==30;PostMessageW(maleModPrivateWindow,down?WM_KEYDOWN:WM_KEYUP,'W',down?0x00110001:0xc0110001);
  Log("Private native walking %s; exact owned HWND only",down?"begin":"end");
 }
 if(game&&(gameFrames==1||gameFrames==90||gameFrames==160||gameFrames==225||gameFrames==360||gameFrames==550))Log("Private gameplay receipt frame=%u Overall=%.0f physics=%d clothing=%u menu=%d pelvis=(%.3f %.3f %.3f) force=(%.3f %.3f)",gameFrames,sliderUI[0],physicsState,clothingStyle,menuOpen,motionPrevPosition[0],motionPrevPosition[1],motionPrevPosition[2],motionPitchForce,motionYawForce);
 const bool commanded=maleModPrivateCaptureRequested;maleModPrivateCaptureRequested=false;
 if(!commanded&&frame!=1&&frame!=2&&frame!=120&&frame!=240&&frame!=600&&frame!=1200&&!(game&&(gameFrames==1||gameFrames==90||gameFrames==160||gameFrames==225||gameFrames==360||gameFrames==550)))return;
 char root[MAX_PATH]{};if(!GetEnvironmentVariableA("MALEMOD_PRIVATE_CAPTURE",root,sizeof(root)))return;
 char file[MAX_PATH]{};sprintf_s(file,"%s-frame-%u.png",root,frame);
 IDirect3DSurface9* back=nullptr;if(SUCCEEDED(d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back))&&back){D3DXSaveSurfaceToFileA(file,D3DXIFF_PNG,back,nullptr,nullptr);back->Release();}
}
static void MaleModPrivateActivate(HWND window){
 if(!maleModPrivateExActive)return;
 DWORD process=0;GetWindowThreadProcessId(window,&process);
 if(process!=GetCurrentProcessId())return;
 MaleModPrivateFocusImports(window);
 // Activation stays inside this process's verified private station. It does
 // not switch desktops or send input to the user's interactive station.
 SetActiveWindow(window);SetFocus(window);SetForegroundWindow(window);
 PostMessageW(window,WM_ACTIVATEAPP,TRUE,GetCurrentThreadId());
 PostMessageW(window,WM_ACTIVATE,WA_ACTIVE,0);
}
// Private iteration test factory. Production callers always use their existing
// classic D3D9 factory. D3D9Ex is allowed only on a verified noninteractive
// station and with both explicit matching launcher environment guards.
static IDirect3D9* MaleModPrivateFactory(UINT sdk,HMODULE systemD3D9,
                                      IDirect3D9* (WINAPI* classic)(UINT)) {
 char enabled[8]{},declared[256]{};wchar_t actual[256]{};
 DWORD bytes=0;USEROBJECTFLAGS flags{};auto station=GetProcessWindowStation();
 bool requested=GetEnvironmentVariableA("MALEMOD_ISOLATED_D3D9EX",enabled,sizeof(enabled))==1&&enabled[0]=='1';
 if(!requested)return classic(sdk);
 if(!GetEnvironmentVariableA("MALEMOD_SEALED_SESSION",declared,sizeof(declared))||
    !GetUserObjectInformationW(station,UOI_FLAGS,&flags,sizeof(flags),&bytes)||
    (flags.dwFlags&WSF_VISIBLE)||
    !GetUserObjectInformationW(station,UOI_NAME,actual,sizeof(actual),&bytes)||
    !_wcsicmp(actual,L"WinSta0"))return classic(sdk);
 char stationName[256]{};WideCharToMultiByte(CP_UTF8,0,actual,-1,stationName,256,nullptr,nullptr);
 if(strcmp(stationName,declared))return classic(sdk);
 auto createEx=reinterpret_cast<HRESULT(WINAPI*)(UINT,IDirect3D9Ex**)>(GetProcAddress(systemD3D9,"Direct3DCreate9Ex"));
 IDirect3D9Ex* extended=nullptr;
 if(!createEx||FAILED(createEx(sdk,&extended)))return nullptr;
 maleModPrivateExActive=true;return static_cast<IDirect3D9*>(extended);
}

// D3D9Ex rejects D3DPOOL_MANAGED. The legacy engine locks these resources
// immediately, so the private test renderer must provide CPU-lockable GPU
// resources. This test path never changes production classic allocation.
static HRESULT(STDMETHODCALLTYPE* privateCreateTexture)(IDirect3DDevice9*,UINT,UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DTexture9**,HANDLE*);
static HRESULT(STDMETHODCALLTYPE* privateCreateCube)(IDirect3DDevice9*,UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DCubeTexture9**,HANDLE*);
static HRESULT(STDMETHODCALLTYPE* privateCreateVB)(IDirect3DDevice9*,UINT,DWORD,DWORD,D3DPOOL,IDirect3DVertexBuffer9**,HANDLE*);
static HRESULT(STDMETHODCALLTYPE* privateCreateIB)(IDirect3DDevice9*,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DIndexBuffer9**,HANDLE*);
static HRESULT STDMETHODCALLTYPE PrivateTexture(IDirect3DDevice9* d,UINT w,UINT h,UINT levels,DWORD usage,D3DFORMAT f,D3DPOOL p,IDirect3DTexture9** out,HANDLE* shared){if(p==D3DPOOL_MANAGED){p=D3DPOOL_DEFAULT;usage=(usage&~D3DUSAGE_AUTOGENMIPMAP)|D3DUSAGE_DYNAMIC;}return privateCreateTexture(d,w,h,levels,usage,f,p,out,shared);}
static HRESULT STDMETHODCALLTYPE PrivateCube(IDirect3DDevice9* d,UINT size,UINT levels,DWORD usage,D3DFORMAT f,D3DPOOL p,IDirect3DCubeTexture9** out,HANDLE* shared){if(p==D3DPOOL_MANAGED){p=D3DPOOL_DEFAULT;usage=(usage&~D3DUSAGE_AUTOGENMIPMAP)|D3DUSAGE_DYNAMIC;}return privateCreateCube(d,size,levels,usage,f,p,out,shared);}
static HRESULT STDMETHODCALLTYPE PrivateVB(IDirect3DDevice9* d,UINT size,DWORD usage,DWORD fvf,D3DPOOL p,IDirect3DVertexBuffer9** out,HANDLE* shared){if(p==D3DPOOL_MANAGED)p=D3DPOOL_DEFAULT;return privateCreateVB(d,size,usage,fvf,p,out,shared);}
static HRESULT STDMETHODCALLTYPE PrivateIB(IDirect3DDevice9* d,UINT size,DWORD usage,D3DFORMAT f,D3DPOOL p,IDirect3DIndexBuffer9** out,HANDLE* shared){if(p==D3DPOOL_MANAGED)p=D3DPOOL_DEFAULT;return privateCreateIB(d,size,usage,f,p,out,shared);}
static void MaleModPrivatePoolTranslation(IDirect3DDevice9* d){if(!maleModPrivateExActive)return;static bool patched=false;if(patched)return;auto vt=*reinterpret_cast<void***>(d);
 auto set=[&](unsigned index,void* target,void** original){DWORD old=0;if(!VirtualProtect(vt+index,sizeof(void*),PAGE_EXECUTE_READWRITE,&old))return;*original=vt[index];vt[index]=target;DWORD ignored=0;VirtualProtect(vt+index,sizeof(void*),old,&ignored);};
 set(23,reinterpret_cast<void*>(PrivateTexture),reinterpret_cast<void**>(&privateCreateTexture));set(25,reinterpret_cast<void*>(PrivateCube),reinterpret_cast<void**>(&privateCreateCube));set(26,reinterpret_cast<void*>(PrivateVB),reinterpret_cast<void**>(&privateCreateVB));set(27,reinterpret_cast<void*>(PrivateIB),reinterpret_cast<void**>(&privateCreateIB));patched=true;
}
