#define REQUIRE(x) do {if(!(x)){printf("FAIL %d: %s\n",__LINE__,#x);return false;}}while(0)
static bool PerformanceContracts(IDirect3DDevice9* d){
 char attract[MAX_PATH]="..\\WGame\\Movies\\fmv3_1000.bik",loading[MAX_PATH]="..\\WGame\\Movies\\loading.bik";
 REQUIRE(IsTitleAttractMovie(attract,0x01004400));
 REQUIRE(!IsTitleAttractMovie(loading,0x01004400));
 REQUIRE(!IsTitleAttractMovie(attract,0x04004400));
 REQUIRE(!IsTitleAttractMovie((const char*)1,0x01004400));
 REQUIRE(!IsTitleAttractMovie(nullptr,0x01004400));
 const char* shader="float4x4 L:register(c0);float4x4 V:register(c4);float4 main(float4 p:POSITION):POSITION{return mul(mul(p,L),V);}";
 ID3DXBuffer* code=nullptr;REQUIRE(SUCCEEDED(D3DXCompileShader(shader,(UINT)strlen(shader),nullptr,nullptr,"main","vs_3_0",0,&code,nullptr,nullptr)));
 IDirect3DVertexShader9* vs=nullptr;REQUIRE(SUCCEEDED(d->CreateVertexShader((DWORD*)code->GetBufferPointer(),&vs)));code->Release();d->SetVertexShader(vs);
 // Seed the exact reflected layout for this isolated shader; test the production guard.
 UINT id=shaderLayoutCount++;ShaderLayout& layout=shaderLayouts[id];layout.shader=vs;vs->AddRef();layout.localCount=4;layout.viewValid=true;layout.viewRegister=4;shaderLayoutIndex[vs]=id;
 D3DXMATRIX ortho;D3DXMatrixIdentity(&ortho);d->SetVertexShaderConstantF(4,(float*)&ortho,4);
 tankCameraSceneTick=GetTickCount();anatomyVisibleFrame=renderFrameSerial;tankCameraPoseCount=1;tankCameraLocation.x=-120;float after[16];
 {TankCameraDrawOverride guard(d);REQUIRE(!guard.applied);d->GetVertexShaderConstantF(4,after,4);REQUIRE(!memcmp(after,&ortho,64));}
 D3DXMATRIX perspective;D3DXMatrixPerspectiveFovLH(&perspective,1.f,1.5f,.1f,1000.f);d->SetVertexShaderConstantF(4,(float*)&perspective,4);
 {TankCameraDrawOverride guard(d);REQUIRE(guard.applied);d->GetVertexShaderConstantF(4,after,4);REQUIRE(memcmp(after,&perspective,64)!=0);}
 d->GetVertexShaderConstantF(4,after,4);REQUIRE(!memcmp(after,&perspective,64));
 anatomyVisibleFrame=renderFrameSerial-2;{TankCameraDrawOverride guard(d);REQUIRE(!guard.applied);}
 // Loading with retained menu resources must neither rebuild anatomy nor touch the target.
 anatomyVisibleFrame=-100;settingsLoaded=true;menuTankSurfaceReady=true;shapeDirty=false;
 unsigned surfaces=perfMetrics[3].count;d->SetVertexShader(nullptr);d->Clear(0,nullptr,D3DCLEAR_TARGET,0xff243648,1,0);
 REQUIRE(SUCCEEDED(d->BeginScene()));OverlayFrame(d);origEndScene(d);REQUIRE(perfMetrics[3].count==surfaces);
 IDirect3DSurface9* back=nullptr,*read=nullptr;d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back);D3DSURFACE_DESC desc{};back->GetDesc(&desc);d->CreateOffscreenPlainSurface(desc.Width,desc.Height,desc.Format,D3DPOOL_SYSTEMMEM,&read,nullptr);
 REQUIRE(SUCCEEDED(d->GetRenderTargetData(back,read)));D3DLOCKED_RECT pixels{};REQUIRE(SUCCEEDED(read->LockRect(&pixels,nullptr,D3DLOCK_READONLY)));
 bool intact=true;for(UINT y=0;y<desc.Height;y++)for(UINT x=0;x<desc.Width;x++)if(((DWORD*)((char*)pixels.pBits+y*pixels.Pitch))[x]!=0xff243648)intact=false;
 read->UnlockRect();read->Release();back->Release();REQUIRE(intact);
 d->SetVertexShader(nullptr);vs->Release();tankCameraSceneTick=0;tankCameraPoseCount=0;settingsLoaded=false;menuTankSurfaceReady=false;
 printf("PASS loading pixels unchanged with retained scene; orthographic camera untouched; perspective override restored; loading/memory Bink streams pass through\n");
 // The retained connectivity must match the uncached construction byte for byte,
 // while the vertex positions and BVH bounds continue following the current pose.
 ResetFluidCollision();tankCameraSceneTick=GetTickCount();settingsLoaded=true;PrepareMenuTankSurface();
 for(int section=0;section<3;section++){fluidBodyPaletteCount[section]=75;for(int bone=0;bone<75;bone++){float* m=fluidBodyPalette[section]+bone*12;memset(m,0,48);m[0]=m[5]=m[10]=1;}}
 ++renderFrameSerial;PrepareFluidCollision();auto triangles=fluidCollisionTriangles;auto vertices=fluidCollisionVertices;unsigned nodes=(unsigned)fluidBvh.size();
 for(int section=0;section<3;section++)for(int bone=0;bone<75;bone++)fluidBodyPalette[section][bone*12+3]=1;
 ++renderFrameSerial;PrepareFluidCollision();REQUIRE(fluidBvh.size()==nodes&&triangles.size()==fluidCollisionTriangles.size());
 for(unsigned i=0;i<triangles.size();i++){auto& a=triangles[i];auto& b=fluidCollisionTriangles[i];REQUIRE(a.a==b.a&&a.b==b.b&&a.c==b.c&&a.source==b.source&&a.section==b.section);}
 for(unsigned i=0;i<vertices.size();i++)REQUIRE(fabsf(fluidCollisionVertices[i].x-vertices[i].x-1)<.001f);
 auto moved=fluidCollisionVertices;fluidBvhTopologyReady=false;++renderFrameSerial;PrepareFluidCollision();REQUIRE(moved.size()==fluidCollisionVertices.size());
 for(unsigned i=0;i<moved.size();i++)REQUIRE(!memcmp(&moved[i],&fluidCollisionVertices[i],sizeof(V3)));
 fluidBodyPaletteCount[0]=0;++renderFrameSerial;PrepareFluidCollision();REQUIRE(fluidTopologyMask==6&&fluidCollisionTriangles.size()<triangles.size());
 printf("PASS cached body topology retained across motion, matches rebuild, invalidates when sections change\n");return true;
}
