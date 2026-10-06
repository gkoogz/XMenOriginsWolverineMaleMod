// Offline partitioned source/cloth integration. Actual canonical native
// sampling and current extracted source physics; no game, install or device.
#include "../src/runtime/d3d9_proxy.cpp"
#include <malemod/surface/wire.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
namespace G=malemod::garments;
namespace S=malemod::surface;
namespace A=JockstrapAdapter;
#include "garment_input_json.hpp"
static G::Point GP(S::Point p){return {p.x,p.y,p.z};}
static void Require(bool c,const char* why){if(!c)throw std::runtime_error(why);}
static void Pack(const S::Output& out,std::vector<unsigned char>& body){
 Require(out.anatomy.positions.size()==nrCount&&out.anatomy.normals.size()==nrCount&&out.anatomyIndices.size()==nrIndexCount,"Complete source anatomy size changed");
 Require(std::equal(out.anatomyIndices.begin(),out.anatomyIndices.end(),nrIndices),"Complete source topology changed");
 for(unsigned section=0;section<2;section++){
  Require(out.body[section].positions.size()==sharedBodyCount[section],"Complete source body resource changed");
  for(unsigned i=0;i<sharedBodyCount[section];i++){
   auto* packed=body.data()+(sharedBodyFirst[section]+i)*32;
   memcpy(packed,sharedBodyBase[section]+i*32,32);memcpy(packed,&out.body[section].positions[i],12);
  }
 }
 for(unsigned i=0;i<nrCount;i++){
  auto* packed=nrPacked+i*32;memcpy(packed,&out.anatomy.positions[i],12);memcpy(packed+12,nrAttributes+i*20,20);
  const auto n=GP(out.anatomy.normals[i]);for(unsigned j=0;j<3;j++)packed[16+j]=static_cast<unsigned char>(std::clamp(std::lround((n[j]+1)*127.5),0l,255l));
 }
 A::currentBody=body.data();
}
static JockstrapKinematics::Pose Pose(G::Point offset,unsigned serial){
 namespace K=JockstrapKinematics;K::Pose pose;pose.serial=serial;
 for(unsigned section=0;section<3;section++){
  pose.count[section]=75;pose.actor[section]=K::Identity();pose.actor[section][3]=offset[0];pose.actor[section][7]=offset[1];pose.actor[section][11]=offset[2];pose.inverseActor[section]=K::Inverse(pose.actor[section]);
  for(unsigned bone=0;bone<75;bone++){pose.skin[section][bone]=K::Identity();pose.world[section][bone]=pose.actor[section];}
 }
 return pose;
}
static G::Input Input(const S::Output& surface,std::vector<unsigned char>& body,G::Point offset,unsigned serial,const S::Controls& controls,double dt){
 Pack(surface,body);auto pose=Pose(offset,serial);auto geometry=A::CaptureGeometry(pose);G::Input input;
 input.characterEpoch=1;input.topologyRevision=1;input.restRevision=1;input.deltaTime=dt;input.frame.origin=offset;input.frame.lateral={0,-1,0};input.frame.forward={1,0,0};input.frame.up={0,0,1};input.gravity={0,0,-72};input.anatomyMass=G::CurrentSourceMasses(controls).Total();
 A::PrepareSnapshotInput(input,pose,*geometry);
 Require(input.anatomy.size()==17528&&input.bodySurface.size()==20783&&input.anatomyTriangles.size()==35000,"Native source sampling omitted physical tissue");return input;
}
static void Vec(std::ostream& out,G::Point p){out<<'['<<p[0]<<','<<p[1]<<','<<p[2]<<']';}
int main(int argc,char** argv){
 std::ofstream trace;unsigned completed=0;
 try{
  Require(argc==5,"usage: garment_source_coupling_test overall(25/50/75/100) feedback(0/1) frames report-directory");
  const unsigned size=std::stoul(argv[1]),feedback=std::stoul(argv[2]),frames=std::stoul(argv[3]);Require((size==25||size==50||size==75||size==100)&&feedback<=1&&frames>=1&&frames<=600,"Closed-loop test arguments outside contract");
  std::filesystem::path report(argv[4]);std::filesystem::create_directories(report);trace.open(report/"frames.jsonl");trace.precision(17);
  S::Controls controls;controls.values[1]=float(size);S::Session source(controls);S::Frame sourceFrame;
  for(unsigned i=0;i<120;i++)source.Step(sourceFrame);auto surface=source.Read();
  std::vector<unsigned char> body((sharedBodyFirst[1]+sharedBodyCount[1])*32);G::Session cloth;S::GarmentImpulseLedger ledger;ledger.Reset(1);S::Frame::GarmentSupport publication;
  auto initial=Input(surface,body,{},0,controls,0);WriteCanonicalGarmentInput(report/"rest-native-input.json",initial);const auto& fitted=cloth.Update(G::Style::WhiteJockstrap,initial);
  {std::ofstream rest(report/"rest.json");rest.precision(17);rest<<"{\"fullAnatomyVertices\":"<<initial.anatomy.size()<<",\"bodyVertices\":"<<initial.bodySurface.size()<<",\"anatomyMass\":"<<initial.anatomyMass<<",\"coverageMargin\":"<<fitted.coverageMargin<<",\"contactsSatisfied\":"<<(fitted.contactBudgetSatisfied?"true":"false")<<",\"garmentVertices\":"<<fitted.mesh.vertices.size()<<",\"garmentFaces\":"<<fitted.mesh.triangles.size()<<",\"sourceWarmupFrames\":120,\"observedGameplay\":false}\n";}
  Require(fitted.contactBudgetSatisfied&&fitted.coverageMargin>=0,"Actual complete source rest cloth fails coverage/contact");
  for(unsigned frame=0;frame<frames;frame++){
   const double t=(frame+1)/60.;
   // A prescribed authoring translation and inertial forcing, not captured gait.
   const G::Point offset{0,1.2*std::sin(t*7),.7*std::sin(t*11)};
   sourceFrame.seconds=1.f/60;sourceFrame.pitchForce=float(.10*std::sin(t*7));sourceFrame.yawForce=float(.08*std::sin(t*11));
   // Coalesce publications every third source frame. The ledger keeps all
   // completed impulses. Every fifth frame first queues the same CDF at dt=0.
   if(feedback&&frame%3==0)sourceFrame.garment=publication;else if(!feedback)sourceFrame.garment={};
   if(feedback&&frame%5==0){auto pending=sourceFrame;pending.seconds=0;source.Step(pending);}
   auto began=std::chrono::steady_clock::now();source.Step(sourceFrame);surface=source.Read();auto evaluated=std::chrono::steady_clock::now();auto input=Input(surface,body,offset,frame+1,controls,1./60);auto prepared=std::chrono::steady_clock::now();const auto& output=cloth.Update(G::Style::WhiteJockstrap,input);auto advanced=std::chrono::steady_clock::now();
   auto binding=[](G::Donor d){G::MechanicalBinding b;Require(d.vertex<nrCount,"Reaction source donor outside actual anatomy");std::copy_n(jockstrapReactionBindings[d.vertex],14,b.weights.begin());return b;};
   std::array<G::Point,2> centers{GP(surface.lobeCenters[0]),GP(surface.lobeCenters[1])},radii{GP(surface.lobeRadii[0]),GP(surface.lobeRadii[1])};
   auto impulse=G::AggregateContactReactions(output,binding,[](G::Point p){return p;},G::Mul(offset,-1),centers,radii,G::CurrentSourceMasses(controls));publication=ledger.Append(impulse);
   const auto finished=std::chrono::steady_clock::now();const double elapsed=std::chrono::duration<double,std::milli>(finished-began).count();
   const double sourceMs=std::chrono::duration<double,std::milli>(evaluated-began).count(),prepareMs=std::chrono::duration<double,std::milli>(prepared-evaluated).count(),clothMs=std::chrono::duration<double,std::milli>(advanced-prepared).count(),reactionMs=std::chrono::duration<double,std::milli>(finished-advanced).count();
   trace<<"{\"frame\":"<<frame<<",\"activeSeconds\":"<<t<<",\"sourceHz\":60,\"clothHz\":120,\"publicationSerial\":"<<publication.contactSerial<<",\"consumedSerial\":"<<sourceFrame.garment.contactSerial<<",\"materialResets\":"<<output.physics.resetCount<<",\"clothSubsteps\":"<<output.physics.substeps<<",\"contactRecords\":"<<output.reaction.records<<",\"clothMass\":"<<output.reaction.clothMass<<",\"reactionSeconds\":"<<output.reaction.activeSeconds<<",\"coverageMargin\":"<<output.coverageMargin<<",\"contactsSatisfied\":"<<(output.contactBudgetSatisfied?"true":"false")<<",\"materialSatisfied\":"<<(output.physics.materialBudgetSatisfied?"true":"false")<<",\"fineStretchRatio\":"<<output.physics.maxRenderStretchRatio<<",\"worstRenderA\":"<<output.physics.worstRenderA<<",\"worstRenderB\":"<<output.physics.worstRenderB<<",\"stepMilliseconds\":"<<elapsed<<",\"sourceStepReadMilliseconds\":"<<sourceMs<<",\"prepareMilliseconds\":"<<prepareMs<<",\"clothUpdateMilliseconds\":"<<clothMs<<",\"reactionMilliseconds\":"<<reactionMs<<",\"clothSolverMilliseconds\":"<<output.physics.solverMilliseconds<<",\"anatomyImpulse\":";Vec(trace,output.reaction.anatomyImpulse);trace<<",\"freeParticleImpulse\":";Vec(trace,output.reaction.particleImpulse);trace<<",\"freeParticleMoment\":";Vec(trace,output.reaction.particleMoment);trace<<",\"prescribedFrameSupportMoment\":";Vec(trace,output.reaction.supportMoment);trace<<",\"sourceTip\":";Vec(trace,GP(surface.shaftGuide.back()));trace<<",\"sourceLobes\":[";Vec(trace,centers[0]);trace<<',';Vec(trace,centers[1]);trace<<"]}\n";trace.flush();completed++;
   for(auto p:surface.shaftGuide)Require(G::Finite(GP(p))&&G::Length(GP(p))<1e6,"Closed-loop source runaway");
   Require(output.physics.substeps==2,"Cloth did not advance two actual120Hz substeps");
   const bool healthy=output.contactBudgetSatisfied&&output.physics.materialBudgetSatisfied&&output.physics.maxRenderStretchRatio<=1.15;
   if(!healthy)WriteCanonicalGarmentInput(report/"failure-native-input.json",input);
   Require(healthy,"Closed-loop cloth physical/material budget failed");
   if(frame%8==0)std::cout<<"frame "<<frame<<" contacts "<<output.reaction.records<<" fine strain "<<output.physics.maxRenderStretchRatio<<" elapsed "<<elapsed<<"ms\n";
  }
  const auto encoded=S::wire::Encode(surface);std::ofstream binary(report/"final-source.bin",std::ios::binary);binary.write(reinterpret_cast<const char*>(encoded.data()),encoded.size());Require(bool(binary),"Final actual source receipt write failed");
  std::ofstream result(report/"result.json");result<<"{\"wire\":6,\"overall\":"<<size<<",\"feedback\":"<<feedback<<",\"completedFrames\":"<<completed<<",\"fullAnatomyVertices\":17528,\"bodyVertices\":20783,\"sourceHz\":60,\"clothHz\":120,\"coalescingEvery\":3,\"pendingZeroStepEvery\":5,\"nonlinearSurfaceJacobian\":false,\"ribbonOffsetProjection\":\"analytic rendered/material-frame gradients; prescribed frame moment reported separately\",\"observedGameplay\":false,\"passed\":true}\n";
  return 0;
 }catch(const std::exception& error){std::cerr<<"Closed-loop completed "<<completed<<" frames: "<<error.what()<<'\n';return 1;}
}
