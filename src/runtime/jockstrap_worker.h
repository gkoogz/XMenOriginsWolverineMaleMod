#pragma once
#include <malemod/garments/jockstrap.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
// SDK-free CPU worker. There is one immutable latest request and one immutable
// published output, so render/physics never wait for a cloth contact solve.
class JockstrapCpuWorker {
 using Input=malemod::garments::Input;using Output=malemod::garments::Output;
 std::mutex mutex;std::condition_variable changed;
 std::shared_ptr<const Input> pending;std::shared_ptr<const Output> published;
 bool stopping=false;unsigned long long currentEpoch=0,generation=0;
 // Last: Run may execute immediately, so every field it reads must exist first.
 std::thread thread;
 void Run(){malemod::garments::Session session;for(;;){std::shared_ptr<const Input> request;unsigned long long requestedGeneration;{std::unique_lock<std::mutex> lock(mutex);changed.wait(lock,[&]{return stopping||pending;});if(stopping)return;request=std::move(pending);requestedGeneration=generation;}try{auto result=std::make_shared<Output>(session.Update(malemod::garments::Style::WhiteJockstrap,*request));std::lock_guard<std::mutex> lock(mutex);if(!stopping&&request->characterEpoch==currentEpoch&&requestedGeneration==generation)published=std::move(result);}catch(const std::exception&){std::lock_guard<std::mutex> lock(mutex);if(request->characterEpoch==currentEpoch&&requestedGeneration==generation)published.reset();}}}
public:
 JockstrapCpuWorker():thread([this]{Run();}){}
 ~JockstrapCpuWorker(){Stop();}
 void Submit(std::shared_ptr<const Input> input){std::lock_guard<std::mutex> lock(mutex);if(stopping)return;if(currentEpoch!=input->characterEpoch){published.reset();generation++;}currentEpoch=input->characterEpoch;pending=std::move(input);changed.notify_one();}
 std::shared_ptr<const Output> Poll(unsigned long long epoch){std::lock_guard<std::mutex> lock(mutex);return published&&published->characterEpoch==epoch?published:nullptr;}
 void Clear(){std::lock_guard<std::mutex> lock(mutex);pending.reset();published.reset();generation++;}
 void Stop(){{std::lock_guard<std::mutex> lock(mutex);if(stopping)return;stopping=true;pending.reset();changed.notify_one();}if(thread.joinable())thread.join();}
};
