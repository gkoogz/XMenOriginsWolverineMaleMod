#define main ExistingSequenceMain
#include "../teaching/sequence_test.cpp"
#undef main
#include "audit_palette_data.h"
static void TestBodyField(unsigned char* full,unsigned char* anatomy){
 for(int s=0;s<2;s++)for(unsigned i=0;i<sharedBodyCount[s];i++){V3 p;auto* v=full+(sharedBodyFirst[s]+i)*32;memcpy(&p,v,12);p.x+=2.f+.015f*p.z;memcpy(v,&p,12);}
 for(unsigned i=0;i<rsCount;i++){V3 p;memcpy(&p,anatomy+i*32,12);p.x+=2.f+.015f*p.z;memcpy(anatomy+i*32,&p,12);}
}
static bool LoadDonor(){
 if(graftBuffer)graftBuffer->Release();graftBuffer=new CpuVertexBuffer(50915*32);graftOffset=47050*32;void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);
 FILE* file=nullptr;fopen_s(&file,"audit-gameplay-base.bin","rb");CHECK(file);CHECK(fread(raw,32,50915,file)==50915);fclose(file);graftBuffer->Unlock();return true;
}
static V3 SkinGlobal(const unsigned char* v,const unsigned* map,float time){
 V3 p;memcpy(&p,v,12);V3 q{};
 for(int i=0;i<4;i++){unsigned b=map[v[20+i]];float a=sinf(time+b*.67f)*.3f,c=cosf(a),s=sinf(a),w=v[24+i]/255.f;q=q+V3{p.x*c-p.y*s+b*.03f,p.x*s+p.y*c,sinf(time*.7f+b)*3+p.z}*w;}
 return q;
}
static bool GeometryTests(){
 std::vector<unsigned char> authored(12082*32),electrodes(authored.size());FILE* f=nullptr;fopen_s(&f,"audit-tank-base.bin","rb");CHECK(f);CHECK(fread(authored.data(),32,12082,f)==12082);fclose(f);
 static const unsigned gameAnatomyBones[5]={3,4,5,113,121};
 for(int field=0;field<2;field++)for(int state=0;state<3;state++)for(float value:{25.f,50.f,85.f}){
  ResetStudyControls();physicsState=state;sliderUI[0]=sliderUI[1]=value;ApplyControlMapping();sharedBodyDeform=field?TestBodyField:nullptr;
  preparedShapeReady=shaftRestFrameReady=eggRestReady=constraintSolverReady=false;paBasisSaved=false;CHECK(LoadDonor());shapeDirty=true;ApplyShape();
  std::vector<unsigned char> game(nrPacked,nrPacked+sizeof(nrPacked));std::vector<unsigned char> body[2];for(int s=0;s<2;s++)body[s].assign(sharedBodyOutput[s],sharedBodyOutput[s]+sharedBodyCount[s]*32);float rest=constraintRestLength;
  preparedShapeReady=shaftRestFrameReady=eggRestReady=constraintSolverReady=false;shapeDirty=true;PrepareMenuTankSurface();
  for(unsigned i=0;i<nrCount;i++){CHECK(!memcmp(game.data()+i*32,menuTankPacked+i*32,20));CHECK(!memcmp(game.data()+i*32+24,menuTankPacked+i*32+24,8));}
  for(int s=0;s<2;s++)CHECK(!memcmp(body[s].data(),sharedBodyOutput[s],body[s].size()));CHECK(fabsf(rest-constraintRestLength)<1e-6f);
  float gap=0;
  for(unsigned k=0;k<paSeamCount;k++){
   int s;UINT local;CHECK(MenuRetargetBodyVertex(paBodyNormalIDs[paSeamBody[k]],s,local));auto* bv=sharedBodyOutput[s]+local*32;unsigned ni=0;for(;ni<nrCount;ni++)if(nrDirect[ni]==paSeamR14[k])break;CHECK(ni<nrCount);auto* av=menuTankPacked+ni*32;
   for(int frame=0;frame<60;frame++){float t=frame*.1f;V3 b=SkinGlobal(bv,s?tankGlobal4:tankGlobal1,t),a=SkinGlobal(av,tankGlobal4,t),g=SkinGlobal(game.data()+ni*32,gameAnatomyBones,t);gap=max(gap,max(Length(a-b),Length(a-g)));}
  }
  CHECK(gap<.0002f);
  // Exercise the production collision consumer, not just the published arrays.
  tankCameraSceneTick=GetTickCount();++renderFrameSerial;fluidCollisionFrame=-99;
  for(int s=0;s<3;s++){fluidBodyPaletteCount[s]=75;for(int b=0;b<75;b++){float* p=fluidBodyPalette[s]+b*12;memset(p,0,48);p[0]=p[5]=p[10]=1;}}
  PrepareFluidCollision();unsigned collisionAt=0;
  for(int s=0;s<2;s++)for(unsigned i=0;i<sharedBodyCount[s];i++){V3 p;memcpy(&p,sharedBodyOutput[s]+i*32,12);CHECK(Length(p-fluidCollisionVertices[collisionAt++])<.0001f);}
  for(unsigned i=0;i<nrCount;i++){V3 p;memcpy(&p,menuTankPacked+i*32,12);CHECK(Length(p-fluidCollisionVertices[collisionAt++])<.0001f);}
  CHECK(collisionAt==fluidCollisionVertices.size());
  tankCameraSceneTick=0;
  // Verify a repeated evaluation starts from authored data, with no accumulation.
  PrepareMenuTankSurface();for(int s=0;s<2;s++)CHECK(!memcmp(body[s].data(),sharedBodyOutput[s],body[s].size()));
  DeformElectrodes(electrodes.data(),authored.data());
  if(!field){CHECK(!memcmp(electrodes.data()+9920*32,authored.data()+9920*32,2162*32));}
  else for(unsigned k=0;k<16;k++){
   unsigned first=0;for(;first<2162;first++)if(electrodeMembership[first]==k)break;V3 a,b;memcpy(&a,authored.data()+(9920+first)*32,12);memcpy(&b,electrodes.data()+(9920+first)*32,12);CHECK(Length(a-b)>2.f);
   for(unsigned i=first+1;i<2162;i++)if(electrodeMembership[i]==k){V3 c,d;memcpy(&c,authored.data()+(9920+i)*32,12);memcpy(&d,electrodes.data()+(9920+i)*32,12);CHECK(fabsf(Length(c-a)-Length(d-b))<.0001f);CHECK(!memcmp(authored.data()+(9920+i)*32+20,electrodes.data()+(9920+i)*32+20,12));}
  }
  printf("PASS shared field=%d state=%d controls=%.0f full geometry/basis/UV/body parity, animated seam gap %.8f, rigid electrodes\n",field,state,value,gap);
 }
 sharedBodyDeform=nullptr;graftBuffer->Release();graftBuffer=nullptr;return true;
}
static bool DensityTests(){
 using namespace volumeFluid;SplatModel model;FluidImpact hit{};hit.kind=FLUID_IMPACT_BODY;hit.n={0,0,1};hit.velocity={60,0,-40};hit.volume=.8f;CHECK(model.Add({hit},1000));CHECK(model.marks.size()==1);CHECK(model.marks[0].span==splatSpan*.5f);model.Shape(model.marks[0],2000);
 double mass=0;for(float h:model.marks[0].density)mass+=h*pow(model.marks[0].span/splatTexture,2);CHECK(fabs(mass-hit.volume)<.00001);
 auto d=model.marks[0];hit.kind=FLUID_IMPACT_WORLD;model.marks.clear();CHECK(model.Add({hit},1000));CHECK(model.marks[0].span==splatSpan);model.Shape(model.marks[0],2000);
 CHECK(d.spread==model.marks[0].spread);CHECK(d.samples.size()==model.marks[0].samples.size());
 hit.kind=FLUID_IMPACT_BODY;hit.volume=80;model.marks.clear();CHECK(model.Add({hit},1000));CHECK(model.marks[0].span==splatSpan);
 printf("PASS body patch 2x linear density, unchanged physical spread/memory, conserved volume, large impact fallback\n");return true;
}
static bool materialObserved=true;static bool expectedBound=true;static bool expectedColor=false;
static HRESULT STDMETHODCALLTYPE InspectMaterial(IDirect3DDevice9* d,D3DPRIMITIVETYPE,INT,UINT,UINT,UINT,UINT){
 float v[4];d->GetPixelShaderConstantF(14,v,1);materialObserved&=v[0]==(expectedBound?16.f:8.f);
 // Independent contract from the captured native shader: s3 is specular,
 // while s4 is decoded from [0,1] to a tangent-space normal.
 for(int i=0;i<2;i++){IDirect3DBaseTexture9* t=nullptr;d->GetTexture(3+i,&t);materialObserved&=t==(expectedBound?sharedSkinMaps[1-i]:nullptr);if(t)t->Release();}
 for(int sampler:{2,10}){IDirect3DBaseTexture9* t=nullptr;d->GetTexture(sampler,&t);materialObserved&=t==(expectedColor?r14SkinTexture[physicsState==0?1:0]:nullptr);if(t)t->Release();}return S_OK;
}
static bool MaterialTests(){
 ID3DXBuffer* assembly=nullptr;CHECK(SUCCEEDED(D3DXDisassembleShader(skinOriginalPS,FALSE,nullptr,&assembly)));
 const char* code=(const char*)assembly->GetBufferPointer();const char* normalRead=strstr(code,"texld r9, v0, s4");const char* normalDecode=strstr(code,"mad r9.xy, r9, c44.z, c44.y");
 CHECK(normalRead&&normalDecode&&normalDecode>normalRead&&normalDecode-normalRead<80);
 CHECK(strstr(code,"texld r3, v0, s3")&&strstr(code,"mul r5.xyz, r3, c13"));assembly->Release();
 WNDCLASSA wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandle(nullptr);wc.lpszClassName="SharedSkinOfflineTest";RegisterClassA(&wc);HWND window=CreateWindowA(wc.lpszClassName,"Shared material offline test",0,0,0,64,64,nullptr,nullptr,wc.hInstance,nullptr);
 LoadReal();CHECK(realCreate9);IDirect3D9* d9=realCreate9(D3D_SDK_VERSION);CHECK(d9);D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.BackBufferWidth=64;pp.BackBufferHeight=64;pp.BackBufferFormat=D3DFMT_A8R8G8B8;IDirect3DDevice9* d=nullptr;CHECK(SUCCEEDED(d9->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d)));
 CHECK(LoadSharedSkin(d));for(auto* map:sharedSkinMaps){D3DSURFACE_DESC desc{};CHECK(SUCCEEDED(map->GetLevelDesc(0,&desc)));CHECK(desc.Width==4096&&desc.Height==4096&&desc.Format==D3DFMT_DXT1&&map->GetLevelCount()==13);}
 IDirect3DVertexShader9* vs=nullptr;IDirect3DPixelShader9* ps=nullptr;CHECK(SUCCEEDED(d->CreateVertexShader(skinOriginalVS,&vs)));CHECK(SUCCEEDED(d->CreatePixelShader(skinOriginalPS,&ps)));d->SetVertexShader(vs);d->SetPixelShader(ps);origDIP=InspectMaterial;
 const float old[4]={8,8,3,1};d->SetPixelShaderConstantF(14,old,1);
 for(bool anatomy:{false,true}){CHECK(SUCCEEDED(DrawSharedSkin(d,D3DPT_TRIANGLELIST,0,0,0,0,0,anatomy)));float v[4];d->GetPixelShaderConstantF(14,v,1);CHECK(!memcmp(v,old,16));for(int i=0;i<2;i++){IDirect3DBaseTexture9* t=nullptr;d->GetTexture(3+i,&t);CHECK(t==nullptr);}IDirect3DVertexShader9* restored=nullptr;d->GetVertexShader(&restored);CHECK(restored==vs);restored->Release();}
 // Different non-null originals detect a swap during restoration as well.
 IDirect3DTexture9* sentinel[2]{};for(int i=0;i<2;i++){CHECK(SUCCEEDED(d->CreateTexture(4+i*4,4,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&sentinel[i],nullptr)));d->SetTexture(3+i,sentinel[i]);}
 for(bool anatomy:{false,true}){CHECK(SUCCEEDED(DrawSharedSkin(d,D3DPT_TRIANGLELIST,0,0,0,0,0,anatomy)));for(int i=0;i<2;i++){IDirect3DBaseTexture9* t=nullptr;d->GetTexture(3+i,&t);CHECK(t==sentinel[i]);t->Release();}}
 for(int i=0;i<2;i++){d->SetTexture(3+i,nullptr);sentinel[i]->Release();}
 for(auto& map:r14SkinTexture)CHECK(SUCCEEDED(d->CreateTexture(4,4,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&map,nullptr)));r14SkinTextureAttempted=true;
 expectedColor=true;for(int state=0;state<3;state++){physicsState=state;CHECK(SUCCEEDED(DrawSharedBodySkin(d,D3DPT_TRIANGLELIST,0,0,0,0,0)));for(int sampler:{2,10}){IDirect3DBaseTexture9* t=nullptr;d->GetTexture(sampler,&t);CHECK(t==nullptr);}}
 expectedColor=false;expectedBound=false;d->SetPixelShader(nullptr);CHECK(SUCCEEDED(DrawSharedBodySkin(d,D3DPT_TRIANGLELIST,0,0,0,0,0)));CHECK(materialObserved);ReleaseR14SkinTextures();
 ReleaseSharedSkin();ReleaseLightingDirections();d->SetVertexShader(nullptr);vs->Release();ps->Release();CHECK(SUCCEEDED(d->Reset(&pp)));CHECK(LoadSharedSkin(d));ReleaseSharedSkin();d->Release();d9->Release();DestroyWindow(window);
 printf("PASS captured native sampler contract (s3 specular, s4 normal), shader-pair map binding, full DDS mip chains, material/shader restoration, unmatched-pass preservation and reset\n");return true;
}
#include "tank_material_adapter_test.h"
int main(int argc,char** argv){setvbuf(stdout,nullptr,_IONBF,0);if(argc>1&&!strcmp(argv[1],"--material"))return MaterialTests()&&TankMaterialTests()?0:1;return GeometryTests()&&DensityTests()&&MaterialTests()&&TankMaterialTests()?0:1;}
