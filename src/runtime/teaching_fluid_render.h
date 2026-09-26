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
static void CancelTeaching(){teachingTimeline.Cancel();teachingFluid.Clear();teachingFluidTime=0;}
static void ReleaseTeaching(){
 CancelTeaching();teachingSceneTick=0;teachingDrawSerial=-2;
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
struct TeachingVertex {V3 p;D3DCOLOR color;};
static void TeachingMesh(std::vector<TeachingVertex>& vertices){
 vertices.clear();
 for(const auto& f:teachingFluid.strands){
  if(f.count<2)continue;
  float fade=1.f-teaching::Ease((f.age-2.5f)/.5f);int alpha=(int)((f.droplet?180:240)*fade);
  for(int i=1;i<f.count;i++){
   V3 a=f.nodes[i-1].p,b=f.nodes[i].p,axis=Unit(b-a);
   float length=Length(b-a);if(length<.001f||length>8.f*teaching::tracerScale)continue;
   float radius=teaching::tracerScale*(f.droplet?.10f:.13f)*sqrtf(teaching::Clamp(f.rest[i]/length,.2f,2.f));
   V3 u=Unit(Cross(axis,fabsf(axis.z)<.9f?V3{0,0,1}:V3{0,1,0})),v=Cross(axis,u);
   for(int side=0;side<6;side++){
    float p=side*6.2831853f/6,q=(side+1)*6.2831853f/6;
    V3 r=(u*cosf(p)+v*sinf(p))*radius,s=(u*cosf(q)+v*sinf(q))*radius;
    int light=180+(side%3)*25;D3DCOLOR c=D3DCOLOR_ARGB(alpha,light*2/3,light,255);
    TeachingVertex tri[6]={{a+r,c},{b+r,c},{a+s,c},{a+s,c},{b+r,c},{b+s,c}};
    vertices.insert(vertices.end(),tri,tri+6);
    if(i==1){TeachingVertex cap[3]={{a,c},{a+s,c},{a+r,c}};vertices.insert(vertices.end(),cap,cap+3);}
    if(i==f.count-1){TeachingVertex cap[3]={{b,c},{b+r,c},{b+s,c}};vertices.insert(vertices.end(),cap,cap+3);}
   }
  }
 }
}
static void DrawTeachingFluid(IDirect3DDevice9* d){
 if(teachingDrawSerial==renderFrameSerial)return;
 DWORD colorWrite=0;d->GetRenderState(D3DRS_COLORWRITEENABLE,&colorWrite);if(!(colorWrite&7))return;
 ShaderLayout* layout=GetShaderLayout(d);if(!layout||!layout->valid||!layout->viewValid)return;
 teachingSceneTick=GetTickCount();teachingDrawSerial=renderFrameSerial;
 if(!teachingTimeline.active)return;
 float local[16],view[16],bone[12];if(!TeachingMatrices(d,local,view,bone))return;
 V3 tip,dir;if(!TeachingEmitter(bone,tip,dir)){CancelTeaching();return;}
 if(!teachingFluid.ready)teachingFluid.Begin(tip);
 float dt=(float)(teachingTimeline.time-teachingFluidTime);teachingFluidTime=teachingTimeline.time;
 // Inverse orthonormal component-to-world rotation; translation is irrelevant.
 V3 gravity={-98.f*local[2],-98.f*local[6],-98.f*local[10]};
 teachingFluid.Advance(dt,tip,dir,gravity);
 if(!teachingFluid.ready){CancelTeaching();return;}
 if(!teachingFluid.Live()||!EnsureTeachingShaders(d))return;
 static std::vector<TeachingVertex> vertices;TeachingMesh(vertices);if(vertices.empty())return;
 IDirect3DStateBlock9* state=nullptr;if(FAILED(d->CreateStateBlock(D3DSBT_ALL,&state))||!state)return;
 // Some D3D9 software-vertex-processing drivers lose high palette constants
 // on state-block application. Preserve the complete vs_3_0 register bank.
 float savedConstants[256*4];if(FAILED(d->GetVertexShaderConstantF(0,savedConstants,256))){state->Release();return;}
 // Keep the scene target/depth surface; restore every modified pipeline state.
 d->SetVertexShader(teachingVS);d->SetPixelShader(teachingPS);
 d->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);d->SetVertexShaderConstantF(0,local,4);d->SetVertexShaderConstantF(4,view,4);
 d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);d->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
 d->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);d->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);d->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);d->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
 d->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,FALSE);d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
 d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_STENCILENABLE,FALSE);d->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);
 d->SetRenderState(D3DRS_CLIPPLANEENABLE,0);d->SetRenderState(D3DRS_COLORWRITEENABLE,7);
 d->SetRenderState(D3DRS_COLORWRITEENABLE1,0);d->SetRenderState(D3DRS_COLORWRITEENABLE2,0);d->SetRenderState(D3DRS_COLORWRITEENABLE3,0);
 d->SetRenderState(D3DRS_DEPTHBIAS,0);d->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,0);
 teachingLastDraw=d->DrawPrimitiveUP(D3DPT_TRIANGLELIST,(UINT)vertices.size()/3,vertices.data(),sizeof(TeachingVertex));
 if(SUCCEEDED(teachingLastDraw))++teachingDraws;state->Apply();
 d->SetVertexShaderConstantF(0,savedConstants,256);state->Release();
}
static void TeachingInput(IDirect3DDevice9* d){
 DWORD now=GetTickCount(),pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);
 bool focused=pid==GetCurrentProcessId();
 bool down=(GetAsyncKeyState(teachingKey)&0x8000)!=0;
 static bool oldDown=false;bool edge=down&&!oldDown;oldDown=down;
 if(!focused){if(teachingTimeline.active)CancelTeaching();teachingLastTick=now;return;}
 if(edge){
  if(teachingTimeline.active)CancelTeaching();
  else if(teachingSceneTick&&now-teachingSceneTick<250&&r14Ready){teachingTimeline.Start();teachingFluid.Clear();teachingFluidTime=0;teachingLastTick=now;}
 }
 float dt=teachingLastTick?(now-teachingLastTick)*.001f:0.f;teachingLastTick=now;
 if(teachingTimeline.active&&(dt>.25f||!teachingSceneTick||now-teachingSceneTick>250)){CancelTeaching();return;}
 teachingTimeline.Advance(dt);
 if(!teachingTimeline.active)teachingFluid.Clear();
}
