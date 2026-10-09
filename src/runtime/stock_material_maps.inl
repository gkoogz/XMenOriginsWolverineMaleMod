// Original licensed maps are generated into the build and embedded as resources.
static IDirect3DTexture9* stockMaps[5]{};
static void ReleaseStockMaps(){for(auto& p:stockMaps){if(p)p->Release();p=nullptr;}}
static bool EnsureStockMaps(IDirect3DDevice9* d){
 static const int anchor=0;HMODULE module=nullptr;
 if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCSTR)&anchor,&module))return false;
#ifdef FABRIC_COTTON_TOP
 const unsigned ids[]={301,302,303};
#else
 const unsigned ids[]={304,305,306,307,308};
#endif
 for(unsigned i=0;i<sizeof(ids)/sizeof(ids[0]);i++)if(!stockMaps[i]){
  auto r=FindResourceA(module,MAKEINTRESOURCEA(ids[i]),RT_RCDATA);if(!r)return false;
  auto memory=LoadResource(module,r);auto bytes=SizeofResource(module,r);auto data=memory?LockResource(memory):nullptr;if(!data)return false;
  // Generate all lower levels once. Full source resolution is retained.
  HRESULT hr=D3DXCreateTextureFromFileInMemoryEx(d,data,bytes,D3DX_DEFAULT,D3DX_DEFAULT,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_FILTER_NONE,D3DX_FILTER_BOX,0,nullptr,nullptr,&stockMaps[i]);
  if(FAILED(hr)){Log("Stock garment map %u failed=%08x",ids[i],hr);ReleaseStockMaps();return false;}
  D3DSURFACE_DESC desc{};stockMaps[i]->GetLevelDesc(0,&desc);Log("Stock garment map %u loaded %ux%u levels=%u",ids[i],desc.Width,desc.Height,stockMaps[i]->GetLevelCount());
 }
 return true;
}
static void ApplyStockMaps(IDirect3DDevice9* d){
 for(unsigned i=0;i<5;i++){
  unsigned s=3+i;d->SetTexture(s,stockMaps[i]);
  d->SetSamplerState(s,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);d->SetSamplerState(s,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);d->SetSamplerState(s,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR);
  d->SetSamplerState(s,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);d->SetSamplerState(s,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
  d->SetSamplerState(s,D3DSAMP_MAXMIPLEVEL,0);d->SetSamplerState(s,D3DSAMP_MIPMAPLODBIAS,0);d->SetSamplerState(s,D3DSAMP_MAXANISOTROPY,1);
  d->SetSamplerState(s,D3DSAMP_SRGBTEXTURE,i==0||i==3?TRUE:FALSE);
 }
}
static float stockCamera[4]{};
static void SetStockCamera(const float* v){
 D3DXVECTOR3 a(v[0],v[4],v[8]),b(v[1],v[5],v[9]),c(v[3],v[7],v[11]),bc,ca,ab;
 D3DXVec3Cross(&bc,&b,&c);D3DXVec3Cross(&ca,&c,&a);D3DXVec3Cross(&ab,&a,&b);float det=D3DXVec3Dot(&a,&bc);
 if(fabsf(det)<1e-9f){stockCamera[3]=0;return;}
 auto p=(bc*(-v[12])+ca*(-v[13])+ab*(-v[15]))/det;stockCamera[0]=p.x;stockCamera[1]=p.y;stockCamera[2]=p.z;stockCamera[3]=1;
}
