#pragma once
#include <malemod/controls/menu_theme.hpp>

// Native presentation only. Rasterize the user's installed font once, never
// copy a font asset. Failure leaves the original bitmap text path available.
namespace MenuHud {
struct Vertex {float x,y,z,rhw;D3DCOLOR color;float u,v;};
static IDirect3DTexture9* atlas=nullptr;
static std::vector<Vertex> vertices;
static float advances[95]{},fontHeight=48.f;
static bool failed=false;
static void Release(){if(atlas)atlas->Release();atlas=nullptr;failed=false;vertices.clear();}
static int CALLBACK FindFont(const LOGFONTW*,const TEXTMETRICW*,DWORD,LPARAM found){*reinterpret_cast<bool*>(found)=true;return 0;}
static HFONT Font(HDC dc,int height){
 LOGFONTW candidate{};candidate.lfCharSet=DEFAULT_CHARSET;wcscpy_s(candidate.lfFaceName,malemod::controls::menu::Font);bool present=false;
 EnumFontFamiliesExW(dc,&candidate,FindFont,reinterpret_cast<LPARAM>(&present),0);
 return CreateFontW(-height,0,0,0,malemod::controls::menu::FontWeight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,present?malemod::controls::menu::Font:malemod::controls::menu::FallbackFont);
}
static bool Ensure(IDirect3DDevice9* d){
 if(atlas)return true;if(failed)return false;failed=true;
 HDC dc=CreateCompatibleDC(nullptr);if(!dc)return false;
 BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=1024;info.bmiHeader.biHeight=-512;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
 void* pixels=nullptr;auto bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);auto font=Font(dc,40);
 if(!bitmap||!font||!pixels){if(font)DeleteObject(font);if(bitmap)DeleteObject(bitmap);DeleteDC(dc);return false;}
 auto oldBitmap=SelectObject(dc,bitmap),oldFont=SelectObject(dc,font);memset(pixels,0,1024*512*4);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(255,255,255));
 TEXTMETRICW metric{};GetTextMetricsW(dc,&metric);fontHeight=float(metric.tmHeight);wchar_t actual[LF_FACESIZE]{};GetTextFaceW(dc,LF_FACESIZE,actual);
 for(unsigned i=0;i<95;i++){wchar_t c=wchar_t(i+32);SIZE extent{};GetTextExtentPoint32W(dc,&c,1,&extent);advances[i]=float(extent.cx);TextOutW(dc,int(i%16)*64+2,int(i/16)*64+2,&c,1);}
 GdiFlush();HRESULT hr=d->CreateTexture(1024,512,1,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&atlas,nullptr);
 if(SUCCEEDED(hr)){D3DLOCKED_RECT lock{};hr=atlas->LockRect(0,&lock,nullptr,D3DLOCK_DISCARD);if(SUCCEEDED(hr)){auto source=static_cast<unsigned*>(pixels);for(unsigned y=0;y<512;y++){auto dest=reinterpret_cast<unsigned*>(static_cast<unsigned char*>(lock.pBits)+y*lock.Pitch);for(unsigned x=0;x<1024;x++){const unsigned alpha=(source[y*1024+x]>>16)&255;dest[x]=(alpha<<24)|0xffffffu;}}atlas->UnlockRect(0);}}
 SelectObject(dc,oldFont);SelectObject(dc,oldBitmap);DeleteObject(font);DeleteObject(bitmap);DeleteDC(dc);
 if(FAILED(hr)){if(atlas)atlas->Release();atlas=nullptr;Log("HUD native font unavailable %08X; original bitmap fallback",hr);return false;}
 failed=false;Log("HUD native font %ls height=%.0f atlas=1024x512",actual,fontHeight);return true;
}
static void Text(const char* text,RECT r,D3DCOLOR color,DWORD flags,float scale){
 float rawWidth=0;for(auto p=text;*p;p++)rawWidth+=advances[unsigned(*p)>=32&&unsigned(*p)<127?unsigned(*p)-32:0];
 float size=malemod::controls::menu::TextHeight/fontHeight;
 if(rawWidth*size>r.right-r.left)size=float(r.right-r.left)/max(1.f,rawWidth);
 float x=(flags&DT_RIGHT)?r.right-rawWidth*size:float(r.left),y=r.top+((r.bottom-r.top)-fontHeight*size)*.5f;
 for(auto p=text;*p;p++){unsigned i=unsigned(*p)>=32&&unsigned(*p)<127?unsigned(*p)-32:0;float gw=advances[i]+2.f;float l=(x-1.f*size)*scale-.5f,t=y*scale-.5f,rr=l+gw*size*scale,b=t+fontHeight*size*scale;
  float u=float((i%16)*64+1)/1024.f,v=float((i/16)*64+2)/512.f,ur=u+gw/1024.f,vb=v+fontHeight/512.f;
  Vertex quad[]={{l,t,0,1,color,u,v},{rr,t,0,1,color,ur,v},{l,b,0,1,color,u,vb},{l,b,0,1,color,u,vb},{rr,t,0,1,color,ur,v},{rr,b,0,1,color,ur,vb}};
  vertices.insert(vertices.end(),quad,quad+6);x+=advances[i]*size;
 }
}
static void Flush(IDirect3DDevice9* d){
 if(vertices.empty()||!atlas)return;
 d->SetTexture(0,atlas);d->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
 d->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);d->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);d->SetTextureStageState(0,D3DTSS_RESULTARG,D3DTA_CURRENT);
 d->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);d->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);d->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
 d->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);d->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);d->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
 d->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);d->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);d->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);d->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);d->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);d->SetSamplerState(0,D3DSAMP_SRGBTEXTURE,FALSE);
 d->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);d->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);d->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);d->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);d->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,FALSE);
 HRESULT hr=d->DrawPrimitiveUP(D3DPT_TRIANGLELIST,UINT(vertices.size()/3),vertices.data(),sizeof(Vertex));static LONG reported=0;if(FAILED(hr)&&InterlockedCompareExchange(&reported,1,0)==0)Log("HUD native font draw failed %08X",hr);
}
}
