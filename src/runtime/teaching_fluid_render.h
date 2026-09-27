#include "fluid_surface.h"
static volumeFluid::Surface fluidSurface;
#pragma once
// Included after the final R14 surface. Fluid is simulated after pelvis skinning
// in character component space. Root travel/terrain collision are not yet solved.
static IDirect3DVertexShader9* teachingVS=nullptr;
static IDirect3DPixelShader9* teachingPS=nullptr;
static LONG teachingDrawSerial=-2;
static DWORD teachingSceneTick=0;
static double teachingFluidTime=0;
static unsigned teachingDraws=0;
static HRESULT teachingLastDraw=S_FALSE;
static LARGE_INTEGER fluidProfileFrequency{};static double fluidProfileSum=0,fluidProfilePeak=0;static unsigned fluidProfileFrames=0;
static void LogFluidProfile(){if(fluidProfileFrames)Log("Fluid2 CPU update+mesh+draw submission: mean=%.3fms max=%.3fms frames=%u (not GPU/game frame time)",fluidProfileSum/fluidProfileFrames,fluidProfilePeak,fluidProfileFrames);fluidProfileSum=fluidProfilePeak=0;fluidProfileFrames=0;}
static void CancelTeaching(){teachingAudio.End(true);LogFluidProfile();teachingTimeline.Cancel();teachingFluid.Clear();teachingFluidTime=0;}
static void ReleaseTeaching(){
 CancelTeaching();fluidSurface.Release();teachingSceneTick=0;teachingDrawSerial=-2;
 if(teachingVS){teachingVS->Release();teachingVS=nullptr;}
 if(teachingPS){teachingPS->Release();teachingPS=nullptr;}
}
static V3 TeachingRing(UINT ring){
 V3 sum{};for(UINT i=0;i<r14SegmentCount;i++)sum=sum+r14Positions[r14NewStart+ring*r14SegmentCount+i];
 return sum/float(r14SegmentCount);
}
static bool TeachingEmitter(const float* bone,V3& tip,V3& direction){
 if(!r14Ready)return false;
 // R16/R14 rings 87..91 and the cap turn back into the meatal recess.
 // Ring 86 is the outer lip. An inner-recess tangent points back/upward.
 V3 lip=TeachingRing(r14RingCount-6);
 V3 ring=TeachingRing(r14CrownRing+(r14RingCount-r14CrownRing)/2);
 tip=TransformPoint(bone,lip);V3 back=TransformPoint(bone,ring);
 if(!std::isfinite(tip.x)||!std::isfinite(tip.y)||!std::isfinite(tip.z)||Length(tip-back)<.001f)return false;
 direction=Unit(tip-back);tip=tip+direction*.06f;return true;
}
static bool TeachingMatrices(IDirect3DDevice9* d,float* local,float* view,float* bone){
 ShaderLayout* layout=GetShaderLayout(d);
 if(!layout||!layout->valid||!layout->viewValid)return false;
 return SUCCEEDED(d->GetVertexShaderConstantF(layout->localRegister,local,4))
  &&SUCCEEDED(d->GetVertexShaderConstantF(layout->viewRegister,view,4))
  &&SUCCEEDED(d->GetVertexShaderConstantF(layout->boneRegister,bone,3));
}
static bool EnsureTeachingShaders(IDirect3DDevice9* d){
 if(teachingVS&&teachingPS)return true;
 const char* vs="float4 L[4]:register(c0);float4 V[4]:register(c4);struct O{float4 p:POSITION;float4 c:COLOR0;};O main(float4 p:POSITION,float4 c:COLOR0){O o;float4 w=p.x*L[0]+p.y*L[1]+p.z*L[2]+L[3];o.p=w.x*V[0]+w.y*V[1]+w.z*V[2]+w.w*V[3];o.c=c;return o;}";
 const char* ps="float4 main(float4 c:COLOR0):COLOR0{return c;}";
 ID3DXBuffer *code=nullptr,*errors=nullptr;
 HRESULT hr=D3DXCompileShader(vs,(UINT)strlen(vs),nullptr,nullptr,"main","vs_3_0",0,&code,&errors,nullptr);
 if(errors)errors->Release();if(FAILED(hr))return false;
 if(!teachingVS)hr=d->CreateVertexShader((DWORD*)code->GetBufferPointer(),&teachingVS);code->Release();if(FAILED(hr))return false;
 code=errors=nullptr;hr=D3DXCompileShader(ps,(UINT)strlen(ps),nullptr,nullptr,"main","ps_3_0",0,&code,&errors,nullptr);
 if(errors)errors->Release();if(FAILED(hr))return false;
 if(!teachingPS)hr=d->CreatePixelShader((DWORD*)code->GetBufferPointer(),&teachingPS);code->Release();return SUCCEEDED(hr);
}

static bool LoadVolumeConfig(){
 char path[MAX_PATH];SiblingPath(path,"TeachingFluid.ini");volumeFluid::Settings cfg;
 auto read=[&](const char* name,float& value,float lo,float hi){char b[80];GetPrivateProfileStringA("Fluid",name,"",b,sizeof(b),path);if(!*b)return true;char* end=nullptr;float v=strtof(b,&end);while(end&&*end==' ')++end;if(!end||*end||!std::isfinite(v)||v<lo||v>hi){Log("Fluid configuration rejected: %s=%s range %.4g..%.4g",name,b,lo,hi);return false;}value=v;return true;};
 float capacity=(float)cfg.capacity,pressure=(float)cfg.iterations,visc=(float)cfg.viscIterations,plane=0;
 if(!read("Volume",cfg.volume,.01f,4000)||!read("Duration",cfg.feed,.15f,10)||!read("PulseVolumeVariation",cfg.pulseVolumeVariation,0,.75f)||!read("PulseDurationVariation",cfg.pulseDurationVariation,0,.9f)||!read("AngleVariation",cfg.angleVariation,0,1)||!read("Viscosity",cfg.viscosity,0,100)||!read("SurfaceTension",cfg.tension,0,100)||!read("NozzleRadius",cfg.nozzle,.15f,4)||!read("Spacing",cfg.spacing,.15f,.8f)||!read("FlowVariation",cfg.flowVariation,0,.7f)||!read("Lifetime",cfg.lifetime,1,15)||!read("DropVolume",cfg.dropVolume,.005f,20)||!read("DropDuration",cfg.dropDuration,.3f,5)||!read("DropHold",cfg.dropHold,.1f,5)||!read("DropLength",cfg.dropLength,.5f,15)||!read("Capacity",capacity,1024,65536)||!read("PressureIterations",pressure,2,12)||!read("ViscosityIterations",visc,2,32)||!read("CatchPlane",plane,0,1)||!read("CatchDepth",cfg.catchDepth,5,150))return false;
 float sides=(float)cfg.meshSides;
 if(!read("ThreadSpacing",cfg.threadSpacing,.25f,2)||!read("Breakup",cfg.breakup,0,4)||!read("MeshSides",sides,8,20))return false;cfg.meshSides=(int)sides;
 cfg.capacity=(int)capacity;cfg.iterations=(int)pressure;cfg.viscIterations=(int)visc;cfg.catchPlane=plane>.5f;
 volumeFluid::config=cfg;Log("Fluid config: Volume=%.3f Duration=%.3f Viscosity=%.3f Spacing=%.3f Capacity=%d",cfg.volume,cfg.feed,cfg.viscosity,cfg.spacing,cfg.capacity);return true;
}
static void DrawTeachingFluid(IDirect3DDevice9* d){
 if(teachingDrawSerial==renderFrameSerial)return;DWORD color=0;d->GetRenderState(D3DRS_COLORWRITEENABLE,&color);if(!(color&7))return;
 ShaderLayout* layout=GetShaderLayout(d);if(!layout||!layout->valid||!layout->viewValid)return;teachingSceneTick=GetTickCount();teachingDrawSerial=renderFrameSerial;
 if(!teachingTimeline.active)return;
 float local[16],view[16],bone[12];if(!TeachingMatrices(d,local,view,bone))return;V3 tip,dir;if(!TeachingEmitter(bone,tip,dir)){CancelTeaching();return;}
 // UE3 LocalToWorld includes pre-view translation. It is render data, not a
 // persistent physics frame: feeding it into the solver makes camera movement
 // stretch the trail or trip the teleport guard. Bones are component-space.
 D3DXMATRIX localMatrix,viewMatrix,componentClip;
 memcpy(&localMatrix,local,64);memcpy(&viewMatrix,view,64);
 D3DXMatrixMultiply(&componentClip,&localMatrix,&viewMatrix);
 if(!teachingFluid.ready){if(!teachingFluid.Begin(tip)){Log("Fluid unavailable: %s",teachingFluid.error.c_str());CancelTeaching();return;}teachingFluid.lastDir=dir;teachingLastTick=GetTickCount();}
 // Conservative moving collision proxies sampled from the actual shaft rings.
 for(int i=0;i<8;i++){UINT ring=2+(r14CrownRing-4)*i/7;V3 center=TransformPoint(bone,TeachingRing(ring));float radius=1e9f;for(UINT j=0;j<r14SegmentCount;j++){V3 p=TransformPoint(bone,r14Positions[r14NewStart+ring*r14SegmentCount+j]);radius=min(radius,Length(p-center));}teachingFluid.collision[i]={center.x,center.y,center.z,radius*.86f};}
 float dt=(float)(teachingTimeline.time-teachingFluidTime);teachingFluidTime=teachingTimeline.time;
 LARGE_INTEGER begin,end;if(!fluidProfileFrequency.QuadPart)QueryPerformanceFrequency(&fluidProfileFrequency);QueryPerformanceCounter(&begin);
 teachingFluid.Advance(dt,tip,dir,{0,0,-98});if(!teachingFluid.ready){Log("Fluid stopped: %s",teachingFluid.error.c_str());CancelTeaching();return;}
 if(!teachingFluid.Live())return;
 teachingLastDraw=fluidSurface.Draw(d,teachingFluid.mesh,(const float*)&componentClip);
 QueryPerformanceCounter(&end);double elapsed=1000.*(end.QuadPart-begin.QuadPart)/fluidProfileFrequency.QuadPart;fluidProfileSum+=elapsed;fluidProfilePeak=max(fluidProfilePeak,elapsed);if(++fluidProfileFrames>=120)LogFluidProfile();
 if(SUCCEEDED(teachingLastDraw))++teachingDraws;else {Log("Fluid render failed: %s hr=%08X",fluidSurface.error.c_str(),(unsigned)teachingLastDraw);CancelTeaching();}
}
static void TeachingInput(IDirect3DDevice9* d){
 DWORD now=GetTickCount(),pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);bool focused=pid==GetCurrentProcessId();bool down=(GetAsyncKeyState(teachingKey)&0x8000)!=0;static bool oldDown=false;bool edge=down&&!oldDown;oldDown=down;
 if(!focused){if(teachingTimeline.active)CancelTeaching();teachingLastTick=now;return;}
  if(edge){if(teachingTimeline.active)CancelTeaching();else if(teachingSceneTick&&now-teachingSceneTick<250&&r14Ready){
  teachingFluid.SetVariationSeed(now);if(LoadVolumeConfig()&&teachingFluid.Prepare()&&fluidSurface.Initialize(d)){teachingTimeline.Start();teachingAudio.Begin();teachingFluid.Clear();teachingFluidTime=0;}else Log("Cannot start fluid: %s / %s",teachingFluid.error.c_str(),fluidSurface.error.c_str());
  now=GetTickCount();teachingLastTick=now;teachingSceneTick=now;
 }}
 float dt=teachingLastTick?(now-teachingLastTick)*.001f:0.f;teachingLastTick=now;
 if(teachingTimeline.active&&(dt>.25f||!teachingSceneTick||now-teachingSceneTick>1000)){CancelTeaching();return;}
 double audioFrom=teachingTimeline.time;teachingTimeline.Advance(dt);teachingAudio.Advance((float)audioFrom,(float)teachingTimeline.time);if(!teachingTimeline.active){teachingAudio.End(false);LogFluidProfile();teachingFluid.Clear();}
}



