#pragma once
// Illustrative teaching timeline and bounded viscoelastic tracer. Material
// coordinates are game units, not medical measurements. No persistent controls.
namespace teaching {
static float Clamp(float x,float a,float b){return x<a?a:x>b?b:x;}
static float Ease(float x){x=Clamp(x,0,1);return x*x*x*(x*(x*6-15)+10);}
static const float duration=20.f, step=1.f/120.f;
// Classroom visibility magnification. This illustrative tracer has no
// calibrated volume variable; radius and emitted arc length scale linearly.
static const float tracerScale=4.f;
static float LaunchSpeed(int event){return (event==2?28.f:40.f)*tracerScale;}
static const float peaks[8]={1.5f,2.5f,3.5f,4.5f,7.f,8.5f,10.f,11.5f};
struct Sample {float blend=0,firm=0,pulse=0;};
struct Timeline {
 bool active=false; double time=0;
 void Start(){active=true;time=0;}
 void Cancel(){active=false;time=0;}
 void Advance(float dt){if(!active||!std::isfinite(dt)||dt<=0)return;time+=dt;if(time>=duration){time=duration;active=false;}}
 Sample Get() const {
  Sample s;if(!active)return s;float t=(float)time;
  s.blend=Ease(t/.6f)*(1-Ease((t-16.f)/4.f));
  s.firm=Ease(t/4.5f)*(1-Ease((t-14.f)/6.f));
  for(float peak:peaks){float d=t-peak;float p=d<0?Ease((d+.22f)/.22f):1-Ease(d/.55f);if(p>s.pulse)s.pulse=p;}
  return s;
 }
};
struct Node {V3 p{},previous{};};
struct Filament {
 Node nodes[32]{};float rest[32]{};int count=0,event=-1;
 float age=0,emissionClock=0;bool attached=false,droplet=false;
};
struct Fluid {
 Filament strands[8]{};unsigned fired=0,emitted=0,detached=0;
 double accumulator=0,clock=0;V3 lastTip{};bool ready=false;
 float viscosity=1.f; // Numerical damping/extension resistance, not Pa s.
 void Clear(){*this=Fluid{};}
 void Begin(V3 tip){Clear();lastTip=tip;ready=true;}
 static float EventTime(int event){return event==0?peaks[1]:event==1?peaks[3]:peaks[event+2];}
 void Spawn(int event,V3 tip,V3 dir,V3 velocity){
  auto& f=strands[event];f=Filament{};f.event=event;f.attached=true;f.droplet=event<2;f.count=2;
  float length=(f.droplet?.10f:.24f)*tracerScale;V3 v=velocity+dir*(f.droplet?0.f:LaunchSpeed(event));
  f.nodes[0]={tip+dir*length,tip+dir*length-v*step};f.nodes[1]={tip,tip};f.rest[1]=length;
  ++emitted;
 }
 void Tick(V3 tip,V3 direction,V3 velocity,V3 gravity){
  clock+=step;
  for(int e=0;e<6;e++)if(!(fired&(1u<<e))&&clock+1e-6>=EventTime(e)){
   fired|=1u<<e;Spawn(e,tip,direction,velocity);
  }
  for(auto& f:strands){
   if(!f.count)continue;f.age+=step;
   if(f.age>3.f){f.count=0;continue;}
   int tail=f.count-1;
   float limit=f.droplet?.70f:.13f;
   if(f.attached&&(f.age>=limit||Length(f.nodes[tail].p-tip)>2.0f)){
    f.attached=false;f.nodes[tail].previous=f.nodes[tail].p-velocity*step;++detached;
   }
   if(f.attached&&!f.droplet){
    f.emissionClock+=step;
    if(f.emissionClock>=.025f&&f.count<32){
     f.emissionClock-=.025f;
     float speed=LaunchSpeed(f.event);V3 v=velocity+direction*speed;
     f.nodes[tail].previous=f.nodes[tail].p-v*step;
     f.nodes[f.count]={tip,tip};f.rest[f.count]=0.f;
     ++f.count;tail=f.count-1;
    }
    // Grow the inlet link while material is being fed. Once another node is
    // inserted its rest length freezes. This prevents either a collapsed bead
    // or new tail nodes overtaking the leading material.
    f.rest[tail]+=LaunchSpeed(f.event)*step;
   }
   for(int i=0;i<f.count;i++){
    auto& n=f.nodes[i];if(f.attached&&i==tail){n.p=n.previous=tip;continue;}
    V3 old=n.p;n.p=n.p+(n.p-n.previous)*.998f+gravity*(step*step);n.previous=old;
   }
   // Position based extension resistance and relative velocity damping.
   // A soft rest length permits visible extension; a hard bound prevents spikes.
   for(int pass=0;pass<5;pass++)for(int i=1;i<f.count;i++){
    auto& a=f.nodes[i-1];auto& b=f.nodes[i];V3 d=b.p-a.p;float n=Length(d);if(n<1e-6f)continue;
    float target=f.rest[i],error=n-target;
    float strength=Clamp(.08f*viscosity,.02f,.25f);if(n>target*2.2f){error=n-target*2.2f;strength=1;}
    bool pinned=f.attached&&i==tail;V3 correction=d*(error/n*strength/(pinned?1.f:2.f));
    a.p=a.p+correction;if(!pinned)b.p=b.p-correction;
   }
   for(int i=1;i<f.count;i++){
    auto& a=f.nodes[i-1];auto& b=f.nodes[i];V3 axis=Unit(b.p-a.p);
    V3 relative=(b.p-b.previous)-(a.p-a.previous);
    V3 impulse=axis*(Dot(relative,axis)*Clamp(.12f*viscosity,0,.45f));
    a.previous=a.previous-impulse;
    if(!(f.attached&&i==tail))b.previous=b.previous+impulse;
   }
   if(f.attached)f.nodes[tail].p=f.nodes[tail].previous=tip;
   bool below=true;for(int i=0;i<f.count;i++){
    V3 p=f.nodes[i].p;if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)){f.count=0;break;}
    if(p.z>=0)below=false;
   }
   if(below)f.count=0; // Reference plane only; terrain queries are future work.
  }
 }
 void Advance(float dt,V3 tip,V3 direction,V3 gravity){
  if(!ready||!std::isfinite(dt)||dt<=0)return;
  if(dt>.25f||Length(tip-lastTip)>25.f){Clear();return;} // Lost scene/teleport.
  V3 velocity=(tip-lastTip)/dt;float speed=Length(velocity);if(speed>80)velocity=velocity*(80/speed);
  accumulator+=dt;int steps=(int)((accumulator+1e-8)/step);
  for(int i=0;i<steps;i++)Tick(lastTip+(tip-lastTip)*((i+1.f)/steps),direction,velocity,gravity);
  accumulator-=steps*step;lastTip=tip;
 }
 int Live() const {int n=0;for(const auto& f:strands)n+=f.count;return n;}
};
}
