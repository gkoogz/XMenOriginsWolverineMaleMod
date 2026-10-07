#pragma once
// Adapter only. Include after the final nrPacked surface, shared body arrays,
// ShaderLayout and the verified gameplay bone-remap data.
#include <malemod/garments/jockstrap.hpp>
#include <malemod/garments/reaction_support.hpp>
#include "jockstrap_reaction_data.h"
#include "jockstrap_measured_data.h"
#include "jockstrap_regions_data.h"
#include "jockstrap_worker.h"
#include <unordered_map>
static bool IsFullResolutionScenePass(IDirect3DDevice9*);
namespace JockstrapAdapter {
namespace G=malemod::garments;
// Heap lifetime avoids joining a worker inside the Windows DLL loader lock.
// Device reset/scene release explicitly stops it outside that lock.
static JockstrapCpuWorker* worker=nullptr;
static std::shared_ptr<const G::Output> snapshot;
static std::shared_ptr<const JockstrapCpuWorker::Delivery> solved;
static std::shared_ptr<const JockstrapKinematics::Pose> displayPose;
static JockstrapKinematics::Clock activeClock;
static std::array<float,10> restControls{};static bool restKnown=false;static unsigned long long restRevision=0;
static unsigned long long styleGeneration=1;
static G::Input input;
static G::Style style=G::Style::Naked;
static const unsigned char* currentBody=nullptr;
static unsigned long long epoch=1;
static bool ready=false;
static LONG drawnFrame=-2,paletteFrame[3]={-2,-2,-2};
static float palettes[3][75*12]{};static UINT paletteCount[3]{};
static float localMatrix[16]{},viewMatrix[16]{},sectionLocal[3][16]{},sectionView[3][16]{};
static IDirect3DSurface9* sectionTarget[3]{};
static IDirect3DVertexShader9* vertexShader=nullptr;
static IDirect3DPixelShader9* pixelShader=nullptr;
static IDirect3DVertexDeclaration9* declaration=nullptr;
static IDirect3DStateBlock9* stateBlock=nullptr;
struct RenderVertex{float p[3],n[3],uv[2],color[4];};
static std::vector<RenderVertex> rendered;
static std::vector<unsigned short> indices;
static G::Point Position(const unsigned char* p){float v[3];memcpy(v,p,12);return {v[0],v[1],v[2]};}
static G::Point Normal(const unsigned char* p){G::Point n{p[16]/127.5-1,p[17]/127.5-1,p[18]/127.5-1};return G::Length(n)>1e-9?G::Unit(n):G::Point{0,0,1};}
namespace K=JockstrapKinematics;
struct GeometrySnapshot {std::array<std::vector<unsigned char>,2> body;std::vector<unsigned char> anatomy;};
static const unsigned char* SnapshotPacked(const GeometrySnapshot& geometry,G::Donor donor){if(donor.surface==G::Surface::Anatomy){if(donor.vertex>=nrCount)throw std::invalid_argument("Snapshot anatomy donor outside mesh");return geometry.anatomy.data()+donor.vertex*32;}int section;UINT local;if(!MenuRetargetBodyVertex(donor.vertex,section,local))throw std::invalid_argument("Snapshot body donor outside measured resources");return geometry.body[section].data()+local*32;}
static std::shared_ptr<const GeometrySnapshot> CaptureGeometry(const K::Pose& pose){if(!currentBody)throw std::invalid_argument("Completed garment body buffer absent");auto geometry=std::make_shared<GeometrySnapshot>();for(unsigned section=0;section<2;section++)geometry->body[section].assign(currentBody+sharedBodyFirst[section]*32,currentBody+(sharedBodyFirst[section]+sharedBodyCount[section])*32);geometry->anatomy.assign(nrPacked,nrPacked+nrCount*32);if(pose.title)for(unsigned vertex=0;vertex<nrCount;vertex++)memcpy(geometry->anatomy.data()+vertex*32+20,menuTankPacked+vertex*32+20,8);return geometry;}
static K::Matrix NativeMap(const G::Lineage& lineage,const K::Pose& pose,bool world,const GeometrySnapshot* geometry=nullptr){
 K::Matrix out{};double total=0;
 for(auto donor:lineage.donors){if(donor.weight<=0)continue;const unsigned char* packed=nullptr;const unsigned char* bones=nullptr;unsigned section=2;
  if(donor.surface==G::Surface::Anatomy){if(donor.vertex>=nrCount)throw std::invalid_argument("Cloth anatomy donor outside native mesh");packed=geometry?SnapshotPacked(*geometry,donor):(pose.title?menuTankPacked:nrPacked)+donor.vertex*32;bones=packed+20;}
  else {int s;UINT local;if(!MenuRetargetBodyVertex(donor.vertex,s,local))throw std::invalid_argument("Cloth body donor outside measured sections");section=unsigned(s);packed=sharedBodyBase[s]+local*32;bones=pose.title?packed+20:(s?fluidGameplayBones1:fluidGameplayBones0)+local*4;}
  double weights=0;for(unsigned k=0;k<4;k++)weights+=packed[24+k];if(weights<=0)throw std::invalid_argument("Cloth native donor weights empty");
  for(unsigned k=0;k<4;k++)if(packed[24+k]){unsigned bone=bones[k];if(bone>=pose.count[section])throw std::invalid_argument("Cloth native donor bone absent");const auto& m=world?pose.world[section][bone]:pose.skin[section][bone];double w=donor.weight*packed[24+k]/weights;for(unsigned j=0;j<12;j++)out[j]+=m[j]*w;total+=w;}
 }
 if(total<=0)throw std::invalid_argument("Cloth lineage empty");for(double& f:out){f/=total;if(!std::isfinite(f))throw std::invalid_argument("Nonfinite used garment native donor transform");}return out;
}
static std::shared_ptr<const K::Pose> CapturePose(LONG frame){
 auto pose=std::make_shared<K::Pose>();pose->serial=frame;pose->title=TankCameraSceneActive();
 if(!pose->title&&fluidWorldCameraFrame!=frame)return {};
 for(unsigned section=0;section<3;section++){if(paletteFrame[section]!=frame||!paletteCount[section]||sectionTarget[section]!=sectionTarget[2])return {};pose->count[section]=paletteCount[section];pose->actor[section]=pose->title?K::Identity():K::ActorFromShader(sectionLocal[section]);if(!pose->title){pose->actor[section][3]+=fluidCameraWorld.x;pose->actor[section][7]+=fluidCameraWorld.y;pose->actor[section][11]+=fluidCameraWorld.z;}for(unsigned bone=0;bone<pose->count[section];bone++)for(unsigned j=0;j<12;j++){double f=palettes[section][bone*12+j];pose->skin[section][bone][j]=f;}pose->inverseActor[section]=K::Inverse(pose->actor[section]);for(unsigned bone=0;bone<pose->count[section];bone++)pose->world[section][bone]=K::Multiply(pose->actor[section],pose->skin[section][bone]);}
 return pose;
}
struct SamplingMaps {
 struct Map{K::Matrix world,inverse;unsigned long long generation=0;};
 struct Entry{Map map;std::array<unsigned,4> bones{};std::array<double,4> weights{};double total=0;};
 struct Slot{unsigned long long key=0;unsigned entry=0;bool known=false;};
 std::array<std::vector<Slot>,3> slots;std::array<std::unordered_map<unsigned long long,unsigned>,3> tupleIds;std::array<std::vector<Entry>,3> entries;
 unsigned long long generation=0;unsigned composed=0;void Begin(){++generation;composed=0;}
 const Map& Get(G::Donor donor,const K::Pose& pose,const GeometrySnapshot& geometry){unsigned section=2,local=donor.vertex;const unsigned char* packed;const unsigned char* bones;if(donor.surface==G::Surface::Anatomy){packed=SnapshotPacked(geometry,donor);bones=packed+20;}else{int si;UINT nativeLocal;if(!MenuRetargetBodyVertex(donor.vertex,si,nativeLocal))throw std::invalid_argument("Cloth cached donor outside measured body");section=unsigned(si);local=nativeLocal;packed=sharedBodyBase[si]+local*32;bones=pose.title?packed+20:(si?fluidGameplayBones1:fluidGameplayBones0)+local*4;}unsigned b,w;memcpy(&b,bones,4);memcpy(&w,packed+24,4);auto key=static_cast<unsigned long long>(b)|(static_cast<unsigned long long>(w)<<32);
 // Revalidate native bytes per donor. Stable aliases go directly to a dense
 // tuple slot; changed title/gameplay skin metadata rebinds that donor exactly.
 if(slots[section].empty())slots[section].resize(section==2?nrCount:sharedBodyCount[section]);auto& slot=slots[section].at(local);
 if(!slot.known||slot.key!=key){auto found=tupleIds[section].find(key);unsigned index;if(found==tupleIds[section].end()){Entry entry;double total=0;for(unsigned k=0;k<4;k++)total+=packed[24+k];if(total<=0)throw std::invalid_argument("Cloth native donor weights empty");for(unsigned k=0;k<4;k++){entry.bones[k]=bones[k];if(packed[24+k]){entry.weights[k]=packed[24+k]/total;entry.total+=entry.weights[k];}}index=unsigned(entries[section].size());entries[section].push_back(entry);tupleIds[section].emplace(key,index);}else index=found->second;slot={key,index,true};}
 auto& entry=entries[section][slot.entry];if(entry.map.generation==generation)return entry.map;K::Matrix world{};for(unsigned k=0;k<4;k++)if(entry.weights[k]){if(entry.bones[k]>=pose.count[section])throw std::invalid_argument("Cloth cached native bone absent");for(unsigned j=0;j<12;j++)world[j]+=pose.world[section][entry.bones[k]][j]*entry.weights[k];}for(double& f:world){f/=entry.total;if(!std::isfinite(f))throw std::invalid_argument("Nonfinite used garment native cached transform");}entry.map={world,K::InverseLinear(world),generation};composed++;return entry.map;
 }
};
static void WorldSample(G::Sample& sample,const K::Pose& pose,const GeometrySnapshot* geometry=nullptr,SamplingMaps* cached=nullptr,unsigned donorCount=unsigned(G::Lineage{}.donors.size())){G::Point position{},normal{};double total=0;if(!donorCount||donorCount>sample.lineage.donors.size())throw std::invalid_argument("Native sampler support count differs from lineage capacity");
 // Interpolate individually skinned donors. Multiplying the interpolated
 // source point by an interpolated matrix introduces cross-donor terms.
 for(unsigned donorIndex=0;donorIndex<donorCount;donorIndex++){auto donor=sample.lineage.donors[donorIndex];if(donor.weight<=0)continue;const auto* ready=cached?&cached->Get(donor,pose,*geometry):nullptr;K::Matrix direct{};if(!ready){G::Lineage one;one.donors[0]={donor.surface,donor.vertex,1};direct=NativeMap(one,pose,true,geometry);}const auto& map=ready?ready->world:direct;const auto* packed=geometry?SnapshotPacked(*geometry,donor):donor.surface==G::Surface::Body?currentBody+donor.vertex*32:nrPacked+donor.vertex*32;position=G::Add(position,G::Mul(K::Point(map,Position(packed)),donor.weight));G::Point n;if(ready){auto p=Normal(packed);n={};for(unsigned i=0;i<3;i++)for(unsigned j=0;j<3;j++)n[i]+=ready->inverse[j*4+i]*p[j];n=G::Unit(n);}else n=K::Normal(map,Normal(packed));normal=G::Add(normal,G::Mul(n,donor.weight));total+=donor.weight;}
 if(total<=0)throw std::invalid_argument("Cloth sampled lineage empty");sample.position=G::Mul(position,1/total);sample.normal=G::Unit(normal);
}
static G::Sample Recipe(const JockstrapMeasuredSample& r,G::Surface surface,const GeometrySnapshot* geometry=nullptr){G::Sample sample;sample.normal={};for(unsigned k=0;k<4;k++){auto d=r.donors[k];if(d.weight<=0)continue;const auto* packed=geometry?SnapshotPacked(*geometry,{surface,d.vertex,d.weight}):surface==G::Surface::Body?currentBody+d.vertex*32:nrPacked+d.vertex*32;sample.position=G::Add(sample.position,G::Mul(Position(packed),d.weight));sample.normal=G::Add(sample.normal,G::Mul(Normal(packed),d.weight));sample.lineage.donors[k]={surface,d.vertex,d.weight};}return sample;}
template<std::size_t N> static void Contour(const JockstrapMeasuredSample(&recipe)[N],std::vector<G::Sample>& out,G::Surface surface,const GeometrySnapshot* geometry=nullptr){out.resize(N);for(unsigned i=0;i<N;i++)out[i]=Recipe(recipe[i],surface,geometry);}
// Topology and measured rest normals are immutable authored inputs. Normalize
// contact winding once; this never changes a native draw index buffer.
static const std::vector<std::array<std::uint32_t,3>>& BodyContactTopology(){
 static const auto faces=[](){std::vector<std::array<std::uint32_t,3>> result;const unsigned short* topology[2]={menuRetargetBodyIndices0,menuRetargetBodyIndices1};const unsigned count[2]={menuRetargetBodyTriangleCount0,menuRetargetBodyTriangleCount1};result.reserve(count[0]+count[1]);
 for(unsigned section=0,first=0;section<2;first+=sharedBodyCount[section++]){
  unsigned inward=0,outward=0,degenerate=0;for(unsigned triangle=0;triangle<count[section];triangle++){G::Point p[3],normal{};for(unsigned k=0;k<3;k++){const auto* packed=sharedBodyBase[section]+topology[section][triangle*3+k]*32;p[k]=Position(packed);normal=G::Add(normal,Normal(packed));}auto cross=G::Cross(G::Sub(p[1],p[0]),G::Sub(p[2],p[0]));if(G::Length(cross)<1e-12)degenerate++;else if(G::Dot(cross,normal)<0)inward++;else outward++;}
  if(inward==outward)throw std::invalid_argument("Native body surface orientation has no measured normal consensus");const bool reverse=inward>outward;
  // The observed native edges already have coherent manifold winding. A few
  // stock interpolated-normal outliers must not reverse isolated contact faces
  // and break volume closure. Orient the complete measured resource together.
  for(unsigned triangle=0;triangle<count[section];triangle++){std::array<std::uint32_t,3> face={first+topology[section][triangle*3],first+topology[section][triangle*3+1],first+topology[section][triangle*3+2]};if(reverse)std::swap(face[1],face[2]);result.push_back(face);}
  Log("Measured body contact resource=%u normalVotes inward=%u outward=%u degenerate=%u completeResourceReverse=%d; native draw indices unchanged",section,inward,outward,degenerate,reverse);
 }return result;}();return faces;
}
static void CompleteBodySurface(G::Input& target,const GeometrySnapshot* geometry=nullptr,bool lineageOnly=false){
 if(!currentBody&&!geometry)throw std::invalid_argument("Measured cloth body surface absent");
 target.bodySurface.resize(sharedBodyCount[0]+sharedBodyCount[1]);
 for(unsigned section=0,first=0;section<2;first+=sharedBodyCount[section++])for(unsigned local=0;local<sharedBodyCount[section];local++){auto& sample=target.bodySurface[first+local];const auto* packed=geometry?geometry->body[section].data()+local*32:currentBody+(sharedBodyFirst[section]+local)*32;sample=lineageOnly?G::Sample{}:G::Sample{Position(packed),Normal(packed),{}};sample.lineage.donors[0]={G::Surface::Body,sharedBodyFirst[section]+local,1};}
 target.bodyTriangles=BodyContactTopology();
}
template<std::size_t N> static void ContourLineage(const JockstrapMeasuredSample(&recipe)[N],std::vector<G::Sample>& out,G::Surface surface){out.resize(N);for(unsigned i=0;i<N;i++){out[i].lineage={};for(unsigned k=0;k<4;k++)out[i].lineage.donors[k]=recipe[i].donors[k].weight>0?G::Donor{surface,recipe[i].donors[k].vertex,recipe[i].donors[k].weight}:G::Donor{};}}
struct PreparationTiming{double lineageMilliseconds=0,worldMilliseconds=0;unsigned uniqueMatrices=0;};
static void PrepareSnapshotInput(G::Input& target,const K::Pose& pose,const GeometrySnapshot& geometry,PreparationTiming* timing=nullptr){
 auto began=std::chrono::steady_clock::now();
 const unsigned* regionRows[]={jockstrap_region_shaft,jockstrap_region_glans,jockstrap_region_leftLobe,jockstrap_region_rightLobe};const unsigned regionCounts[]={sizeof(jockstrap_region_shaft)/sizeof(unsigned),sizeof(jockstrap_region_glans)/sizeof(unsigned),sizeof(jockstrap_region_leftLobe)/sizeof(unsigned),sizeof(jockstrap_region_rightLobe)/sizeof(unsigned)};for(unsigned i=0;i<4;i++)target.anatomyRegions[i].assign(regionRows[i],regionRows[i]+regionCounts[i]);
 ContourLineage(jockstrap_waist,target.waist,G::Surface::Body);ContourLineage(jockstrap_opening,target.opening,G::Surface::Anatomy);ContourLineage(jockstrap_strapLeft,target.rearStraps[0],G::Surface::Body);ContourLineage(jockstrap_strapRight,target.rearStraps[1],G::Surface::Body);
 target.anatomy.resize(nrCount);for(unsigned vertex=0;vertex<nrCount;vertex++){target.anatomy[vertex]={};target.anatomy[vertex].lineage.donors[0]={G::Surface::Anatomy,vertex,1};}
 target.anatomyTriangles.resize(nrIndexCount/3);for(unsigned triangle=0;triangle<nrIndexCount/3;triangle++)target.anatomyTriangles[triangle]={nrIndices[triangle*3],nrIndices[triangle*3+2],nrIndices[triangle*3+1]};
 // Use the final unified collar's authored fine-to-coarse edge ownership.
 // Native skinning need not preserve geometric collinearity of packed fine
 // vertices. Only the virtual volume closure consumes these source bindings;
 // the complete native physical/rendered anatomy and body remain unchanged.
 target.rootSubdivisions.clear();
 for(unsigned k=0;k<ucSeamCount;k++){unsigned x=ucKeep[ucSeamVertices[k*3]],a=ucKeep[ucSeamVertices[k*3+1]],b=ucKeep[ucSeamVertices[k*3+2]];if(x>=nrCount||a>=nrCount||b>=nrCount)throw std::invalid_argument("Native collar source subdivision outside anatomical root");target.rootSubdivisions.push_back({x,a,b,ucSeamWeights[k]});}
 CompleteBodySurface(target,&geometry,true);
 // These are legacy thigh proxies used by the anatomy solver. Cloth receives
 // both complete measured body resources; inflated proxies would impose a
 // different obstacle on top of their actual skin and invalidate the drape.
 target.bodyContacts.clear();
 auto beforeWorld=std::chrono::steady_clock::now();static thread_local SamplingMaps cached;cached.Begin();// Full native tissue/body rows were reconstructed immediately above with
 // exactly one native donor. Only those rows use the bounded single-donor
 // input path; arbitrary material support/output lineage retains all16.
 for(auto* contour:{&target.waist,&target.opening,&target.anatomy,&target.rearStraps[0],&target.rearStraps[1],&target.bodySurface})for(auto& sample:*contour)WorldSample(sample,pose,&geometry,&cached,contour==&target.anatomy||contour==&target.bodySurface?1:unsigned(G::Lineage{}.donors.size()));
 if(timing){timing->lineageMilliseconds=std::chrono::duration<double,std::milli>(beforeWorld-began).count();timing->worldMilliseconds=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-beforeWorld).count();timing->uniqueMatrices=cached.composed;}
}
static const G::Output& InitializeMaterial(G::Session& session,const G::Input& current,const K::Pose& pose,const GeometrySnapshot& geometry){
 // Source positions and final collar recipes are already in model units.
 // Identity palettes recover that measured reference; no engine bone names
 // or guessed character dimensions enter the shared fitter.
 for(unsigned section=0;section<3;section++)for(unsigned axis=0;axis<3;axis++){
  G::Point basis{};basis[axis]=1;
  if(std::abs(G::Length(K::Direction(pose.actor[section],basis))-1)>1e-4)throw std::invalid_argument("Cloth rest placement requires a verified unit-scale actor");
 }
 K::Pose identity;identity.title=pose.title;identity.count=pose.count;
 for(unsigned section=0;section<3;section++){identity.actor[section]=identity.inverseActor[section]=K::Identity();for(unsigned bone=0;bone<identity.count[section];bone++)identity.skin[section][bone]=identity.world[section][bone]=K::Identity();}
 auto reference=current;reference.frame={};reference.frame.lateral={0,-1,0};reference.frame.forward={1,0,0};reference.frame.up={0,0,1};reference.deltaTime=0;reference.bodyContacts.clear();
 PrepareSnapshotInput(reference,identity,geometry);
 return session.InitializeDraped(reference,current);
}
static void SetStyle(unsigned value){style=value==1?G::Style::WhiteJockstrap:G::Style::Naked;styleGeneration++;snapshot.reset();solved.reset();ready=false;restKnown=false;activeClock.Reset();if(worker)worker->Clear();}
static unsigned GetStyle(){return unsigned(style);}
static const G::Output* latest=nullptr;
static void Refresh(){if(!worker||style==G::Style::Naked)return;static std::string reportedError;auto error=worker->Error();if(!error.empty()&&error!=reportedError){Log("Garment shared solver rejected coherent native request: %s",error.c_str());reportedError=error;}else if(error.empty())reportedError.clear();auto published=worker->PollDelivery(epoch);if(published){solved=std::move(published);snapshot=solved->output;latest=snapshot.get();ready=!snapshot->mesh.vertices.empty()&&snapshot->contactBudgetSatisfied&&snapshot->physics.materialBudgetSatisfied;static unsigned long long reportedSerial=0;if(solved->serial!=reportedSerial){reportedSerial=solved->serial;const auto& p=snapshot->physics;if(!snapshot->contactBudgetSatisfied||!p.materialBudgetSatisfied||solved->serial%120==1)Log("Cloth solver epoch=%llu serial=%llu active=%.3f advanced=%.6f steps=%u nodes=%u constraints=%u stretch=%.6f seam=%.6g contact=%d material=%d preparation=%.3fms shared=%.3fms worker=%.3fms reset=%d",epoch,solved->serial,solved->activeSeconds,p.advancedSeconds,p.substeps,p.nodes,p.constraints,p.maxStretchRatio,p.maxSeamGap,snapshot->contactBudgetSatisfied,p.materialBudgetSatisfied,solved->preparationMilliseconds,solved->solverMilliseconds,solved->milliseconds,solved->clockReset);}}else{snapshot.reset();solved.reset();latest=nullptr;ready=false;}}
static const G::Output* Latest(){Refresh();return ready?latest:nullptr;}
static malemod::surface::Frame::GarmentSupport LatestReaction(){Refresh();return ready&&solved?solved->reaction:malemod::surface::Frame::GarmentSupport{};}
static bool Update(const unsigned char* body,unsigned long long characterEpoch=1,G::Point measuredGravity={0,0,-72},const std::vector<G::Capsule>& measuredContacts={},std::array<float,10> morphology={},double anatomyMass=0,JockstrapCpuWorker::Reaction reaction={}){
 currentBody=body;if(!body)return false;DWORD now=GetTickCount();auto requestEpoch=characterEpoch^(styleGeneration<<32);if(epoch!=requestEpoch){snapshot.reset();solved.reset();latest=nullptr;ready=false;restKnown=false;}epoch=requestEpoch;
 try{input=G::Input{};input.characterEpoch=epoch;input.topologyRevision=1;input.frame={};input.frame.lateral={0,-1,0};input.frame.forward={1,0,0};input.frame.up={0,0,1};input.gravity=measuredGravity;input.bodyContacts=measuredContacts;
  input.anatomyMass=anatomyMass;
  if(style==G::Style::Naked){snapshot.reset();solved.reset();latest=nullptr;ready=false;activeClock.Suspend(now);return true;}
  auto pose=CapturePose(renderFrameSerial-1);if(!pose||pose->title){activeClock.Suspend(now);return false;}
  // Garment collision uses the actual completed body, not the anatomy solver's
  // deliberately conservative pelvic/thigh proxy volumes. Both native body
  // resources share the captured actor/skin pose and current morphology.
  auto geometry=CaptureGeometry(*pose);
  // Native bone matrices and translated LocalToWorld are measured together.
  // The existing same-frame absolute origin observation removes UE3 pre-view
  // translation; camera motion is absent from persistent world-space cloth.
  const unsigned section=pose->title?1:2,bone=pose->title?30:0;if(bone>=pose->count[section])throw std::invalid_argument("Measured cloth pelvis palette missing");
  auto pelvis=K::Multiply(pose->actor[section],pose->skin[section][bone]);
  input.frame.origin=K::Point(pelvis,{0,0,0});input.frame.up=G::Unit(K::Direction(pelvis,{0,0,1}));auto lateral=K::Direction(pelvis,{0,-1,0});input.frame.lateral=G::Unit(G::Sub(lateral,G::Mul(input.frame.up,G::Dot(lateral,input.frame.up))));input.frame.forward=G::Unit(G::Cross(input.frame.up,input.frame.lateral));
  // Preserve 72 source length units after the observed actor unit conversion.
  double scale=G::Length(K::Direction(pose->actor[section],{0,0,1}));input.gravity=G::Mul(measuredGravity,scale);
  for(auto& capsule:input.bodyContacts){capsule.a=K::Point(pelvis,capsule.a);capsule.b=K::Point(pelvis,capsule.b);capsule.radius*=scale;}
  if(!restKnown||restControls!=morphology){restControls=morphology;restKnown=true;restRevision++;}input.restRevision=restRevision;
  auto time=activeClock.Advance(now,epoch);if(!worker)worker=new JockstrapCpuWorker;if(time.reset){worker->Clear();snapshot.reset();solved.reset();latest=nullptr;ready=false;}
  worker->Submit(std::make_shared<G::Input>(input),std::move(pose),time.activeSeconds,[geometry](G::Input& target,const K::Pose& captured){PrepareSnapshotInput(target,captured,*geometry);},std::move(reaction),[geometry](G::Session& session,const G::Input& current,const K::Pose& captured)->const G::Output&{return InitializeMaterial(session,current,captured,*geometry);});Refresh();return true;
 }catch(const std::exception& e){ready=false;Log("Garment source update rejected: %s",e.what());return false;}
}
// Capture at each verified native body/anatomy draw. Frame tags prevent mixing
// previous camera/body palettes into a present-frame cloth pose.
static void CaptureSection(IDirect3DDevice9* d,int section){if(style==G::Style::Naked||section<0||section>2)return;DWORD color=0;if(FAILED(d->GetRenderState(D3DRS_COLORWRITEENABLE,&color))||!(color&7)||!IsHdrSceneColorPass(d))return;auto* layout=GetShaderLayout(d);if(!layout||!layout->valid||!layout->viewValid)return;UINT count=(std::min)(75u,layout->boneCount/3);if(!count||FAILED(d->GetVertexShaderConstantF(layout->boneRegister,palettes[section],count*3)))return;if(FAILED(d->GetVertexShaderConstantF(layout->localRegister,localMatrix,4))||FAILED(d->GetVertexShaderConstantF(layout->viewRegister,viewMatrix,4)))return;memcpy(sectionLocal[section],localMatrix,64);memcpy(sectionView[section],viewMatrix,64);IDirect3DSurface9* target=nullptr;if(FAILED(d->GetRenderTarget(0,&target))||!target)return;if(sectionTarget[section])sectionTarget[section]->Release();sectionTarget[section]=target;paletteCount[section]=count;paletteFrame[section]=renderFrameSerial;}
static bool Pose(const G::Vertex& v,RenderVertex& out){
 if(solved&&solved->pose)try{const auto& current=displayPose;if(!current||current->serial!=static_cast<unsigned long long>(renderFrameSerial))return false;auto bind=K::Inverse(NativeMap(v.lineage,*solved->pose,true));auto map=K::Multiply(current->inverseActor[2],K::Multiply(NativeMap(v.lineage,*current,true),bind));auto p=K::Point(map,v.position),n=K::Normal(map,v.normal);for(unsigned k=0;k<3;k++){out.p[k]=float(p[k]);out.n[k]=float(n[k]);}out.uv[0]=float(v.uv[0]);out.uv[1]=float(v.uv[1]);return true;}catch(const std::exception&){return false;}
 G::Point point{},normal{};double total=0;bool title=TankCameraSceneActive();
 for(auto donor:v.lineage.donors){if(donor.weight<=0)continue;const unsigned char* packed=nullptr;const unsigned char* overrideBones=nullptr;unsigned section=2;
  if(donor.surface==G::Surface::Anatomy){if(donor.vertex>=nrCount)return false;packed=(title?menuTankPacked:nrPacked)+donor.vertex*32;}
  else{int s;UINT local;if(!MenuRetargetBodyVertex(donor.vertex,s,local))return false;section=unsigned(s);packed=sharedBodyBase[s]+local*32;if(!title)overrideBones=(s?fluidGameplayBones1:fluidGameplayBones0)+local*4;}
  if(paletteFrame[section]!=renderFrameSerial)return false;
  double weightTotal=0;for(unsigned k=0;k<4;k++){unsigned bone=overrideBones?overrideBones[k]:packed[20+k];double weight=packed[24+k]/255.;if(weight<=0)continue;if(bone>=paletteCount[section])return false;auto* matrix=palettes[section]+bone*12;double w=weight*donor.weight;
   G::Point p{},n{};for(unsigned r=0;r<3;r++){p[r]=matrix[r*4]*v.position[0]+matrix[r*4+1]*v.position[1]+matrix[r*4+2]*v.position[2]+matrix[r*4+3];n[r]=matrix[r*4]*v.normal[0]+matrix[r*4+1]*v.normal[1]+matrix[r*4+2]*v.normal[2];}point=G::Add(point,G::Mul(p,w));normal=G::Add(normal,G::Mul(n,w));total+=w;weightTotal+=weight;
  }if(weightTotal<.5)return false;
 }
 if(total<.5||G::Length(normal)<1e-12)return false;point=G::Mul(point,1/total);normal=G::Unit(normal);for(unsigned k=0;k<3;k++){out.p[k]=float(point[k]);out.n[k]=float(normal[k]);}out.uv[0]=float(v.uv[0]);out.uv[1]=float(v.uv[1]);return true;
}
static void Release(){ready=false;latest=nullptr;drawnFrame=-2;if(worker){worker->Stop();delete worker;worker=nullptr;}snapshot.reset();solved.reset();displayPose.reset();activeClock.Reset();restKnown=false;styleGeneration++;input=G::Input{};for(auto& target:sectionTarget){if(target)target->Release();target=nullptr;}for(auto& frame:paletteFrame)frame=-2;memset(paletteCount,0,sizeof(paletteCount));if(vertexShader){vertexShader->Release();vertexShader=nullptr;}if(pixelShader){pixelShader->Release();pixelShader=nullptr;}if(declaration){declaration->Release();declaration=nullptr;}if(stateBlock){stateBlock->Release();stateBlock=nullptr;}rendered.clear();indices.clear();}
static void Suspend(){activeClock.Suspend(GetTickCount());}
static G::Point SourceAcceleration(G::Point world){if(!solved||!solved->pose)return world;auto& pose=*solved->pose;unsigned section=pose.title?1:2,bone=pose.title?30:0;return K::Direction(K::Inverse(K::Multiply(pose.actor[section],pose.skin[section][bone])),world);}
static bool EnsureShaders(IDirect3DDevice9* d){if(vertexShader&&pixelShader&&declaration&&stateBlock)return true;
 const char* vs="float4 L[4]:register(c0);float4 V[4]:register(c4);struct I{float3 p:POSITION;float3 n:NORMAL;float2 uv:TEXCOORD0;float4 c:COLOR0;};struct O{float4 p:POSITION;float3 n:TEXCOORD1;float2 uv:TEXCOORD0;float4 c:COLOR0;};O main(I i){O o;float4 w=i.p.x*L[0]+i.p.y*L[1]+i.p.z*L[2]+L[3];o.p=w.x*V[0]+w.y*V[1]+w.z*V[2]+w.w*V[3];o.n=normalize(i.n);o.uv=i.uv;o.c=i.c;return o;}";
 const char* ps="float4 tint:register(c0);float4 main(float3 n:TEXCOORD1,float2 uv:TEXCOORD0,float4 c:COLOR0):COLOR0{float knit=1+.018*cos(uv.x*804.2477)+.004*cos(uv.y*1608.4954);float lighting=.76+.24*abs(dot(normalize(n),normalize(float3(.35,.55,.76))));return float4(tint.rgb*knit*lighting,tint.a);}";
 ID3DXBuffer *code=nullptr,*errors=nullptr;HRESULT hr=D3DXCompileShader(vs,(UINT)strlen(vs),nullptr,nullptr,"main","vs_3_0",0,&code,&errors,nullptr);if(errors){Log("Garment VS: %s",(char*)errors->GetBufferPointer());errors->Release();}if(FAILED(hr))return false;hr=d->CreateVertexShader((DWORD*)code->GetBufferPointer(),&vertexShader);code->Release();if(FAILED(hr))return false;code=nullptr;errors=nullptr;hr=D3DXCompileShader(ps,(UINT)strlen(ps),nullptr,nullptr,"main","ps_3_0",0,&code,&errors,nullptr);if(errors){Log("Garment PS: %s",(char*)errors->GetBufferPointer());errors->Release();}if(FAILED(hr))return false;hr=d->CreatePixelShader((DWORD*)code->GetBufferPointer(),&pixelShader);code->Release();if(FAILED(hr))return false;
 D3DVERTEXELEMENT9 elements[]={{0,0,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},{0,12,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_NORMAL,0},{0,24,D3DDECLTYPE_FLOAT2,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TEXCOORD,0},{0,32,D3DDECLTYPE_FLOAT4,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_COLOR,0},D3DDECL_END()};return SUCCEEDED(d->CreateVertexDeclaration(elements,&declaration))&&SUCCEEDED(d->CreateStateBlock(D3DSBT_ALL,&stateBlock));
}
static void Draw(IDirect3DDevice9* d){Refresh();if(!ready||!latest||style==G::Style::Naked||drawnFrame==renderFrameSerial)return;DWORD color;d->GetRenderState(D3DRS_COLORWRITEENABLE,&color);if(!(color&7)||!EnsureShaders(d))return;if(solved&&solved->pose){try{displayPose=CapturePose(renderFrameSerial);}catch(const std::exception&){return;}if(!displayPose){static LONG reported=-2;if(reported!=renderFrameSerial&&renderFrameSerial%120==0){reported=renderFrameSerial;Log("Cloth draw awaiting coherent HDR pose frame=%ld palette=%ld,%ld,%ld origin=%ld",renderFrameSerial,paletteFrame[0],paletteFrame[1],paletteFrame[2],fluidWorldCameraFrame);}return;}}IDirect3DSurface9* target=nullptr;d->GetRenderTarget(0,&target);bool sameTarget=target==sectionTarget[2];if(target)target->Release();if(sectionTarget[2]&&!sameTarget)return;const auto& mesh=latest->mesh;rendered.resize(mesh.vertices.size());indices.resize(mesh.triangles.size()*3);for(unsigned i=0;i<mesh.vertices.size();i++)if(!Pose(mesh.vertices[i],rendered[i]))return;for(auto& v:rendered){v.color[0]=v.color[1]=v.color[2]=v.color[3]=1;}
 if(FAILED(stateBlock->Capture()))return;
 d->SetVertexShader(vertexShader);d->SetPixelShader(pixelShader);d->SetVertexDeclaration(declaration);
 d->SetVertexShaderConstantF(0,sectionLocal[2],4);d->SetVertexShaderConstantF(4,sectionView[2],4);
 d->SetRenderState(D3DRS_ZENABLE,TRUE);d->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
 d->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
 d->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);d->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
 d->SetRenderState(D3DRS_STENCILENABLE,FALSE);d->SetRenderState(D3DRS_COLORWRITEENABLE,0);
 // Select the nearest actual cloth surface before blending. Two-sided fabric
 // must not compound its 95% opacity by also blending the hidden back surface.
 unsigned used=0;for(const auto& triangle:mesh.triangles)for(auto vertex:triangle.vertices)indices[used++]=(unsigned short)vertex;
 HRESULT hr=d->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,(UINT)rendered.size(),used/3,indices.data(),D3DFMT_INDEX16,rendered.data(),sizeof(RenderVertex));
 d->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);d->SetRenderState(D3DRS_ZFUNC,D3DCMP_EQUAL);
 d->SetRenderState(D3DRS_COLORWRITEENABLE,15);d->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
 d->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);d->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
 d->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);d->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,TRUE);
 d->SetRenderState(D3DRS_SRCBLENDALPHA,D3DBLEND_ONE);d->SetRenderState(D3DRS_DESTBLENDALPHA,D3DBLEND_INVSRCALPHA);
 d->SetRenderState(D3DRS_BLENDOPALPHA,D3DBLENDOP_ADD);
 for(unsigned material=0;material<4&&SUCCEEDED(hr);material++){
  used=0;for(const auto& triangle:mesh.triangles)if(unsigned(triangle.material)==material)for(auto vertex:triangle.vertices)indices[used++]=(unsigned short)vertex;
  if(!used)continue;const auto& clothMaterial=latest->materials[material];const auto& color=clothMaterial.color;
  float tint[4]={float(color[0]),float(color[1]),float(color[2]),float(clothMaterial.opacity)};
  d->SetPixelShaderConstantF(0,tint,1);
  hr=d->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,(UINT)rendered.size(),used/3,indices.data(),D3DFMT_INDEX16,rendered.data(),sizeof(RenderVertex));
 }
 stateBlock->Apply();if(SUCCEEDED(hr))drawnFrame=renderFrameSerial;
}
}
