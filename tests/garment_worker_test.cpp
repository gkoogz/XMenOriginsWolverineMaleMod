#include "../src/runtime/jockstrap_worker.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <future>
#include <atomic>
namespace G=malemod::garments;
static G::Sample S(G::Point p,unsigned i){G::Sample s{p,{0,1,0},{}};s.lineage.donors[0]={G::Surface::Anatomy,i,1};return s;}
static std::shared_ptr<G::Input> Fixture(unsigned long long epoch){auto in=std::make_shared<G::Input>();in->characterEpoch=epoch;in->topologyRevision=1;in->gravity={0,0,-9.81};for(unsigned i=0;i<24;i++){double t=i*6.283185307179586/24;in->waist.push_back(S({.2*cos(t),.15*sin(t),1},i));in->opening.push_back(S({.05*cos(t),.12,.7+.05*sin(t)},i));}for(unsigned j=0;j<24;j++)for(unsigned i=0;i<48;i++){double p=3.141592653589793*(j+.1)/24,t=i*6.283185307179586/48;in->anatomy.push_back(S({.05*sin(p)*cos(t),.2+.15*sin(p)*sin(t),.65+.1*cos(p)},i+j*48));}for(unsigned side=0;side<2;side++){double s=side?1:-1;in->rearStraps[side]={S({s*.15,-.1,.99},0),S({s*.13,-.1,.7},1),S({s*.05,.12,.62},2)};}return in;}
static std::shared_ptr<const G::Output> Wait(JockstrapCpuWorker& worker,unsigned long long epoch){auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);while(std::chrono::steady_clock::now()<until){auto result=worker.Poll(epoch);if(result)return result;auto error=worker.Error();if(!error.empty())throw std::runtime_error(error);std::this_thread::sleep_for(std::chrono::milliseconds(1));}throw std::runtime_error("Garment worker timed out at epoch "+std::to_string(epoch));}
int main(){try{
 JockstrapKinematics::Clock clock;auto tick=clock.Advance(1000,1);if(!tick.reset||tick.activeSeconds)throw std::runtime_error("Clock init differs");tick=clock.Advance(1100,1);if(fabs(tick.activeSeconds-.1)>1e-10)throw std::runtime_error("Clock active span lost");clock.Suspend(5000);tick=clock.Advance(9000,1);if(tick.reset||fabs(tick.activeSeconds-.1)>1e-10)throw std::runtime_error("Suspended wall time caught up");tick=clock.Advance(9025,1);if(fabs(tick.activeSeconds-.125)>1e-10)throw std::runtime_error("Resume loses active span");tick=clock.Advance(12000,1);if(tick.reset||fabs(tick.activeSeconds-3.1)>1e-10)throw std::runtime_error("Render hitch discarded active time or reset material");tick=clock.Advance(12025,1);if(tick.reset||fabs(tick.activeSeconds-3.125)>1e-10)throw std::runtime_error("Clock lost continuity after render hitch");
clock.Reset();clock.Advance(0xfffffff0u,2);tick=clock.Advance(0x20u,2);if(tick.reset||fabs(tick.activeSeconds-.048)>1e-10)throw std::runtime_error("Native millisecond clock wrap lost time");
 JockstrapCpuWorker worker;auto input=Fixture(42);auto before=std::chrono::steady_clock::now();worker.Submit(input);double submit=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-before).count();auto first=Wait(worker,42);if(first->mesh.vertices.empty()||submit>5)throw std::runtime_error("Generation blocked submit");for(unsigned i=0;i<128;i++)worker.Submit(Fixture(43+i));auto final=Wait(worker,170);if(final->characterEpoch!=170||worker.Poll(42))throw std::runtime_error("Stale epoch published");auto retained=final;worker.Clear();if(worker.Poll(170)||retained->mesh.vertices.empty())throw std::runtime_error("Clear corrupted immutable snapshot");worker.Submit(Fixture(170));auto resumed=Wait(worker,170);if(resumed->mesh.vertices.size()!=retained->mesh.vertices.size())throw std::runtime_error("Topology changed across lifecycle");auto previous=worker.PollDelivery(170);auto current=Fixture(170);for(unsigned i=1;i<=12;i++)worker.Submit(current,{},double(i)/120);
 auto until=std::chrono::steady_clock::now()+std::chrono::seconds(5);std::shared_ptr<const JockstrapCpuWorker::Delivery> completed;do{completed=worker.PollDelivery(170);if(completed&&fabs(completed->activeSeconds-.1)<1e-10)break;std::this_thread::sleep_for(std::chrono::milliseconds(1));}while(std::chrono::steady_clock::now()<until);if(!completed||fabs(completed->output->physics.accumulatedSeconds-.1)>1e-8||completed->serial<=previous->serial)throw std::runtime_error("Overwritten cloth requests lose cumulative active time");// Hold conversion in-flight, clear the epoch, and replace pending work twice.
 // The completed pose/clock pair must survive; the cancelled result must not.
 auto pose=std::make_shared<JockstrapKinematics::Pose>();pose->serial=912;
 std::promise<void> entered,release;auto released=release.get_future().share();auto caller=std::this_thread::get_id();std::atomic<unsigned> executions{0};worker.Clear();
 worker.Submit(Fixture(171),pose,0,[&](G::Input& in,const JockstrapKinematics::Pose& captured){if(std::this_thread::get_id()==caller||captured.serial!=912||in.characterEpoch!=171)throw std::runtime_error("Deferred conversion is on SDK thread or has wrong pose");entered.set_value();released.wait();executions++;});
 if(entered.get_future().wait_for(std::chrono::seconds(2))!=std::future_status::ready)throw std::runtime_error("Deferred worker did not begin");worker.Clear();
 worker.Submit(Fixture(172),pose,.025,[&](G::Input&,const JockstrapKinematics::Pose&){throw std::runtime_error("Overwritten conversion executed");});
 worker.Submit(Fixture(173),pose,.05,[&](G::Input& in,const JockstrapKinematics::Pose& captured){if(std::this_thread::get_id()==caller||captured.serial!=912||in.characterEpoch!=173)throw std::runtime_error("Latest deferred input lost coherent pose");executions++;});release.set_value();auto newest=Wait(worker,173);auto paired=worker.PollDelivery(173);if(worker.Poll(171)||worker.Poll(172)||executions!=2||!paired||paired->pose!=pose||fabs(paired->activeSeconds-.05)>1e-12||!paired->clockReset||paired->preparationMilliseconds<0||paired->solverMilliseconds<0)throw std::runtime_error("Deferred conversion cancellation/coherence/timing failed");
 // Header-only production requests reuse vector storage, but must retain the
 // newest scalar controls/pose/epoch. A complete caller-owned input is never
 // replaced with recycled geometry (the earlier tests exercise that path).
 worker.Clear();std::size_t anatomyCapacity=0,bodyCapacity=0;unsigned recycled=0;
 auto prepareHeader=[&](G::Input& in,const JockstrapKinematics::Pose& captured){
  if(captured.serial!=912||in.characterEpoch!=174+recycled||in.restRevision!=31+recycled||in.gravity[2]!=-9.81-double(recycled)||in.bodyContacts.size()!=1||in.bodyContacts[0].radius!=.01+double(recycled)*.001)throw std::runtime_error("Recycled geometry overwrote current request header");
  if(recycled&&(in.anatomy.capacity()!=anatomyCapacity||in.bodySurface.capacity()!=bodyCapacity))throw std::runtime_error("Deferred worker discarded reusable geometry capacity");
  auto fixture=Fixture(in.characterEpoch);in.waist.assign(fixture->waist.begin(),fixture->waist.end());in.opening.assign(fixture->opening.begin(),fixture->opening.end());in.anatomy.assign(fixture->anatomy.begin(),fixture->anatomy.end());in.rearStraps=fixture->rearStraps;
  // Reserve storage without adding synthetic collision geometry.
  in.bodySurface.reserve(2500);anatomyCapacity=in.anatomy.capacity();bodyCapacity=in.bodySurface.capacity();++recycled;
 };
 for(unsigned i=0;i<2;i++){auto header=std::make_shared<G::Input>();header->characterEpoch=174+i;header->topologyRevision=1;header->restRevision=31+i;header->gravity={0,0,-9.81-double(i)};G::Capsule capsule;capsule.a={9,9,9};capsule.b={10,10,10};capsule.radius=.01+double(i)*.001;header->bodyContacts.push_back(capsule);worker.Submit(header,pose,0,prepareHeader);auto output=Wait(worker,174+i);if(output->mesh.vertices.empty()||!header->anatomy.empty()||!header->bodySurface.empty())throw std::runtime_error("Deferred recycling changed immutable caller input");}
 if(recycled!=2)throw std::runtime_error("Deferred recycle test did not execute both requests");
 worker.Clear();std::atomic<unsigned> initializations{0};
 JockstrapCpuWorker::Initialize initialize=[&](G::Session& session,const G::Input& in,const JockstrapKinematics::Pose& captured)->const G::Output&{
  if(captured.serial!=912||std::this_thread::get_id()==caller)throw std::runtime_error("Material placement lost worker/pose ownership");
  initializations++;return session.Initialize(in,in,[](const G::Sample& source){return source;});
 };
 auto placed=Fixture(180);placed->restRevision=9;worker.Submit(placed,pose,0,{}, {},initialize);Wait(worker,180);
 auto waitSerial=[&](unsigned long long previous){auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(std::chrono::steady_clock::now()<deadline){auto delivery=worker.PollDelivery(180);if(delivery&&delivery->serial>previous)return delivery;auto error=worker.Error();if(!error.empty())throw std::runtime_error(error);std::this_thread::sleep_for(std::chrono::milliseconds(1));}throw std::runtime_error("Placed material update timed out");};
 auto placedFirst=worker.PollDelivery(180);worker.Submit(placed,pose,1./120,{}, {},initialize);auto placedNext=waitSerial(placedFirst->serial);
 if(initializations!=1||placedNext->output->physics.reset||placedNext->output->physics.substeps!=1)throw std::runtime_error("Animation recreated reference material");
 auto morphology=std::make_shared<G::Input>(*placed);morphology->restRevision++;worker.Submit(morphology,pose,2./120,{}, {},initialize);waitSerial(placedNext->serial);
 if(initializations!=2)throw std::runtime_error("Explicit rest revision did not replace reference material");
 // Material construction is not gameplay time: a delayed first fit must not
 // turn queued wall time into seconds of cloth integration on publication.
 worker.Clear();std::promise<void> fitting,finishFit;auto finish=finishFit.get_future().share();
 auto delayed=[&](G::Session& session,const G::Input& in,const JockstrapKinematics::Pose&)->const G::Output&{fitting.set_value();finish.wait();return session.Initialize(in,in,[](const G::Sample& source){return source;});};
 auto fitInput=Fixture(181);worker.Submit(fitInput,pose,0,{}, {},delayed);
 if(fitting.get_future().wait_for(std::chrono::seconds(2))!=std::future_status::ready)throw std::runtime_error("Delayed material test did not enter initialization");
 worker.Submit(fitInput,pose,10,{}, {},delayed);finishFit.set_value();
 auto waitTime=[&](double time){auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(std::chrono::steady_clock::now()<deadline){auto d=worker.PollDelivery(181);if(d&&fabs(d->activeSeconds-time)<1e-10)return d;auto e=worker.Error();if(!e.empty())throw std::runtime_error(e);std::this_thread::sleep_for(std::chrono::milliseconds(1));}throw std::runtime_error("Delayed material publication timed out");};
 auto dressed=waitTime(10);if(dressed->output->physics.substeps||dressed->output->physics.accumulatedSeconds)throw std::runtime_error("Dressing time became simulation backlog");
 worker.Submit(fitInput,pose,10+1./120,{}, {},delayed);auto advanced=waitTime(10+1./120);if(advanced->output->physics.substeps!=1)throw std::runtime_error("First active cloth step was lost after dressing");
 worker.Stop();std::cout<<"PASS CPU worker latest request, epoch rejection, immutable snapshots, clear/resume/stop, reusable geometry and one-time reference placement; submit="<<submit<<"ms\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
