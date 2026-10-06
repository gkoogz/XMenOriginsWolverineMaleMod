// Production worker's cumulative reaction publication and cancellation contract.
// Synthetic impulses isolate transport; physical contact has separate gates.
#include "../src/runtime/jockstrap_worker.h"
#include <iostream>
#include <future>
namespace G=malemod::garments;
namespace S=malemod::surface;
static G::Sample Sample(G::Point p,unsigned id){G::Sample s{p,{0,1,0},{}};s.lineage.donors[0]={G::Surface::Anatomy,id,1};return s;}
static std::shared_ptr<G::Input> Fixture(unsigned epoch){auto in=std::make_shared<G::Input>();in->characterEpoch=epoch;in->topologyRevision=1;for(unsigned i=0;i<24;i++){double t=i*6.283185307179586/24;in->waist.push_back(Sample({.2*cos(t),.15*sin(t),1},i));in->opening.push_back(Sample({.05*cos(t),.12,.7+.05*sin(t)},i));}for(unsigned j=0;j<24;j++)for(unsigned i=0;i<48;i++){double p=3.141592653589793*(j+.1)/24,t=i*6.283185307179586/48;in->anatomy.push_back(Sample({.05*sin(p)*cos(t),.2+.15*sin(p)*sin(t),.65+.1*cos(p)},i+j*48));}for(unsigned side=0;side<2;side++){double a=side?1:-1;in->rearStraps[side]={Sample({a*.15,-.1,.99},0),Sample({a*.13,-.1,.7},1),Sample({a*.05,.12,.62},2)};}return in;}
static auto Wait(JockstrapCpuWorker& worker,unsigned epoch,double time){auto until=std::chrono::steady_clock::now()+std::chrono::seconds(10);for(;;){auto p=worker.PollDelivery(epoch);if(p&&std::abs(p->activeSeconds-time)<1e-10)return p;auto error=worker.Error();if(!error.empty())throw std::runtime_error(error);if(std::chrono::steady_clock::now()>until)throw std::runtime_error("Reaction worker timed out");std::this_thread::sleep_for(std::chrono::milliseconds(1));}}
static void Check(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(){try{
 JockstrapCpuWorker worker;auto pose=std::make_shared<JockstrapKinematics::Pose>();pose->serial=531;
 std::promise<void> entered,release;auto released=release.get_future().share();unsigned calls=0;double total=0;
 auto reaction=[&](const G::Input& in,const G::Output& out,const JockstrapKinematics::Pose& captured){Check(in.anatomy.size()==1152&&out.mesh.vertices.size()>0&&captured.serial==531,"Reaction callback lost full input/output/pose");S::Frame::GarmentSupport delta;delta.enabled=delta.contactReaction=true;delta.rodImpulseTotals[4]={0,double(++calls),0};delta.lobeAngularImpulseTotals[0]={double(calls),0,0};total+=calls;return delta;};
 worker.Submit(Fixture(12),pose,0,[&](G::Input&,const JockstrapKinematics::Pose&){entered.set_value();released.wait();},reaction);
 Check(entered.get_future().wait_for(std::chrono::seconds(2))==std::future_status::ready,"Worker failed to enter blocked preparation");
 for(unsigned i=1;i<=20;i++)worker.Submit(Fixture(12),pose,double(i)/200,{},reaction);
 release.set_value();auto last=Wait(worker,12,.1);Check(calls==2&&total==3,"Overwritten pending requests ran or completed reactions were lost");
 S::GarmentImpulseCursor consumer;auto first=consumer.Consume(last->reaction);Check(first.rod[4].y==total&&first.angular[0].x==total,"Latest publication dropped a completed reaction");
 Check(consumer.Consume(last->reaction).rod[4].y==0,"Retry replayed worker reaction");auto oldEpoch=last->reaction.contactEpoch;
 worker.Clear();worker.Submit(Fixture(12),pose,0,{},reaction);auto reset=Wait(worker,12,0);auto next=consumer.Consume(reset->reaction);Check(reset->reaction.contactEpoch!=oldEpoch&&next.reset&&next.rod[4].y==3,"Clear/restart replayed prior reaction totals");
 worker.Stop();std::cout<<"PASS production cloth worker completed-reaction accumulation, pending request coalescing, coherent full input/output/pose, source retry and clear epoch reset (synthetic transport impulses)\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
