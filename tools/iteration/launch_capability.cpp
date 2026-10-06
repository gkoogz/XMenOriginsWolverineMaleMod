// Development capability test only. Creates a noninteractive station, debugs
// exactly its child, and traps CRT pure virtual calls before their error UI.
// No host desktop switch, physical input, endpoint mute or installation occurs.
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <mmdeviceapi.h>
#include <audiopolicy.h>

// Mute only sessions that Windows attributes to this exact owned debug child.
// Never change endpoint/master volume, system sounds or another process session.
static std::vector<ISimpleAudioVolume*> privateMutedVolumes;
static HRESULT muteOwnedVolume(ISimpleAudioVolume* volume){
 const HRESULT result=volume->SetMute(TRUE,nullptr);
 if(SUCCEEDED(result)){volume->AddRef();privateMutedVolumes.push_back(volume);}
 return result;
}
static void restorePrivateAudio(){
 // Only volumes changed from unmuted by this launcher are retained. Holding
 // their exact COM identities avoids searching by path or reusing a PID.
 for(auto* volume:privateMutedVolumes){
  const HRESULT result=volume->SetMute(FALSE,nullptr);
  std::cout<<"{\"ownedAudioRestoreHRESULT\":"<<result<<"}\n"<<std::flush;
  volume->Release();
 }
 privateMutedVolumes.clear();
}
static void mutePrivateAudio(DWORD processID){
 static ULONGLONG last=0;if(GetTickCount64()-last<200)return;last=GetTickCount64();
 IMMDeviceEnumerator* devices=nullptr;if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void**)&devices)))return;
 IMMDeviceCollection* endpoints=nullptr;if(SUCCEEDED(devices->EnumAudioEndpoints(eRender,DEVICE_STATE_ACTIVE,&endpoints))){UINT count=0;endpoints->GetCount(&count);for(UINT n=0;n<count;++n){IMMDevice* endpoint=nullptr;if(FAILED(endpoints->Item(n,&endpoint)))continue;IAudioSessionManager2* manager=nullptr;if(SUCCEEDED(endpoint->Activate(__uuidof(IAudioSessionManager2),CLSCTX_ALL,nullptr,(void**)&manager))){IAudioSessionEnumerator* sessions=nullptr;if(SUCCEEDED(manager->GetSessionEnumerator(&sessions))){int total=0;sessions->GetCount(&total);for(int i=0;i<total;++i){IAudioSessionControl* control=nullptr;if(FAILED(sessions->GetSession(i,&control)))continue;IAudioSessionControl2* identity=nullptr;if(SUCCEEDED(control->QueryInterface(__uuidof(IAudioSessionControl2),(void**)&identity))){DWORD pid=0;if(identity->GetProcessId(&pid)==S_OK&&pid==processID){ISimpleAudioVolume* volume=nullptr;if(SUCCEEDED(control->QueryInterface(__uuidof(ISimpleAudioVolume),(void**)&volume))){BOOL muted=TRUE;if(SUCCEEDED(volume->GetMute(&muted))&&!muted&&SUCCEEDED(muteOwnedVolume(volume)))std::cout<<"{\"privateAudioSessionMuted\":"<<processID<<"}\n"<<std::flush;volume->Release();}}identity->Release();}control->Release();}sessions->Release();}manager->Release();}endpoint->Release();}endpoints->Release();}devices->Release();
}

static void require(bool good,const char* what){if(!good)throw std::runtime_error(std::string(what)+":"+std::to_string(GetLastError()));}
static std::wstring quote(const std::wstring& s){if(!s.empty()&&s.find_first_of(L" \t\n\v\"")==std::wstring::npos)return s;std::wstring r=L"\"";unsigned n=0;for(auto c:s){if(c==L'\\'){++n;continue;}r.append(c==L'\"'?2*n+1:n,L'\\');n=0;r+=c;}r.append(2*n,L'\\');return r+L"\"";}
template<class T>static T read(HANDLE p,uintptr_t a){T t{};SIZE_T n=0;require(ReadProcessMemory(p,(void*)a,&t,sizeof(t),&n)&&n==sizeof(t),"read debuggee");return t;}
static std::string stringAt(HANDLE p,uintptr_t a){std::string r;for(unsigned i=0;i<256;++i){char c=read<char>(p,a+i);if(!c)return r;r+=c;}throw std::runtime_error("oversize export name");}
static uintptr_t exported(HANDLE p,uintptr_t b,const char* symbol){
 auto dos=read<IMAGE_DOS_HEADER>(p,b);if(dos.e_magic!=IMAGE_DOS_SIGNATURE)return 0;
 auto nt=read<IMAGE_NT_HEADERS32>(p,b+dos.e_lfanew);if(nt.Signature!=IMAGE_NT_SIGNATURE||nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC)return 0;
 auto dir=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];if(!dir.VirtualAddress)return 0;
 auto ex=read<IMAGE_EXPORT_DIRECTORY>(p,b+dir.VirtualAddress);
 for(DWORD i=0;i<ex.NumberOfNames;++i){auto name=read<DWORD>(p,b+ex.AddressOfNames+4*i);if(stringAt(p,b+name)!=symbol)continue;
  auto ord=read<WORD>(p,b+ex.AddressOfNameOrdinals+2*i);auto rva=read<DWORD>(p,b+ex.AddressOfFunctions+4*ord);if(rva>=dir.VirtualAddress&&rva<dir.VirtualAddress+dir.Size)return 0;return b+rva;
 }return 0;
}
int wmain(int argc,wchar_t** argv){
 if(argc<3){std::cerr<<"usage: launch_capability.exe dump-path executable [arguments]\n";return 2;}
 HWINSTA station=nullptr;HDESK desktop=nullptr;PROCESS_INFORMATION child{};auto original=GetProcessWindowStation();
 try{
  CoInitializeEx(nullptr,COINIT_MULTITHREADED);
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
  station=CreateWindowStationW(nullptr,0,WINSTA_CREATEDESKTOP|WINSTA_ENUMDESKTOPS|WINSTA_READATTRIBUTES|WINSTA_ACCESSGLOBALATOMS,nullptr);require(station!=nullptr,"private station");
  USEROBJECTFLAGS flags{};DWORD bytes=0;require(GetUserObjectInformationW(station,UOI_FLAGS,&flags,sizeof(flags),&bytes)&&!(flags.dwFlags&WSF_VISIBLE),"noninteractive station");
  wchar_t name[256]{};require(GetUserObjectInformationW(station,UOI_NAME,name,sizeof(name),&bytes)&&_wcsicmp(name,L"WinSta0"),"private identity");
  require(SetProcessWindowStation(station)!=0,"select private station");
  std::wstring dn=L"MaleModCapability."+std::to_wstring(GetCurrentProcessId());desktop=CreateDesktopW(dn.c_str(),nullptr,nullptr,0,DESKTOP_CREATEWINDOW|DESKTOP_READOBJECTS|DESKTOP_WRITEOBJECTS,nullptr);require(desktop!=nullptr,"private desktop");require(SetProcessWindowStation(original)!=0,"restore launcher station");
  std::wstring target=std::wstring(name)+L"\\"+dn,cmd=quote(argv[2]);for(int i=3;i<argc;++i)cmd+=L" "+quote(argv[i]);
  SetEnvironmentVariableW(L"MALEMOD_SEALED_SESSION",name);
  wchar_t compat[256]{};if(!GetEnvironmentVariableW(L"__COMPAT_LAYER",compat,256))SetEnvironmentVariableW(L"__COMPAT_LAYER",L"RunAsInvoker");
  STARTUPINFOW si{};si.cb=sizeof(si);si.lpDesktop=target.data();si.dwFlags=STARTF_USESHOWWINDOW;si.wShowWindow=SW_SHOWNORMAL;auto cwd=std::filesystem::path(argv[2]).parent_path();
  require(CreateProcessW(argv[2],cmd.data(),nullptr,nullptr,FALSE,DEBUG_ONLY_THIS_PROCESS,nullptr,cwd.c_str(),&si,&child)!=0,"create exact private debug child");
  FILETIME created{},e{},k{},u{};require(GetProcessTimes(child.hProcess,&created,&e,&k,&u)!=0,"child lifetime");
  std::cout<<"{\"processID\":"<<child.dwProcessId<<",\"processCreationTime\":"<<((uint64_t(created.dwHighDateTime)<<32)|created.dwLowDateTime)<<",\"noninteractiveWindowStation\":true,\"physicalInputUsed\":false,\"debuggerAttached\":true}\n"<<std::flush;
  std::vector<uintptr_t> traps;bool running=true;auto begin=GetTickCount64();
  struct IOTrap{BYTE original;std::string symbol;};std::map<uintptr_t,IOTrap> ioTraps;std::map<DWORD,uintptr_t> stepping;
  wchar_t traceText[8]{};const bool traceIO=GetEnvironmentVariableW(L"MALEMOD_TRACE_BOOT_IO",traceText,8)==1&&traceText[0]==L'1';
  auto writeByte=[&](uintptr_t a,BYTE byte){DWORD old=0;SIZE_T n=0;require(VirtualProtectEx(child.hProcess,(void*)a,1,PAGE_EXECUTE_READWRITE,&old)!=0,"debug protection");require(WriteProcessMemory(child.hProcess,(void*)a,&byte,1,&n)&&n==1,"debug write");DWORD ignored=0;VirtualProtectEx(child.hProcess,(void*)a,1,old,&ignored);FlushInstructionCache(child.hProcess,(void*)a,1);};
  DWORD boundedTimeout=45000;wchar_t timeoutText[16]{};if(GetEnvironmentVariableW(L"MALEMOD_PRIVATE_TIMEOUT_MS",timeoutText,16)){auto requested=wcstoul(timeoutText,nullptr,10);if(requested>=1000&&requested<=1800000)boundedTimeout=requested;}
  auto stopPath=std::filesystem::path(argv[1]).parent_path()/L"stop.request";bool closeRequested=false;
  while(running){if(!closeRequested&&std::filesystem::exists(stopPath)){closeRequested=true;std::cout<<"{\"ownedCloseRequested\":true}\n"<<std::flush;TerminateProcess(child.hProcess,0);}mutePrivateAudio(child.dwProcessId);if(GetTickCount64()-begin>boundedTimeout){std::cout<<"{\"timeoutMs\":"<<boundedTimeout<<"}\n";auto snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snap!=INVALID_HANDLE_VALUE){THREADENTRY32 te{};te.dwSize=sizeof(te);if(Thread32First(snap,&te))do{if(te.th32OwnerProcessID!=child.dwProcessId)continue;auto th=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,te.th32ThreadID);if(!th)continue;if(SuspendThread(th)!=DWORD(-1)){WOW64_CONTEXT c{};c.ContextFlags=WOW64_CONTEXT_CONTROL|WOW64_CONTEXT_INTEGER;if(Wow64GetThreadContext(th,&c)){std::cout<<"timeoutX86 thread="<<te.th32ThreadID<<" eip="<<std::hex<<c.Eip<<" esp="<<c.Esp<<" ebp="<<c.Ebp<<" stack:";for(unsigned i=0;i<80;++i){try{std::cout<<" "<<read<DWORD>(child.hProcess,c.Esp+4*i);}catch(const std::exception&){break;}}std::cout<<" ebpFrames:";auto bp=c.Ebp;for(unsigned i=0;i<16;++i){try{auto next=read<DWORD>(child.hProcess,bp),ret=read<DWORD>(child.hProcess,bp+4);std::cout<<" "<<ret;if(next<=bp||next-bp>1000000)break;bp=next;}catch(const std::exception&){break;}}std::cout<<std::dec<<"\n";}ResumeThread(th);}CloseHandle(th);}while(Thread32Next(snap,&te));CloseHandle(snap);}auto f=CreateFileW(argv[1],GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(f!=INVALID_HANDLE_VALUE){BOOL dump=MiniDumpWriteDump(child.hProcess,child.dwProcessId,f,MiniDumpWithThreadInfo,nullptr,nullptr,nullptr);CloseHandle(f);std::cout<<"{\"timeoutDumpWritten\":"<<(dump?"true":"false")<<"}\n";}TerminateProcess(child.hProcess,124);begin=GetTickCount64();}DEBUG_EVENT ev{};if(!WaitForDebugEvent(&ev,250)){require(GetLastError()==ERROR_SEM_TIMEOUT,"wait debug event");continue;}
   DWORD status=DBG_CONTINUE;
   if(ev.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT){auto& d=ev.u.LoadDll;try{auto a=exported(child.hProcess,(uintptr_t)d.lpBaseOfDll,"_purecall");if(a){writeByte(a,0xcc);traps.push_back(a);std::cout<<"{\"crtPurecallTrap\":"<<a<<"}\n"<<std::flush;}if(traceIO){for(auto symbol:{"CreateFileW","CreateFileA"}){auto f=exported(child.hProcess,(uintptr_t)d.lpBaseOfDll,symbol);if(f&&!ioTraps.count(f)){ioTraps.emplace(f,IOTrap{read<BYTE>(child.hProcess,f),symbol});writeByte(f,0xcc);}}}}catch(const std::exception&){std::cout<<"{\"moduleInspectionSkipped\":true}\n";}if(d.hFile)CloseHandle(d.hFile);}
   else if(ev.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT){if(ev.u.CreateProcessInfo.hFile)CloseHandle(ev.u.CreateProcessInfo.hFile);}
   else if(ev.dwDebugEventCode==OUTPUT_DEBUG_STRING_EVENT){auto& d=ev.u.DebugString;std::vector<char> s(size_t(d.nDebugStringLength)*(d.fUnicode?2:1)+2);SIZE_T n=0;if(ReadProcessMemory(child.hProcess,d.lpDebugStringData,s.data(),s.size()-2,&n)){if(d.fUnicode)std::wcout<<L"debug: "<<(wchar_t*)s.data()<<L"\n";else std::cout<<"debug: "<<s.data()<<"\n";}}
   else if(ev.dwDebugEventCode==EXCEPTION_DEBUG_EVENT){auto& x=ev.u.Exception;const auto a=(uintptr_t)x.ExceptionRecord.ExceptionAddress;bool fatal=std::find(traps.begin(),traps.end(),a)!=traps.end()||!x.dwFirstChance;
    const bool breakpoint=x.ExceptionRecord.ExceptionCode==EXCEPTION_BREAKPOINT||x.ExceptionRecord.ExceptionCode==0x4000001f;
    const bool singleStep=x.ExceptionRecord.ExceptionCode==EXCEPTION_SINGLE_STEP||x.ExceptionRecord.ExceptionCode==0x4000001e;
    if(traceIO&&((breakpoint&&ioTraps.count(a))||(singleStep&&stepping.count(ev.dwThreadId)))){
     auto thread=OpenThread(THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,ev.dwThreadId);require(thread!=nullptr,"IO trace thread");WOW64_CONTEXT ctx{};ctx.ContextFlags=WOW64_CONTEXT_CONTROL|WOW64_CONTEXT_INTEGER;require(Wow64GetThreadContext(thread,&ctx)!=0,"IO trace context");
     if(singleStep){auto trap=stepping.at(ev.dwThreadId);writeByte(trap,0xcc);ctx.EFlags&=~0x100;stepping.erase(ev.dwThreadId);}
     else{auto t=ioTraps.at(a);try{auto arg=read<DWORD>(child.hProcess,ctx.Esp+4);std::wstring path;const bool wide=t.symbol.back()=='W';for(unsigned i=0;i<1024;++i){wchar_t c=wide?read<wchar_t>(child.hProcess,arg+2*i):read<unsigned char>(child.hProcess,arg+i);if(!c)break;path+=c;}if(path.find(L".ini")!=std::wstring::npos||path.find(L"SoundModes")!=std::wstring::npos||path.find(L"Content")!=std::wstring::npos||path.find(L"Startup")!=std::wstring::npos||path.find(L"rgame")!=std::wstring::npos||path.find(L"MaleModSandbox")!=std::wstring::npos||path.find(L"WStart")!=std::wstring::npos||path.find(L"sandbox-exec")!=std::wstring::npos||path.find(L"SaveData")!=std::wstring::npos){std::cout<<"fileIO("<<t.symbol<<"): ";std::wcout<<path;if(t.symbol.rfind("CreateFile",0)==0)std::cout<<" desiredAccess="<<std::hex<<read<DWORD>(child.hProcess,ctx.Esp+8)<<std::dec;std::wcout<<L"\n"<<std::flush;}}catch(const std::exception&){}writeByte(a,t.original);ctx.Eip=static_cast<DWORD>(a);ctx.EFlags|=0x100;stepping.emplace(ev.dwThreadId,a);}
     require(Wow64SetThreadContext(thread,&ctx)!=0,"resume traced instruction");CloseHandle(thread);require(ContinueDebugEvent(ev.dwProcessId,ev.dwThreadId,DBG_CONTINUE)!=0,"continue IO trace");continue;
    }
    if(x.ExceptionRecord.ExceptionCode==0xe06d7363&&x.ExceptionRecord.NumberParameters>=3){try{auto object=x.ExceptionRecord.ExceptionInformation[1],ti=x.ExceptionRecord.ExceptionInformation[2];auto ca=read<DWORD>(child.hProcess,ti+12);auto ct=read<DWORD>(child.hProcess,ca+4);auto td=read<DWORD>(child.hProcess,ct+4);std::cout<<"cppType: "<<stringAt(child.hProcess,td+8)<<"\n";auto value=read<DWORD>(child.hProcess,object);std::wstring text;for(unsigned i=0;i<2048;++i){auto c=read<wchar_t>(child.hProcess,value+2*i);if(!c)break;if(c<32&&c!=10&&c!=13)break;text+=c;}std::wcout<<L"cppText: "<<text<<L"\n";}catch(const std::exception&){}}
    std::cout<<"{\"exceptionCode\":"<<x.ExceptionRecord.ExceptionCode<<",\"address\":"<<a<<",\"firstChance\":"<<x.dwFirstChance<<",\"purecallBeforeDialog\":"<<(fatal&&breakpoint?"true":"false")<<"}\n"<<std::flush;
    if(fatal||x.ExceptionRecord.ExceptionCode==EXCEPTION_ACCESS_VIOLATION){auto thread=OpenThread(THREAD_GET_CONTEXT,FALSE,ev.dwThreadId);if(thread){WOW64_CONTEXT c{};c.ContextFlags=WOW64_CONTEXT_CONTROL|WOW64_CONTEXT_INTEGER;if(Wow64GetThreadContext(thread,&c)){std::cout<<"x86Context eip="<<std::hex<<c.Eip<<" esp="<<c.Esp<<" ebp="<<c.Ebp<<" ecx="<<c.Ecx<<" stack:";for(unsigned n=0;n<64;++n){try{std::cout<<" "<<read<DWORD>(child.hProcess,c.Esp+4*n);}catch(const std::exception&){break;}}std::cout<<std::dec<<"\n";}CloseHandle(thread);}auto f=CreateFileW(argv[1],GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(f!=INVALID_HANDLE_VALUE){BOOL dump=MiniDumpWriteDump(child.hProcess,child.dwProcessId,f,MiniDumpWithThreadInfo,nullptr,nullptr,nullptr);CloseHandle(f);std::cout<<"{\"dumpWritten\":"<<(dump?"true":"false")<<"}\n";}if(fatal)TerminateProcess(child.hProcess,3);}if(!fatal&&!breakpoint)status=DBG_EXCEPTION_NOT_HANDLED;
   }
   else if(ev.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT){std::cout<<"{\"exitCode\":"<<ev.u.ExitProcess.dwExitCode<<"}\n";running=false;}
   require(ContinueDebugEvent(ev.dwProcessId,ev.dwThreadId,status)!=0,"continue debug event");
  }
  restorePrivateAudio();CloseHandle(child.hThread);CloseHandle(child.hProcess);CloseDesktop(desktop);CloseWindowStation(station);return 0;
 }catch(const std::exception& e){restorePrivateAudio();SetProcessWindowStation(original);if(child.hProcess)TerminateProcess(child.hProcess,3);if(child.hThread)CloseHandle(child.hThread);if(child.hProcess)CloseHandle(child.hProcess);if(desktop)CloseDesktop(desktop);if(station)CloseWindowStation(station);std::cerr<<e.what()<<'\n';return 1;}
}
