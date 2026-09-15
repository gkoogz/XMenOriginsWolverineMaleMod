#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9shader.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <vector>
#include <cmath>
#include "morph_targets.h"
#include "physics_weights.h"
#include "graft_normals.h"
#include "pelvis_control.h"
#include "collar_fairing.h"
extern "C" IMAGE_DOS_HEADER __ImageBase;

static HMODULE realDll;
static IDirect3D9* (WINAPI *realCreate9)(UINT);
static int (WINAPI *realBegin)(D3DCOLOR,LPCWSTR);
static int (WINAPI *realEnd)();
typedef HRESULT (STDMETHODCALLTYPE *CreateDeviceFn)(IDirect3D9*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice9**);
typedef HRESULT (STDMETHODCALLTYPE *DIPFn)(IDirect3DDevice9*,D3DPRIMITIVETYPE,INT,UINT,UINT,UINT,UINT);
typedef HRESULT (STDMETHODCALLTYPE *EndSceneFn)(IDirect3DDevice9*);
typedef HRESULT (STDMETHODCALLTYPE *ResetFn)(IDirect3DDevice9*,D3DPRESENT_PARAMETERS*);
typedef HRESULT (STDMETHODCALLTYPE *PresentFn)(IDirect3DDevice9*,const RECT*,const RECT*,HWND,const RGNDATA*);
typedef HRESULT (STDMETHODCALLTYPE *SwapPresentFn)(IDirect3DSwapChain9*,const RECT*,const RECT*,HWND,const RGNDATA*,DWORD);
static CreateDeviceFn origCreateDevice;
static DIPFn origDIP;
static EndSceneFn origEndScene;
static ResetFn origReset;
static PresentFn origPresent;
static SwapPresentFn origSwapPresent;
static volatile LONG logged;
static void Log(const char* fmt,...);
static IDirect3DVertexBuffer9* seenBuffers[256];
static UINT seenCount;
static IDirect3DVertexBuffer9* graftBuffer;
static UINT graftOffset;
static const UINT graftCount=2388, graftStride=32;
static const UINT graftTriangleIndexStart=249804;
static bool menuOpen=true, shapeDirty=true;
static int selectedSlider;
// Public controls use a uniform 1..100 scale.  The user's selected revision
// 133 preset is exactly 50; sliderValues/physValues remain the legacy physical
// units consumed by the established morph and solver code.
static const float neutralShape[7]={1.2f,1.6f,1.59f,1.53f,30.f,-.7f,.400001f};
static float sliderUI[7]={50.f,50.f,50.f,50.f,50.f,50.f,50.f};
static float sliderValues[7]={1.2f,1.6f,1.59f,1.53f,30.f,-.7f,.400001f};
struct PhysSpec {const char* name;float lo,hi,def,step;};
static const PhysSpec physSpecs[8]={{"SHAFT STIFF",-100,400,70,1},{"SHAFT WEIGHT",0,100,65,1},{"SHAFT BOUNCE",0,100,20,1},{"SHAFT VELOCITY",10,200,40,1},{"BALLS STIFF",0,100,70,1},{"BALLS WEIGHT",0,100,75,1},{"BALLS BOUNCE",0,100,35,1},{"BALLS VELOCITY",10,200,105,1}};
static const float neutralPhysics[8]={70.f,65.f,20.f,40.f,70.f,75.f,35.f,105.f};
static float physUI[8]={50.f,50.f,50.f,50.f,50.f,50.f,50.f,50.f};
static float physValues[8]={70.f,65.f,20.f,40.f,70.f,75.f,35.f,105.f};
static int physicsState=2,menuPage=0,selectedPhysics=0;
struct Spring2 {float pitch,yaw,pitchVelocity,yawVelocity;};
static Spring2 shaftSpring{},ballsSpring{};
struct V3 {float x,y,z;};
static V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
static V3 operator-(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
static V3 operator*(V3 a,float s){return {a.x*s,a.y*s,a.z*s};}
static V3 operator/(V3 a,float s){return {a.x/s,a.y/s,a.z/s};}
static float Dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static V3 Cross(V3 a,V3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
static float Length(V3 a){return sqrtf(Dot(a,a));}
static V3 Unit(V3 a){float n=Length(a);return n>1e-6f?a/n:V3{1,0,0};}
static float Smooth01(float value);
static float Smoother01(float value);
// A dense chain is required to form a real bend gradient.  Six long links let
// gravity concentrate the turn at a single ring even when the rendered spline
// was mathematically smooth.
static const int shaftNodeCount=12;
static V3 shaftNodes[shaftNodeCount],shaftPrevious[shaftNodeCount],ballNodes[2],ballPrevious[2];
static const int shaftRestSampleCount=18;
static V3 shaftRestCenters[shaftRestSampleCount];
static bool shaftRestFrameReady;
static V3 graftDeformedPositions[graftCount],graftDynamicNormalSums[graftNormalGroupCount],graftDynamicTangentSums[graftCount];
static V3 collarFairA[collarFairGroupCount],collarFairB[collarFairGroupCount];
static V3 collarNormalSums[collarFairGroupCount],collarTangentSums[collarFairGroupCount];
static bool constraintSolverReady;static int constraintSolverState=-1;static float constraintAccumulator,constraintRestLength=24.f;
static V3 constraintBallRest[2]={{14.30f,-2.0f,72.3f},{14.30f,2.0f,72.3f}};
static DWORD physicsLastTick;
static float physicsPhase;
static const float* overallWidthTargets[3][3]={{morph_ow_lo_lo,morph_ow_lo_def,morph_ow_lo_hi},{morph_ow_def_lo,morph_ow_def_def,morph_ow_def_hi},{morph_ow_hi_lo,morph_ow_hi_def,morph_ow_hi_hi}};
struct ShaderLayout {IDirect3DVertexShader9* shader;UINT boneRegister,boneCount,localRegister,localCount;D3DXPARAMETER_CLASS localClass;bool valid;};
static ShaderLayout shaderLayouts[16]{};static UINT shaderLayoutCount;
static bool motionTracked,motionBasisReady;static float motionPitchForce,motionYawForce,motionSpinSpeed,motionPrevPosition[3],motionPrevVelocity[3],motionFilteredAccel[3],motionPrevBasis[9],motionPrevAngularVelocity[3];static DWORD motionLastTick,motionLastCaptureTick;static LONG motionSamples;static int motionWarmupSamples,motionQuietFrames;
static LONG renderFrameSerial=-1,motionCaptureSerial=-2;
static bool settingsLoaded,settingsPending;static DWORD settingsChangedTick;
static bool frameRendered;
static volatile LONG presentLogged;
static volatile LONG endSceneLogged;
static volatile LONG endSceneCount;
static volatile LONG overlayLogged;
static volatile LONG motionPassLogged;
static volatile LONG motionCandidateLogs;
static volatile LONG motionBoneLogged;
static float motionPelvisMatrix[12],motionLeftThighMatrix[12],motionRightThighMatrix[12];
static bool motionCollisionBonesReady;
static __declspec(thread) bool inOverlay;

static ShaderLayout* GetShaderLayout(IDirect3DDevice9* d){
  IDirect3DVertexShader9* shader=nullptr;if(FAILED(d->GetVertexShader(&shader))||!shader)return nullptr;
  for(UINT i=0;i<shaderLayoutCount;i++)if(shaderLayouts[i].shader==shader){shader->Release();return &shaderLayouts[i];}
  if(shaderLayoutCount>=16){shader->Release();return nullptr;}ShaderLayout& layout=shaderLayouts[shaderLayoutCount++];layout.shader=shader;
  UINT bytes=0;if(FAILED(shader->GetFunction(nullptr,&bytes))||!bytes)return &layout;std::vector<DWORD> code((bytes+3)/4);if(FAILED(shader->GetFunction(code.data(),&bytes)))return &layout;
  ID3DXConstantTable* table=nullptr;if(FAILED(D3DXGetShaderConstantTable(code.data(),&table))||!table)return &layout;D3DXHANDLE bh=table->GetConstantByName(nullptr,"BoneMatrices"),lh=table->GetConstantByName(nullptr,"LocalToWorld");D3DXCONSTANT_DESC bd{},ld{};UINT one=1;
  if(bh&&SUCCEEDED(table->GetConstantDesc(bh,&bd,&one))){one=1;if(lh&&SUCCEEDED(table->GetConstantDesc(lh,&ld,&one))){layout.boneRegister=bd.RegisterIndex;layout.boneCount=bd.RegisterCount;layout.localRegister=ld.RegisterIndex;layout.localCount=ld.RegisterCount;layout.localClass=ld.Class;layout.valid=bd.RegisterCount>=3&&ld.RegisterCount>=4;}}
  table->Release();Log("Wolverine shader layout bone=c%u count=%u local=c%u count=%u class=%u valid=%d",layout.boneRegister,layout.boneCount,layout.localRegister,layout.localCount,layout.localClass,layout.valid?1:0);return &layout;
}
static void Normalize3(float* v){float n=sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);if(n>1e-6f){v[0]/=n;v[1]/=n;v[2]/=n;}}
static float SoftDeadzone(float value,float threshold){float magnitude=fabsf(value);return magnitude<=threshold?0.f:copysignf(magnitude-threshold,value);}
static void CaptureCharacterMotion(IDirect3DDevice9* d){
  ShaderLayout* layout=GetShaderLayout(d);if(!layout||!layout->valid)return;
  // The section's actual bone map is: slot 0 = pelvis, slot 3 = left proximal
  // thigh twist, slot 4 = right proximal thigh twist. LocalToWorld is excluded
  // because UE3 folds camera-relative/pre-view translation into that matrix.
  const UINT motionBone=0;float bone[12]{},leftThigh[12]{},rightThigh[12]{};
  if(layout->boneCount<15||FAILED(d->GetVertexShaderConstantF(layout->boneRegister,bone,3))||FAILED(d->GetVertexShaderConstantF(layout->boneRegister+9,leftThigh,3))||FAILED(d->GetVertexShaderConstantF(layout->boneRegister+12,rightThigh,3)))return;
  memcpy(motionPelvisMatrix,bone,sizeof(bone));memcpy(motionLeftThighMatrix,leftThigh,sizeof(leftThigh));memcpy(motionRightThighMatrix,rightThigh,sizeof(rightThigh));motionCollisionBonesReady=true;
  float world[3]={bone[3],bone[7],bone[11]},basis[9]{};
  for(int axis=0;axis<3;axis++){for(int a=0;a<3;a++)basis[axis*3+a]=bone[axis*4+a];Normalize3(&basis[axis*3]);}
  if(InterlockedCompareExchange(&motionBoneLogged,1,0)==0)Log("camera-invariant bones selected: pelvis=slot0 leftThighTwist=slot3 rightThighTwist=slot4; LocalToWorld excluded");
  DWORD now=GetTickCount();motionLastCaptureTick=now;if(!motionLastTick){memcpy(motionPrevPosition,world,12);memcpy(motionPrevBasis,basis,sizeof(basis));motionBasisReady=true;motionLastTick=now;motionWarmupSamples=motionQuietFrames=0;motionTracked=false;return;}float dt=(now-motionLastTick)*.001f;if(dt<.008f)return;motionLastTick=now;if(dt>.50f){memcpy(motionPrevPosition,world,12);memcpy(motionPrevBasis,basis,sizeof(basis));memset(motionPrevVelocity,0,12);memset(motionFilteredAccel,0,12);memset(motionPrevAngularVelocity,0,12);motionSpinSpeed=0;motionPitchForce=motionYawForce=0;motionBasisReady=true;motionWarmupSamples=0;motionQuietFrames=0;motionTracked=false;return;}
  float delta[3]={world[0]-motionPrevPosition[0],world[1]-motionPrevPosition[1],world[2]-motionPrevPosition[2]};memcpy(motionPrevPosition,world,12);float distance=sqrtf(delta[0]*delta[0]+delta[1]*delta[1]+delta[2]*delta[2]);float teleportDistance=max(8.f,dt*120.f);if(distance>teleportDistance){memset(motionPrevVelocity,0,12);memset(motionFilteredAccel,0,12);memset(motionPrevAngularVelocity,0,12);motionSpinSpeed=motionPitchForce=motionYawForce=0;motionTracked=false;motionWarmupSamples=0;motionQuietFrames=0;memcpy(motionPrevBasis,basis,sizeof(basis));return;}
  // A critically-smoothed velocity/acceleration estimate avoids the severe
  // noise amplification caused by taking two raw frame-to-frame derivatives.
  float rawVelocity[3]={delta[0]/dt,delta[1]/dt,delta[2]/dt},accel[3]{};
  float velocityAlpha=1.f-expf(-10.f*dt),accelAlpha=1.f-expf(-8.f*dt);
  for(int axis=0;axis<3;axis++){float oldVelocity=motionPrevVelocity[axis];motionPrevVelocity[axis]+=velocityAlpha*(rawVelocity[axis]-oldVelocity);float rawAccel=(motionPrevVelocity[axis]-oldVelocity)/dt;motionFilteredAccel[axis]+=accelAlpha*(rawAccel-motionFilteredAccel[axis]);accel[axis]=motionFilteredAccel[axis];}
  // Bone translation is already expressed in character/model space.
  float localAccel[3]={accel[0],accel[1],accel[2]};
  float worldOmega[3]{},localOmega[3]{};if(motionBasisReady){for(int axis=0;axis<3;axis++){const float* a=&motionPrevBasis[axis*3];const float* b=&basis[axis*3];worldOmega[0]+=(a[1]*b[2]-a[2]*b[1])*.5f/dt;worldOmega[1]+=(a[2]*b[0]-a[0]*b[2])*.5f/dt;worldOmega[2]+=(a[0]*b[1]-a[1]*b[0])*.5f/dt;}float omegaAlpha=1.f-expf(-12.f*dt);for(int axis=0;axis<3;axis++){float raw=worldOmega[0]*basis[axis*3]+worldOmega[1]*basis[axis*3+1]+worldOmega[2]*basis[axis*3+2];motionPrevAngularVelocity[axis]+=omegaAlpha*(raw-motionPrevAngularVelocity[axis]);localOmega[axis]=motionPrevAngularVelocity[axis];}}
  memcpy(motionPrevBasis,basis,sizeof(basis));motionBasisReady=true;
  if(motionWarmupSamples<3){motionWarmupSamples++;motionPitchForce=motionYawForce=0;motionTracked=false;return;}
  for(int axis=0;axis<3;axis++){localAccel[axis]=max(-800.f,min(800.f,SoftDeadzone(localAccel[axis],32.f)));localOmega[axis]=max(-10.f,min(10.f,SoftDeadzone(localOmega[axis],.16f)));}
  motionSpinSpeed=motionSpinSpeed*.78f+localOmega[2]*.22f;
  float pitch=max(-2.5f,min(2.5f,-localAccel[2]*.0052f-localAccel[0]*.0033f-localOmega[1]*.72f));
  float yaw=max(-2.5f,min(2.5f,-localAccel[1]*.0048f-localOmega[2]*1.40f));
  bool active=fabsf(pitch)+fabsf(yaw)>.035f;if(active)motionQuietFrames=0;else{pitch=yaw=0;motionQuietFrames++;}
  motionPitchForce=motionPitchForce*.70f+pitch*.30f;motionYawForce=motionYawForce*.70f+yaw*.30f;
  if(motionQuietFrames>10){motionPitchForce*=.55f;motionYawForce*=.55f;motionSpinSpeed*=.55f;shaftSpring.pitchVelocity*=.82f;shaftSpring.yawVelocity*=.82f;ballsSpring.pitchVelocity*=.82f;ballsSpring.yawVelocity*=.82f;if(fabsf(motionPitchForce)<.008f)motionPitchForce=0;if(fabsf(motionYawForce)<.008f)motionYawForce=0;}
  motionTracked=true;LONG sample=InterlockedIncrement(&motionSamples);if(sample==1||sample==120)Log("filtered character motion bone=(%.2f %.2f %.2f) accel=(%.1f %.1f %.1f) omega=(%.2f %.2f %.2f) force=(%.3f %.3f) quiet=%d",world[0],world[1],world[2],localAccel[0],localAccel[1],localAccel[2],localOmega[0],localOmega[1],localOmega[2],motionPitchForce,motionYawForce,motionQuietFrames);
}
static void StepSpring(Spring2& s,float stiffness,float weight,float bounce,float velocity,float pitchTarget,float yawTarget,float pitchForce,float yawForce,float dt){
  float speed=.50f+velocity*.014f;dt=min(dt*speed,.045f);float k=.18f+stiffness*.12f;float mass=.5f+weight*.02f;float damping=2.f*sqrtf(k*mass)*(1.f-bounce*.009f);
  float coupling=8.f+weight*.32f;float ap=(k*(pitchTarget-s.pitch)+pitchForce*coupling-damping*s.pitchVelocity)/mass;
  float ay=(k*(yawTarget-s.yaw)+yawForce*coupling-damping*s.yawVelocity)/mass;
  s.pitchVelocity+=ap*dt;s.yawVelocity+=ay*dt;s.pitch+=s.pitchVelocity*dt;s.yaw+=s.yawVelocity*dt;
  s.pitch=max(-1.65f,min(1.65f,s.pitch));s.yaw=max(-1.55f,min(1.55f,s.yaw));
}
static void StepFloppySpring(Spring2& s,float stiffness,float weight,float bounce,float velocity,float pitchTarget,float yawTarget,float pitchForce,float yawForce,float dt){
  float speed=.50f+velocity*.014f;dt=min(dt*speed,.045f);float mass=.60f+weight*.018f;
  float gravity=5.8f+weight*.045f,rigidity=stiffness*.105f;
  float damping=.18f+(100.f-bounce)*.025f,coupling=12.f+weight*.48f;
  float pitchError=pitchTarget-s.pitch,yawError=yawTarget-s.yaw;
  while(pitchError>3.14159265f)pitchError-=6.2831853f;while(pitchError<-3.14159265f)pitchError+=6.2831853f;
  damping=.90f+(100.f-bounce)*.05f;
  float ap=(gravity*max(-1.5f,min(1.5f,pitchError))-rigidity*s.pitch+pitchForce*coupling-damping*s.pitchVelocity)/mass;
  float ay=((gravity*.32f)*sinf(yawError)-rigidity*s.yaw+yawForce*coupling-damping*s.yawVelocity)/mass;
  s.pitchVelocity+=ap*dt;s.yawVelocity+=ay*dt;s.pitch+=s.pitchVelocity*dt;s.yaw+=s.yawVelocity*dt;
  s.pitchVelocity=max(-4.f,min(4.f,s.pitchVelocity));s.yawVelocity=max(-4.f,min(4.f,s.yawVelocity));
  float pitchLo=max(-3.10f,pitchTarget-1.15f),pitchHi=min(3.10f,pitchTarget+1.15f);
  s.pitch=max(pitchLo,min(pitchHi,s.pitch));s.yaw=max(-1.75f,min(1.75f,s.yaw));
}
static V3 TransformPoint(const float m[12],V3 p){return {m[0]*p.x+m[1]*p.y+m[2]*p.z+m[3],m[4]*p.x+m[5]*p.y+m[6]*p.z+m[7],m[8]*p.x+m[9]*p.y+m[10]*p.z+m[11]};}
static V3 InverseRigidPoint(const float m[12],V3 p){V3 q={p.x-m[3],p.y-m[7],p.z-m[11]};return {m[0]*q.x+m[4]*q.y+m[8]*q.z,m[1]*q.x+m[5]*q.y+m[9]*q.z,m[2]*q.x+m[6]*q.y+m[10]*q.z};}
static V3 RestShaftDirection(){float a=sliderValues[4]*3.1415926535f/180.f;return {cosf(a),0.f,-sinf(a)};}
static V3 ShaftRoot(){return {12.65f,0.f,84.3f};}
static float OverallShapeScale(){return sliderValues[0]/sliderSpecs[0].def;}
static float ShaftWidthScale(){return OverallShapeScale()*(sliderValues[2]/sliderSpecs[2].def);}
static float BallShapeScale(){return OverallShapeScale()*(sliderValues[3]/sliderSpecs[3].def);}
// Collision envelopes must follow the rendered morph all the way down.  The
// old large hard minima made a small scrotum collide as though it were nearly
// default-sized, especially when both thigh capsules rotated inward in crouch.
static float ShaftCollisionRadius(){return max(1.05f,2.75f*ShaftWidthScale());}
static float BallCollisionRadius(){return max(.85f,2.70f*BallShapeScale());}
static V3 BallAnchor(int side){float sign=side?1.f:-1.f;float shaftRadius=ShaftCollisionRadius();float base=max(1.15f,fabsf(constraintBallRest[side].y)*.78f);float desired=base+max(0.f,shaftRadius-2.75f)*.42f;return {12.55f,sign*desired,76.35f};}
static void SampleRestShaftFrame(float t,V3& center,V3& tangent);
static void InitializeConstraintSolver(){
  V3 root=ShaftRoot(),dir=RestShaftDirection();float segment=constraintRestLength/(shaftNodeCount-1);
  for(int i=0;i<shaftNodeCount;i++)shaftNodes[i]=shaftPrevious[i]=root+dir*(segment*i);
  ballNodes[0]=ballPrevious[0]=constraintBallRest[0];ballNodes[1]=ballPrevious[1]=constraintBallRest[1];
  constraintAccumulator=0;constraintSolverReady=true;constraintSolverState=physicsState;
  Log("constraint solver initialized nodes=%d restLength=%.2f state=%d",shaftNodeCount,constraintRestLength,physicsState);
}
static void SolveDistance(V3& a,V3& b,float target,float aInvMass,float bInvMass){V3 d=b-a;float n=Length(d),sum=aInvMass+bInvMass;if(n<1e-6f||sum<=0)return;V3 correction=d*((n-target)/(n*sum));a=a+correction*aInvMass;b=b-correction*bInvMass;}
static V3 SlerpDirection(V3 a,V3 b,float fraction){
  a=Unit(a);b=Unit(b);float c=max(-1.f,min(1.f,Dot(a,b))),angle=acosf(c);
  if(angle<1e-4f)return b;float s=sinf(angle);if(fabsf(s)<1e-4f)return Unit(a*(1.f-fraction)+b*fraction);
  return Unit(a*(sinf((1.f-fraction)*angle)/s)+b*(sinf(fraction*angle)/s));
}
static void ConstrainCurvatureGradient(V3 root,V3 restDir,float segment,float stiffnessSweep){
  shaftNodes[0]=root;shaftNodes[1]=root+restDir*segment;
  float stiffCurve=stiffnessSweep*stiffnessSweep;
  for(int i=1;i<shaftNodeCount-1;i++){
    float t=(float)(i-1)/(shaftNodeCount-2);
    // Maximum turn is deliberately smallest at the root and opens gradually
    // downstream.  Stiffness narrows the whole envelope without creating a
    // separate rigid/free hinge.
    float maxTurn=physicsState==0?(.050f+.035f*t):physicsState==1?(.085f+.115f*t):(.115f+.190f*t);
    maxTurn*=1.f-.42f*stiffCurve;
    V3 incoming=Unit(shaftNodes[i]-shaftNodes[i-1]);V3 outgoing=Unit(shaftNodes[i+1]-shaftNodes[i]);
    float angle=acosf(max(-1.f,min(1.f,Dot(incoming,outgoing))));
    if(angle>maxTurn){V3 limited=SlerpDirection(incoming,outgoing,maxTurn/angle);shaftNodes[i+1]=shaftNodes[i]+limited*segment;}
  }
  shaftNodes[0]=root;shaftNodes[1]=root+restDir*segment;
}
static void ResolveCapsule(V3& p,V3 a,V3 b,float radius){V3 ab=b-a;float denom=Dot(ab,ab),t=denom>1e-6f?max(0.f,min(1.f,Dot(p-a,ab)/denom)):0.f;V3 q=a+ab*t,d=p-q;float n=Length(d);if(n<radius){V3 normal=n>1e-5f?d/n:V3{1,0,0};p=q+normal*radius;}}
static void ResolveSphere(V3& p,V3 center,float radius){V3 d=p-center;float n=Length(d);if(n<radius){V3 normal=n>1e-5f?d/n:V3{1,0,0};p=center+normal*radius;}}
static void ResolvePairMinimum(V3& a,V3& b,float minimum){V3 d=b-a;float n=Length(d);if(n<minimum){V3 normal=n>1e-5f?d/n:V3{0,1,0};V3 correction=normal*((minimum-n)*.5f);a=a-correction;b=b+correction;}}
static void ResolvePairMaximum(V3& a,V3& b,float maximum){V3 d=b-a;float n=Length(d);if(n>maximum){V3 correction=d*((n-maximum)/(n*2.f));a=a+correction;b=b-correction;}}
static V3 LimitedCorrection(V3 correction,float compliance,float maximumStep){
  correction=correction*compliance;float distance=Length(correction);
  return distance>maximumStep?correction*(maximumStep/distance):correction;
}
static void ApplyNonimpulsiveCorrection(V3& point,V3& previous,V3 correction,float compliance,float maximumStep){
  V3 shift=LimitedCorrection(correction,compliance,maximumStep);point=point+shift;previous=previous+shift;
}
static void ResolvePairMaximumCompliant(V3& a,V3& previousA,V3& b,V3& previousB,float maximum){
  V3 d=b-a;float n=Length(d);if(n<=maximum||n<1e-6f)return;
  V3 shift=LimitedCorrection(d*((n-maximum)/(n*2.f)),.12f,.10f);
  a=a+shift;previousA=previousA+shift;b=b-shift;previousB=previousB-shift;
}
static void ResolvePairWeighted(V3& a,V3& b,float minimum,float aShare){
  V3 d=b-a;float n=Length(d);if(n>=minimum)return;
  V3 normal=n>1e-5f?d/n:V3{0,1,0};float penetration=minimum-n;
  a=a-normal*(penetration*aShare);b=b+normal*(penetration*(1.f-aShare));
}
static void ResolvePairMinimumCompliant(V3& a,V3& previousA,V3& b,V3& previousB,float minimum){
  V3 d=b-a;float distance=Length(d);if(distance>=minimum)return;
  V3 normal=distance>1e-5f?d/distance:V3{0,1,0};
  V3 shift=LimitedCorrection(normal*((minimum-distance)*.5f),.20f,.11f);
  a=a-shift;previousA=previousA-shift;b=b+shift;previousB=previousB+shift;
}
static void ResolvePairWeightedHistory(V3& a,V3& previousA,V3& b,V3& previousB,float minimum,float aShare){
  V3 beforeA=a,beforeB=b;ResolvePairWeighted(a,b,minimum,aShare);
  previousA=previousA+(a-beforeA);previousB=previousB+(b-beforeB);
}
static void ResolveBallCapsule(V3& p,V3& previous,V3 a,V3 b,float radius,int side,float relief=0.f){
  V3 ab=b-a;float denom=Dot(ab,ab),t=denom>1e-6f?max(0.f,min(1.f,Dot(p-a,ab)/denom)):0.f;V3 q=a+ab*t,d=p-q;float n=Length(d);if(n>=radius)return;
  V3 normal=n>1e-5f?d/n:V3{.25f,side?.95f:-.95f,-.12f};
  // A thigh contact may separate in several mathematically valid directions.
  // Prefer lateral/forward/downward escape so the lobe cannot be solved above
  // the shaft or behind the pelvis and then remain trapped there.
  if(normal.x<.05f||normal.z>.22f){V3 preferred={.24f,side?.82f:-.82f,-(.12f+.55f*relief)};normal=Unit(normal*.28f+preferred*.72f);}
  // Contact correction is compliant and is also applied to the Verlet history.
  // That removes the artificial velocity spike which made a deeply contacted
  // lobe visibly snap across the shaft or thigh on the following step.
  ApplyNonimpulsiveCorrection(p,previous,normal*(radius-n),.14f-.035f*relief,.12f-.035f*relief);
}
static void ResolveAnatomyCapsule(V3& p,V3& previous,V3 a,V3 b,float radius,float compliance,float maxStep,V3 preferred){
  V3 ab=b-a;float denominator=Dot(ab,ab),t=denominator>1e-6f?max(0.f,min(1.f,Dot(p-a,ab)/denominator)):0.f;
  V3 nearest=a+ab*t,d=p-nearest;float distance=Length(d);if(distance>=radius)return;
  V3 normal=distance>1e-5f?d/distance:preferred;
  // Contacts in this region have one anatomically useful escape direction:
  // anterior and slightly downward.  Blending toward it prevents a valid SDF
  // normal from routing a lobe over the shaft or around the back of a thigh.
  if(Dot(normal,preferred)<.35f)normal=Unit(normal*.30f+preferred*.70f);
  ApplyNonimpulsiveCorrection(p,previous,normal*(radius-distance),compliance,maxStep);
}
static void ResolveAnteriorEllipsoid(V3& p,V3& previous,V3 center,V3 radii,float compliance,float maxStep,float downwardBias){
  V3 q={(p.x-center.x)/radii.x,(p.y-center.y)/radii.y,(p.z-center.z)/radii.z};float level=Dot(q,q);if(level>=1.f)return;
  // Gradient of the implicit ellipsoid, forced onto the anterior hemisphere.
  // This is a one-sided body collision surface, never a trap that can eject
  // anatomy through the rear of the torso.
  V3 normal=Unit(V3{fabsf(q.x)/radii.x,q.y/radii.y,q.z/radii.z});
  if(normal.x<.32f)normal=Unit(normal*.35f+V3{.92f,0.f,-downwardBias}*.65f);
  float radial=sqrtf(max(level,1e-6f));float scale=1.f/radial;
  V3 surface={center.x+q.x*scale*radii.x,center.y+q.y*scale*radii.y,center.z+q.z*scale*radii.z};
  float penetration=Length(surface-p);ApplyNonimpulsiveCorrection(p,previous,normal*penetration,compliance,maxStep);
}
static void ResolvePelvisAndGlutes(V3& p,V3& previous,float renderedRadius,float relief){
  float pad=max(.15f,renderedRadius*.58f);
  // Pelvic saddle and lower abdomen. These overlap deliberately so there is no
  // numerical crack between analytic primitives for a lobe to tunnel through.
  ResolveAnteriorEllipsoid(p,previous,{1.2f,0.f,87.0f},{10.9f+pad,13.6f+pad,15.0f+pad},.13f-.025f*relief,.11f,.20f);
  ResolveAnteriorEllipsoid(p,previous,{.4f,0.f,75.2f},{9.2f+pad,10.2f+pad,10.0f+pad},.12f-.025f*relief,.10f,.34f);
  // Paired glute volumes close the rear pockets exposed by deep crouches. The
  // anterior-biased response guides contact under the body instead of snapping
  // through a cheek or leaving the lobe lodged behind it.
  ResolveAnteriorEllipsoid(p,previous,{-6.8f,-7.1f,88.0f},{8.2f+pad,9.1f+pad,15.2f+pad},.11f,.09f,.16f);
  ResolveAnteriorEllipsoid(p,previous,{-6.8f, 7.1f,88.0f},{8.2f+pad,9.1f+pad,15.2f+pad},.11f,.09f,.16f);
  ResolveAnatomyCapsule(p,previous,{3.0f,0.f,70.0f},{5.4f,0.f,86.0f},6.4f+pad,.11f-.02f*relief,.09f,Unit(V3{.88f,0.f,-.34f}));
}
static void KeepBallAboveMinimum(float& value,float& previous,float minimum,float relief=0.f){if(value<minimum){float strength=.12f*(1.f-.55f*relief),cap=.10f*(1.f-.40f*relief);float shift=min((minimum-value)*strength,cap);value+=shift;previous+=shift;}}
static void KeepBallBelowMaximum(float& value,float& previous,float maximum,float relief=0.f){if(value>maximum){float strength=.12f*(1.f-.55f*relief),cap=.10f*(1.f-.40f*relief);float shift=min((value-maximum)*strength,cap);value-=shift;previous-=shift;}}
static void CollisionCapsules(V3& leftA,V3& leftB,V3& rightA,V3& rightB){
  const V3 restLeftA={2.f,-7.8f,79.f},restLeftB={1.f,-8.2f,43.f},restRightA={2.f,7.8f,79.f},restRightB={1.f,8.2f,43.f};
  leftA=restLeftA;leftB=restLeftB;rightA=restRightA;rightB=restRightB;
  if(motionCollisionBonesReady){V3 la=InverseRigidPoint(motionPelvisMatrix,TransformPoint(motionLeftThighMatrix,restLeftA)),lb=InverseRigidPoint(motionPelvisMatrix,TransformPoint(motionLeftThighMatrix,restLeftB)),ra=InverseRigidPoint(motionPelvisMatrix,TransformPoint(motionRightThighMatrix,restRightA)),rb=InverseRigidPoint(motionPelvisMatrix,TransformPoint(motionRightThighMatrix,restRightB));if(Length(lb-la)>18.f&&Length(lb-la)<55.f&&Length(rb-ra)>18.f&&Length(rb-ra)<55.f&&Length(la-restLeftA)<45.f&&Length(ra-restRightA)<45.f){leftA=la;leftB=lb;rightA=ra;rightB=rb;}}
}
static float CrouchFactor(V3 leftA,V3 leftB,V3 rightA,V3 rightB){
  V3 ld=Unit(leftB-leftA),rd=Unit(rightB-rightA);float verticality=(fabsf(ld.z)+fabsf(rd.z))*.5f;
  return Smoother01(max(0.f,min(1.f,(.90f-verticality)/.55f)));
}
static void StepConstraintSolver(float dt,float gait,float side){
  if(!constraintSolverReady||constraintSolverState!=physicsState)InitializeConstraintSolver();
  V3 root=ShaftRoot(),restDir=RestShaftDirection();float segment=max(2.f,constraintRestLength/(shaftNodeCount-1));
  float shaftDamping=.995f-(100.f-physValues[2])*.00035f+physValues[1]*.00008f;shaftDamping=max(.955f,min(.997f,shaftDamping));
  float ballDamping=.995f-(100.f-physValues[6])*.00035f;ballDamping=max(.955f,min(.997f,ballDamping));
  // Weight is rotational inertia, not extra gravitational acceleration.  A
  // heavier chain accelerates more deliberately and carries momentum longer.
  float shaftMass=.75f+physValues[1]*.0125f;
  float response=(.65f+physValues[3]*.009f)/sqrtf(shaftMass),ballResponse=.65f+physValues[7]*.009f;
  float gravity=physicsState==0?6.f:physicsState==1?24.f:58.f;
  V3 inertial={0.f,side*125.f*response,gait*125.f*response-gravity};
  for(int i=1;i<shaftNodeCount;i++){V3 velocity=(shaftNodes[i]-shaftPrevious[i])*shaftDamping;shaftPrevious[i]=shaftNodes[i];shaftNodes[i]=shaftNodes[i]+velocity+inertial*(dt*dt);}
  for(int i=0;i<2;i++){
    // A tiny left/right response mismatch models independent suspended masses
    // and lets the lobes meet and knock instead of moving as one welded unit.
    float independentResponse=i?1.025f:.975f;
    V3 ballAccel={0.f,side*110.f*ballResponse*independentResponse,gait*110.f*ballResponse/independentResponse-(72.f+physValues[5]*.45f)};
    V3 velocity=(ballNodes[i]-ballPrevious[i])*ballDamping;ballPrevious[i]=ballNodes[i];ballNodes[i]=ballNodes[i]+velocity+ballAccel*(dt*dt);
  }
  // Experimental wide-range nonlinear compliance sweep.  -100 is nearly
  // unconstrained, 400 approaches rigid, and the lower half receives most of
  // the useful resolution.  One control drives hinge, bend and shape memory.
  float stiffnessSweep=max(0.f,min(1.f,(physValues[0]+100.f)/500.f));
  float stiffnessCurve=stiffnessSweep*stiffnessSweep;
  float bend=physicsState==0?.52f+.43f*stiffnessCurve:physicsState==1?.20f+.60f*stiffnessCurve:.010f+.85f*stiffnessCurve;
  // Collision envelopes are derived from the complete live morph.  The old
  // fixed-size radii underestimated the largest shapes by several times, so
  // their centers could be non-intersecting while the rendered surfaces were
  // deeply embedded in each other.
  float shaftRadius=ShaftCollisionRadius(),ballRadius=BallCollisionRadius(),thighRadius=7.2f;
  V3 leftA{},leftB{},rightA{},rightB{};CollisionCapsules(leftA,leftB,rightA,rightB);
  float crouch=CrouchFactor(leftA,leftB,rightA,rightB);
  float smallShape=Smoother01(max(0.f,min(1.f,(.95f-BallShapeScale())/.55f)));
  // Relief is intentionally concentrated where the bug occurs: a small live
  // morph in a folded-leg pose. Standing/default and large shapes retain the
  // established anti-ride-up protection.
  float relief=smallShape*(.35f+.65f*crouch);
  V3 ballAnchors[2]={BallAnchor(0),BallAnchor(1)};
  float ballTether=max(2.45f,Length(constraintBallRest[0]-ballAnchors[0])*(.92f+.05f*BallShapeScale()))+1.10f*relief;
  float restSeparation=max(2.f,fabsf(constraintBallRest[1].y-constraintBallRest[0].y));
  // Sphere-like lobe cores may touch and knock independently, but their centers
  // may not cross deeply enough for the rendered sacks to pass through.
  float pairMinimum=max(restSeparation*(.72f-.10f*relief),ballRadius*(1.24f-.16f*relief));
  float pairMaximum=max(restSeparation*1.19f,shaftRadius*.55f+ballRadius*.42f);pairMaximum=max(pairMaximum,pairMinimum*1.18f);
  // Shape-aware rescue bounds. These are inactive in the normal hanging pose,
  // but prevent a lobe center from settling behind the pelvis or above the
  // proximal shaft after a deep leg/shaft contact.
  float ballForwardFloor=11.75f+min(1.65f,shaftRadius*.18f+ballRadius*.08f)-1.15f*relief;
  float ballVerticalCeiling=ShaftRoot().z-max(2.80f,ballRadius*.45f)+.65f*relief;
  float ballThighContact=thighRadius-.35f*relief+ballRadius*(.70f-.18f*relief);
  for(int iteration=0;iteration<18;iteration++){
    shaftNodes[0]=root;
    for(int i=0;i<shaftNodeCount-1;i++)SolveDistance(shaftNodes[i],shaftNodes[i+1],segment,i==0?0.f:1.f,1.f);
    // The first segment is the anatomical attachment, not a free joint.
    V3 hingeTarget=root+restDir*segment;shaftNodes[1]=hingeTarget;
    for(int i=1;i<shaftNodeCount-1;i++){float profile=physicsState==1?(1.f-(float)i/(shaftNodeCount-1)):.7f;V3 midpoint=(shaftNodes[i-1]+shaftNodes[i+1])*.5f;shaftNodes[i]=shaftNodes[i]+(midpoint-shaftNodes[i])*(bend*profile*.16f);}
    ConstrainCurvatureGradient(root,restDir,segment,stiffnessSweep);
    if(physicsState==0){for(int i=2;i<shaftNodeCount;i++){V3 line=root+restDir*(segment*i);shaftNodes[i]=shaftNodes[i]+(line-shaftNodes[i])*.32f;}}
    else if(physicsState==2){for(int i=2;i<shaftNodeCount;i++){float proximal=1.f-(float)(i-1)/(shaftNodeCount-1);float memory=(.00010f+.012f*stiffnessCurve)*proximal*proximal;V3 line=root+restDir*(segment*i);shaftNodes[i]=shaftNodes[i]+(line-shaftNodes[i])*memory;}}
    for(int i=1;i<shaftNodeCount-1;i++)SolveDistance(shaftNodes[i],shaftNodes[i+1],segment,1.f,1.f);SolveDistance(shaftNodes[0],shaftNodes[1],segment,0.f,1.f);
    for(int i=2;i<shaftNodeCount;i++){
      float renderedRadius=shaftRadius*(.58f+.42f*min(1.f,(float)i/4.f));float contactRadius=1.35f+max(0.f,renderedRadius-2.50f)*.35f;
      ResolveAnatomyCapsule(shaftNodes[i],shaftPrevious[i],leftA,leftB,thighRadius+contactRadius,.18f,.15f,Unit(V3{.36f,.90f,-.08f}));
      ResolveAnatomyCapsule(shaftNodes[i],shaftPrevious[i],rightA,rightB,thighRadius+contactRadius,.18f,.15f,Unit(V3{.36f,-.90f,-.08f}));
    }
    for(int b=0;b<2;b++){
      SolveDistance(ballAnchors[b],ballNodes[b],ballTether,0.f,1.f);
      ResolvePelvisAndGlutes(ballNodes[b],ballPrevious[b],ballRadius,relief);
      ResolveBallCapsule(ballNodes[b],ballPrevious[b],leftA,leftB,ballThighContact,b,relief);
      ResolveBallCapsule(ballNodes[b],ballPrevious[b],rightA,rightB,ballThighContact,b,relief);
      // The earlier solver checked lobes against downstream shaft nodes but
      // left a blind pocket around its first few links. A compliant capsule
      // chain closes that pocket without welding the independently moving
      // lobes together or applying a one-frame snap.
      for(int link=2;link<6;link++){
        float profile=.64f+.36f*min(1.f,(float)link/4.f);
        ResolveBallCapsule(ballNodes[b],ballPrevious[b],shaftNodes[link],shaftNodes[link+1],shaftRadius*profile*.72f+ballRadius*(.74f-.14f*relief),b,relief);
      }
      float sign=b?1.f:-1.f,minimumSide=fabsf(ballAnchors[b].y)*(.38f-.13f*relief);
      float signedValue=sign*ballNodes[b].y,signedPrevious=sign*ballPrevious[b].y;
      KeepBallAboveMinimum(signedValue,signedPrevious,minimumSide,relief);ballNodes[b].y=sign*signedValue;ballPrevious[b].y=sign*signedPrevious;
      KeepBallAboveMinimum(ballNodes[b].x,ballPrevious[b].x,ballForwardFloor,relief);
      KeepBallBelowMaximum(ballNodes[b].z,ballPrevious[b].z,ballVerticalCeiling,relief);
    }
    ResolvePairMinimumCompliant(ballNodes[0],ballPrevious[0],ballNodes[1],ballPrevious[1],pairMinimum);
    ResolvePairMaximumCompliant(ballNodes[0],ballPrevious[0],ballNodes[1],ballPrevious[1],pairMaximum);
    // Coupled contacts let the scrotal lobes splay around a growing shaft.
    // Most correction is assigned to the freely hanging lobe, while the chain
    // still receives enough feedback to rest naturally rather than tunnelling.
    for(int i=2;i<shaftNodeCount;i++)for(int b=0;b<2;b++){
      float localRadius=shaftRadius*(.62f+.38f*min(1.f,(float)i/4.f));
      ResolvePairWeightedHistory(shaftNodes[i],shaftPrevious[i],ballNodes[b],ballPrevious[b],localRadius*.82f+ballRadius*(.88f-.12f*relief),.03f);
    }
    // A second body pass closes intersections introduced while resolving the
    // mutually coupled shaft/lobe system. Applying the same shift to Verlet
    // history keeps this positional correction from becoming false velocity.
    for(int b=0;b<2;b++)ResolvePelvisAndGlutes(ballNodes[b],ballPrevious[b],ballRadius,relief);
    for(int i=2;i<shaftNodeCount;i++){
      float profile=.58f+.42f*min(1.f,(float)i/4.f);
      ResolvePelvisAndGlutes(shaftNodes[i],shaftPrevious[i],shaftRadius*profile,.15f*relief);
    }
    ResolvePairMaximumCompliant(ballNodes[0],ballPrevious[0],ballNodes[1],ballPrevious[1],pairMaximum);
    for(int b=0;b<2;b++){KeepBallAboveMinimum(ballNodes[b].x,ballPrevious[b].x,ballForwardFloor,relief);KeepBallBelowMaximum(ballNodes[b].z,ballPrevious[b].z,ballVerticalCeiling,relief);}
    // Use a saturating support envelope instead of per-node steps.  The shaft
    // projects clear of the body, while no individual link receives a sudden
    // forward clamp that can manufacture another elbow.
    float supportReach=max(3.0f,min(12.0f,segment*1.35f+shaftRadius*.65f));
    for(int i=2;i<shaftNodeCount;i++){float minimum=root.x+supportReach*(1.f-expf(-.62f*i));if(shaftNodes[i].x<minimum)shaftNodes[i].x=minimum;}
    // Collision/support corrections are allowed to move the chain first; this
    // final pass then guarantees their result still obeys the bend gradient.
    ConstrainCurvatureGradient(root,restDir,segment,stiffnessSweep);
  }
  shaftNodes[0]=root;shaftNodes[1]=root+restDir*segment;shaftPrevious[1]=shaftNodes[1];
  if(motionQuietFrames>24){for(int i=1;i<shaftNodeCount;i++){V3 v=shaftNodes[i]-shaftPrevious[i];if(Length(v)<.018f)shaftPrevious[i]=shaftNodes[i];}for(int i=0;i<2;i++){V3 v=ballNodes[i]-ballPrevious[i];if(Length(v)<.018f)ballPrevious[i]=ballNodes[i];}}
}
static void UpdateConstraintSolver(float dt,float gait,float side){constraintAccumulator+=min(dt,.05f);const float fixed=1.f/180.f;int steps=0;while(constraintAccumulator>=fixed&&steps++<12){StepConstraintSolver(fixed,gait,side);constraintAccumulator-=fixed;}if(steps>=12)constraintAccumulator=0;}
static void UpdatePhysics(){
  DWORD now=GetTickCount();if(!physicsLastTick){physicsLastTick=now;return;}float dt=(now-physicsLastTick)*.001f;physicsLastTick=now;if(dt<=0||dt>.15f)dt=1.f/60.f;
  // Unified-skin Weapon X passes can be sparse. Hold the last validated bone
  // sample through ordinary render gaps while forces continue to decay.
  bool fresh=motionTracked&&motionLastCaptureTick&&now-motionLastCaptureTick<=600;
  float gait=0.f,side=0.f;if(fresh){gait=motionPitchForce;side=motionYawForce;motionPitchForce*=.985f;motionYawForce*=.985f;}else{motionTracked=false;motionPitchForce=motionYawForce=motionSpinSpeed=0.f;}
  float spinLift=fresh?min(1.f,motionSpinSpeed*motionSpinSpeed*1.15f):0.f;
  float downTarget=(90.f-sliderValues[4])*3.1415926535f/180.f;
  float shaftTarget=physicsState==2?downTarget-1.65f*spinLift:physicsState==1?.12f-.82f*spinLift:0.f;
  float shaftYawTarget=max(-1.20f,min(1.20f,-motionSpinSpeed*.75f));
  UpdateConstraintSolver(dt,gait,side);
  // Erect and semi use a rigid shaft pivot with one deliberately damped axis.
  // Full floppy retains the two-axis distributed flex model.
  if(physicsState==0){
    StepSpring(shaftSpring,90.f,35.f,18.f,110.f,0.f,0.f,gait*.40f,0.f,dt);
    shaftSpring.yaw=shaftSpring.yawVelocity=0.f;
    shaftSpring.pitch=max(-.10f,min(.10f,shaftSpring.pitch));
  }else if(physicsState==1){
    StepSpring(shaftSpring,72.f,42.f,30.f,125.f,.10f,0.f,gait*.65f,0.f,dt);
    shaftSpring.yaw=shaftSpring.yawVelocity=0.f;
    shaftSpring.pitch=max(-.16f,min(.26f,shaftSpring.pitch));
  }else StepFloppySpring(shaftSpring,max(0.f,physValues[0]),physValues[1],physValues[2],physValues[3],shaftTarget,shaftYawTarget,gait*2.0f,side*2.2f,dt);
  StepSpring(ballsSpring,physValues[4],physValues[5],physValues[6],physValues[7],.24f,0,gait*2.0f,side*2.2f,dt);
}
static void SiblingPath(char* path,const char* name){GetModuleFileNameA((HMODULE)&__ImageBase,path,MAX_PATH);char* slash=strrchr(path,'\\');if(slash)strcpy_s(slash+1,MAX_PATH-(slash+1-path),name);}
static bool ReadIniFloat(const char* path,const char* section,const char* key,float lo,float hi,float& value){char text[64]{};GetPrivateProfileStringA(section,key,"",text,sizeof(text),path);if(!text[0])return false;char* end=nullptr;float parsed=strtof(text,&end);if(end==text||!std::isfinite(parsed)||parsed<lo||parsed>hi)return false;value=parsed;return true;}
static float MapControl100(float ui,float lo,float neutral,float hi){ui=max(1.f,min(100.f,ui));return ui<=50.f?lo+(neutral-lo)*((ui-1.f)/49.f):neutral+(hi-neutral)*((ui-50.f)/50.f);}
static float UnmapControl100(float value,float lo,float neutral,float hi){value=max(lo,min(hi,value));return value<=neutral?1.f+49.f*(value-lo)/max(1e-6f,neutral-lo):50.f+50.f*(value-neutral)/max(1e-6f,hi-neutral);}
static void ApplyControlMapping(){for(int i=0;i<7;i++)sliderValues[i]=MapControl100(sliderUI[i],sliderSpecs[i].lo,neutralShape[i],sliderSpecs[i].hi);for(int i=0;i<8;i++)physValues[i]=MapControl100(physUI[i],physSpecs[i].lo,neutralPhysics[i],physSpecs[i].hi);}
static void LoadSettings(){
  if(settingsLoaded)return;settingsLoaded=true;char path[MAX_PATH];SiblingPath(path,"WolverineLive.ini");
  int version=GetPrivateProfileIntA("Meta","ControlScaleVersion",0,path);
  if(version>=2){
    for(int i=0;i<7;i++)ReadIniFloat(path,"Shape",sliderSpecs[i].name,1.f,100.f,sliderUI[i]);
    for(int i=0;i<8;i++)ReadIniFloat(path,"Physics",physSpecs[i].name,1.f,100.f,physUI[i]);
  }else{
    for(int i=0;i<7;i++){float legacy=neutralShape[i];ReadIniFloat(path,"Shape",sliderSpecs[i].name,sliderSpecs[i].lo,sliderSpecs[i].hi,legacy);sliderUI[i]=UnmapControl100(legacy,sliderSpecs[i].lo,neutralShape[i],sliderSpecs[i].hi);}
    for(int i=0;i<8;i++){float legacy=neutralPhysics[i];ReadIniFloat(path,"Physics",physSpecs[i].name,physSpecs[i].lo,physSpecs[i].hi,legacy);physUI[i]=UnmapControl100(legacy,physSpecs[i].lo,neutralPhysics[i],physSpecs[i].hi);}
    settingsPending=true;settingsChangedTick=GetTickCount();
  }
  ApplyControlMapping();float state=(float)physicsState;if(ReadIniFloat(path,"Physics","State",0,2,state))physicsState=(int)(state+.5f);
  Log("1-100 controls loaded version=%d state=%d neutral shape=(%.3f %.3f %.3f %.3f %.1f %.3f %.3f)",version,physicsState,sliderValues[0],sliderValues[1],sliderValues[2],sliderValues[3],sliderValues[4],sliderValues[5],sliderValues[6]);
}
static void SaveSettings(){char path[MAX_PATH],value[64];SiblingPath(path,"WolverineLive.ini");WritePrivateProfileStringA("Meta","ControlScaleVersion","2",path);for(int i=0;i<7;i++){sprintf_s(value,"%.0f",sliderUI[i]);WritePrivateProfileStringA("Shape",sliderSpecs[i].name,value,path);}for(int i=0;i<8;i++){sprintf_s(value,"%.0f",physUI[i]);WritePrivateProfileStringA("Physics",physSpecs[i].name,value,path);}sprintf_s(value,"%d",physicsState);WritePrivateProfileStringA("Physics","State",value,path);settingsPending=false;Log("1-100 control settings saved to WolverineLive.ini");}
static void QueueSettingsSave(){settingsPending=true;settingsChangedTick=GetTickCount();}
static void FlushSettingsIfDue(){if(settingsPending&&GetTickCount()-settingsChangedTick>=700)SaveSettings();}
static float Smooth01(float value){value=max(0.f,min(1.f,value));return value*value*(3.f-2.f*value);}
static float Smoother01(float value){value=max(0.f,min(1.f,value));return value*value*value*(value*(value*6.f-15.f)+10.f);}
static float PelvisCollarGrowth(){
  // Overall and width both contribute to the actual proximal diameter.  Keep
  // the stock/default body bit-identical, then recruit progressively more of
  // the surrounding pelvis only as that diameter grows.
  float diameter=(sliderValues[0]/sliderSpecs[0].def)*(sliderValues[2]/sliderSpecs[2].def);
  return max(0.f,min(1.5f,diameter-1.f));
}
static void ApplyPelvisCollar(float value[3],float distance,float growth,bool graftSide){
  // Grow the actual support radius with diameter. At normal size a compact
  // fillet is still present; wider shapes progressively recruit the additional
  // pelvis loops generated for this revision.
  float radius=(graftSide?5.0f:6.0f)+(graftSide?3.0f:5.0f)*growth;
  float influence=1.f-Smoother01(distance/radius);
  if(influence<=.0001f)return;
  // Raise the surrounding skin into an annular ramp. Both sides receive the
  // same seam lift; the welded fairing below solves their common tangent.
  float seamLift=.46f+1.08f*growth;
  float radialScale=1.f+.14f*growth*influence;
  value[0]+=seamLift*influence;
  value[1]*=radialScale;
  value[2]=84.3f+(value[2]-84.3f)*radialScale;
}
static float ShaftPhysicsTaper(float flex){
  // Flex is a geometry-derived axial coordinate shared by every vertex around
  // a cross-section.  This avoids the lumpy non-monotonic transition produced
  // by imported overlapping skin weights.
  if(physicsState<2)return 1.f;
  return .45f+.55f*Smooth01(flex/.55f);
}
static void ApplyShaftPoseAngle(float value[3],UINT i){
  float bw=min(1.f,phys_scrotum_weight[i]);
  float membership=max(phys_shaft_weight[i],phys_attachment_weight[i]);
  float active=membership*(1.f-bw);
  // Treat the complete low-scrotum shaft and root collar as one rigid unit.
  // The ten duplicated body-edge vertices have zero membership and stay welded.
  if(bw<.05f&&membership>.02f)active=1.f;
  if(active<=.001f)return;
  float delta=sliderValues[4]*3.1415926535f/180.f*active;
  float c=cosf(delta),s=sinf(delta),x=value[0]-12.65f,z=value[2]-84.3f;
  value[0]=12.65f+c*x+s*z;
  value[2]=84.3f-s*x+c*z;
}
static void FlareAttachment(float value[3],UINT i){
  // The duplicated body-side seam is the final ten graft vertices and has no
  // shaft/attachment membership, so it remains exactly welded to the torso.
  // Expand the donor collar most strongly beside that seam, then ease back to
  // the normal shaft over roughly its first third.  Scaling in the two axes
  // perpendicular to the posed shaft creates a true bell flare at every angle.
  float membership=max(phys_shaft_weight[i],phys_attachment_weight[i])*(1.f-min(1.f,phys_scrotum_weight[i]));
  if(membership<=.01f)return;
  float flex=phys_flex_coordinate[i];
  // The source topology jumps directly from attachment flex 0 to the first
  // shaft ring near .278.  Ending the flare at .34 collapsed almost its whole
  // radius change into that one ring, creating a second visible collar.  Carry
  // a C2-continuous bell across several rings so radius and curvature approach
  // the regular shaft without a ridge or constriction.
  float collar=1.f-Smoother01(flex/.70f);
  float influence=Smoother01(min(1.f,membership*2.5f))*collar;
  if(influence<=.001f)return;
  float angle=sliderValues[4]*3.1415926535f/180.f,sa=sinf(angle),ca=cosf(angle);
  float dx=value[0]-12.65f,dz=value[2]-84.3f;
  // Match the shaft-local frame used by the centerline solver: X is axial;
  // Y and local Z are the true cross-section.  The former basis scaled X at
  // zero angle and was the source of the longitudinal buttress-like ridges.
  float axial=ca*dx-sa*dz,radial=sa*dx+ca*dz;
  // The old fixed 16% bell created a secondary raised ring. The pelvis now
  // supplies the flare; retain only a shallow graft-side continuation.
  float scale=1.f+(.055f+.035f*PelvisCollarGrowth())*influence;
  radial*=scale;value[1]*=scale;
  value[0]=12.65f+ca*axial+sa*radial;
  value[2]=84.3f-sa*axial+ca*radial;
}
static void BuildShaftRestFrame(){
  const float sigma=.082f,invTwoSigma2=1.f/(2.f*sigma*sigma);
  for(int sample=0;sample<shaftRestSampleCount;sample++){
    float target=(float)sample/(shaftRestSampleCount-1),sum=0.f;V3 center{};
    for(UINT i=0;i<graftCount;i++){
      float shaft=max(phys_shaft_weight[i],phys_attachment_weight[i]);
      float ball=min(1.f,phys_scrotum_weight[i]);
      if(shaft<.20f||ball>.35f)continue;
      float q=phys_flex_coordinate[i]-target;
      float w=shaft*shaft*(1.f-ball)*(1.f-ball)*expf(-q*q*invTwoSigma2);
      center=center+graftDeformedPositions[i]*w;sum+=w;
    }
    if(sum>1e-5f)center=center/sum;
    else center=ShaftRoot()+RestShaftDirection()*(constraintRestLength*target);
    center.y=0.f;shaftRestCenters[sample]=center;
  }
  shaftRestCenters[0]=ShaftRoot();
  // The donor has uneven vertex density around several cross-sections. Smooth
  // its measured centerline before it becomes the radial frame, while keeping
  // the anatomical root exact and the terminal location stable.
  for(int pass=0;pass<6;pass++){
    V3 copy[shaftRestSampleCount];memcpy(copy,shaftRestCenters,sizeof(copy));
    for(int i=1;i<shaftRestSampleCount-1;i++)shaftRestCenters[i]=copy[i]+((copy[i-1]+copy[i+1])*.5f-copy[i])*.34f;
  }
  shaftRestCenters[0]=ShaftRoot();
  for(int i=1;i<shaftRestSampleCount;i++)shaftRestCenters[i].x=max(shaftRestCenters[i].x,shaftRestCenters[i-1].x+.10f);
  float arc=0.f;for(int i=1;i<shaftRestSampleCount;i++)arc+=Length(shaftRestCenters[i]-shaftRestCenters[i-1]);
  float measured=max(8.f,min(60.f,arc));constraintRestLength=shaftRestFrameReady?constraintRestLength*.82f+measured*.18f:measured;
  shaftRestFrameReady=true;
}
static void SampleRestShaftFrame(float t,V3& center,V3& tangent){
  t=max(0.f,min(1.f,t));float u=t*(shaftRestSampleCount-1);int i=min(shaftRestSampleCount-2,(int)u);float q=u-i;
  center=shaftRestCenters[i]*(1.f-q)+shaftRestCenters[i+1]*q;
  V3 before=shaftRestCenters[max(0,i-1)],after=shaftRestCenters[min(shaftRestSampleCount-1,i+2)];
  tangent=Unit(after-before);
}
static void PreserveShaftJunctionTube(){
  // Maintain a single monotone proximal tube. The previous fixed-radius
  // correction entered and left over short axial bands, exposing a second
  // circumferential ridge just beyond the scrotal junction.
  float rootRadius=2.52f*ShaftWidthScale();
  for(UINT i=0;i<graftCount;i++){
    float t=phys_flex_coordinate[i];if(t>.72f)continue;
    float shaft=max(phys_shaft_weight[i],phys_attachment_weight[i]),ball=min(1.f,phys_scrotum_weight[i]);
    float ownership=shaft/(shaft+ball+.0001f);if(shaft<.30f||ownership<.58f)continue;
    V3 center{},tangent{};SampleRestShaftFrame(t,center,tangent);
    V3 point=graftDeformedPositions[i],offset=point-center,radial=offset-tangent*Dot(offset,tangent);float radius=Length(radial);
    // A C2, axisymmetric support floor holds the full shaft circumference
    // through the root and eases only seven percent into the regular tube.
    float support=1.f-Smoother01(t/.72f),targetRadius=rootRadius*(.93f+.07f*support);
    if(radius<1e-4f||radius>=targetRadius)continue;
    float correction=min(targetRadius-radius,rootRadius*.14f)*Smoother01((ownership-.58f)/.42f);
    graftDeformedPositions[i]=point+radial*(correction/radius);
  }
}
static V3 ReadCollarMember(UINT globalIndex,unsigned char* controlled,UINT graftFirstVertex){
  if(globalIndex>=graftFirstVertex&&globalIndex<graftFirstVertex+graftCount)
    return graftDeformedPositions[globalIndex-graftFirstVertex];
  float value[3];memcpy(value,controlled+(globalIndex-pelvisControlFirstVertex)*graftStride,12);
  return {value[0],value[1],value[2]};
}
static void WriteCollarMember(UINT globalIndex,V3 value,unsigned char* controlled,UINT graftFirstVertex){
  if(globalIndex>=graftFirstVertex&&globalIndex<graftFirstVertex+graftCount){graftDeformedPositions[globalIndex-graftFirstVertex]=value;return;}
  float packed[3]={value.x,value.y,value.z};memcpy(controlled+(globalIndex-pelvisControlFirstVertex)*graftStride,packed,12);
}
static void CollarFairPass(const V3* source,V3* target,float strength){
  for(UINT group=0;group<collarFairGroupCount;group++){
    UINT begin=collarFairNeighborOffsets[group],end=collarFairNeighborOffsets[group+1];
    if(begin==end||collarFairWeights[group]<=.0001f){target[group]=source[group];continue;}
    // The broad support solve intentionally remains an umbrella average. The
    // regular-band correction below removes diagonal-valence corrugation
    // without letting highly irregular body triangles dominate this solve.
    V3 average{};for(UINT edge=begin;edge<end;edge++)average=average+source[collarFairNeighbors[edge]];
    average=average/(float)(end-begin);
    target[group]=source[group]+(average-source[group])*(strength*collarFairWeights[group]);
  }
}
static void FairRetopologyBands(){
  // The four new open horseshoes occupy the reserved contiguous tail of the
  // graft.  A regular 1-D Taubin filter suppresses the alternating-diagonal
  // sawtooth while preserving the low-frequency anatomical arc and endpoints.
  static const UINT first=2304u,ringCount=4u,ringVertices=21u;
  V3 source[ringVertices];
  for(int pair=0;pair<4;pair++)for(int phase=0;phase<2;phase++){
    float strength=phase==0?.45f:-.47f;
    for(UINT ring=0;ring<ringCount;ring++){
      UINT start=first+ring*ringVertices;
      for(UINT j=0;j<ringVertices;j++)source[j]=graftDeformedPositions[start+j];
      for(UINT j=1;j+1<ringVertices;j++)graftDeformedPositions[start+j]=source[j]+((source[j-1]+source[j+1])*.5f-source[j])*strength;
    }
  }
}
static void FairUnifiedCollar(unsigned char* controlled,UINT graftFirstVertex,float growth){
  for(UINT group=0;group<collarFairGroupCount;group++){
    V3 average{};UINT begin=collarFairMemberOffsets[group],end=collarFairMemberOffsets[group+1];
    for(UINT member=begin;member<end;member++)average=average+ReadCollarMember(collarFairMembers[member],controlled,graftFirstVertex);
    collarFairA[group]=average/(float)(end-begin);
  }
  // A constrained diffusion fairing actually converges the two formerly
  // independent surface derivatives. A Taubin reversal preserved the crease
  // at large widths; the broad anchored support field makes shrinkage local
  // and intentional here—it is the tight fillet into the shaft.
  float lambda=.24f+.012f*growth;
  // The new dorsal horseshoe bands need more convergence only as the collar
  // recruits additional pelvis at very large diameters.  Keep the normal-size
  // solve compact, then ramp to 280 passes at maximum growth; the support
  // weights pin the outer field so this remains local.
  int iterations=160+(int)(80.f*growth+.5f);
  for(int iteration=0;iteration<iterations;iteration++){
    CollarFairPass(collarFairA,collarFairB,lambda);
    memcpy(collarFairA,collarFairB,sizeof(collarFairA));
  }
  for(UINT group=0;group<collarFairGroupCount;group++)
    for(UINT member=collarFairMemberOffsets[group];member<collarFairMemberOffsets[group+1];member++)
      WriteCollarMember(collarFairMembers[member],collarFairA[group],controlled,graftFirstVertex);
}
static unsigned char PackSigned(float value){return (unsigned char)max(0.f,min(255.f,floorf((value+1.f)*127.5f+.5f)));}
static V3 ReadAnyCollarPosition(UINT globalIndex,UINT corner,unsigned char* controlled,UINT graftFirstVertex){
  if(globalIndex>=graftFirstVertex&&globalIndex<graftFirstVertex+graftCount)return graftDeformedPositions[globalIndex-graftFirstVertex];
  if(globalIndex>=pelvisControlFirstVertex&&globalIndex<graftFirstVertex){float value[3];memcpy(value,controlled+(globalIndex-pelvisControlFirstVertex)*graftStride,12);return {value[0],value[1],value[2]};}
  return {collarNormalTriangleBasePositions[corner*3],collarNormalTriangleBasePositions[corner*3+1],collarNormalTriangleBasePositions[corner*3+2]};
}
static void RebuildUnifiedCollarNormals(unsigned char* controlled,UINT graftFirstVertex){
  memset(collarNormalSums,0,sizeof(collarNormalSums));
  for(UINT triangle=0;triangle<collarNormalTriangleCount;triangle++){
    UINT corner=triangle*3;
    V3 a=ReadAnyCollarPosition(collarNormalTriangleIndices[corner],corner,controlled,graftFirstVertex);
    V3 b=ReadAnyCollarPosition(collarNormalTriangleIndices[corner+1],corner+1,controlled,graftFirstVertex);
    V3 c=ReadAnyCollarPosition(collarNormalTriangleIndices[corner+2],corner+2,controlled,graftFirstVertex);
    V3 face=Cross(c-a,b-a);
    for(UINT q=0;q<3;q++){
      UINT group=collarNormalTriangleGroups[corner+q];if(group==65535u)continue;
      V3 reference={collarFairBaseNormals[group*3],collarFairBaseNormals[group*3+1],collarFairBaseNormals[group*3+2]};
      // A highly posed triangle can cross the neutral tangent plane.  Keep
      // every contribution in the fitted surface's stable hemisphere so
      // opposing faces cannot cancel and flicker between specular extremes.
      collarNormalSums[group]=collarNormalSums[group]+(Dot(face,reference)<0.f?face*-1.f:face);
    }
  }
  for(UINT group=0;group<collarFairGroupCount;group++)collarNormalSums[group]=Unit(collarNormalSums[group]);
  // The two materials meet under the most unforgiving grazing highlights.
  // Diffuse their shared geometric normal over the same recruited collar graph
  // so the weld has continuous shading curvature, not merely equal positions.
  for(int pass=0;pass<10;pass++){
    for(UINT group=0;group<collarFairGroupCount;group++){
      UINT begin=collarFairNeighborOffsets[group],end=collarFairNeighborOffsets[group+1];
      if(begin==end){collarFairA[group]=collarNormalSums[group];continue;}
      V3 average{};float weightSum=0.f;for(UINT edge=begin;edge<end;edge++){float weight=collarFairNeighborWeights[edge];average=average+collarNormalSums[collarFairNeighbors[edge]]*weight;weightSum+=weight;}average=Unit(average/max(weightSum,1e-8f));
      float strength=.22f*Smoother01(collarFairWeights[group]);collarFairA[group]=Unit(collarNormalSums[group]*(1.f-strength)+average*strength);
    }
    memcpy(collarNormalSums,collarFairA,sizeof(collarNormalSums));
  }
  // Body-side positions now have matching body-side normals. Reproject the
  // existing tangent to the new normal instead of inventing a UV direction.
  for(UINT group=0;group<collarFairGroupCount;group++){
    V3 normal=collarNormalSums[group];
    for(UINT member=collarFairMemberOffsets[group];member<collarFairMemberOffsets[group+1];member++){
      UINT globalIndex=collarFairMembers[member];if(globalIndex>=graftFirstVertex)continue;
      unsigned char* vertex=controlled+(globalIndex-pelvisControlFirstVertex)*graftStride;
      unsigned char* packedTangent=vertex+12;unsigned char* packedNormal=vertex+16;
      V3 tangent={packedTangent[0]/127.5f-1.f,packedTangent[1]/127.5f-1.f,packedTangent[2]/127.5f-1.f};
      tangent=Unit(tangent-normal*Dot(normal,tangent));
      packedTangent[0]=PackSigned(tangent.x);packedTangent[1]=PackSigned(tangent.y);packedTangent[2]=PackSigned(tangent.z);
      packedNormal[0]=PackSigned(normal.x);packedNormal[1]=PackSigned(normal.y);packedNormal[2]=PackSigned(normal.z);
    }
  }
}
static void RebuildUnifiedCollarTangents(){
  for(UINT group=0;group<collarFairGroupCount;group++){
    V3 normal=collarNormalSums[group];
    V3 tangent=collarTangentSums[group]-normal*Dot(normal,collarTangentSums[group]);
    if(Length(tangent)<1e-5f){V3 axis=fabsf(normal.y)<.85f?V3{0,1,0}:V3{0,0,1};tangent=Cross(axis,normal);}
    collarTangentSums[group]=Unit(tangent);
  }
  // UV derivatives vary sharply where the old and new atlases meet. Average
  // their direction over the same welded graph, aligning signs before every
  // contribution so the tangent basis cannot alternate triangle by triangle.
  for(int pass=0;pass<8;pass++){
    for(UINT group=0;group<collarFairGroupCount;group++){
      V3 source=collarTangentSums[group],average{};UINT begin=collarFairNeighborOffsets[group],end=collarFairNeighborOffsets[group+1];
      if(begin==end){collarFairA[group]=source;continue;}
      float weightSum=0.f;for(UINT edge=begin;edge<end;edge++){V3 candidate=collarTangentSums[collarFairNeighbors[edge]];float weight=collarFairNeighborWeights[edge];average=average+(Dot(candidate,source)<0.f?candidate*-1.f:candidate)*weight;weightSum+=weight;}
      average=Unit(average/max(weightSum,1e-8f));V3 normal=collarNormalSums[group];
      V3 tangent=source*(1.f-.20f*Smoother01(collarFairWeights[group]))+average*(.20f*Smoother01(collarFairWeights[group]));
      collarFairA[group]=Unit(tangent-normal*Dot(normal,tangent));
    }
    memcpy(collarTangentSums,collarFairA,sizeof(collarTangentSums));
  }
}
static void ApplyFloppyCurve(float value[3],UINT i,float pitch,float yaw){
  float bw=min(1.f,phys_scrotum_weight[i]),membership=max(phys_shaft_weight[i],phys_attachment_weight[i]);
  float active=membership*(1.f-bw);if(bw<.05f&&membership>.02f)active=1.f;
  float t=phys_flex_coordinate[i],bend=sqrtf(pitch*pitch+yaw*yaw);
  if(active<=.001f||t<=.0001f||bend<=.0001f)return;
  // Express the straight posed mesh in a shaft-local frame.  Each axial slice
  // is then placed on a constant-curvature centerline and its complete radial
  // cross-section is rotated with the local tangent.  This preserves length
  // and diameter instead of stretching vertices around one common pivot.
  float a=sliderValues[4]*3.1415926535f/180.f,ca=cosf(a),sa=sinf(a);
  float dx=value[0]-12.65f,dz=value[2]-84.3f;
  float axial=ca*dx-sa*dz,radial=sa*dx+ca*dz,lateral=value[1];
  float q=bend*t,sq=sinf(q),cq=cosf(q),inv=1.f/bend;
  float ny=yaw*inv,nz=-pitch*inv,ky=pitch*inv,kz=yaw*inv;
  float centerX=axial*sq/q,transverse=axial*(1.f-cq)/q;
  float dot=ky*lateral+kz*radial,crossX=ky*radial-kz*lateral;
  float rx=crossX*sq,ry=lateral*cq+ky*dot*(1.f-cq),rz=radial*cq+kz*dot*(1.f-cq);
  float lx=centerX+rx,ly=ny*transverse+ry,lz=nz*transverse+rz;
  float tx=12.65f+ca*lx+sa*lz,ty=ly,tz=84.3f-sa*lx+ca*lz;
  value[0]+=(tx-value[0])*active;value[1]+=(ty-value[1])*active;value[2]+=(tz-value[2])*active;
}
static V3 RotateFromTo(V3 value,V3 from,V3 to){from=Unit(from);to=Unit(to);V3 axis=Cross(from,to);float s=Length(axis),c=max(-1.f,min(1.f,Dot(from,to)));if(s<1e-5f)return c>0?value:value*-1.f;return value*c+Cross(axis,value)+axis*(Dot(axis,value)*(1.f-c)/(s*s));}
static void SampleShaftChain(float t,V3& center,V3& tangent){
  // Cubic Hermite interpolation gives a C1-continuous centerline.  The old
  // piecewise-linear sampler changed tangent abruptly at solver nodes, which
  // appeared as a hard crease and could split the sparse attachment rings.
  t=max(0.f,min(1.f,t));float u=t*(shaftNodeCount-1);int s=min(shaftNodeCount-2,(int)u);float q=u-s,q2=q*q,q3=q2*q;
  V3 p0=shaftNodes[s],p1=shaftNodes[s+1];
  V3 m0=s==0?RestShaftDirection()*Length(p1-p0):(shaftNodes[s+1]-shaftNodes[s-1])*.5f;
  V3 m1=s+1==shaftNodeCount-1?(p1-p0):(shaftNodes[s+2]-shaftNodes[s])*.5f;
  center=p0*(2*q3-3*q2+1)+m0*(q3-2*q2+q)+p1*(-2*q3+3*q2)+m1*(q3-q2);
  tangent=Unit(p0*(6*q2-6*q)+m0*(3*q2-4*q+1)+p1*(-6*q2+6*q)+m1*(3*q2-2*q));
}
static void ApplyConstraintCurve(float value[3],UINT i){
  if(!constraintSolverReady||!shaftRestFrameReady)return;float bw=min(1.f,phys_scrotum_weight[i]),membership=max(phys_shaft_weight[i],phys_attachment_weight[i]);
  float ownership=membership/(membership+bw+.0001f);float active=membership*Smoother01(max(0.f,min(1.f,(ownership-.34f)/.50f)));if(bw<.05f&&membership>.02f)active=1.f;if(active<=.001f)return;
  // Zero motion at the welded seam.  The first shaft ring is already near
  // t=.278, so the former .14 transition made solver influence jump straight
  // from zero to one.  A longer C2 ramp distributes deformation through the
  // proximal rings and cannot manufacture a hinge at the ball junction.
  // Spread constraint influence well beyond the sparse first shaft rings so
  // the root, semi, and floppy states share one continuous bend gradient.
  float t=max(0.f,min(1.f,phys_flex_coordinate[i]));active*=Smoother01(t/.78f);
  V3 point={value[0],value[1],value[2]},center{},tangent{},restCenter{},restTangent{};SampleShaftChain(t,center,tangent);SampleRestShaftFrame(t,restCenter,restTangent);
  V3 fromCenter=point-restCenter;V3 radial=fromCenter-restTangent*Dot(fromCenter,restTangent);V3 target=center+RotateFromTo(radial,restTangent,tangent);
  value[0]+=(target.x-value[0])*active;value[1]+=(target.y-value[1])*active;value[2]+=(target.z-value[2])*active;
}
static void ApplyBallRigidEnvelope(float value[3],UINT i){
  float bw=min(1.f,phys_scrotum_weight[i]);if(bw<=.001f||!constraintSolverReady)return;
  float shaft=max(phys_shaft_weight[i],phys_attachment_weight[i]);
  // The shaft owns shared junction vertices. The scrotal surface only becomes
  // rigid once its lobe membership dominates, leaving a compliant skin neck
  // without allowing the egg-shaped mass itself to collapse.
  float ownership=bw/(bw+shaft+.0001f);float rigidity=Smoother01(max(0.f,min(1.f,(bw-.08f)/.48f)))*Smoother01(max(0.f,min(1.f,(ownership-.40f)/.42f)));
  if(rigidity<=.001f)return;
  V3 point={value[0],value[1],value[2]};int side=point.y<0.f?0:1;V3 anchor=BallAnchor(side),rest=constraintBallRest[side];
  V3 restAxis=Unit(rest-anchor),posedAxis=Unit(ballNodes[side]-anchor),offset=point-rest;
  V3 hardTarget=ballNodes[side]+RotateFromTo(offset,restAxis,posedAxis);
  // The lobe cores retain one rigid transform, but a narrow band of sack skin
  // around the anatomical midline interpolates both frames continuously. This
  // membrane prevents adjacent rear/medial triangles from choosing opposite
  // transforms and opening into the apparent holes seen during wide splay.
  float membraneRadius=max(.55f,BallCollisionRadius()*.30f),membrane=1.f-Smoother01(fabsf(point.y)/membraneRadius);
  float sideBlend=Smooth01(max(0.f,min(1.f,.5f+point.y/(2.f*membraneRadius))));V3 softTarget{};
  for(int b=0;b<2;b++){V3 a=BallAnchor(b),r=constraintBallRest[b],ra=Unit(r-a),pa=Unit(ballNodes[b]-a),o=point-r;V3 candidate=ballNodes[b]+RotateFromTo(o,ra,pa);softTarget=softTarget+candidate*(b?sideBlend:1.f-sideBlend);}
  V3 target=hardTarget*(1.f-membrane)+softTarget*membrane;
  // Preserve the rigid egg-shaped core, but transition through a broad neck
  // measured along the real anchor-to-lobe axis. This removes the sharp
  // lateral-swing vertex without coupling the independently moving lobes.
  float span=max(.5f,Length(rest-anchor));
  float axial=Dot(point-anchor,restAxis);
  float neck=Smoother01((axial-span*.05f)/(span*.45f));
  float neckRigidity=rigidity*(.25f+.75f*neck);
  // Preserve the rigid lobe core exactly; only the mixed-weight skin bridge
  // receives the neck taper. That keeps the egg volume intact under motion.
  if(bw>=.82f)neckRigidity=rigidity;
  value[0]+=(target.x-value[0])*neckRigidity;value[1]+=(target.y-value[1])*neckRigidity;value[2]+=(target.z-value[2])*neckRigidity;
}
static void ApplyShape(){
  if(!graftBuffer)return;bool report=shapeDirty;void* raw=nullptr;
  const UINT graftFirstVertex=graftOffset/graftStride;
  const UINT controlledVertexCount=graftFirstVertex+graftCount-pelvisControlFirstVertex;
  HRESULT hr=graftBuffer->Lock(pelvisControlFirstVertex*graftStride,controlledVertexCount*graftStride,&raw,0);
  if(FAILED(hr)){Log("live shape lock failed %08X",hr);return;}
  auto* controlled=(unsigned char*)raw;
  auto* p=controlled+(graftFirstVertex-pelvisControlFirstVertex)*graftStride;
  int oi0=sliderValues[0]<sliderSpecs[0].def?0:1,oi1=oi0+1,wi0=sliderValues[2]<sliderSpecs[2].def?0:1,wi1=wi0+1;
  float omin=oi0==0?sliderSpecs[0].lo:sliderSpecs[0].def,omax=oi1==1?sliderSpecs[0].def:sliderSpecs[0].hi,wmin=wi0==0?sliderSpecs[2].lo:sliderSpecs[2].def,wmax=wi1==1?sliderSpecs[2].def:sliderSpecs[2].hi;
  float ot=(sliderValues[0]-omin)/(omax-omin),wt=(sliderValues[2]-wmin)/(wmax-wmin);
  float collarGrowth=PelvisCollarGrowth();
  for(UINT i=0;i<pelvisControlCount;i++){
    float value[3]={pelvisControlBasePositions[i*3],pelvisControlBasePositions[i*3+1],pelvisControlBasePositions[i*3+2]};
    ApplyPelvisCollar(value,pelvisControlDistances[i],collarGrowth,false);
    unsigned char* bodyVertex=controlled+(pelvisControlIndices[i]-pelvisControlFirstVertex)*graftStride;
    memcpy(bodyVertex,value,12);
  }
  V3 tipSum{},ballSum[2]{};int tipCount=0,ballCount[2]{};
  for(UINT i=0;i<graftCount;i++){
    float value[3];for(UINT axis=0;axis<3;axis++){UINT q=i*3+axis;float a=overallWidthTargets[oi0][wi0][q]*(1-wt)+overallWidthTargets[oi0][wi1][q]*wt,b=overallWidthTargets[oi1][wi0][q]*(1-wt)+overallWidthTargets[oi1][wi1][q]*wt;value[axis]=a*(1-ot)+b*ot;for(int s=1;s<7;s++){if(s==2||s==4)continue;const auto& spec=sliderSpecs[s];float v=sliderValues[s];if(v<spec.def)value[axis]+=(spec.low[q]-morph_base[q])*(spec.def-v)/(spec.def-spec.lo);else if(v>spec.def)value[axis]+=(spec.high[q]-morph_base[q])*(v-spec.def)/(spec.hi-spec.def);}}
    ApplyPelvisCollar(value,graftCollarDistances[i],collarGrowth,true);
    ApplyShaftPoseAngle(value,i);
    FlareAttachment(value,i);
    graftDeformedPositions[i]=V3{value[0],value[1],value[2]};
  }
  FairUnifiedCollar(controlled,graftFirstVertex,collarGrowth);
  BuildShaftRestFrame();
  PreserveShaftJunctionTube();
  FairRetopologyBands();
  BuildShaftRestFrame();
  for(UINT i=0;i<graftCount;i++){
    V3 value=graftDeformedPositions[i];
    if(phys_flex_coordinate[i]>.98f&&phys_shaft_weight[i]>.5f){tipSum=tipSum+value;tipCount++;}
    float rawBallWeight=phys_scrotum_weight[i];if(rawBallWeight>.55f){int side=value.y<0?0:1;ballSum[side]=ballSum[side]+value;ballCount[side]++;}
  }
  for(int side=0;side<2;side++)if(ballCount[side]){
    V3 newRest=ballSum[side]/(float)ballCount[side],shift=newRest-constraintBallRest[side];
    if(constraintSolverReady){ballNodes[side]=ballNodes[side]+shift;ballPrevious[side]=ballPrevious[side]+shift;}
    constraintBallRest[side]=newRest;
  }
  if(!constraintSolverReady)InitializeConstraintSolver();
  for(UINT i=0;i<graftCount;i++){
    float value[3]={graftDeformedPositions[i].x,graftDeformedPositions[i].y,graftDeformedPositions[i].z};
    float bw=phys_scrotum_weight[i],membership=max(phys_shaft_weight[i],phys_attachment_weight[i]);float motionWeight=membership;if(bw<.05f&&membership>.02f)motionWeight=1.f;
    if(motionWeight>.001f)ApplyConstraintCurve(value,i);
    if(bw>.001f)ApplyBallRigidEnvelope(value,i);
    graftDeformedPositions[i]=V3{value[0],value[1],value[2]};
  }
  // The proxy changes vertex positions after UE3 has prepared the skeletal
  // buffer.  Rebuild the normals from that final deformed surface so lighting
  // follows every physics bend.  Wolverine's meshes use the opposite of the
  // raw index-buffer cross-product; the topology header also welds UV-split
  // duplicates into shared smoothing groups.
  memset(graftDynamicNormalSums,0,sizeof(graftDynamicNormalSums));
  memset(graftDynamicTangentSums,0,sizeof(graftDynamicTangentSums));
  memset(collarTangentSums,0,sizeof(collarTangentSums));
  for(UINT k=0;k<graftTriangleIndexCount;k+=3){
    UINT ia=graftTriangleIndices[k],ib=graftTriangleIndices[k+1],ic=graftTriangleIndices[k+2];
    V3 edge1=graftDeformedPositions[ib]-graftDeformedPositions[ia],edge2=graftDeformedPositions[ic]-graftDeformedPositions[ia];
    V3 face=Cross(edge2,edge1);
    UINT ga=graftNormalGroup[ia],gb=graftNormalGroup[ib],gc=graftNormalGroup[ic];
    graftDynamicNormalSums[ga]=graftDynamicNormalSums[ga]+face;
    graftDynamicNormalSums[gb]=graftDynamicNormalSums[gb]+face;
    graftDynamicNormalSums[gc]=graftDynamicNormalSums[gc]+face;
    float du1=graftUVs[ib*2]-graftUVs[ia*2],dv1=graftUVs[ib*2+1]-graftUVs[ia*2+1];
    float du2=graftUVs[ic*2]-graftUVs[ia*2],dv2=graftUVs[ic*2+1]-graftUVs[ia*2+1],det=du1*dv2-dv1*du2;
    if(fabsf(det)>1e-8f){
      V3 tangent=(edge1*dv2-edge2*dv1)/det;graftDynamicTangentSums[ia]=graftDynamicTangentSums[ia]+tangent;graftDynamicTangentSums[ib]=graftDynamicTangentSums[ib]+tangent;graftDynamicTangentSums[ic]=graftDynamicTangentSums[ic]+tangent;
      V3 direction=Unit(tangent);UINT groups[3]={graftCollarGroups[ia],graftCollarGroups[ib],graftCollarGroups[ic]};
      for(int q=0;q<3;q++)if(groups[q]!=65535u)collarTangentSums[groups[q]]=collarTangentSums[groups[q]]+direction;
    }
  }
  RebuildUnifiedCollarNormals(controlled,graftFirstVertex);
  RebuildUnifiedCollarTangents();
  for(UINT i=0;i<graftCount;i++){
    memcpy(p+i*graftStride,&graftDeformedPositions[i],12);
    UINT collarGroup=graftCollarGroups[i];
    V3 dynamic=collarGroup!=65535u?collarNormalSums[collarGroup]:Unit(graftDynamicNormalSums[graftNormalGroup[i]]);
    V3 baseNormal={graftBaseNormals[i*3],graftBaseNormals[i*3+1],graftBaseNormals[i*3+2]};
    float lock=collarGroup!=65535u?0.f:graftSeamNormalLock[i];V3 normal=Unit(dynamic*(1.f-lock)+baseNormal*lock);
    V3 tangent=collarGroup!=65535u?collarTangentSums[collarGroup]:graftDynamicTangentSums[i]-normal*Dot(normal,graftDynamicTangentSums[i]);tangent=Unit(tangent);
    unsigned char* packedTangent=p+i*graftStride+12;unsigned char* packedNormal=p+i*graftStride+16;
    packedTangent[0]=PackSigned(tangent.x);packedTangent[1]=PackSigned(tangent.y);packedTangent[2]=PackSigned(tangent.z);
    packedNormal[0]=PackSigned(normal.x);packedNormal[1]=PackSigned(normal.y);packedNormal[2]=PackSigned(normal.z);
  }
  if(tipCount){V3 tip=tipSum/(float)tipCount;float newLength=max(8.f,min(60.f,Length(tip-ShaftRoot())));constraintRestLength=constraintRestLength*.92f+newLength*.08f;}
  float written[3];memcpy(written,p,12);graftBuffer->Unlock();shapeDirty=false;if(report)Log("live controls, recruited pelvis collar, and dynamic tangent basis applied state=%d collar=%.3f shape=%.2f %.2f %.2f %.2f %.1f %.2f %.2f shaft=%.0f %.0f %.0f %.0f balls=%.0f %.0f %.0f %.0f first=(%.4f %.4f %.4f)",physicsState,collarGrowth,sliderValues[0],sliderValues[1],sliderValues[2],sliderValues[3],sliderValues[4],sliderValues[5],sliderValues[6],physValues[0],physValues[1],physValues[2],physValues[3],physValues[4],physValues[5],physValues[6],physValues[7],written[0],written[1],written[2]);
}
static bool KeyEdge(int vk){static bool old[256]{};bool now=(GetAsyncKeyState(vk)&0x8000)!=0;bool edge=now&&!old[vk];old[vk]=now;return edge;}
struct OV {float x,y,z,rhw;D3DCOLOR color;};
static void Rect(IDirect3DDevice9* d,float x,float y,float w,float h,D3DCOLOR c){OV v[]={{x-.5f,y-.5f,0,1,c},{x+w-.5f,y-.5f,0,1,c},{x-.5f,y+h-.5f,0,1,c},{x+w-.5f,y+h-.5f,0,1,c}};d->SetTexture(0,nullptr);d->SetVertexShader(nullptr);d->SetPixelShader(nullptr);d->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);d->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);d->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);d->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);d->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);d->SetRenderState(D3DRS_ZENABLE,FALSE);d->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);d->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);d->SetRenderState(D3DRS_COLORWRITEENABLE,0xF);HRESULT hr=d->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(OV));if(InterlockedCompareExchange(&overlayLogged,1,0)==0)Log("HUD rectangle DrawPrimitiveUP=%08X",hr);}
static const char* Glyph(char c){
  switch(c){
  case 'A':return "01110100011000111111100011000100000";case 'B':return "11110100011000111110100011000111110";
  case 'C':return "01111100001000010000100001000001111";case 'D':return "11110100011000110001100011000111110";
  case 'E':return "11111100001000011110100001000011111";case 'F':return "11111100001000011110100001000010000";
  case 'G':return "01111100001000010111100011000101111";case 'H':return "10001100011000111111100011000110001";
  case 'I':return "11111001000010000100001000010011111";case 'J':return "00111000100001000010000101001001100";
  case 'K':return "10001100101010011000101001001010001";case 'L':return "10000100001000010000100001000011111";
  case 'M':return "10001110111010110101100011000110001";case 'N':return "10001110011010110011100011000110001";
  case 'O':return "01110100011000110001100011000101110";case 'P':return "11110100011000111110100001000010000";
  case 'Q':return "01110100011000110001101011001001101";case 'R':return "11110100011000111110101001001010001";
  case 'S':return "01111100001000001110000010000111110";case 'T':return "11111001000010000100001000010000100";
  case 'U':return "10001100011000110001100011000101110";case 'V':return "10001100011000110001100010101000100";
  case 'W':return "10001100011000110101101011101110001";case 'X':return "10001100010101000100010101000110001";
  case 'Y':return "10001100010101000100001000010000100";case 'Z':return "11111000010001000100010001000011111";
  case '0':return "01110100011001110101110011000101110";case '1':return "00100011000010000100001000010001110";
  case '2':return "01110100010000100010001000100011111";case '3':return "11110000010000101110000010000111110";
  case '4':return "00010001100101010010111110001000010";case '5':return "11111100001000011110000010000111110";
  case '6':return "01110100001000011110100011000101110";case '7':return "11111000010001000100010000100001000";
  case '8':return "01110100011000101110100011000101110";case '9':return "01110100011000101111000010000101110";
  case '.':return "00000000000000000000000000011000110";case '-':return "00000000000000011111000000000000000";
  case '/':return "00001000100001000100010001000010000";case '[':return "01110010000100001000010000100001110";
  case ']':return "01110000100001000010000100001001110";case '(':return "00100010000100001000010000010000100";
  case ')':return "00100000100001000010000100100000100";case '=':return "00000111110000011111000000000000000";
  case ':':return "00000001100011000000001100011000000";default:return "00000000000000000000000000000000000";}
}
static void Text(IDirect3DDevice9* d,const char* text,RECT r,D3DCOLOR c,DWORD flags=DT_LEFT|DT_VCENTER|DT_SINGLELINE){
  const float s=1.5f,advance=9.f;float width=(float)strlen(text)*advance;float x=(flags&DT_RIGHT)?r.right-width:(float)r.left;float y=(flags&DT_VCENTER)?r.top+((r.bottom-r.top)-10.5f)*.5f:(float)r.top;
  std::vector<OV> v;v.reserve(strlen(text)*35*6);
  for(const char* q=text;*q;q++,x+=advance){char ch=*q;if(ch>='a'&&ch<='z')ch-=32;const char* g=Glyph(ch);for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(g[row*5+col]=='1'){
    float l=x+col*s-.5f,t=y+row*s-.5f,rr=l+s,b=t+s;OV px[6]={{l,t,0,1,c},{rr,t,0,1,c},{l,b,0,1,c},{l,b,0,1,c},{rr,t,0,1,c},{rr,b,0,1,c}};v.insert(v.end(),px,px+6);
  }}
  if(v.empty())return;d->SetTexture(0,nullptr);d->SetVertexShader(nullptr);d->SetPixelShader(nullptr);d->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);d->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);d->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);d->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);d->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);d->SetRenderState(D3DRS_ZENABLE,FALSE);d->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);d->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);d->SetRenderState(D3DRS_COLORWRITEENABLE,0xF);HRESULT hr=d->DrawPrimitiveUP(D3DPT_TRIANGLELIST,(UINT)v.size()/3,v.data(),sizeof(OV));if(InterlockedCompareExchange(&overlayLogged,1,0)==0)Log("HUD bitmap text DrawPrimitiveUP=%08X vertices=%u",hr,(unsigned)v.size());
}
static void OverlayFrame(IDirect3DDevice9* d){
  if(inOverlay)return;inOverlay=true;
  InterlockedIncrement(&renderFrameSerial);
  if(KeyEdge(VK_F6))menuOpen=!menuOpen;
  if(menuOpen){
    const int count=16;
    if(KeyEdge(VK_UP))selectedSlider=(selectedSlider+count-1)%count;
    if(KeyEdge(VK_DOWN))selectedSlider=(selectedSlider+1)%count;
    if(KeyEdge(VK_F8)){for(int i=0;i<7;i++)sliderUI[i]=50.f;for(int i=0;i<8;i++)physUI[i]=50.f;ApplyControlMapping();physicsState=2;shaftSpring=Spring2{};ballsSpring=Spring2{};constraintSolverReady=false;shapeDirty=true;}
    int dir=KeyEdge(VK_LEFT)?-1:KeyEdge(VK_RIGHT)?1:0;
    if(dir){float mult=(GetAsyncKeyState(VK_SHIFT)&0x8000)?5.f:1.f;if(selectedSlider<7)sliderUI[selectedSlider]=max(1.f,min(100.f,sliderUI[selectedSlider]+dir*mult));else if(selectedSlider==7){physicsState=(physicsState+dir+3)%3;shaftSpring=Spring2{};}else physUI[selectedSlider-8]=max(1.f,min(100.f,physUI[selectedSlider-8]+dir*mult));ApplyControlMapping();shapeDirty=true;}
  }
  if(shapeDirty&&settingsLoaded)QueueSettingsSave();UpdatePhysics();ApplyShape();FlushSettingsIfDue();
  IDirect3DStateBlock9* state=nullptr;d->CreateStateBlock(D3DSBT_ALL,&state);float x=14,y=14,w=370;const int rows=16;float statusY=y+39+rows*31.f,h=menuOpen?(statusY-y+80.f):32.f;Rect(d,x,y,w,h,D3DCOLOR_ARGB(255,18,20,24));Rect(d,x,y,w,32,D3DCOLOR_ARGB(255,69,35,92));
  RECT title{(LONG)x+10,(LONG)y,(LONG)(x+w-8),(LONG)y+32};Text(d,menuOpen?"BIG DICK LOGAN MOD (F6 TO HIDE)":"BIG DICK LOGAN MOD (F6 TO SHOW)",title,D3DCOLOR_ARGB(255,255,255,255));
  if(menuOpen){
    for(int i=0;i<rows;i++){float row=y+39+i*31;bool selected=i==selectedSlider;D3DCOLOR tc=selected?D3DCOLOR_ARGB(255,255,221,86):D3DCOLOR_ARGB(255,230,230,230);const char* name;float value,lo,hi;char val[32];
      if(i<7){const auto& s=sliderSpecs[i];name=s.name;value=sliderUI[i];lo=1;hi=100;sprintf_s(val,"%.0f",value);}else if(i==7){name="STATE";value=(float)physicsState;lo=0;hi=2;sprintf_s(val,"%s",physicsState==0?"ERECT":physicsState==1?"SEMI":"FULL FLOPPY");}else{const auto& s=physSpecs[i-8];name=s.name;value=physUI[i-8];lo=1;hi=100;sprintf_s(val,"%.0f",value);}
      RECT label{(LONG)x+10,(LONG)row,(LONG)x+128,(LONG)row+24};Text(d,name,label,tc);if(i==7){RECT stateValue{(LONG)x+135,(LONG)row,(LONG)x+362,(LONG)row+24};Text(d,val,stateValue,tc,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);continue;}float bx=x+135,bw=150;Rect(d,bx,row+9,bw,5,D3DCOLOR_ARGB(255,70,70,76));float t=(value-lo)/(hi-lo);Rect(d,bx,row+6,bw*t,11,D3DCOLOR_ARGB(255,155,80,202));Rect(d,bx+bw*t-3,row+3,7,17,tc);RECT vr{(LONG)x+292,(LONG)row,(LONG)x+362,(LONG)row+24};Text(d,val,vr,tc,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
    }
    DWORD transformAge=motionLastCaptureTick?GetTickCount()-motionLastCaptureTick:0xFFFFFFFFu;bool transformLive=motionCollisionBonesReady&&transformAge<=1200u;
    D3DCOLOR statusColor=transformLive?D3DCOLOR_ARGB(255,92,230,130):graftBuffer?D3DCOLOR_ARGB(255,80,190,235):D3DCOLOR_ARGB(255,255,190,70);const char* statusText=transformLive?"STATUS: CHARACTER TRANSFORM LIVE":graftBuffer?"STATUS: TRANSFORM UNAVAILABLE":"STATUS: WAITING FOR WOLVERINE";RECT status{(LONG)x+10,(LONG)statusY,(LONG)(x+w-10),(LONG)statusY+20};Text(d,statusText,status,statusColor);RECT help1{(LONG)x+10,(LONG)statusY+22,(LONG)(x+w-10),(LONG)statusY+41};Text(d,"UP/DOWN SELECT  LEFT/RIGHT ADJUST",help1,D3DCOLOR_ARGB(255,185,185,190));RECT help2{(LONG)x+10,(LONG)statusY+42,(LONG)(x+w-10),(LONG)statusY+63};Text(d,"SHIFT = COARSE  F8 = RESET ALL",help2,D3DCOLOR_ARGB(255,185,185,190));
  }
  if(state){state->Apply();state->Release();}inOverlay=false;
}
static HRESULT STDMETHODCALLTYPE HookEndScene(IDirect3DDevice9* d){if(InterlockedCompareExchange(&endSceneLogged,1,0)==0)Log("EndScene hook active");IDirect3DSurface9* rt=nullptr;IDirect3DSurface9* bb=nullptr;d->GetRenderTarget(0,&rt);d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&bb);LONG n=InterlockedIncrement(&endSceneCount);if(n<=12){D3DSURFACE_DESC rd{},bd{};if(rt)rt->GetDesc(&rd);if(bb)bb->GetDesc(&bd);Log("EndScene %ld rt=%p %ux%u fmt=%u bb=%p %ux%u fmt=%u",n,rt,rd.Width,rd.Height,rd.Format,bb,bd.Width,bd.Height,bd.Format);}bool final=rt&&bb&&rt==bb;if(final){OverlayFrame(d);frameRendered=true;}if(rt)rt->Release();if(bb)bb->Release();return origEndScene(d);}
static HRESULT STDMETHODCALLTYPE HookPresent(IDirect3DDevice9* d,const RECT* src,const RECT* dst,HWND wnd,const RGNDATA* dirty){
  if(InterlockedCompareExchange(&presentLogged,1,0)==0)Log("Present hook active");
  if(!frameRendered&&SUCCEEDED(d->BeginScene())){OverlayFrame(d);origEndScene(d);}frameRendered=false;
  return origPresent(d,src,dst,wnd,dirty);
}
static HRESULT STDMETHODCALLTYPE HookSwapPresent(IDirect3DSwapChain9* sc,const RECT* src,const RECT* dst,HWND wnd,const RGNDATA* dirty,DWORD flags){
  if(InterlockedCompareExchange(&presentLogged,1,0)==0)Log("SwapChain Present hook active");IDirect3DDevice9* d=nullptr;
  if(SUCCEEDED(sc->GetDevice(&d))&&d){if(!frameRendered&&SUCCEEDED(d->BeginScene())){OverlayFrame(d);origEndScene(d);}frameRendered=false;d->Release();}
  return origSwapPresent(sc,src,dst,wnd,dirty,flags);
}
static HRESULT STDMETHODCALLTYPE HookReset(IDirect3DDevice9* d,D3DPRESENT_PARAMETERS* pp){if(graftBuffer){graftBuffer->Release();graftBuffer=nullptr;}for(UINT i=0;i<shaderLayoutCount;i++)if(shaderLayouts[i].shader)shaderLayouts[i].shader->Release();memset(shaderLayouts,0,sizeof(shaderLayouts));shaderLayoutCount=0;motionTracked=false;motionBasisReady=false;motionCollisionBonesReady=false;motionSpinSpeed=0;motionLastTick=motionLastCaptureTick=0;motionSamples=0;motionWarmupSamples=motionQuietFrames=0;memset(motionPrevVelocity,0,sizeof(motionPrevVelocity));memset(motionFilteredAccel,0,sizeof(motionFilteredAccel));memset(motionPrevAngularVelocity,0,sizeof(motionPrevAngularVelocity));renderFrameSerial=-1;motionCaptureSerial=-2;motionPassLogged=motionCandidateLogs=motionBoneLogged=0;seenCount=0;physicsLastTick=0;shaftSpring=Spring2{};ballsSpring=Spring2{};constraintSolverReady=false;shaftRestFrameReady=false;constraintAccumulator=0;constraintSolverState=-1;HRESULT hr=origReset(d,pp);shapeDirty=true;return hr;}

static void Log(const char* fmt, ...) {
  char path[MAX_PATH]; GetModuleFileNameA((HMODULE)&__ImageBase,path,MAX_PATH);
  char* slash=strrchr(path,'\\'); if(slash) strcpy_s(slash+1,MAX_PATH-(slash+1-path),"WolverineLive.log");
  FILE* f=nullptr; fopen_s(&f,path,"a"); if(!f)return;
  va_list a; va_start(a,fmt); vfprintf(f,fmt,a); va_end(a); fputc('\n',f); fclose(f);
}
static void Patch(void** slot, void* replacement, void** original) {
  DWORD old; VirtualProtect(slot,sizeof(void*),PAGE_EXECUTE_READWRITE,&old);
  if(original && !*original)*original=*slot; *slot=replacement;
  VirtualProtect(slot,sizeof(void*),old,&old); FlushInstructionCache(GetCurrentProcess(),slot,sizeof(void*));
}
static bool IsFullResolutionScenePass(IDirect3DDevice9* dev){
  IDirect3DSurface9* rt=nullptr;IDirect3DSurface9* bb=nullptr;
  dev->GetRenderTarget(0,&rt);dev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&bb);
  D3DSURFACE_DESC rd{},bd{};D3DVIEWPORT9 vp{};
  bool have=rt&&bb&&SUCCEEDED(rt->GetDesc(&rd))&&SUCCEEDED(bb->GetDesc(&bd))&&SUCCEEDED(dev->GetViewport(&vp));
  bool full=have&&rd.Width==bd.Width&&rd.Height==bd.Height&&vp.X==0&&vp.Y==0&&vp.Width==rd.Width&&vp.Height==rd.Height;
  LONG candidate=InterlockedIncrement(&motionCandidateLogs);
  if(candidate<=20)Log("motion pass candidate rt=%p bb=%p rt=%ux%u fmt=%u bb=%ux%u fmt=%u vp=%u,%u %ux%u accepted=%d",rt,bb,rd.Width,rd.Height,(UINT)rd.Format,bd.Width,bd.Height,(UINT)bd.Format,vp.X,vp.Y,vp.Width,vp.Height,full?1:0);
  if(full&&InterlockedCompareExchange(&motionPassLogged,1,0)==0)Log("character motion source selected: full-resolution scene target (%ux%u, backbuffer=%d)",rd.Width,rd.Height,rt==bb?1:0);
  if(rt)rt->Release();if(bb)bb->Release();return full;
}
static HRESULT STDMETHODCALLTYPE HookDIP(IDirect3DDevice9* dev,D3DPRIMITIVETYPE type,INT base,UINT minv,UINT nv,UINT start,UINT count) {
  if(inOverlay)return origDIP(dev,type,base,minv,nv,start,count);
  IDirect3DVertexBuffer9* vb=nullptr; UINT offset=0,stride=0;
  HRESULT gs=dev->GetStreamSource(0,&vb,&offset,&stride);
  if(SUCCEEDED(gs)&&vb){
    // Keep transform capture coupled to the generated graft topology.  The
    // watertight sack repair increased this section from 4408 to 4472
    // triangles; a duplicated numeric literal left the overlay connected to
    // the vertex buffer while silently rejecting its draw call.
    const UINT graftTriangleCount=graftTriangleIndexCount/3u;
    if(vb==graftBuffer&&start==249804u&&count==graftTriangleCount&&motionCaptureSerial!=renderFrameSerial&&IsFullResolutionScenePass(dev)){motionCaptureSerial=renderFrameSerial;CaptureCharacterMotion(dev);}
    if(!graftBuffer){
      D3DVERTEXBUFFER_DESC desc{};HRESULT gd=vb->GetDesc(&desc);
      if(SUCCEEDED(gd)&&stride==graftStride&&desc.Size==50915u*graftStride){
        void* data=nullptr;HRESULT lk=vb->Lock(47050u*graftStride,3u*graftStride,&data,D3DLOCK_READONLY);
        const float known[3][3]={{14.075339f,-2.7741053f,83.369194f},{12.718411f,-2.7821724f,84.149310f},{12.683769f,-2.4691443f,83.122450f}};bool match=SUCCEEDED(lk);
        // UPK reserialization can alter the last few bits of a position.  Use
        // a strict geometric tolerance instead of rejecting the correct mesh
        // through byte-for-byte floating-point comparison.
        if(match){auto* p=(unsigned char*)data;for(UINT i=0;i<3;i++){const float* value=(const float*)(p+i*graftStride);for(UINT axis=0;axis<3;axis++)if(fabsf(value[axis]-known[i][axis])>.0005f)match=false;}vb->Unlock();}
        // Revision 154 deliberately moved section 7 off the electrode material.
        // UE3 may rewrite a WRITEONLY buffer before our read lock, making its
        // contents unsuitable as an identity test even though rendering is
        // valid. The exact section draw range plus exact full-mesh byte layout
        // is a much stronger invariant and cannot match another scene mesh by
        // accident. Retain the old positional test as a compatible fallback.
        bool drawSignature=start==graftTriangleIndexStart&&count==graftTriangleIndexCount/3u;
        LONG candidate=InterlockedIncrement(&motionCandidateLogs);
        if(candidate<=16)Log("candidate Wolverine VB=%p size=%u offset=%u stride=%u lock=%08X fingerprint=%d drawSignature=%d start=%u count=%u",vb,desc.Size,offset,stride,lk,match?1:0,drawSignature?1:0,start,count);
        if(match||drawSignature){graftBuffer=vb;graftBuffer->AddRef();graftOffset=47050u*graftStride;InterlockedExchange(&logged,1);shapeDirty=true;ApplyShape();Log("graft buffer solidly connected at vertex %u via %s signature",graftOffset/graftStride,match?"geometry":"section-draw");}
      }
    }
    vb->Release();
  }
  return origDIP(dev,type,base,minv,nv,start,count);
}
static HRESULT STDMETHODCALLTYPE HookCreateDevice(IDirect3D9* self,UINT adapter,D3DDEVTYPE type,HWND wnd,DWORD flags,D3DPRESENT_PARAMETERS* pp,IDirect3DDevice9** out) {
  HRESULT hr=origCreateDevice(self,adapter,type,wnd,flags,pp,out);
  if(SUCCEEDED(hr)&&out&&*out){LoadSettings();void** vt=*(void***)*out;Patch(&vt[16],(void*)HookReset,(void**)&origReset);Patch(&vt[17],(void*)HookPresent,(void**)&origPresent);Patch(&vt[42],(void*)HookEndScene,(void**)&origEndScene);Patch(&vt[82],(void*)HookDIP,(void**)&origDIP);IDirect3DSwapChain9* sc=nullptr;if(SUCCEEDED((*out)->GetSwapChain(0,&sc))&&sc){void** svt=*(void***)sc;Patch(&svt[3],(void*)HookSwapPresent,(void**)&origSwapPresent);sc->Release();}Log("CreateDevice hooked %ux%u windowed=%d",pp->BackBufferWidth,pp->BackBufferHeight,pp->Windowed);}
  return hr;
}
static void LoadReal(){
  if(realDll)return; char sys[MAX_PATH];GetSystemDirectoryA(sys,MAX_PATH);strcat_s(sys,"\\d3d9.dll");realDll=LoadLibraryA(sys);
  realCreate9=(decltype(realCreate9))GetProcAddress(realDll,"Direct3DCreate9");realBegin=(decltype(realBegin))GetProcAddress(realDll,"D3DPERF_BeginEvent");realEnd=(decltype(realEnd))GetProcAddress(realDll,"D3DPERF_EndEvent");
}
IDirect3D9* WINAPI Direct3DCreate9(UINT sdk){LoadReal();IDirect3D9* d=realCreate9(sdk);if(d){void** vt=*(void***)d;Patch(&vt[16],(void*)HookCreateDevice,(void**)&origCreateDevice);}return d;}
int WINAPI D3DPERF_BeginEvent(D3DCOLOR c,LPCWSTR n){LoadReal();return realBegin?realBegin(c,n):-1;}
int WINAPI D3DPERF_EndEvent(){LoadReal();return realEnd?realEnd():-1;}
BOOL APIENTRY DllMain(HMODULE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){DisableThreadLibraryCalls(h);LoadReal();Log("proxy loaded");}return TRUE;}
