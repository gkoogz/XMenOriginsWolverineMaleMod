#pragma once
// Clinical demonstration timeline. Volumetric fluid lives in teaching_volume.h.
// Simulation quantities use game units, not calibrated medical measurements.
namespace teaching {
static float sequenceEnd=20.f;
static float Clamp(float x,float a,float b){return x<a?a:x>b?b:x;}
static float Ease(float x){x=Clamp(x,0,1);return x*x*x*(x*(x*6-15)+10);}
static const float duration=20.f, step=1.f/120.f;
static const float peaks[8]={1.5f,2.5f,3.5f,4.5f,7.f,8.5f,10.f,11.5f};
struct Sample {float blend=0,firm=0,pulse=0,finalBlend=0,finalPulse=0,hangPulse=0;};
struct Timeline {
 bool active=false; double time=0;
 void Start(){active=true;time=0;}
 void Cancel(){active=false;time=0;}
 void Advance(float dt){if(!active||!std::isfinite(dt)||dt<=0)return;time+=dt;if(time>=sequenceEnd){time=sequenceEnd;active=false;}}
 Sample Get() const {
  Sample s;if(!active)return s;float t=(float)time;
  s.blend=Ease(t/.6f)*(1-Ease((t-16.f)/4.f));
  s.firm=Ease(t/4.5f)*(1-Ease((t-14.f)/6.f));
  // Settle before the first emission; hold through the last emission and pulse.
  s.finalBlend=Ease((t-5.f)/1.5f)*(1-Ease((t-12.1f)/1.9f));
  for(float peak:peaks){float d=t-peak;float p=d<0?Ease((d+.22f)/.22f):1-Ease(d/.55f);if(p>s.pulse)s.pulse=p;}
  for(int i=4;i<8;i++){float d=t-peaks[i];float p=d<0?Ease((d+.22f)/.22f):1-Ease(d/.55f);if(p>s.finalPulse)s.finalPulse=p;}
  // Release the suspension sooner than the size envelope, leaving a clear
  // lowered rest interval before the next contraction.
  for(int i=4;i<8;i++){float d=t-peaks[i];float p=d<0?Ease((d+.22f)/.22f):1-Ease(d/.35f);if(p>s.hangPulse)s.hangPulse=p;}
  return s;
 }
};
}



