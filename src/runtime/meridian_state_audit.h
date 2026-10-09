#pragma once
// Opt-in diagnostic only; no readbacks in ordinary play. Compare all state the
// garment changes, including constants, UV declaration and sampler bindings.
static bool MeridianStateAuditEnabled(){static bool enabled=[](){char v[8]{};return GetEnvironmentVariableA("MALEMOD_MERIDIAN_LIGHT_TRACE",v,8)==1&&v[0]=='1';}();return enabled;}
static std::vector<unsigned char> MeridianStateSnapshot(IDirect3DDevice9* d){
 std::vector<unsigned char> result;if(!MeridianStateAuditEnabled())return result;
 auto add=[&](const void* p,size_t n){auto b=static_cast<const unsigned char*>(p);result.insert(result.end(),b,b+n);};
 auto object=[&](IUnknown* p){add(&p,sizeof(p));if(p)p->Release();};
 IDirect3DVertexShader9* vs=nullptr;d->GetVertexShader(&vs);object(vs);IDirect3DPixelShader9* ps=nullptr;d->GetPixelShader(&ps);object(ps);
 IDirect3DVertexDeclaration9* decl=nullptr;d->GetVertexDeclaration(&decl);object(decl);IDirect3DIndexBuffer9* ib=nullptr;d->GetIndices(&ib);object(ib);
 IDirect3DVertexBuffer9* vb=nullptr;UINT offset=0,stride=0;d->GetStreamSource(0,&vb,&offset,&stride);object(vb);add(&offset,4);add(&stride,4);
 float v[32]{},p[84]{};d->GetVertexShaderConstantF(0,v,8);d->GetPixelShaderConstantF(0,p,21);add(v,sizeof(v));add(p,sizeof(p));
 for(unsigned slot=0;slot<16;slot++){IDirect3DBaseTexture9* t=nullptr;d->GetTexture(slot,&t);object(t);for(unsigned state=1;state<=13;state++){DWORD value=0;d->GetSamplerState(slot,D3DSAMPLERSTATETYPE(state),&value);add(&value,4);}}
 for(auto state:{D3DRS_ZENABLE,D3DRS_ZWRITEENABLE,D3DRS_ZFUNC,D3DRS_CULLMODE,D3DRS_ALPHABLENDENABLE,D3DRS_ALPHATESTENABLE,D3DRS_STENCILENABLE,D3DRS_COLORWRITEENABLE}){DWORD value=0;d->GetRenderState(state,&value);add(&value,4);}
 return result;
}
