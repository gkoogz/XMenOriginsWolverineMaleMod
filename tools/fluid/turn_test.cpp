#define main ThreadReferenceMain
#include "thread_test.cpp"
#undef main
static bool RapidTurns(int fps,int mode){
 volumeFluid::config={};auto& c=volumeFluid::config;c.viscosity=24;c.catchPlane=mode==2;c.catchDepth=15;
 teaching::Fluid f;CHECK(f.Begin({20,0,50}));float worst=0,aspect=0;double massError=0;int cutCount=0;size_t maxNodes=0;
 for(int frame=0;frame<16*fps;frame++){
  float t=(frame+1.f)/fps;
  float angle=mode==0?0.f:(t<7.3f?0.f:t<7.35f?(t-7.3f)*62.831853f:3.14159265f);
  if(mode==2&&t>8)angle+=sinf((t-8)*9)*1.5f;
  V3 direction{cosf(angle),sinf(angle),.12f},tip={20*cosf(angle),20*sinf(angle),50};
  f.Advance(1.f/fps,tip,Unit(direction),{});CHECK(f.ready);
  for(const auto& s:f.streams){double total=s.retiredVolume;for(const auto& link:s.links)total+=link.volume;for(const auto& node:s.nodes){total+=node.lump;CHECK(Finite(node.p)&&Finite(node.v)&&node.lump>=0);}massError=max(massError,fabs(total-s.volume));}
  for(const auto& s:f.streams){maxNodes=max(maxNodes,s.nodes.size());cutCount=max(cutCount,s.breaks);for(size_t i=0;i<s.links.size();i++)if(!s.links[i].broken){float len=Length(s.nodes[i+1].p-s.nodes[i].p);worst=max(worst,len);aspect=max(aspect,len/sqrtf(max(1e-8f,s.links[i].volume)/(3.14159265f*max(.001f,len))));}}
 }
 CHECK(worst<=teaching::ViscousThread::MaxSpan(c)+.001f&&maxNodes<1000&&massError<.0001);
 printf("PASS TURN fps=%d mode=%d max connected length=%.5f aspect=%.3f cuts=%d peak nodes=%u mass error=%.8f\n",fps,mode,worst,aspect,cutCount,(unsigned)maxNodes,massError);return true;
}
static bool SeparationConservation(){
 volumeFluid::Settings c;auto s=Make();for(size_t i=15;i<s.nodes.size();i++)s.nodes[i].p.y+=50;
 double initial=s.volume;V3 before=Momentum(s);s.SeparateOverstretched(c);
 CHECK(s.breaks==1&&s.links[14].broken&&s.nodes[14].lump==.5f&&s.nodes[15].lump==.5f);
 CHECK(Length(Momentum(s)-before)<.00001f);double remaining=0;for(const auto& l:s.links)remaining+=l.volume;for(const auto& n:s.nodes)remaining+=n.lump;CHECK(fabs(remaining-initial)<.00001);
 teaching::LiquidMesh mesh;std::vector<volumeFluid::Particle> bounds;s.Append(mesh,12,bounds);CHECK(mesh.components==2&&Topology(mesh));
 for(size_t i=0;i<mesh.indices.size();i+=3){for(int j=0;j<3;j++)CHECK(Length(mesh.vertices[mesh.indices[i+j]].p-mesh.vertices[mesh.indices[i+(j+1)%3]].p)<5);}
 // First/last links and one-node fragments must be safe and retain all mass.
 s.SeparateOverstretched(c);s.nodes.front().p.y-=40;s.nodes.back().p.y+=40;s.SeparateOverstretched(c);mesh.Clear();bounds.clear();s.Append(mesh,12,bounds);CHECK(mesh.components==4&&Topology(mesh));
 volumeFluid::F4 collision[14]{};for(int i=0;i<120;i++)s.Step(1.f/120,i/120.f,c,collision,{});
 before=Momentum(s);s.Feed(2,{80,0,50},{1,0,0},{20,0,0},1,1.f/120,c);CHECK(s.links[s.links.size()-2].broken);CHECK(Length(Momentum(s)-(before+V3{40,0,0}))<.0001f);
 double total=s.retiredVolume;for(const auto& l:s.links)total+=l.volume;for(const auto& n:s.nodes)total+=n.lump;CHECK(fabs(total-s.volume)<.0001);
 printf("PASS splits conserve volume/momentum, create separate watertight rounded components with no bridging triangles, handle endpoint fragments and moving-outlet restart\n");return true;
}
static bool ViscosityResponse(){
 auto low=Make(),high=low;volumeFluid::Settings a,b;a.gravity=b.gravity=0;a.breakup=b.breakup=0;a.viscosity=16;b.viscosity=24;volumeFluid::F4 collision[14]{};
 V3 momentum=Momentum(low);for(int i=0;i<20;i++){low.Step(1.f/120,i/120.f,a,collision,{});high.Step(1.f/120,i/120.f,b,collision,{});}
 CHECK(Energy(high)<Energy(low)&&Length(Momentum(high)-momentum)<.001f);CHECK(high.breaks==0);
 printf("PASS increased viscosity damps relative motion %.4f -> %.4f without breaking a coherent strand or damping bulk momentum\n",Energy(low),Energy(high));return true;
}
int main(){return SeparationConservation()&&ViscosityResponse()&&RapidTurns(60,0)&&RapidTurns(30,1)&&RapidTurns(120,1)&&RapidTurns(60,2)?0:1;}
