#pragma once
// Production adapter. Shared wrapping and analytic rig math stay in pinned Base.
#include "meridian_recipe.h"
#include "meridian_material.h"
#include "meridian_state_audit.h"
#include "meridian_attempt.h"
#include <malemod/garments/meridian_clearance.hpp>
#include <malemod/garments/meridian_rig.hpp>
#include <malemod/garments/pouch_cage.hpp>
namespace MeridianAdapter {
namespace M=malemod::garments::meridian;
static_assert(MeridianRecipe::contractRevision==6,"Meridian geometry/binding contract mismatch");
static IDirect3DVertexBuffer9* vb=nullptr;
static IDirect3DIndexBuffer9* ib=nullptr,*uncovered=nullptr;
static bool ready=false;
static LONG drawn=-3,prepared=-3;
static unsigned drawnPass=~0u;
static MeridianAttemptGate attemptGate;
static unsigned long long preparationEpoch=0;
// Include rejected attempts and duplicate-hook input sampling in CPU cost.
// Fixed windows avoid allocating or sorting on every render call.
struct PreparationTimer {
 std::chrono::steady_clock::time_point start=std::chrono::steady_clock::now();
 LONG frame;bool success=false,duplicate=false;const char* stage="inputs";
 explicit PreparationTimer(LONG f):frame(f){}
 ~PreparationTimer(){
  static double samples[120]{},sum=0,peak=0;static unsigned count=0,failed=0,duplicates=0;
  double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  samples[count]=ms;sum+=ms;peak=(std::max)(peak,ms);failed+=!success&&!duplicate;duplicates+=duplicate;
  if(++count==120){double sorted[120];std::copy(samples,samples+120,sorted);std::sort(sorted,sorted+120);
   Log("Meridian preparation allCalls=120 rejected=%u duplicateHooks=%u mean=%.4fms p95=%.4fms p99=%.4fms max=%.4fms lastFrame=%ld lastStage=%s",failed,duplicates,sum/120,sorted[113],sorted[118],peak,frame,stage);
   count=failed=duplicates=0;sum=peak=0;
  }
 }
};
static std::vector<JockstrapAdapter::RenderVertex> vertices;
struct Sample {M::Binding binding;M::Vec donor[3];};
static std::vector<Sample> posed;
static float sectionLocal[3][16]{},sectionView[3][16]{};
static IDirect3DSurface9* sectionTarget[3]{};
static bool Enabled(){return true;}
static bool Active(){return Enabled()&&clothingStyle==1;}
static bool CaptureRaw(){static bool value=[](){char s[8]{};return GetEnvironmentVariableA("MALEMOD_MERIDIAN_RAW_CAPTURE",s,sizeof(s))==1&&s[0]=='1';}();return value;}
// Sealed diagnostics use explicit request files and the opt-in light trace.
// Neither capture nor geometry freezing is available during ordinary play.
static bool DiagnosticRequested(const char* name){
 static bool trace=[](){char value[8]{};return GetEnvironmentVariableA("MALEMOD_MERIDIAN_LIGHT_TRACE",value,8)==1&&value[0]=='1';}();
 if(!trace)return false;char path[MAX_PATH]{};SiblingPath(path,name);return GetFileAttributesA(path)!=INVALID_FILE_ATTRIBUTES;
}
#include "anatomy_contact_trace.inl"
static void CaptureRenderMesh(){
 if(!DiagnosticRequested("MeridianGeometry.request"))return;
 char path[MAX_PATH]{};SiblingPath(path,"MeridianRender.bin");FILE* file=nullptr;
 if(!fopen_s(&file,path,"wb")&&file){
  const unsigned header[]={MeridianRecipe::count,unsigned(sizeof(vertices[0])),unsigned(prepared)};
  fwrite(header,sizeof(header),1,file);fwrite(sectionLocal[2],sizeof(float),16,file);
  fwrite(vertices.data(),sizeof(vertices[0]),MeridianRecipe::count,file);fclose(file);
 }
}
// UI morphology/state and scene transitions invalidate material bindings;
// ordinary motion of the existing rig does not.
static void CheckFollowEpoch(){
 static float controls[9]{};static int state=-1,scene=-2;static unsigned long long epoch=~0ull;bool changed=state!=physicsState||scene!=anatomyScene||epoch!=clothingEpoch;
 for(unsigned k=0;k<9;k++){float value=k<7?sliderUI[k]:(k==7?hangUI:glansUI);changed|=controls[k]!=value;controls[k]=value;}
 state=physicsState;scene=anatomyScene;epoch=clothingEpoch;if(changed){++preparationEpoch;attemptGate.Reset();}
}
static void Release(){for(auto* p:{ib,uncovered})if(p)p->Release();ib=uncovered=nullptr;if(vb)vb->Release();vb=nullptr;for(auto& t:sectionTarget){if(t)t->Release();t=nullptr;}MeridianMaterial::Release();attemptGate.Reset();++preparationEpoch;ready=false;drawn=prepared=-3;drawnPass=~0u;}
static void Update(const unsigned char* body){
 if(!Active()||!body)return;
 CheckFollowEpoch();
 auto began=std::chrono::steady_clock::now();
 try{
  auto fetch=[&](unsigned surface,unsigned id)->M::Vec{
   if(surface){if(id>=sharedBodyFirst[1]+sharedBodyCount[1])throw std::runtime_error("Meridian body donor out of range");float p[3];memcpy(p,body+id*32,12);return {p[0],p[1],p[2]};}
   if(id>=nrCount)throw std::runtime_error("Meridian anatomy donor out of range");auto p=nrPositions[id];return {p.x,p.y,p.z};
  };
  posed.resize(MeridianRecipe::sampleCount);vertices.resize(MeridianRecipe::sampleCount);
  // The small pouch cage seeds every interior cloth point. Seam, pole, proxy
  // controls and trim are donor inputs; retain full raw donors for diagnostics.
  static std::vector<unsigned> sampleIDs; if(sampleIDs.empty()){sampleIDs.assign(std::begin(MeridianRecipe::runtimeSamples),std::end(MeridianRecipe::runtimeSamples));if(CaptureRaw())for(unsigned i=MeridianRecipe::columns;i<MeridianRecipe::clothCount-1;i++)sampleIDs.push_back(i);}
  for(unsigned i:sampleIDs){
   auto point=M::Transport(MeridianRecipe::bindings[i],fetch);auto& dst=posed[i];auto b=MeridianRecipe::bindings[i];
   for(unsigned side=0;side<2;side++)for(unsigned control=0;control<6;control++)if(i==MeridianRecipe::lobeControls[side][control]){
    auto center=CPCenter(side),r=CPRadii(side),scale=CPDiv(r,side?V3{5.724f,4.86f,7.81f}:V3{5.724f,4.86f,7.93f});
    // Enclose the rendered skin, including both current pressure transforms,
    // rather than the smaller internal physics support.
    V3 outer=r+CPMul({.80f,.95f,.65f},scale);unsigned axis=control<2?2:(control<4?0:1);float sign=control%2?-1.f:1.f;
    V3 local=axis==0?V3{outer.x,0,0}:axis==1?V3{0,outer.y,0}:V3{0,0,outer.z};
    auto p=center+CPTransform(local*(1.03f*sign),side);point={p.x,p.y,p.z};b=MeridianRecipe::bindings[MeridianRecipe::lobeControls[side][0]];
   }
   dst.binding=b;
   M::Vec bary{};for(unsigned k=0;k<3;k++)bary=M::Add(bary,M::Mul(fetch(b.surface,b.source[k]),b.weights[k]));
   auto offset=M::Sub(point,bary);
   bool lobe=false;for(auto& controls:MeridianRecipe::lobeControls)for(auto id:controls)if(i==id)lobe=true;
   for(unsigned k=0;k<3;k++)dst.donor[k]=lobe?point:M::Add(fetch(b.surface,b.source[k]),offset);

  }
  ready=true;
  static LONG diagnosticFrame=-1000;
  if(renderFrameSerial-diagnosticFrame>=240){
   diagnosticFrame=renderFrameSerial;
   auto root=shaftNodes[0],tip=shaftNodes[shaftNodeCount-1],mid=shaftNodes[shaftNodeCount/2],a=CPCenter(0),b=CPCenter(1);
   Log("Meridian anatomy frame=%ld state=%d root=(%.3f,%.3f,%.3f) mid=(%.3f,%.3f,%.3f) tip=(%.3f,%.3f,%.3f) lobes=(%.3f,%.3f,%.3f)(%.3f,%.3f,%.3f)",renderFrameSerial,physicsState,root.x,root.y,root.z,mid.x,mid.y,mid.z,tip.x,tip.y,tip.z,a.x,a.y,a.z,b.x,b.y,b.z);
  }
  static unsigned serial=0;static double sum=0,maximum=0;
  double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();sum+=ms;maximum=(std::max)(maximum,ms);
  if(++serial%120==1)Log("Meridian update serial=%u vertices=%u triangles=%u mean=%.4fms max=%.4fms directDonors=1 sampledContact=1 continuousContactCertified=0 clothDynamics=0",serial,MeridianRecipe::count,MeridianRecipe::faceCount,sum/serial,maximum);
 }catch(const std::exception& e){ready=false;Log("Meridian candidate rejected: %s",e.what());}
}
// Record only coherent current HDR body/anatomy draws, retaining each actor map.
static void CaptureSection(IDirect3DDevice9* d,int section){
 if(!Enabled()){JockstrapAdapter::CaptureSection(d,section);return;}
 if(!Active()||section<0||section>2||!IsHdrSceneColorPass(d))return;
 DWORD color=0;if(FAILED(d->GetRenderState(D3DRS_COLORWRITEENABLE,&color))||!(color&7))return;
 auto* layout=GetShaderLayout(d);if(!layout||!layout->valid||!layout->viewValid)return;
 auto count=(std::min)(75u,layout->boneCount/3);if(!count)return;
 if(FAILED(d->GetVertexShaderConstantF(layout->boneRegister,JockstrapAdapter::palettes[section],count*3))||
 FAILED(d->GetVertexShaderConstantF(layout->localRegister,sectionLocal[section],4))||
 FAILED(d->GetVertexShaderConstantF(layout->viewRegister,sectionView[section],4)))return;
 IDirect3DSurface9* target=nullptr;if(FAILED(d->GetRenderTarget(0,&target))||!target)return;
 if(sectionTarget[section])sectionTarget[section]->Release();sectionTarget[section]=target;
 JockstrapAdapter::paletteCount[section]=count;JockstrapAdapter::paletteFrame[section]=renderFrameSerial;
 if(section==2)MeridianMaterial::Capture(d,renderFrameSerial);
}
static bool PoseSample(const Sample& sample,const D3DXMATRIX* maps,M::Vec& out){
 out={};float sum=0;bool title=TankCameraSceneActive();
 for(unsigned j=0;j<3;j++){
  auto b=sample.binding;float weight=b.weights[j];if(weight<=0)continue;
  unsigned id=b.source[j],section=2;const unsigned char *packed=nullptr,*overrideBones=nullptr;
  if(!b.surface){if(id>=nrCount)return false;packed=(title?menuTankPacked:nrPacked)+id*32;}
  else{int s;UINT local;if(!MenuRetargetBodyVertex(id,s,local))return false;section=s;packed=sharedBodyBase[s]+local*32;if(!title)overrideBones=(s?fluidGameplayBones1:fluidGameplayBones0)+local*4;}
  if(JockstrapAdapter::paletteFrame[section]!=renderFrameSerial)return false;
  M::Vec p{};float total=0;
  for(unsigned k=0;k<4;k++){unsigned bone=overrideBones?overrideBones[k]:packed[20+k];float w=packed[24+k]/255.f;if(w<=0)continue;if(bone>=JockstrapAdapter::paletteCount[section])return false;
   auto* matrix=JockstrapAdapter::palettes[section]+bone*12;auto q=sample.donor[j];
   for(unsigned r=0;r<3;r++)p[r]+=w*(matrix[r*4]*q[0]+matrix[r*4+1]*q[1]+matrix[r*4+2]*q[2]+matrix[r*4+3]);total+=w;
  }if(total<.5f)return false;
  D3DXVECTOR3 input(p[0]/total,p[1]/total,p[2]/total),output;
  D3DXVec3TransformCoord(&output,&input,&maps[section]);
  out=M::Add(out,M::Mul({output.x,output.y,output.z},weight));sum+=weight;
 }if(sum<.5f)return false;out=M::Mul(out,1.f/sum);return true;
}
static bool Buffers(IDirect3DDevice9* d){
 if(vb&&ib&&uncovered)return true;
 if(!vb&&FAILED(d->CreateVertexBuffer(MeridianRecipe::count*sizeof(vertices[0]),D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&vb,nullptr)))return false;
 auto create=[&](IDirect3DIndexBuffer9*& buffer,const void* data,UINT bytes){if(buffer)return true;if(FAILED(d->CreateIndexBuffer(bytes,D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&buffer,nullptr)))return false;void* raw=nullptr;if(FAILED(buffer->Lock(0,bytes,&raw,0))){buffer->Release();buffer=nullptr;return false;}memcpy(raw,data,bytes);if(FAILED(buffer->Unlock())){buffer->Release();buffer=nullptr;return false;}return true;};
 return create(ib,MeridianRecipe::faces,sizeof(MeridianRecipe::faces))&&create(uncovered,MeridianRecipe::uncoveredIndices,sizeof(MeridianRecipe::uncoveredIndices));
}
static bool HideCovered(){return Active()&&ready&&drawn==renderFrameSerial&&vb&&ib&&uncovered;}
static void Draw(IDirect3DDevice9* d){
 if(!Enabled()){JockstrapAdapter::Draw(d);return;}if(!Active()||!ready||!MeridianMaterial::valid||MeridianMaterial::frame!=renderFrameSerial||drawnPass==MeridianMaterial::pass||!IsHdrSceneColorPass(d))return;
 if(!JockstrapAdapter::EnsureShaders(d)||!MeridianMaterial::Ensure(d)||!Buffers(d))return;
 for(unsigned k=0;k<3;k++)if(JockstrapAdapter::paletteFrame[k]!=renderFrameSerial||!sectionTarget[k]||sectionTarget[k]!=sectionTarget[2])return;
 IDirect3DSurface9* target=nullptr;d->GetRenderTarget(0,&target);bool same=target==sectionTarget[2];if(target)target->Release();if(!same)return;
 auto began=std::chrono::steady_clock::now();
 const bool frozen=prepared>=0&&DiagnosticRequested("MeridianFreeze.request");
 bool preparedNow=prepared!=renderFrameSerial&&!frozen;
 if(preparedNow){
 PreparationTimer preparationTimer(renderFrameSerial);
 D3DXMATRIX inverse,maps[3];if(!D3DXMatrixInverse(&inverse,nullptr,reinterpret_cast<D3DXMATRIX*>(sectionLocal[2])))return;
 for(unsigned k=0;k<3;k++)D3DXMatrixMultiply(&maps[k],reinterpret_cast<D3DXMATRIX*>(sectionLocal[k]),&inverse);
 static std::vector<M::Vec> points;points.resize(MeridianRecipe::sampleCount);
 for(unsigned i:MeridianRecipe::runtimeSamples)if(!PoseSample(posed[i],maps,points[i]))return;
 if(CaptureRaw())for(unsigned i=MeridianRecipe::columns;i<MeridianRecipe::clothCount-1;i++)if(!PoseSample(posed[i],maps,points[i]))return;
 if(!attemptGate.Begin(renderFrameSerial,preparationEpoch,points,MeridianRecipe::runtimeSamples,unsigned(std::size(MeridianRecipe::runtimeSamples)))){preparationTimer.duplicate=true;return;}
 preparationTimer.stage="supports";

 const bool captureRaw=CaptureRaw();
 static unsigned rawCaptures=0;
 if(captureRaw&&renderFrameSerial%30==0&&rawCaptures<64){
  char name[80]{},path[MAX_PATH]{};sprintf_s(name,"MeridianRaw-%ld.bin",renderFrameSerial);SiblingPath(path,name);FILE* file=nullptr;
  if(!fopen_s(&file,path,"wb")&&file){fwrite(points.data(),sizeof(points[0]),points.size(),file);fclose(file);++rawCaptures;}
 }

 try{
  M::CircularSection rings[7];
  for(unsigned ring=0;ring<7;ring++){auto c=MeridianRecipe::ringControls[ring];rings[ring]=M::FitCircularSection(points[c[0]],points[c[1]],points[c[2]],points[c[3]]);}
  auto apex=points[MeridianRecipe::domeApex];M::AlignDomeRim(rings[6],apex);
  M::Vec lobeControls[2][6];for(unsigned h=0;h<2;h++)for(unsigned k=0;k<6;k++)lobeControls[h][k]=points[MeridianRecipe::lobeControls[h][k]];
  for(unsigned h=0;h<6;h++)M::WriteLink(points.data()+MeridianRecipe::proxyRanges[h][0],rings[h],rings[h+1],64);
  M::WriteDome(points.data()+MeridianRecipe::proxyRanges[6][0],rings[6],apex,24,64);
  for(unsigned h=0;h<2;h++){auto c=lobeControls[h];M::WriteOvoid(points.data()+MeridianRecipe::proxyRanges[h+7][0],c[0],c[1],c[2],c[3],c[4],c[5],24,48);}
  const auto raw=points;
  preparationTimer.stage="pouch-cage";
  auto cage=M::FitPouchCage(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::count,MeridianRecipe::sampleCount-1,apex,.12f,16);
  if(renderFrameSerial%120==0)Log("Pouch cage sections=%u samples=%u length=%.4f radius=%.4f continuousContactCertified=0",cage.sections,cage.samples,cage.length,cage.maxRadius);
  std::vector<M::Vec> seamDelta(MeridianRecipe::columns);
  for(unsigned i=0;i<MeridianRecipe::columns;i++)seamDelta[i]=M::Sub(points[i],raw[i]);
  for(unsigned i=MeridianRecipe::clothCount;i<MeridianRecipe::aliasStart;i++){auto f=MeridianRecipe::trimFollowers[i-MeridianRecipe::clothCount];points[i]=M::Add(points[i],M::Mul(M::Add(M::Mul(seamDelta[f.a],1-f.fraction),M::Mul(seamDelta[f.b],f.fraction)),f.weight));}
  for(unsigned row=0;row<MeridianRecipe::rows;row++){unsigned alias=MeridianRecipe::aliasStart+row;points[alias]=points[row*MeridianRecipe::columns];vertices[alias].uv[0]=1;vertices[alias].uv[1]=float(row)/MeridianRecipe::rows;for(float& c:vertices[alias].color)c=1;}
  for(unsigned i=0;i<MeridianRecipe::count;i++){for(unsigned k=0;k<3;k++){vertices[i].p[k]=points[i][k];vertices[i].n[k]=0;}if(i<MeridianRecipe::clothCount){vertices[i].uv[0]=float(i%MeridianRecipe::columns)/MeridianRecipe::columns;vertices[i].uv[1]=float(i/MeridianRecipe::columns)/MeridianRecipe::rows;for(float& c:vertices[i].color)c=1;}}
  for(auto f:MeridianRecipe::faces){auto n=M::Cross(M::Sub(points[f[1]],points[f[0]]),M::Sub(points[f[2]],points[f[0]]));for(auto id:f)for(unsigned k=0;k<3;k++)vertices[id].n[k]+=n[k];}
  for(unsigned row=0;row<MeridianRecipe::rows;row++){unsigned alias=MeridianRecipe::aliasStart+row,original=row*MeridianRecipe::columns;for(unsigned k=0;k<3;k++){float n=vertices[alias].n[k]+vertices[original].n[k];vertices[alias].n[k]=vertices[original].n[k]=n;}}
  for(unsigned i=0;i<MeridianRecipe::count;i++){auto n=M::Unit({vertices[i].n[0],vertices[i].n[1],vertices[i].n[2]});for(unsigned k=0;k<3;k++)vertices[i].n[k]=n[k];}
 }catch(const std::exception& e){static LONG reported=-1000;if(renderFrameSerial-reported>60){reported=renderFrameSerial;Log("Meridian live pose rejected frame=%ld: %s",renderFrameSerial,e.what());static unsigned dumps=0;if(CaptureRaw()&&dumps++<3){char path[MAX_PATH]{};SiblingPath(path,"MeridianRejectedPose.bin");FILE* file=nullptr;if(!fopen_s(&file,path,"wb")&&file){fwrite(points.data(),sizeof(points[0]),points.size(),file);fclose(file);}}}return;}
 for(unsigned i=0;i<MeridianRecipe::count;i++){
  auto& v=vertices[i];v.color[0]=0;v.color[1]=v.color[2]=v.color[3]=1;
  if(i>=MeridianRecipe::clothCount&&i<MeridianRecipe::clothCount+MeridianRecipe::bandVertices){
   unsigned index=i-MeridianRecipe::clothCount,col=MeridianRecipe::bandColumns;
   v.uv[0]=float(index%col)/(col-1);v.uv[1]=M::bandMaterialRows[(index/col)%7];v.color[0]=1;
  }else if(i>=MeridianRecipe::clothCount&&i<MeridianRecipe::aliasStart){v.color[0]=2;v.uv[0]=points[i][0]*.1f;v.uv[1]=points[i][2]*.1f;}
 }
 preparationTimer.stage="upload";
 void* raw=nullptr;if(FAILED(vb->Lock(0,0,&raw,D3DLOCK_DISCARD))){attemptGate.Reset();return;}memcpy(raw,vertices.data(),MeridianRecipe::count*sizeof(vertices[0]));if(FAILED(vb->Unlock())){attemptGate.Reset();return;}
 prepared=renderFrameSerial;
 preparationTimer.success=true;
 CaptureRenderMesh();
 }
 auto audit=MeridianStateSnapshot(d);
 auto* state=JockstrapAdapter::stateBlock;if(FAILED(state->Capture()))return;
 d->SetVertexShader(MeridianMaterial::vs);d->SetPixelShader(MeridianMaterial::ps);d->SetVertexDeclaration(JockstrapAdapter::declaration);
 d->SetVertexShaderConstantF(0,sectionLocal[2],4);d->SetVertexShaderConstantF(4,sectionView[2],4);
 MeridianMaterial::Apply(d);
 d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,MeridianMaterial::additive?FALSE:TRUE);d->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
 d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);d->SetRenderState(D3DRS_ALPHABLENDENABLE,MeridianMaterial::additive);d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
 d->SetRenderState(D3DRS_STENCILENABLE,FALSE);d->SetRenderState(D3DRS_COLORWRITEENABLE,MeridianMaterial::additive?7:15);
 d->SetStreamSource(0,vb,0,sizeof(vertices[0]));d->SetIndices(ib);
 HRESULT hr=origDIP(d,D3DPT_TRIANGLELIST,0,0,MeridianRecipe::count,0,MeridianRecipe::faceCount);HRESULT restored=state->Apply();if(FAILED(restored)){ready=false;Log("Meridian state restoration failure %08x",restored);return;}
 if(!audit.empty()){static unsigned audits=0,failures=0;++audits;if(audit!=MeridianStateSnapshot(d)){++failures;Log("Meridian state audit MISMATCH frame=%ld pass=%u",renderFrameSerial,MeridianMaterial::pass);}if(audits%120==1)Log("Meridian state audit passes=%u failures=%u",audits,failures);}
 if(SUCCEEDED(hr)){
  drawn=renderFrameSerial;drawnPass=MeridianMaterial::pass;if(!preparedNow)return;static unsigned count=0;static double sum=0,maxMs=0;double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();sum+=ms;maxMs=(std::max)(maxMs,ms);
  if(++count%120==1)Log("Meridian draw frame=%ld triangles=%u coveredAnatomyTriangles=%u retainedCollarTriangles=%u mean=%.4fms max=%.4fms geometryUpdates=1 nativeLightingPasses=1 guidesDrawn=0",renderFrameSerial,MeridianRecipe::faceCount,nrIndexCount/3-sizeof(MeridianRecipe::uncoveredIndices)/6,sizeof(MeridianRecipe::uncoveredIndices)/6,sum/count,maxMs);
 }else{ready=false;Log("Meridian draw failure %08x",hr);}
}
}
static IDirect3DIndexBuffer9* MeridianAnatomyIndices(IDirect3DDevice9* d){MeridianAdapter::Draw(d);return MeridianAdapter::HideCovered()?MeridianAdapter::uncovered:nullptr;}
static unsigned MeridianAnatomyIndexCount(){return sizeof(MeridianRecipe::uncoveredIndices)/sizeof(unsigned short);}
