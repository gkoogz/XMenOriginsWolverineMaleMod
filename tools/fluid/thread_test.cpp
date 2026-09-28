#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <cmath>
#include <cstdio>
#include <chrono>
#include <map>
struct V3 {float x,y,z;};
static V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
static V3 operator-(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
static V3 operator*(V3 a,float s){return {a.x*s,a.y*s,a.z*s};}
static V3 operator/(V3 a,float s){return a*(1/s);}
static float Dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static V3 Cross(V3 a,V3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
static float Length(V3 a){return sqrtf(Dot(a,a));}
static V3 Unit(V3 a){float l=Length(a);return l>1e-6?a/l:V3{1,0,0};}
#include "../../src/runtime/teaching_sequence.h"
#include "../../src/runtime/teaching_volume.h"
#define CHECK(x) do{if(!(x)){printf("FAIL %d %s\n",__LINE__,#x);return false;}}while(0)
static bool Finite(V3 p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);}
static bool Topology(const teaching::LiquidMesh& m){
 std::map<std::pair<unsigned,unsigned>,int> edges;double volume=0;
 for(size_t i=0;i<m.indices.size();i+=3){unsigned a=m.indices[i],b=m.indices[i+1],c=m.indices[i+2];CHECK(a<m.vertices.size()&&b<m.vertices.size()&&c<m.vertices.size());CHECK(a!=b&&b!=c&&a!=c);for(auto e:{std::make_pair(a,b),std::make_pair(b,c),std::make_pair(c,a)}){if(e.first>e.second)std::swap(e.first,e.second);++edges[e];}V3 p=m.vertices[a].p,q=m.vertices[b].p,r=m.vertices[c].p;CHECK(Length(Cross(q-p,r-p))>1e-11f);volume+=Dot(p,Cross(q,r))/6.;}
 for(const auto& e:edges)CHECK(e.second==2);
 for(const auto& v:m.vertices)CHECK(Finite(v.p)&&Finite(v.n)&&fabsf(Length(v.n)-1)<.002f);
 CHECK(volume>=0);printf("Topology watertight components=%d vertices=%u triangles=%u volume=%.4f\n",m.components,(unsigned)m.vertices.size(),(unsigned)m.indices.size()/3,volume);return true;
}
static bool Sequence(int fps,int preset,bool moving){
 volumeFluid::config={};auto& c=volumeFluid::config;if(preset==1){c.volume=600;c.feed=3;c.nozzle=1.5f;c.viscosity=24;c.flowVariation=.2f;c.breakup=.5f;}if(preset==2){c.volume=.15f;c.feed=.25f;c.nozzle=.4f;c.viscosity=.3f;c.dropVolume=.03f;c.flowVariation=.1f;c.breakup=1.5f;}if(preset==3){c.volume=2000;c.feed=10;c.nozzle=2;c.viscosity=100;c.catchPlane=true;}if(preset==4){c.volume=120;c.feed=1.1f;c.pulseVolumeVariation=.4f;c.pulseDurationVariation=.7f;c.angleVariation=.65f;}
 teaching::Fluid f;f.SetVariationSeed(0x13579bdfu);CHECK(f.Begin({0,0,50}));std::vector<double> timings;int peak=0;size_t peakVertices=0;double maxVolumeError=0,expected=2*c.dropVolume;for(float v:f.pulseVolume)expected+=v;
 if(preset==4){CHECK(f.pulseChannelCount<4);bool volumeVar=false,durationVar=false,angleVar=false,overlap=false;for(int i=0;i<4;i++){volumeVar|=fabsf(f.pulseVolume[i]-c.volume)>.01f;durationVar|=fabsf(f.pulseDuration[i]-c.feed)>.01f;angleVar|=fabsf(f.angleGain[i]-1.f)>.01f;if(i<3&&f.pulseChannel[i]==f.pulseChannel[i+1]&&teaching::peaks[i+4]+f.pulseDuration[i]>teaching::peaks[i+5])overlap=true;CHECK(fabsf(teaching::WeightedMainPulse(teaching::peaks[i+4],f.angleGain)-f.angleGain[i])<.001f);}CHECK(volumeVar&&durationVar&&angleVar&&overlap);}
 for(int frame=0;frame<int((teaching::sequenceEnd+1)*fps);frame++){
  float t=(frame+1.f)/fps;V3 tip=moving?V3{t*1.5f,sinf(t)*.3f,50}:V3{0,0,50};
  auto start=std::chrono::steady_clock::now();f.Advance(1.f/fps,tip,Unit({1,0,.4f}),{});double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();CHECK(f.ready);
  if(t>=7&&t<11.5f+c.feed)timings.push_back(ms);peak=max(peak,f.Live());peakVertices=max(peakVertices,f.mesh.vertices.size());
  for(const auto& s:f.streams){double total=s.retiredVolume;for(const auto& node:s.nodes)total+=node.lump;for(const auto& l:s.links){CHECK(l.volume>=0&&std::isfinite(l.volume));total+=l.volume;}maxVolumeError=max(maxVolumeError,fabs(total-s.volume));for(const auto& n:s.nodes)CHECK(Finite(n.p)&&Finite(n.v));}
  for(const auto& v:f.mesh.vertices)CHECK(Finite(v.p)&&Finite(v.n));
  if(preset==0&&frame==int(7.95f*fps))CHECK(Topology(f.mesh));
 }
 for(int i=f.pulseChannelCount;i<4;i++)CHECK(f.streams[i].nodes.empty());
 CHECK(f.Live()==0);CHECK(fabs(f.emittedVolume-expected)<.005);CHECK(maxVolumeError<.02);
 std::sort(timings.begin(),timings.end());printf("PASS sequence fps=%d preset=%d moving=%d volume=%.6f error=%.6f peakNodes=%d peakVertices=%u median=%.3fms p95=%.3fms\n",fps,preset,moving,f.emittedVolume,maxVolumeError,peak,(unsigned)peakVertices,timings[timings.size()/2],timings[timings.size()*95/100]);
 f.Begin({});f.Advance(.3f,{},V3{1,0,0},{});CHECK(!f.ready&&f.Live()==0&&f.mesh.indices.empty());f.Begin({});f.Advance(.01f,{101,0,0},{1,0,0},{});CHECK(!f.ready);return true;
}
static bool Variations(){
 volumeFluid::config={};auto& c=volumeFluid::config;c.pulseVolumeVariation=.4f;c.pulseDurationVariation=.7f;c.angleVariation=.65f;
 teaching::Fluid a,b;a.SetVariationSeed(0x2468ace1u);b.SetVariationSeed(0x2468ace1u);CHECK(a.Begin({})&&b.Begin({}));CHECK(a.pulseChannelCount<4);
 bool volume=false,duration=false,angle=false,overlap=false;
 for(int i=0;i<4;i++){
  CHECK(a.pulseVolume[i]>=c.volume*(1-c.pulseVolumeVariation)&&a.pulseVolume[i]<=c.volume*(1+c.pulseVolumeVariation));
  CHECK(a.pulseDuration[i]>=.15f&&a.pulseDuration[i]<=10.f);CHECK(a.angleGain[i]>=.05f&&a.angleGain[i]<=1.f+c.angleVariation);
  CHECK(fabsf(a.pulseVolume[i]-b.pulseVolume[i])<1e-5f&&fabsf(a.pulseDuration[i]-b.pulseDuration[i])<1e-5f&&fabsf(a.angleGain[i]-b.angleGain[i])<1e-5f);
  volume|=fabsf(a.pulseVolume[i]-c.volume)>.01f;duration|=fabsf(a.pulseDuration[i]-c.feed)>.01f;angle|=fabsf(a.angleGain[i]-1.f)>.01f;
  if(i<3&&a.pulseChannel[i]==a.pulseChannel[i+1]&&teaching::peaks[i+4]+a.pulseDuration[i]>teaching::peaks[i+5])overlap=true;
  CHECK(fabsf(teaching::WeightedMainPulse(teaching::peaks[i+4],a.angleGain)-a.angleGain[i])<.001f);
 }
 CHECK(volume&&duration&&angle&&overlap);CHECK(a.pulseChannel[0]==a.pulseChannel[1]);
 int longPulse=-1;for(int i=0;i<3;i++)if(a.pulseDuration[i]>1.5f&&a.pulseChannel[i]==a.pulseChannel[i+1]){longPulse=i;break;}CHECK(longPulse>=0);
 double connectedEnd=teaching::peaks[longPulse+5]+a.pulseDuration[longPulse+1];
 for(double t=teaching::peaks[longPulse+4]+.01;t<connectedEnd-.01;t+=.01){double rate=0;for(int i=0;i<4;i++)if(a.pulseChannel[i]==a.pulseChannel[longPulse]){double x=(t-teaching::peaks[i+4])/a.pulseDuration[i];if(x>0&&x<1)rate+=a.pulseVolume[i]/a.pulseDuration[i]*a.Rate((float)x);}CHECK(rate>0);}
 c.pulseVolumeVariation=c.pulseDurationVariation=c.angleVariation=0;teaching::Fluid fixed;fixed.SetVariationSeed(123);CHECK(fixed.Begin({}));
 for(int i=0;i<4;i++){CHECK(fabsf(fixed.pulseVolume[i]-c.volume)<1e-5f&&fabsf(fixed.pulseDuration[i]-c.feed)<1e-5f&&fabsf(fixed.angleGain[i]-1)<1e-5f);}CHECK(fixed.pulseChannelCount==4);
 printf("PASS per-pump volume/duration/angle variance is seeded and repeatable; overlapping pumps share a continuous feed channel\n");return true;
}
static bool PulseTaper(){
 volumeFluid::config={};auto& c=volumeFluid::config;c.flowVariation=0;c.pulseVolumeVariation=0;c.pulseDurationVariation=0;c.angleVariation=0;c.gravity=0;c.viscosity=0;c.breakup=0;c.lifetime=15;
 teaching::Fluid symmetric; c.pulseTaper=0;CHECK(symmetric.Begin({}));CHECK(fabs(symmetric.Cumulative(.5)-.5)<.002);CHECK(fabsf(symmetric.Rate(.2f)-symmetric.Rate(.8f))<.002f);
 teaching::Fluid f;c.pulseTaper=.55f;CHECK(f.Begin({}));CHECK(f.Cumulative(1)==1&&f.Cumulative(.5)>.57);CHECK(f.Rate(.15f)>f.Rate(.75f)*1.7f);
 double early=0,late=0;int earlyN=0,lateN=0;
 for(int frame=0;frame<int(8.04*120);frame++)f.Advance(1.f/120,{0,0,50},{1,0,0},{});
 const auto& s=f.streams[0];for(size_t i=0;i<s.links.size()&&i+1<s.nodes.size();i++){
  double x=(s.nodes[i+1].born-teaching::Fluid::EventTime(2))/f.pulseDuration[0];if(x<.1||x>.92)continue;
  double radius=sqrt(s.links[i].volume/(3.141592653589793*max(.002f,s.links[i].initial)));
  if(x>=.12&&x<=.32){early+=radius;++earlyN;}if(x>=.68&&x<=.88){late+=radius;++lateN;}
 }
 CHECK(earlyN>=8&&lateN>=8);early/=earlyN;late/=lateN;CHECK(early>late*1.18);
 c.flowVariation=.3f;teaching::Fluid textured;CHECK(textured.Begin({}));for(int frame=0;frame<int(8.04*120);frame++)textured.Advance(1.f/120,{0,0,50},{1,0,0},{});
 double texturedEarly=0,texturedLate=0;int texturedEarlyN=0,texturedLateN=0;const auto& ts=textured.streams[0];for(size_t i=0;i<ts.links.size()&&i+1<ts.nodes.size();i++){
  double x=(ts.nodes[i+1].born-teaching::Fluid::EventTime(2))/textured.pulseDuration[0];double radius=sqrt(ts.links[i].volume/(3.141592653589793*max(.002f,ts.links[i].initial)));
  if(x>=.12&&x<=.32){texturedEarly+=radius;++texturedEarlyN;}if(x>=.68&&x<=.88){texturedLate+=radius;++texturedLateN;}
 }
 CHECK(texturedEarlyN>=8&&texturedLateN>=8);texturedEarly/=texturedEarlyN;texturedLate/=texturedLateN;CHECK(texturedEarly>texturedLate*1.08);
 printf("PASS normalized pump volume with a faster attack, thick onset and smooth taper: cumulative(0.5)=%.3f rate %.3f->%.3f radius %.3f->%.3f texture-default radius %.3f->%.3f (%d/%d samples)\n",f.Cumulative(.5),f.Rate(.15f),f.Rate(.75f),early,late,texturedEarly,texturedLate,earlyN,lateN);
 return true;
}
static bool PassiveClear(){
 teaching::PassiveThrobGate gate;CHECK(!gate.Update(0.f)&&!gate.Update(.3f)&&gate.Update(.6f)&&!gate.Update(.9f));CHECK(!gate.Update(.2f)&&gate.Update(.6f));
 volumeFluid::config={};CHECK(fabsf(volumeFluid::config.dropVolume-.42f)<1e-6f&&fabsf(volumeFluid::config.dropDuration-1.75f)<1e-6f&&fabsf(volumeFluid::config.dropHold-1.45f)<1e-6f&&fabsf(volumeFluid::config.dropLength-5.7f)<1e-6f);
 teaching::Fluid f;f.SetVariationSeed(0xabcdef01u);CHECK(f.BeginPassive({0,0,50})&&f.passiveMode);CHECK(f.TriggerPassiveClear());
 int peakLive=0;const int totalFrames=14*60;
 for(int frame=0;frame<totalFrames;frame++){
  if(frame%180==0&&frame>0){gate.Update(.2f);CHECK(gate.Update(.6f)&&f.TriggerPassiveClear());}
  f.Advance(1.f/60,{0,0,50},{1,0,0},{});CHECK(f.ready&&f.passiveMode);
  int live=0;for(const auto& strand:f.clear)if(strand.live)++live;peakLive=max(peakLive,live);
  for(const auto& stream:f.streams)CHECK(stream.nodes.empty()&&!stream.live);
 }
 CHECK(peakLive<=3&&peakLive>=2);for(int e=0;e<4;e++)CHECK(f.emitted[e]==0&&f.delivered[e]==0);
 CHECK(f.emittedVolume>4.8*f.settings.dropVolume&&f.emittedVolume<5.2*f.settings.dropVolume);
 CHECK(f.TriggerPassiveClear()); // Expired slots are recycled during a long idle session.
 f.Clear();CHECK(!f.ready&&!f.passiveMode&&f.Live()==0);
 printf("PASS passive pulse edge gate, clear-only emissions, larger/longer shared preliminary strand, bounded 8-slot pool (peak concurrent=%d)\n",peakLive);return true;
}
static teaching::ViscousThread Make(){teaching::ViscousThread s;s.live=true;for(int i=0;i<30;i++){s.nodes.push_back({{float(i),0,50},{float(i)-14.5f,2,0},0});if(i){s.links.push_back({1,1,1,false,(unsigned)(i-1)});s.volume+=1;}}return s;}
static V3 Momentum(const teaching::ViscousThread& s){V3 p{};for(size_t i=0;i<s.links.size();i++)p=p+(s.nodes[i].v+s.nodes[i+1].v)*(s.links[i].volume*.5f);for(const auto& node:s.nodes)p=p+node.v*node.lump;return p;}
static float Energy(const teaching::ViscousThread& s){float e=0;for(size_t i=0;i<s.links.size();i++)e+=s.links[i].volume*(Dot(s.nodes[i].v,s.nodes[i].v)+Dot(s.nodes[i+1].v,s.nodes[i+1].v))*.25f;for(const auto& node:s.nodes)e+=.5f*node.lump*Dot(node.v,node.v);return e;}
static bool Material(){
 volumeFluid::Settings c;volumeFluid::F4 collisions[8]{};c.gravity=0;c.breakup=0;auto s=Make();V3 p=Momentum(s);float energy=Energy(s);for(int k=0;k<120;k++)s.Step(1.f/120,k/120.f,c,collisions,{});CHECK(Length(Momentum(s)-p)<.01f&&Energy(s)<energy);printf("PASS viscous deformation dissipates energy %.3f -> %.3f; momentum error %.6f\n",energy,Energy(s),Length(Momentum(s)-p));
 s=Make();for(auto& n:s.nodes)n.v={3,2,1};V3 before=s.nodes[15].p;c.gravity=98;for(int k=0;k<36;k++)s.Step(1.f/120,k/120.f,c,collisions,{});CHECK(Length(s.nodes[15].p-(before+V3{.9f,.6f,.3f-98.f/120/120*36*37*.5f}))<.002f);printf("PASS detached ballistic translation; no global drag\n");
 s=Make();c.gravity=0;c.breakup=4;c.tension=100;c.viscosity=0;for(auto& n:s.nodes){n.p.x*=4;n.v={};}for(int k=0;k<300;k++)s.Step(1.f/120,1+k/120.f,c,collisions,{});CHECK(s.breaks>0);teaching::LiquidMesh m;std::vector<volumeFluid::Particle>b;s.Append(m,12,b);CHECK(m.components>=2&&Topology(m));double amount=0;for(auto& l:s.links)amount+=l.volume;for(const auto& node:s.nodes)amount+=node.lump;CHECK(fabs(amount-s.volume)<.0001);printf("PASS necking/breakup preserves volume %.6f, cuts=%d\n",amount,s.breaks);
 volumeFluid::config={};teaching::Fluid a,bf;CHECK(a.Begin({0,0,50})&&bf.Begin({0,0,50}));for(int i=0;i<270;i++)a.Advance(1.f/30,{0,0,50},{1,0,0},{});for(int i=0;i<1080;i++)bf.Advance(1.f/120,{0,0,50},{1,0,0},{});CHECK(a.streams[0].nodes.size()==bf.streams[0].nodes.size());float error=0;for(size_t i=0;i<a.streams[0].nodes.size();i++)error=max(error,Length(a.streams[0].nodes[i].p-bf.streams[0].nodes[i].p));CHECK(error<.001f);printf("PASS 30/120 Hz fixed-step agreement maxError=%.8f\n",error);
 return true;
}
static bool LateralWobble(){
 volumeFluid::config={};teaching::Fluid a,b;a.SetVariationSeed(123);b.SetVariationSeed(123);CHECK(a.Prepare()&&b.Prepare());
 for(int i=0;i<4;i++){
  float peak=a.MainLateralYaw(teaching::peaks[i+4]);CHECK(fabsf(peak)>.015f&&fabsf(peak)<=3.f*.0174533f);
  CHECK(peak==b.MainLateralYaw(teaching::peaks[i+4]));
  if(i)CHECK(peak*a.MainLateralYaw(teaching::peaks[i+3])<0);
 }
 CHECK(a.MainLateralYaw(3.5)==0&&a.MainLateralYaw(6)==0&&a.MainLateralYaw(13)==0);
 float previous=0;for(int i=0;i<15000;i++){float yaw=a.MainLateralYaw(i*.001);CHECK(std::isfinite(yaw)&&fabsf(yaw-previous)<.002f);previous=yaw;}
 a.settings.lateralWobbleDegrees=0;for(double t=0;t<15;t+=.01)CHECK(a.MainLateralYaw(t)==0);
 printf("PASS bounded, smooth, alternating per-pulse lateral yaw; seeded repeatability, preliminary/rest exclusion and zero-disable\n");return true;
}
static bool ForceVariation(){
 volumeFluid::config={};int weak=0,strong=0,ordinary=0;
 for(unsigned seed=1;seed<=600;seed++){
  teaching::Fluid f;f.SetVariationSeed(seed);CHECK(f.Prepare());
  for(float gain:f.forceGain){CHECK(gain>=.8199f&&gain<=1.1201f);if(gain<.9f)weak++;else if(gain>1.06f)strong++;else{CHECK(gain>=.9699f&&gain<=1.0301f);ordinary++;}}
 }
 CHECK(weak>320&&weak<480&&strong>320&&strong<480);
 teaching::Fluid a,b;a.SetVariationSeed(31);b.SetVariationSeed(31);CHECK(a.Begin({}));volumeFluid::config.pulseForceVariation=0;CHECK(b.Begin({}));
 for(int i=0;i<4;i++){CHECK(b.forceGain[i]==1&&a.pulseVolume[i]==b.pulseVolume[i]&&a.pulseDuration[i]==b.pulseDuration[i]&&a.angleGain[i]==b.angleGain[i]&&a.lateralGain[i]==b.lateralGain[i]);}
 // Start inside an emission window and check the actual newly injected velocity.
 a.clock=b.clock=7.1;a.steps=b.steps=1;CHECK(a.Tick(1.f/120,{0,0,50},{1,0,0},{})&&b.Tick(1.f/120,{0,0,50},{1,0,0},{}));
 CHECK(!a.streams[0].nodes.empty()&&!b.streams[0].nodes.empty());
 float ratio=a.streams[0].nodes.back().v.x/b.streams[0].nodes.back().v.x;CHECK(fabsf(ratio-a.forceGain[0])<.0001f&&fabs(a.emittedVolume-b.emittedVolume)<1e-6);
 printf("PASS launch force: weak/normal/strong=%d/%d/%d of 2400; actual velocity ratio %.6f; volumes and prior random parameters unchanged; zero disables\n",weak,ordinary,strong,ratio);return true;
}
int main(int argc,char**argv){setvbuf(stdout,nullptr,_IONBF,0);if(argc>1&&strcmp(argv[1],"force")==0)return ForceVariation()?0:1;if(argc>1&&strcmp(argv[1],"wobble")==0)return LateralWobble()?0:1;if(argc>1&&strcmp(argv[1],"material")==0)return Material()?0:1;if(argc>1&&strcmp(argv[1],"variance")==0)return Variations()?0:1;if(argc>1&&strcmp(argv[1],"taper")==0)return PulseTaper()?0:1;if(argc>1&&strcmp(argv[1],"passive")==0)return PassiveClear()?0:1;return Sequence(argc>1?atoi(argv[1]):60,argc>2?atoi(argv[2]):0,argc>3)?0:1;}
