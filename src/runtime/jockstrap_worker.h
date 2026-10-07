#pragma once
#include "jockstrap_pose.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <string>
#include <functional>
#include <atomic>
#include <malemod/surface/garment_impulse.hpp>
// One immutable latest request preserves accumulated active time, coherent
// native pose metadata and source output. Rendering never waits for a solve.
class JockstrapCpuWorker {
 using Input=malemod::garments::Input;using Output=malemod::garments::Output;
public:
 using Prepare=std::function<void(Input&,const JockstrapKinematics::Pose&)>;
 using Initialize=std::function<const Output&(malemod::garments::Session&,const Input&,const JockstrapKinematics::Pose&)>;
 using Reaction=std::function<malemod::surface::Frame::GarmentSupport(const Input&,const Output&,const JockstrapKinematics::Pose&)>;
 struct Delivery{std::shared_ptr<const Output> output;std::shared_ptr<const JockstrapKinematics::Pose> pose;malemod::surface::Frame::GarmentSupport reaction;double activeSeconds=0,milliseconds=0,preparationMilliseconds=0,solverMilliseconds=0;unsigned long long serial=0;bool clockReset=false;};
private:
 struct Request{std::shared_ptr<const Input> input;std::shared_ptr<const JockstrapKinematics::Pose> pose;double activeSeconds=0;Prepare prepare;Reaction reaction;Initialize initialize;};
 static inline std::atomic<unsigned long long> reactionEpochCounter{1};
 std::mutex mutex;std::condition_variable changed;std::shared_ptr<const Request> pending;std::shared_ptr<const Delivery> published;
 bool stopping=false;unsigned long long currentEpoch=0,generation=0;double submittedActive=0;std::string error;
 malemod::garments::Parameters parameters;
 std::thread thread;
 static void SwapGeometry(Input& a,Input& b){a.waist.swap(b.waist);a.opening.swap(b.opening);a.anatomy.swap(b.anatomy);a.rearStraps.swap(b.rearStraps);a.anatomyTriangles.swap(b.anatomyTriangles);a.rootSubdivisions.swap(b.rootSubdivisions);a.bodySurface.swap(b.bodySurface);a.bodyTriangles.swap(b.bodyTriangles);for(unsigned i=0;i<4;i++)a.anatomyRegions[i].swap(b.anatomyRegions[i]);}
 static bool HeaderOnly(const Input& a){for(const auto& group:a.anatomyRegions)if(!group.empty())return false;return a.waist.empty()&&a.opening.empty()&&a.anatomy.empty()&&a.rearStraps[0].empty()&&a.rearStraps[1].empty()&&a.anatomyTriangles.empty()&&a.rootSubdivisions.empty()&&a.bodySurface.empty()&&a.bodyTriangles.empty();}
 void Run(){Input geometryCache;malemod::garments::Session session(parameters);malemod::surface::GarmentImpulseLedger reactionLedger;unsigned long long processedGeneration=~0ull,processedEpoch=0,serial=0;double active=0;unsigned long long processedRest=0;bool materialReady=false;
  for(;;){std::shared_ptr<const Request> request;unsigned long long requestedGeneration;{std::unique_lock<std::mutex> lock(mutex);changed.wait(lock,[&]{return stopping||pending;});if(stopping)return;request=std::move(pending);requestedGeneration=generation;}
   try{auto began=std::chrono::steady_clock::now();auto input=*request->input;const bool recycle=request->prepare&&HeaderOnly(input);if(recycle)SwapGeometry(input,geometryCache);if(request->prepare){if(!request->pose)throw std::invalid_argument("Deferred garment conversion has no coherent pose");request->prepare(input,*request->pose);}auto prepared=std::chrono::steady_clock::now();bool reset=processedGeneration!=requestedGeneration||processedEpoch!=input.characterEpoch||request->activeSeconds<active;if(reset){session.Reset();materialReady=false;reactionLedger.Reset(reactionEpochCounter.fetch_add(1));}input.deltaTime=reset?0:request->activeSeconds-active;active=request->activeSeconds;processedGeneration=requestedGeneration;processedEpoch=input.characterEpoch;
    auto result=std::make_shared<Delivery>();const bool dressing=request->initialize&&(!materialReady||processedRest!=input.restRevision);if(dressing){if(!request->pose)throw std::invalid_argument("Reference cloth initialization has no pose");result->output=std::make_shared<Output>(request->initialize(session,input,*request->pose));}else result->output=std::make_shared<Output>(session.Update(malemod::garments::Style::WhiteJockstrap,input,malemod::garments::TimeContinuity::Continuous));processedRest=input.restRevision;materialReady=true;if(request->reaction&&result->output->contactBudgetSatisfied&&result->output->physics.materialBudgetSatisfied){if(!request->pose)throw std::invalid_argument("Physical garment reaction pose missing");result->reaction=reactionLedger.Append(request->reaction(input,*result->output,*request->pose));}if(recycle)SwapGeometry(input,geometryCache);auto finished=std::chrono::steady_clock::now();result->preparationMilliseconds=std::chrono::duration<double,std::milli>(prepared-began).count();result->solverMilliseconds=std::chrono::duration<double,std::milli>(finished-prepared).count();result->pose=request->pose;result->activeSeconds=active;result->serial=++serial;result->clockReset=reset;result->milliseconds=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();std::lock_guard<std::mutex> lock(mutex);if(!stopping&&input.characterEpoch==currentEpoch&&requestedGeneration==generation){// The garment does not yet exist while its material is being dressed.
     // Start its active clock at publication, not at the older preparation
     // request; otherwise dressing wall time becomes a multi-second backlog.
     if(dressing&&pending&&pending->input->characterEpoch==currentEpoch)active=(std::max)(active,pending->activeSeconds);
     published=std::move(result);error.clear();}
   }catch(const std::exception& e){session.Reset();materialReady=false;std::lock_guard<std::mutex> lock(mutex);if(request->input->characterEpoch==currentEpoch&&requestedGeneration==generation){published.reset();error=e.what();}}
  }
 }
public:
 explicit JockstrapCpuWorker(malemod::garments::Parameters p={}):parameters(p),thread([this]{Run();}){}
 ~JockstrapCpuWorker(){Stop();}
 void Submit(std::shared_ptr<const Input> input,std::shared_ptr<const JockstrapKinematics::Pose> pose={},double activeSeconds=-1,Prepare prepare={},Reaction reaction={},Initialize initialize={}){if(!input||!std::isfinite(input->deltaTime)||input->deltaTime<0||!std::isfinite(activeSeconds))throw std::invalid_argument("Invalid garment immutable request clock");std::lock_guard<std::mutex> lock(mutex);if(stopping)return;if(currentEpoch!=input->characterEpoch){published.reset();generation++;submittedActive=0;}currentEpoch=input->characterEpoch;if(activeSeconds<0)submittedActive+=input->deltaTime;else submittedActive=activeSeconds;pending=std::make_shared<Request>(Request{std::move(input),std::move(pose),submittedActive,std::move(prepare),std::move(reaction),std::move(initialize)});changed.notify_one();}
 std::shared_ptr<const Delivery> PollDelivery(unsigned long long epoch){std::lock_guard<std::mutex> lock(mutex);return published&&published->output->characterEpoch==epoch?published:nullptr;}
 std::shared_ptr<const Output> Poll(unsigned long long epoch){auto result=PollDelivery(epoch);return result?result->output:nullptr;}
 std::string Error(){std::lock_guard<std::mutex> lock(mutex);return error;}
 void Clear(){std::lock_guard<std::mutex> lock(mutex);pending.reset();published.reset();submittedActive=0;error.clear();generation++;}
 void Stop(){{std::lock_guard<std::mutex> lock(mutex);if(stopping)return;stopping=true;pending.reset();changed.notify_one();}if(thread.joinable())thread.join();}
};
