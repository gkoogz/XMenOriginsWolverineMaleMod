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
  for(const auto& s:f.streams){double total=s.retiredVolume;for(const auto& l:s.links){CHECK(l.volume>=0&&std::isfinite(l.volume));total+=l.volume;}maxVolumeError=max(maxVolumeError,fabs(total-s.volume));for(const auto& n:s.nodes)CHECK(Finite(n.p)&&Finite(n.v));}
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
static teaching::ViscousThread Make(){teaching::ViscousThread s;s.live=true;for(int i=0;i<30;i++){s.nodes.push_back({{float(i),0,50},{float(i)-14.5f,2,0},0});if(i){s.links.push_back({1,1,1,false,(unsigned)(i-1)});s.volume+=1;}}return s;}
static V3 Momentum(const teaching::ViscousThread& s){V3 p{};for(size_t i=0;i<s.links.size();i++)p=p+(s.nodes[i].v+s.nodes[i+1].v)*(s.links[i].volume*.5f);return p;}
static float Energy(const teaching::ViscousThread& s){float e=0;for(size_t i=0;i<s.links.size();i++)e+=s.links[i].volume*(Dot(s.nodes[i].v,s.nodes[i].v)+Dot(s.nodes[i+1].v,s.nodes[i+1].v))*.25f;return e;}
static bool Material(){
 volumeFluid::Settings c;volumeFluid::F4 collisions[8]{};c.gravity=0;c.breakup=0;auto s=Make();V3 p=Momentum(s);float energy=Energy(s);for(int k=0;k<120;k++)s.Step(1.f/120,k/120.f,c,collisions,{});CHECK(Length(Momentum(s)-p)<.01f&&Energy(s)<energy);printf("PASS viscous deformation dissipates energy %.3f -> %.3f; momentum error %.6f\n",energy,Energy(s),Length(Momentum(s)-p));
 s=Make();for(auto& n:s.nodes)n.v={3,2,1};V3 before=s.nodes[15].p;c.gravity=98;for(int k=0;k<36;k++)s.Step(1.f/120,k/120.f,c,collisions,{});CHECK(Length(s.nodes[15].p-(before+V3{.9f,.6f,.3f-98.f/120/120*36*37*.5f}))<.002f);printf("PASS detached ballistic translation; no global drag\n");
 s=Make();c.gravity=0;c.breakup=4;c.tension=100;c.viscosity=0;for(auto& n:s.nodes){n.p.x*=4;n.v={};}for(int k=0;k<300;k++)s.Step(1.f/120,1+k/120.f,c,collisions,{});CHECK(s.breaks>0);teaching::LiquidMesh m;std::vector<volumeFluid::Particle>b;s.Append(m,12,b);CHECK(m.components>=2&&Topology(m));double amount=0;for(auto& l:s.links)amount+=l.volume;CHECK(fabs(amount-s.volume)<.0001);printf("PASS necking/breakup preserves volume %.6f, cuts=%d\n",amount,s.breaks);
 volumeFluid::config={};teaching::Fluid a,bf;CHECK(a.Begin({0,0,50})&&bf.Begin({0,0,50}));for(int i=0;i<270;i++)a.Advance(1.f/30,{0,0,50},{1,0,0},{});for(int i=0;i<1080;i++)bf.Advance(1.f/120,{0,0,50},{1,0,0},{});CHECK(a.streams[0].nodes.size()==bf.streams[0].nodes.size());float error=0;for(size_t i=0;i<a.streams[0].nodes.size();i++)error=max(error,Length(a.streams[0].nodes[i].p-bf.streams[0].nodes[i].p));CHECK(error<.001f);printf("PASS 30/120 Hz fixed-step agreement maxError=%.8f\n",error);
 return true;
}
int main(int argc,char**argv){setvbuf(stdout,nullptr,_IONBF,0);if(argc>1&&strcmp(argv[1],"material")==0)return Material()?0:1;if(argc>1&&strcmp(argv[1],"variance")==0)return Variations()?0:1;return Sequence(argc>1?atoi(argv[1]):60,argc>2?atoi(argv[2]):0,argc>3)?0:1;}
