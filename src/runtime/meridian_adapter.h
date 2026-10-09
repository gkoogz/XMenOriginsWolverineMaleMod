#pragma once
// Production adapter. Shared wrapping and analytic rig math stay in pinned Base.
#include "meridian_recipe.h"
#include "meridian_material.h"
#include "meridian_state_audit.h"
#include <malemod/garments/meridian_clearance.hpp>
#include <malemod/garments/meridian_rig.hpp>
#include <malemod/garments/meridian_continuity.hpp>
#include <malemod/garments/meridian_follow.hpp>
namespace MeridianAdapter {
namespace M=malemod::garments::meridian;
static_assert(MeridianRecipe::contractRevision==5,"Meridian geometry/binding contract mismatch");
static IDirect3DVertexBuffer9* vb=nullptr;
static IDirect3DIndexBuffer9* ib=nullptr,*uncovered=nullptr;
static bool ready=false;
static M::SurfaceContinuity continuity;
static M::SurfaceFollower follower;
static std::vector<unsigned> followCertificates;
static std::vector<unsigned> repairCertificates;
static std::vector<unsigned> interiorCertificates;
static std::vector<M::Face> interiorFaces;
static LONG drawn=-3,prepared=-3;
static unsigned drawnPass=~0u;
static std::vector<JockstrapAdapter::RenderVertex> vertices;
struct Sample {M::Binding binding;M::Vec donor[3];};
static std::vector<Sample> posed;
static float sectionLocal[3][16]{},sectionView[3][16]{};
static IDirect3DSurface9* sectionTarget[3]{};
static bool Enabled(){return true;}
static bool Active(){return Enabled()&&clothingStyle==1;}
static bool CaptureRaw(){static bool value=[](){char s[8]{};return GetEnvironmentVariableA("MALEMOD_MERIDIAN_RAW_CAPTURE",s,sizeof(s))==1&&s[0]=='1';}();return value;}
// UI morphology/state and scene transitions invalidate material bindings;
// ordinary motion of the existing rig does not.
static void CheckFollowEpoch(){
 static float controls[9]{};static int state=-1,scene=-2;static unsigned long long epoch=~0ull;bool changed=state!=physicsState||scene!=anatomyScene||epoch!=clothingEpoch;
 for(unsigned k=0;k<9;k++){float value=k<7?sliderUI[k]:(k==7?hangUI:glansUI);changed|=controls[k]!=value;controls[k]=value;}
 state=physicsState;scene=anatomyScene;epoch=clothingEpoch;if(changed){follower.Reset();followCertificates.clear();interiorCertificates.clear();}
}
static void Release(){for(auto* p:{ib,uncovered})if(p)p->Release();ib=uncovered=nullptr;if(vb)vb->Release();vb=nullptr;for(auto& t:sectionTarget){if(t)t->Release();t=nullptr;}MeridianMaterial::Release();continuity.Reset();follower.Reset();followCertificates.clear();repairCertificates.clear();interiorCertificates.clear();ready=false;drawn=prepared=-3;drawnPass=~0u;}
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
  static std::vector<unsigned> sampleIDs; if(sampleIDs.empty()){sampleIDs.assign(std::begin(MeridianRecipe::runtimeSamples),std::end(MeridianRecipe::runtimeSamples));for(unsigned i=MeridianRecipe::columns;i<MeridianRecipe::clothCount-1;i++)sampleIDs.push_back(i);}
  for(unsigned i:sampleIDs){
   if(i>=MeridianRecipe::columns&&i<MeridianRecipe::clothCount-1&&follower.Ready()&&!CaptureRaw())continue;
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
  if(++serial%120==1)Log("Meridian update serial=%u vertices=%u triangles=%u mean=%.4fms max=%.4fms directDonors=1 liveClearance=1 clothDynamics=0",serial,MeridianRecipe::count,MeridianRecipe::faceCount,sum/serial,maximum);
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
 bool preparedNow=prepared!=renderFrameSerial;
 if(preparedNow){
 D3DXMATRIX inverse,maps[3];if(!D3DXMatrixInverse(&inverse,nullptr,reinterpret_cast<D3DXMATRIX*>(sectionLocal[2])))return;
 for(unsigned k=0;k<3;k++)D3DXMatrixMultiply(&maps[k],reinterpret_cast<D3DXMATRIX*>(sectionLocal[k]),&inverse);
 static std::vector<M::Vec> points;points.resize(MeridianRecipe::sampleCount);
 for(unsigned i:MeridianRecipe::runtimeSamples)if(!PoseSample(posed[i],maps,points[i]))return;
 if(!follower.Ready()||CaptureRaw())for(unsigned i=MeridianRecipe::columns;i<MeridianRecipe::clothCount-1;i++)if(!PoseSample(posed[i],maps,points[i]))return;

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
  auto liveAxis=M::PrepareEnvelopePole(points,MeridianRecipe::columns,MeridianRecipe::clothCount-1,MeridianRecipe::count,MeridianRecipe::sampleCount-1);
  std::vector<M::Hull> hulls;hulls.reserve(9);
  for(unsigned h=0;h<9;h++){
   auto* frame=MeridianRecipe::proxyFrames[h];M::Vec e=M::Unit(M::Sub(points[frame[1]],points[frame[0]])),n=M::Unit(M::Cross(e,M::Sub(points[frame[2]],points[frame[0]]))),v=M::Cross(n,e);
   auto* range=MeridianRecipe::normalRanges[h];std::vector<M::Vec> normals;normals.reserve(range[1]+2);normals.push_back(liveAxis);normals.push_back(M::Mul(liveAxis,-1));
   for(unsigned k=0;k<range[1];k++){auto q=MeridianRecipe::supportNormals[range[0]+k];normals.push_back(M::Add(M::Mul(e,q[0]),M::Add(M::Mul(v,q[1]),M::Mul(n,q[2]))));}
   auto* proxy=MeridianRecipe::proxyRanges[h];M::Hull hull;hull.reserve(normals.size());for(auto normal:normals){normal=M::Unit(normal);float support=h<6?M::LinkSupport(rings[h],rings[h+1],normal):(h==6?M::DomeSupport(rings[6],apex,normal):M::OvoidSupport(lobeControls[h-7],normal));hull.push_back({normal,support+.0002f});}
   if(h<6){auto a=rings[h],b=rings[h+1];hull.support=[a,b](M::Vec n){return M::LinkSupport(a,b,n)+.0002f;};}
   else if(h==6){auto rim=rings[6];hull.support=[rim,apex](M::Vec n){return M::DomeSupport(rim,apex,n)+.0002f;};}
   else{std::array<M::Vec,6> controls;std::copy(lobeControls[h-7],lobeControls[h-7]+6,controls.begin());hull.support=[controls](M::Vec n){return M::OvoidSupport(controls.data(),n)+.0002f;};}
   hulls.push_back(std::move(hull));
  }
  const auto raw=points;std::vector<M::Vec> anchors;
  for(unsigned k=0;k<10;k++)anchors.push_back(points[k*MeridianRecipe::columns/10]);
  for(auto ring:rings)anchors.push_back(ring.center);
  for(auto& lobe:lobeControls)anchors.push_back(M::Mul(M::Add(lobe[0],lobe[1]),.5f));anchors.push_back(apex);
  std::vector<M::FollowFrame> followRig;
  for(auto ring:rings)followRig.push_back({ring.center,M::Mul(ring.u,ring.radius),M::Mul(ring.v,ring.radius),M::Mul(M::Cross(ring.u,ring.v),ring.radius)});
  for(auto& lobe:lobeControls)followRig.push_back({M::Mul(M::Add(lobe[0],lobe[1]),.5f),M::Mul(M::Sub(lobe[2],lobe[3]),.5f),M::Mul(M::Sub(lobe[4],lobe[5]),.5f),M::Mul(M::Sub(lobe[0],lobe[1]),.5f)});
  M::WrapReceipt receipt{};bool wrapped=false,certified=false;
  if(interiorFaces.empty())M::MovableClothFaces(MeridianRecipe::clothFaces,MeridianRecipe::clothFaceCount,MeridianRecipe::columns,MeridianRecipe::clothCount,interiorFaces);
  auto refitInterior=[&](){return !interiorFaces.empty()&&M::RefitFollowedSurface(points,MeridianRecipe::columns,MeridianRecipe::clothCount,interiorFaces.data(),unsigned(interiorFaces.size()),hulls,interiorCertificates);};
  static unsigned attempts=0,wraps=0,transported=0,uncertified=0,followed=0;
  ++attempts;
  bool moved=follower.Move(points,followRig,MeridianRecipe::columns);
  certified=moved&&M::RefitFollowedSurface(points,MeridianRecipe::columns,MeridianRecipe::clothCount,MeridianRecipe::clothFaces,MeridianRecipe::clothFaceCount,hulls,followCertificates);
  if(certified||(moved&&refitInterior())){++followed;if(!certified)++uncertified;}else{
  points=raw;
  std::vector<M::Hull> coverHulls{M::ConvexCover(hulls,liveAxis,.04f)};
  try{
   M::FitSeam(points,MeridianRecipe::columns,points[MeridianRecipe::clothCount-1],liveAxis,hulls,.12f,6.f);
   M::WalkMeridians(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::rowHeights,coverHulls,liveAxis,.04f);
   static std::vector<unsigned> certificates;
   // This measured rig's last two solids are the moving testicle ovoïds.
   receipt=M::ClearMeridians(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::clothFaces,MeridianRecipe::clothFaceCount,hulls,liveAxis,.04f,12,&certificates,7);
   continuity.Remember(points,raw,anchors,MeridianRecipe::columns,MeridianRecipe::clothCount);
   follower.Remember(points,raw,followRig,MeridianRecipe::columns,MeridianRecipe::clothCount);
   wrapped=certified=true;++wraps;
  }catch(const std::exception& e){
   // Failed triangle projection cannot become render geometry. Reconstruct
   // from the current pose instead of distorting a transported old chart.
   points=raw;
   try{M::WalkMeridians(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::rowHeights,coverHulls,liveAxis,.04f);}
   catch(const std::exception&){points=raw;if(!follower.Move(points,followRig,MeridianRecipe::columns))continuity.Transport(points,anchors,MeridianRecipe::columns);}
   ++transported;
   certified=M::RefitFollowedSurface(points,MeridianRecipe::columns,MeridianRecipe::clothCount,MeridianRecipe::clothFaces,MeridianRecipe::clothFaceCount,hulls,repairCertificates);
   if(!certified&&refitInterior()){continuity.Remember(points,raw,anchors,MeridianRecipe::columns,MeridianRecipe::clothCount);follower.Remember(points,raw,followRig,MeridianRecipe::columns,MeridianRecipe::clothCount);}
   if(!certified)++uncertified;
   if(attempts%120==1)Log("Meridian chart fallback frame=%ld transported=%d certified=%d: %s",renderFrameSerial,continuity.Ready(),certified,e.what());
  }
  }
  if(!wrapped)M::FairMeridianReversals(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::clothFaces,MeridianRecipe::clothFaceCount,hulls,4);
  std::vector<M::Vec> seamDelta(MeridianRecipe::columns);
  for(unsigned i=0;i<MeridianRecipe::columns;i++)seamDelta[i]=M::Sub(points[i],raw[i]);
  if(attempts%120==1)Log("Meridian continuity attempts=%u wrapped=%u followed=%u transported=%u uncertified=%u frame=%ld",attempts,wraps,followed,transported,uncertified,renderFrameSerial);
  for(unsigned i=MeridianRecipe::clothCount;i<MeridianRecipe::aliasStart;i++){auto f=MeridianRecipe::trimFollowers[i-MeridianRecipe::clothCount];points[i]=M::Add(points[i],M::Mul(M::Add(M::Mul(seamDelta[f.a],1-f.fraction),M::Mul(seamDelta[f.b],f.fraction)),f.weight));}
  for(unsigned row=0;row<MeridianRecipe::rows;row++){unsigned alias=MeridianRecipe::aliasStart+row;points[alias]=points[row*MeridianRecipe::columns];vertices[alias].uv[0]=1;vertices[alias].uv[1]=float(row)/MeridianRecipe::rows;for(float& c:vertices[alias].color)c=1;}
  for(unsigned i=0;i<MeridianRecipe::count;i++){for(unsigned k=0;k<3;k++){vertices[i].p[k]=points[i][k];vertices[i].n[k]=0;}if(i<MeridianRecipe::clothCount){vertices[i].uv[0]=float(i%MeridianRecipe::columns)/MeridianRecipe::columns;vertices[i].uv[1]=float(i/MeridianRecipe::columns)/MeridianRecipe::rows;for(float& c:vertices[i].color)c=1;}}
  for(auto f:MeridianRecipe::faces){auto n=M::Cross(M::Sub(points[f[1]],points[f[0]]),M::Sub(points[f[2]],points[f[0]]));for(auto id:f)for(unsigned k=0;k<3;k++)vertices[id].n[k]+=n[k];}
  for(unsigned row=0;row<MeridianRecipe::rows;row++){unsigned alias=MeridianRecipe::aliasStart+row,original=row*MeridianRecipe::columns;for(unsigned k=0;k<3;k++){float n=vertices[alias].n[k]+vertices[original].n[k];vertices[alias].n[k]=vertices[original].n[k]=n;}}
  for(unsigned i=0;i<MeridianRecipe::count;i++){auto n=M::Unit({vertices[i].n[0],vertices[i].n[1],vertices[i].n[2]});for(unsigned k=0;k<3;k++)vertices[i].n[k]=n[k];}
  if(wrapped&&renderFrameSerial%120==0)Log("Meridian live clearance frame=%ld passes=%u minimum=%.6f correction=%.4f",renderFrameSerial,receipt.iterations,receipt.minimumSeparation,receipt.maximumDisplacement);
 }catch(const std::exception& e){static LONG reported=-1000;if(renderFrameSerial-reported>60){reported=renderFrameSerial;Log("Meridian live pose rejected frame=%ld: %s",renderFrameSerial,e.what());static unsigned dumps=0;if(CaptureRaw()&&dumps++<3){char path[MAX_PATH]{};SiblingPath(path,"MeridianRejectedPose.bin");FILE* file=nullptr;if(!fopen_s(&file,path,"wb")&&file){fwrite(points.data(),sizeof(points[0]),points.size(),file);fclose(file);}}}return;}
 for(unsigned i=0;i<MeridianRecipe::count;i++){
  auto& v=vertices[i];v.color[0]=0;v.color[1]=v.color[2]=v.color[3]=1;
  if(i>=MeridianRecipe::clothCount&&i<MeridianRecipe::clothCount+MeridianRecipe::bandVertices){
   unsigned index=i-MeridianRecipe::clothCount,col=MeridianRecipe::bandColumns;
   v.uv[0]=float(index%col)/(col-1);v.uv[1]=M::bandMaterialRows[(index/col)%7];v.color[0]=1;
  }else if(i>=MeridianRecipe::clothCount&&i<MeridianRecipe::aliasStart){v.color[0]=2;v.uv[0]=points[i][0]*.1f;v.uv[1]=points[i][2]*.1f;}
 }
 void* raw=nullptr;if(FAILED(vb->Lock(0,0,&raw,D3DLOCK_DISCARD)))return;memcpy(raw,vertices.data(),MeridianRecipe::count*sizeof(vertices[0]));if(FAILED(vb->Unlock()))return;
 prepared=renderFrameSerial;
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
