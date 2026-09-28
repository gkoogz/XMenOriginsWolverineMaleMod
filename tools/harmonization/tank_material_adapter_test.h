// Contracts independently transcribed from the live tank capture, 2026-09-28.
static IDirect3DTexture9* tankTestOriginal[16]{};
static IDirect3DVertexShader9* tankTestVS;
static IDirect3DPixelShader9* tankTestPS;
static int tankTestPass;
static bool tankTestObserved=true;
static const int tankTestSlots[2][4]={{0,-1,1,2},{1,3,2,-1}};
static HRESULT STDMETHODCALLTYPE InspectTankMaterial(IDirect3DDevice9* d,D3DPRIMITIVETYPE,INT,UINT,UINT,UINT,UINT){
 for(int slot=0;slot<16;slot++){
  IDirect3DBaseTexture9* expected=tankTestOriginal[slot];
  if(tankTestPass>=0){const int* s=tankTestSlots[tankTestPass];if(slot==s[0])expected=sharedSkinMaps[0];if(slot==s[1])expected=sharedSkinMaps[1];if(slot==s[2]||slot==s[3])expected=r14SkinTexture[physicsState==0?1:0];}
  IDirect3DBaseTexture9* actual=nullptr;d->GetTexture(slot,&actual);tankTestObserved&=actual==expected;if(actual)actual->Release();
 }
 IDirect3DVertexShader9* vs=nullptr;IDirect3DPixelShader9* ps=nullptr;d->GetVertexShader(&vs);d->GetPixelShader(&ps);tankTestObserved&=vs==tankTestVS&&ps==tankTestPS;if(vs)vs->Release();if(ps)ps->Release();
 float c[4];d->GetPixelShaderConstantF(14,c,1);tankTestObserved&=c[0]==8&&c[1]==9&&c[2]==3&&c[3]==1;return S_OK;
}
static bool TankMaterialTests(){
 ID3DXBuffer* code=nullptr;CHECK(SUCCEEDED(D3DXDisassembleShader(skinTankBasePS,FALSE,nullptr,&code)));
 const char* text=(const char*)code->GetBufferPointer();CHECK(strstr(text,"texld r1, v0, s0")&&strstr(text,"mad r1.xyz, r1, c1.x, c1.y"));CHECK(strstr(text,"texld r0, v0, s2"));code->Release();
 CHECK(SUCCEEDED(D3DXDisassembleShader(skinTankSpotPS,FALSE,nullptr,&code)));text=(const char*)code->GetBufferPointer();CHECK(strstr(text,"texld r3, v0, s1")&&strstr(text,"mad r1.xyz, r3, c2.x, c2.y"));CHECK(strstr(text,"texld r2, v0, s3")&&strstr(text,"texld r3, v0, s2"));code->Release();
 WNDCLASSA wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandle(nullptr);wc.lpszClassName="TankSkinAdapterTest";RegisterClassA(&wc);HWND window=CreateWindowA(wc.lpszClassName,"Tank skin adapter test",0,0,0,64,64,nullptr,nullptr,wc.hInstance,nullptr);
 LoadReal();IDirect3D9* d9=realCreate9(D3D_SDK_VERSION);CHECK(d9);D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.BackBufferWidth=pp.BackBufferHeight=64;pp.BackBufferFormat=D3DFMT_A8R8G8B8;IDirect3DDevice9* d=nullptr;CHECK(SUCCEEDED(d9->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d)));
 CHECK(LoadSharedSkin(d));for(auto& p:r14SkinTexture)CHECK(SUCCEEDED(d->CreateTexture(4,4,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&p,nullptr)));r14SkinTextureAttempted=true;
 for(int i=0;i<16;i++){CHECK(SUCCEEDED(d->CreateTexture(4+i*4,4,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&tankTestOriginal[i],nullptr)));d->SetTexture(i,tankTestOriginal[i]);}
 IDirect3DVertexShader9* vs[2]{};IDirect3DPixelShader9* ps[2]{};CHECK(SUCCEEDED(d->CreateVertexShader(skinTankBaseVS,&vs[0])));CHECK(SUCCEEDED(d->CreateVertexShader(skinTankSpotVS,&vs[1])));CHECK(SUCCEEDED(d->CreatePixelShader(skinTankBasePS,&ps[0])));CHECK(SUCCEEDED(d->CreatePixelShader(skinTankSpotPS,&ps[1])));
 float constants[224*4];for(unsigned i=0;i<224*4;i++)constants[i]=float(i%17)/17;constants[56]=8;constants[57]=9;constants[58]=3;constants[59]=1;d->SetPixelShaderConstantF(0,constants,224);origDIP=InspectTankMaterial;
 for(int pass=0;pass<2;pass++)for(int state=0;state<3;state++)for(bool anatomy:{false,true}){
  physicsState=state;tankTestPass=pass;tankTestVS=vs[pass];tankTestPS=ps[pass];d->SetVertexShader(vs[pass]);d->SetPixelShader(ps[pass]);
  if(anatomy)CHECK(SUCCEEDED(DrawSharedSkin(d,D3DPT_TRIANGLELIST,0,0,0,0,0,true,r14SkinTexture[state==0?1:0])));else CHECK(SUCCEEDED(DrawSharedBodySkin(d,D3DPT_TRIANGLELIST,0,0,0,0,0)));
  CHECK(tankTestObserved);for(int i=0;i<16;i++){IDirect3DBaseTexture9* p=nullptr;d->GetTexture(i,&p);CHECK(p==tankTestOriginal[i]);p->Release();}
  float after[224*4];d->GetPixelShaderConstantF(0,after,224);CHECK(!memcmp(after,constants,sizeof(after)));
 }
 // An unrecognized pass must retain its native textures, including s0 light
 // attenuation. The tank spotlight adapter also preserves that s0 above.
 tankTestPass=-1;tankTestPS=nullptr;d->SetPixelShader(nullptr);CHECK(SUCCEEDED(DrawSharedBodySkin(d,D3DPT_TRIANGLELIST,0,0,0,0,0)));CHECK(tankTestObserved);
 d->SetVertexShader(nullptr);ReleaseSharedSkin();ReleaseLightingDirections();ReleaseR14SkinTextures();for(int i=0;i<16;i++){d->SetTexture(i,nullptr);tankTestOriginal[i]->Release();tankTestOriginal[i]=nullptr;}for(auto p:vs)p->Release();for(auto p:ps)p->Release();CHECK(SUCCEEDED(d->Reset(&pp)));CHECK(LoadSharedSkin(d));ReleaseSharedSkin();d->Release();d9->Release();DestroyWindow(window);
 printf("PASS live-captured tank base/spotlight contracts: normal/diffuse/SSS/specular binding, body/anatomy parity in 3 states, all 16 slots and 224 constants restored, native lights/shaders retained, unknown-pass safety and reset\n");return true;
}
