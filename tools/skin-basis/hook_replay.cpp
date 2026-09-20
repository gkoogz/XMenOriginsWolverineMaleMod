#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath>

#include <cstdarg>
static void Log(const char* fmt,...){va_list a;va_start(a,fmt);vprintf(fmt,a);puts("");va_end(a);}
static HRESULT STDMETHODCALLTYPE ForwardDraw(IDirect3DDevice9*d,D3DPRIMITIVETYPE t,INT b,UINT m,UINT n,UINT s,UINT c){return d->DrawIndexedPrimitive(t,b,m,n,s,c);}
static auto origDIP=ForwardDraw;
#include "../../src/runtime/skin_basis_fix.h"
static void require(bool ok,const char* what){if(!ok){printf("FAIL %s\n",what);fflush(stdout);ExitProcess(31);}}
static void checkedDraw(IDirect3DDevice9*d,IDirect3DVertexShader9*vs,IDirect3DPixelShader9*ps,bool expect){
 float pc[224*4],vc[256*4],pcAfter[224*4],vcAfter[256*4];
 d->GetPixelShaderConstantF(0,pc,224);d->GetVertexShaderConstantF(0,vc,256);
 IDirect3DBaseTexture9* tex[16]{};for(int i=0;i<16;i++)d->GetTexture(i,&tex[i]);
 UINT before=skinBasisDraws;require(SUCCEEDED(DrawWithSkinBasis(d,D3DPT_TRIANGLELIST,0,47050,3865,249804,4596)),"draw");
 require(skinBasisDraws-before==(expect?1u:0u),"expected override count");
 IDirect3DVertexShader9* av=nullptr;IDirect3DPixelShader9* ap=nullptr;d->GetVertexShader(&av);d->GetPixelShader(&ap);
 require(av==vs&&ap==ps,"shader restoration");if(av)av->Release();if(ap)ap->Release();
 d->GetPixelShaderConstantF(0,pcAfter,224);d->GetVertexShaderConstantF(0,vcAfter,256);
 require(!memcmp(pc,pcAfter,sizeof(pc))&&!memcmp(vc,vcAfter,sizeof(vc)),"all constants unchanged");
 for(int i=0;i<16;i++){IDirect3DBaseTexture9* after=nullptr;d->GetTexture(i,&after);require(after==tex[i],"textures unchanged");if(after)after->Release();if(tex[i])tex[i]->Release();}
}

static const std::string folder="C:/Games/X-Men Origins Wolverine/Binaries/Capture168_20260916_055428/";
static std::vector<char> read(const std::string& p){std::ifstream f(p,std::ios::binary);return std::vector<char>((std::istreambuf_iterator<char>(f)),{});}
static void replace(std::string& s,const std::string& a,const std::string& b){size_t pos=0;while((pos=s.find(a,pos))!=s.npos){s.replace(pos,a.size(),b);pos+=b.size();}}
static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){return DefWindowProc(h,m,w,l);}
static ID3DXBuffer* assemble(std::string s){ID3DXBuffer *b=nullptr,*e=nullptr;HRESULT hr=D3DXAssembleShader(s.c_str(),(UINT)s.size(),nullptr,nullptr,0,&b,&e);if(FAILED(hr)){printf("assemble %08X %s\n",hr,e?(char*)e->GetBufferPointer():"");fflush(stdout);ExitProcess(20);}if(e)e->Release();return b;}
int main(int argc,char**argv){
 std::string mode=argc>1?argv[1]:"original";
 WNDCLASSA wc{};wc.lpfnWndProc=proc;wc.hInstance=GetModuleHandle(nullptr);wc.lpszClassName="SavedDrawReplay";RegisterClassA(&wc);HWND wnd=CreateWindowA(wc.lpszClassName,"Saved draw replay",0,0,0,1920,1080,nullptr,nullptr,wc.hInstance,nullptr);
 IDirect3D9* d9=Direct3DCreate9(D3D_SDK_VERSION);D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.hDeviceWindow=wnd;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.BackBufferWidth=1920;pp.BackBufferHeight=1080;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.EnableAutoDepthStencil=TRUE;pp.AutoDepthStencilFormat=D3DFMT_D24S8;
 IDirect3DDevice9* d=nullptr;HRESULT hr=d9->CreateDevice(0,D3DDEVTYPE_HAL,wnd,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d);if(FAILED(hr))return 11;
 auto bytes=read(folder+"draw_023_stream0.bin");if(mode=="sign"||mode=="sign255"||mode=="constant_tangent")for(unsigned i=47050;i<49438;i++){
 unsigned char* v=(unsigned char*)bytes.data()+i*32;
 v[19]=mode=="sign255"?255:0;
 if(mode=="constant_tangent"){
  D3DXVECTOR3 n(v[16]/127.5f-1,v[17]/127.5f-1,v[18]/127.5f-1),axis(0,1,0),t;
  if(fabsf(n.y)>.9f)axis=D3DXVECTOR3(0,0,1);D3DXVec3Cross(&t,&axis,&n);D3DXVec3Normalize(&t,&t);
  v[12]=(unsigned char)((t.x+1)*127.5f);v[13]=(unsigned char)((t.y+1)*127.5f);v[14]=(unsigned char)((t.z+1)*127.5f);
 }
 }
 IDirect3DVertexBuffer9* vb=nullptr;d->CreateVertexBuffer((UINT)bytes.size(),0,0,D3DPOOL_MANAGED,&vb,nullptr);void* raw=nullptr;vb->Lock(0,0,&raw,0);memcpy(raw,bytes.data(),bytes.size());vb->Unlock();d->SetStreamSource(0,vb,0,32);
 bytes=read(folder+"draw_023_indices.bin");IDirect3DIndexBuffer9* ib=nullptr;d->CreateIndexBuffer((UINT)bytes.size(),0,D3DFMT_INDEX16,D3DPOOL_MANAGED,&ib,nullptr);ib->Lock(0,0,&raw,0);memcpy(raw,bytes.data(),bytes.size());ib->Unlock();d->SetIndices(ib);
 bytes=read(folder+"draw_023_decl.bin");IDirect3DVertexDeclaration9* decl=nullptr;d->CreateVertexDeclaration((D3DVERTEXELEMENT9*)bytes.data(),&decl);d->SetVertexDeclaration(decl);
 auto v=read(folder+"draw_023_vs.txt"),p=read(folder+"draw_023_ps.txt");std::string vs(v.data()),ps(p.data());
 if(mode=="texcoord"){
  replace(vs,"dcl_color o0.xyz","dcl_texcoord2 o0.xyz");replace(vs,"dcl_color1 o1","dcl_texcoord3 o1");replace(ps,"dcl_color v1.xyz","dcl_texcoord2 v1.xyz");replace(ps,"dcl_color1 v2","dcl_texcoord3 v2");
 }
 if(mode=="world_skin"){
  replace(vs,"    dcl_position o7", "    dcl_texcoord1 o8.xyz\n    dcl_texcoord2 o9.xyz\n    dcl_texcoord3 o10.xyz\n    dcl_position o7");
  replace(vs,"    mad r0, v1.xyzx", "    mul r9.xyz, r1.x, c230\n    mad r9.xyz, r1.y, c231, r9\n    mad o8.xyz, r1.z, c232, r9\n    mul r9.xyz, r4.x, c230\n    mad r9.xyz, r4.y, c231, r9\n    mad o9.xyz, r4.z, c232, r9\n    mul r9.xyz, r8.x, c230\n    mad r9.xyz, r8.y, c231, r9\n    mad o10.xyz, r8.z, c232, r9\n    mad r0, v1.xyzx");
  replace(ps,"    dcl vFace", "    dcl_texcoord1 v6.xyz\n    dcl_texcoord2 v7.xyz\n    dcl_texcoord3 v8.xyz\n    dcl vFace");
  replace(ps,"    dp3_sat r5.x, r12, r5", "    mul r14.xyz, v6, r12.x\n    mad r14.xyz, v7, r12.y, r14\n    mad r14.xyz, v8, r12.z, r14\n    nrm r15.xyz, r14\n    mov r16.x, c10.y\n    mov r16.y, c11.y\n    mov r16.z, c12.y\n    mov r17.x, -c10.x\n    mov r17.y, -c11.x\n    mov r17.z, -c12.x\n    lrp r14.xyz, c21.x, r17, r16\n    nrm r16.xyz, r14\n    dp3_sat r5.x, r15, r16");
 }
 if(mode=="third_row"){
  replace(vs,"    dcl_position o7", "    dcl_texcoord2 o8.xyz\n    dcl_position o7");
  replace(vs,"    mad r0, v1.xyzx", "    mul r9.x, r1.x, c230.y\n    mad r9.x, r1.y, c231.y, r9.x\n    mad r9.x, r1.z, c232.y, r9.x\n    mul r9.y, r4.x, c230.y\n    mad r9.y, r4.y, c231.y, r9.y\n    mad r9.y, r4.z, c232.y, r9.y\n    mul r9.z, r8.x, c230.y\n    mad r9.z, r8.y, c231.y, r9.z\n    mad r9.z, r8.z, c232.y, r9.z\n    nrm r10.xyz, r9\n    mov o8.xyz, r10\n    mad r0, v1.xyzx");
  replace(ps,"    dcl vFace", "    dcl_texcoord2 v6.xyz\n    dcl vFace");
  replace(ps,"    mul_pp r3.xyw, r5.zxzy, v1.yzzx\n    mad_pp r3.xyw, r5.yzzx, v1.zxzy, -r3\n    mul_pp r3.xyw, r3, v2.w", "    mov r3.xyw, v6.xyzz");
 }
 if(mode=="orthogonal"){
  replace(ps,"v1","r14");replace(ps,"v2","r15");replace(ps,"dcl_color r14.xyz","dcl_color v1.xyz");replace(ps,"dcl_color1 r15","dcl_color1 v2");
  replace(ps,"    texld r0,", "    nrm r14.xyz, v1\n    dp3 r15.x, v2, r14\n    mad r15.xyz, r14, -r15.x, v2\n    nrm r16.xyz, r15\n    mov r15.xyz, r16\n    mov r15.w, v2.w\n    texld r0,");
 }
 if(mode=="full_precision")replace(ps,"_pp","");
 if(mode=="pixel_basis"){
  replace(ps,"    lrp r7.xyz, c21.x, r6, r5", "    lrp r7.xyz, c21.x, r6, r5\n    mov r16.xyz, r7");
  replace(ps,"    add_pp oC0.xyz, r0, r1", "    mad oC0.xyz, r16, c44.w, c44.w");
 }
 if(mode=="skin_vector")replace(ps,"    nrm r5.xyz, r7", "    mov r7.xyz, c44.wwxw\n    nrm r5.xyz, r7");
 auto vbcode=assemble(vs),pbcode=assemble(ps);if(mode=="world_skin"){std::ofstream a("fixed_vs.bin",std::ios::binary);a.write((char*)vbcode->GetBufferPointer(),vbcode->GetBufferSize());std::ofstream b("fixed_ps.bin",std::ios::binary);b.write((char*)pbcode->GetBufferPointer(),pbcode->GetBufferSize());}IDirect3DVertexShader9* vertex=nullptr;IDirect3DPixelShader9* pixel=nullptr;d->CreateVertexShader((DWORD*)vbcode->GetBufferPointer(),&vertex);d->CreatePixelShader((DWORD*)pbcode->GetBufferPointer(),&pixel);
 vertex->Release();pixel->Release();
 require(SUCCEEDED(d->CreateVertexShader(skinOriginalVS,&vertex)),"original VS");
 require(SUCCEEDED(d->CreatePixelShader(skinOriginalPS,&pixel)),"original PS");
 d->SetVertexShader(vertex);d->SetPixelShader(pixel);
 bytes=read(folder+"draw_023_vs_constants.bin");d->SetVertexShaderConstantF(0,(float*)bytes.data(),256);bytes=read(folder+"draw_023_ps_constants.bin");float* c=(float*)bytes.data();
 if(mode=="skin_off")c[21*4+1]=0;
 // Cubemaps were not saved in the old capture; neutral placeholders below.
 // Disable only the unavailable environment contribution for every replay.
 c[17*4]=c[17*4+1]=c[17*4+2]=0;d->SetPixelShaderConstantF(0,c,224);
 std::ifstream state(folder+"draw_023.txt");std::string line;std::vector<IDirect3DBaseTexture9*> textures;
 while(std::getline(state,line)){
  unsigned a,b,value,type;char pointer[64];
  if(sscanf_s(line.c_str(),"renderstate %u %u",&a,&value)==2&&value!=3131951870u)d->SetRenderState((D3DRENDERSTATETYPE)a,value);
  if(sscanf_s(line.c_str(),"sampler %u %u %u",&a,&b,&value)==3)d->SetSamplerState(a,(D3DSAMPLERSTATETYPE)b,value);
  if(sscanf_s(line.c_str(),"texture %u %63s type=%u",&a,pointer,(unsigned)sizeof(pointer),&type)==3){
   IDirect3DBaseTexture9* tex=nullptr;
   if(type==3){IDirect3DTexture9* t=nullptr;std::string path=folder+"texture_"+pointer+".dds";hr=D3DXCreateTextureFromFileA(d,path.c_str(),&t);if(FAILED(hr)){printf("missing %s %08X\n",path.c_str(),hr);return 12;}tex=t;}
   else if(type==5){IDirect3DCubeTexture9* t=nullptr;d->CreateCubeTexture(1,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&t,nullptr);for(int side=0;side<6;side++){D3DLOCKED_RECT l{};t->LockRect((D3DCUBEMAP_FACES)side,0,&l,nullptr,0);*(DWORD*)l.pBits=0xff808080;t->UnlockRect((D3DCUBEMAP_FACES)side,0);}tex=t;}
   if(mode=="zero9"&&a==9){tex->Release();IDirect3DTexture9* t=nullptr;d->CreateTexture(1,1,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&t,nullptr);D3DLOCKED_RECT l{};t->LockRect(0,&l,nullptr,0);*(DWORD*)l.pBits=0;t->UnlockRect(0);tex=t;}
   d->SetTexture(a,tex);textures.push_back(tex);
  }
 }
 d->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);d->SetRenderState(D3DRS_COLORWRITEENABLE,15);
 d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0xff303840,1,0);d->BeginScene();
 skinBasisEnabled=false;checkedDraw(d,vertex,pixel,false);
 skinBasisEnabled=true;checkedDraw(d,vertex,pixel,true);
 require(skinPairs.size()==1&&skinPairs[0].matched,"exact captured bytecode match");
 // An unknown shader pair is left untouched.
 IDirect3DPixelShader9* other=nullptr;require(SUCCEEDED(d->CreatePixelShader(skinFixedPS,&other)),"other PS");
 d->SetPixelShader(other);checkedDraw(d,vertex,other,false);d->SetPixelShader(pixel);other->Release();
 ReleaseSkinBasis();require(!skinBasisVS&&!skinBasisPS&&skinPairs.empty(),"release reset resources");
 // Recreate after release; clear so the exported image is only this draw.
 d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xff303840,1,0);
 checkedDraw(d,vertex,pixel,true);hr=S_OK;
 printf("PASS: exact pair, toggle off, unknown pair, shader restoration, constants, textures, release/recreate\n");
d->EndScene();
 IDirect3DSurface9* back=nullptr;d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back);std::string output=mode+".png";HRESULT saved=D3DXSaveSurfaceToFileA(output.c_str(),D3DXIFF_PNG,back,nullptr,nullptr);printf("mode=%s draw=%08X saved=%08X\n",mode.c_str(),hr,saved);
 ReleaseSkinBasis();return FAILED(hr)||FAILED(saved);
}
