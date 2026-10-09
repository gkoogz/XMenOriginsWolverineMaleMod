#pragma once

// Wolverine-only main-menu artwork and Direct3D9 integration. The two PNGs
// are generated from the user's licensed extraction and installed beside the
// proxy DLL; no game-derived pixels are checked into Git.
namespace MenuBranding {
struct Vertex { float x,y,z,rhw; D3DCOLOR color; float u,v; };
static IDirect3DTexture9* title=nullptr;
static IDirect3DTexture9* stamp=nullptr;
static bool attempted=false;
static LONG suppressedFrame=-1;

static void DropTextures(){ if(title)title->Release(); if(stamp)stamp->Release(); title=stamp=nullptr; }
static void Release(){ DropTextures(); attempted=false; suppressedFrame=-1; }
static bool Ready(){ return title&&stamp; }

static bool AssetPath(char (&out)[MAX_PATH],const char* name){
  char module[MAX_PATH]{};
  if(!GetModuleFileNameA(reinterpret_cast<HMODULE>(&__ImageBase),module,MAX_PATH))return false;
  const char* slash=strrchr(module,'\\');
  if(!slash)return false;
  const size_t root=size_t(slash-module)+1;
  const char* folder="MenuBranding\\";
  if(root+strlen(folder)+strlen(name)+1>MAX_PATH)return false;
  memcpy(out,module,root);out[root]=0;
  strcat_s(out,MAX_PATH,folder);strcat_s(out,MAX_PATH,name);
  return true;
}
static bool Ensure(IDirect3DDevice9* d){
  if(Ready())return true;
  if(attempted)return false;
  attempted=true;
  char first[MAX_PATH]{},second[MAX_PATH]{};
  if(!AssetPath(first,"title-and-blood.png")||!AssetPath(second,"edition-stamp.png")){
    Log("menu branding path unavailable; original title retained");return false;
  }
  const auto load=[&](const char* path,IDirect3DTexture9** output){
    return D3DXCreateTextureFromFileExA(d,path,D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,D3DX_FILTER_LINEAR,D3DX_FILTER_LINEAR,0,nullptr,nullptr,output);
  };
  HRESULT a=load(first,&title),b=SUCCEEDED(a)?load(second,&stamp):E_FAIL;
  if(FAILED(a)||FAILED(b)){
    DropTextures();Log("menu branding assets unavailable title=%08X stamp=%08X; original title retained",a,b);return false;
  }
  Log("menu branding title/blood and edition stamp loaded from owned local assets");
  return true;
}

static bool IsOriginalTitleDraw(IDirect3DDevice9* d,UINT count,UINT stride){
  if(!Ready()||anatomyScene!=1)return false;
  IDirect3DSurface9* target=nullptr;IDirect3DSurface9* back=nullptr;
  d->GetRenderTarget(0,&target);d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back);
  const bool final=target&&back&&target==back;
  if(target)target->Release();if(back)back->Release();
  if(!final)return false;
  IDirect3DBaseTexture9* bound=nullptr;
  if(FAILED(d->GetTexture(0,&bound))||!bound)return false;
  bool match=false;
  if(bound->GetType()==D3DRTYPE_TEXTURE){
    D3DSURFACE_DESC desc{};
    if(SUCCEEDED(static_cast<IDirect3DTexture9*>(bound)->GetLevelDesc(0,&desc)))
      match=(desc.Width==1024u&&desc.Height==256u&&desc.Format==D3DFMT_DXT5&&count==10u&&stride==12u) ||
            (desc.Width==512u&&desc.Height==512u&&desc.Format==D3DFMT_DXT1&&(count==10u||count==2u||count==12u)&&(stride==12u||stride==4u)) ||
            (desc.Width==256u&&desc.Height==128u&&desc.Format==D3DFMT_DXT5&&count==10u&&stride==12u);
  }
  bound->Release();
  if(match){
    suppressedFrame=renderFrameSerial;
    static volatile LONG logged=0;
    if(InterlockedCompareExchange(&logged,1,0)==0)Log("menu branding suppressed native title and old blood art draw signatures");
  }
  return match;
}

static void Quad(IDirect3DDevice9* d,IDirect3DTexture9* texture,float x,float y,float width,float height,float degrees,float alpha,const D3DVIEWPORT9& vp){
  constexpr float pi=3.14159265358979323846f;
  const float radians=degrees*pi/180.f,c=cosf(radians),s=sinf(radians);
  const float sx=float(vp.Width)/1280.f,sy=float(vp.Height)/720.f;
  const float cx=x+width*.5f,cy=y+height*.5f;
  const float dx[4]={-width*.5f,width*.5f,-width*.5f,width*.5f};
  const float dy[4]={-height*.5f,-height*.5f,height*.5f,height*.5f};
  const float u[4]={0.f,1.f,0.f,1.f},v[4]={0.f,0.f,1.f,1.f};
  const D3DCOLOR tint=D3DCOLOR_ARGB(unsigned(alpha*255.f+.5f),255,255,255);
  Vertex corners[4]{};
  for(int i=0;i<4;i++){
    corners[i]={vp.X+(cx+dx[i]*c-dy[i]*s)*sx-.5f,vp.Y+(cy+dx[i]*s+dy[i]*c)*sy-.5f,0.f,1.f,tint,u[i],v[i]};
  }
  Vertex triangles[6]={corners[0],corners[1],corners[2],corners[2],corners[1],corners[3]};
  d->SetTexture(0,texture);
  HRESULT result=d->DrawPrimitiveUP(D3DPT_TRIANGLELIST,2,triangles,sizeof(Vertex));
  if(FAILED(result)){static volatile LONG logged=0;if(InterlockedCompareExchange(&logged,1,0)==0)Log("menu branding textured draw failed %08X",result);}
}
static void Draw(IDirect3DDevice9* d,LONG frame){
  if(!Ready()||suppressedFrame!=frame||anatomyScene!=1)return;
  D3DVIEWPORT9 vp{};if(FAILED(d->GetViewport(&vp))||!vp.Width||!vp.Height)return;
  d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);d->SetRenderState(D3DRS_STENCILENABLE,FALSE);
  d->SetRenderState(D3DRS_FOGENABLE,FALSE);d->SetRenderState(D3DRS_LIGHTING,FALSE);
  d->SetRenderState(D3DRS_ZENABLE,FALSE);d->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
  d->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
  d->SetRenderState(D3DRS_COLORWRITEENABLE,0xF);d->SetRenderState(D3DRS_SRGBWRITEENABLE,FALSE);
  d->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);d->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
  d->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);d->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
  d->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,FALSE);
  d->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
  d->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
  d->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
  d->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
  d->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);d->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
  d->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
  d->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);d->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
  d->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);d->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
  d->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);d->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
  d->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);d->SetSamplerState(0,D3DSAMP_SRGBTEXTURE,FALSE);
  d->SetVertexShader(nullptr);d->SetPixelShader(nullptr);
  d->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
  Quad(d,title,284.906942f,78.109306f,1280.f,370.f,0.f,1.f,vp);
  constexpr float stampWidth=438.413064f,stampHeight=146.14f;
  const float phase=6.2831853f*float(GetTickCount()%2200u)/2200.f;
  const float rise=.5f-.5f*cosf(phase);
  const float scale=1.f+.02f*rise;
  const float opacity=.88f+.12f*rise;
  const float scaledWidth=stampWidth*scale,scaledHeight=stampHeight*scale;
  const float centerX=742.523903f+stampWidth*.5f,centerY=299.109968f+stampHeight*.5f;
  Quad(d,stamp,centerX-scaledWidth*.5f,centerY-scaledHeight*.5f,scaledWidth,scaledHeight,-4.f,opacity,vp);
}
}
