#include <d3dx9.h>
static bool TestR14Draw(IDirect3DDevice9* dev,const char* output,D3DPRESENT_PARAMETERS& pp){
 IDirect3DIndexBuffer9* ib=nullptr;
 if(FAILED(dev->CreateIndexBuffer((graftTriangleIndexStart+graftTriangleIndexCount)*2,D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&ib,nullptr)))return false;
 void* raw=nullptr;ib->Lock(0,0,&raw,0);memset(raw,0,(graftTriangleIndexStart+graftTriangleIndexCount)*2);
 for(UINT i=0;i<graftTriangleIndexCount;i++)((unsigned short*)raw)[graftTriangleIndexStart+i]=(unsigned short)(47050+graftTriangleIndices[i]);ib->Unlock();
 D3DVERTEXELEMENT9 elems[]={{0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},{0,16,D3DDECLTYPE_UBYTE4N,0,D3DDECLUSAGE_NORMAL,0},{0,28,D3DDECLTYPE_FLOAT16_2,0,D3DDECLUSAGE_TEXCOORD,0},D3DDECL_END()};
 IDirect3DVertexDeclaration9* decl=nullptr;dev->CreateVertexDeclaration(elems,&decl);
 const char* shader="float4 screen:register(c10);struct O{float4 p:POSITION;float4 c:COLOR0;float2 uv:TEXCOORD0;};O main(float4 p:POSITION,float4 n:NORMAL,float2 uv:TEXCOORD0){O o;o.uv=uv;o.p=float4((p.x-screen.x)/screen.z,(p.z-screen.y)/screen.w,(p.y+40)/80,1);float3 nn=normalize(n.xyz*2-1);float l=.25+.55*max(0,dot(nn,normalize(float3(-.3,-.7,.6))))+.2*max(0,dot(nn,normalize(float3(.7,.3,.2))));o.c=float4(l*.72,l*.76,l*.81,1);return o;}";
 ID3DXBuffer* code=nullptr;ID3DXBuffer* errors=nullptr;
 if(FAILED(D3DXCompileShader(shader,(UINT)strlen(shader),nullptr,nullptr,"main","vs_2_0",0,&code,&errors,nullptr))){if(errors)printf("%s",(char*)errors->GetBufferPointer());return false;}
 IDirect3DVertexShader9* vs=nullptr;dev->CreateVertexShader((DWORD*)code->GetBufferPointer(),&vs);code->Release();
 dev->SetVertexDeclaration(decl);dev->SetVertexShader(vs);dev->SetPixelShader(nullptr);dev->SetTexture(0,nullptr);
 dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);
 IDirect3DTexture9* texture=nullptr;char texturePath[MAX_PATH];
 if(GetEnvironmentVariableA("R14_TEST_TEXTURE",texturePath,MAX_PATH)&&SUCCEEDED(D3DXCreateTextureFromFileA(dev,texturePath,&texture))){dev->SetTexture(0,texture);dev->SetTexture(2,texture);dev->SetTexture(10,texture);dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);dev->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_TEXTURE);dev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);dev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);}
 dev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);dev->SetRenderState(D3DRS_ZENABLE,TRUE);dev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
 dev->SetStreamSource(0,graftBuffer,0,32);dev->SetIndices(ib);
 float loX=1e6f,hiX=-1e6f,loZ=1e6f,hiZ=-1e6f;
 for(UINT i=0;i<r14Count;i++){loX=min(loX,r14Positions[i].x);hiX=max(hiX,r14Positions[i].x);loZ=min(loZ,r14Positions[i].z);hiZ=max(hiZ,r14Positions[i].z);}
 float halfW=max((hiX-loX)*.55f,(hiZ-loZ)*.55f*4/3);float screen[4]={(loX+hiX)*.5f,(loZ+hiZ)*.5f,halfW,halfW*.75f};dev->SetVertexShaderConstantF(10,screen,1);
 origDIP=(DIPFn)(*(void***)dev)[82];origReset=(ResetFn)(*(void***)dev)[16];
 dev->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(58,64,72),1,0);dev->BeginScene();
 HRESULT hr=HookDIP(dev,D3DPT_TRIANGLELIST,0,47050,2388,graftTriangleIndexStart,graftTriangleIndexCount/3);dev->EndScene();
 IDirect3DVertexBuffer9* check=nullptr;IDirect3DIndexBuffer9* checkIB=nullptr;UINT offset=0,stride=0;
 dev->GetStreamSource(0,&check,&offset,&stride);dev->GetIndices(&checkIB);
 bool restored=check==graftBuffer&&checkIB==ib&&offset==0&&stride==32;if(check)check->Release();if(checkIB)checkIB->Release();
 IDirect3DBaseTexture9* checkDiffuse=nullptr,*checkSkin=nullptr;dev->GetTexture(2,&checkDiffuse);dev->GetTexture(10,&checkSkin);
 bool texturesRestored=!texture||(checkDiffuse==texture&&checkSkin==texture);if(checkDiffuse)checkDiffuse->Release();if(checkSkin)checkSkin->Release();
 IDirect3DSurface9* back=nullptr;dev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back);char file[MAX_PATH];sprintf_s(file,"%s.png",output);HRESULT saved=D3DXSaveSurfaceToFileA(file,D3DXIFF_PNG,back,nullptr,nullptr);back->Release();
 dev->SetStreamSource(0,nullptr,0,0);dev->SetIndices(nullptr);dev->SetVertexShader(nullptr);dev->SetVertexDeclaration(nullptr);ib->Release();vs->Release();decl->Release();
 dev->SetTexture(0,nullptr);dev->SetTexture(2,nullptr);dev->SetTexture(10,nullptr);if(texture)texture->Release();
 HRESULT reset=HookReset(dev,&pp);bool recreated=SUCCEEDED(reset)&&EnsureR14(dev);ReleaseR14();
 printf("R14 draw=%08X draws=%u restored=%d texture-restored=%d screenshot=%08X reset=%08X recreated=%d\n",hr,r14SuccessfulDraws,restored,texturesRestored,saved,reset,recreated);
 return SUCCEEDED(hr)&&r14SuccessfulDraws>0&&restored&&texturesRestored&&SUCCEEDED(saved)&&recreated;
}
