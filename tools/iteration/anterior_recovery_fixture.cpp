#include <malemod/physics/anterior_envelope.hpp>
#include <cstdio>
#include <cstring>
static bool fixtureEnvelopeEnabled=false;
#include "runtime.cpp"
int main(int argc,char** argv){
 if(argc!=3)return 2;
 fixtureEnvelopeEnabled=std::strcmp(argv[1],"enabled")==0;
 const bool trapped=std::strcmp(argv[2],"trapped")==0;
 namespace k=malemod::surface::source;
 malemod::surface::Session session;
 for(int i=0;i<120;i++)session.Step();
 auto before=session.Read();
 if(trapped)for(int i=2;i<k::shaftNodeCount;i++){
  auto p=k::shaftNodes[1];float step=k::constraintRestLength/(k::shaftNodeCount-1);
  p.x+=.1f*(i-1);p.y=0;p.z-=step*(i-1);
  k::shaftNodes[i]=k::shaftPrevious[i]=k::pdPosition[i]=k::pdOldPosition[i]=p;
  k::pdVelocity[i]={};
 }
 for(int i=0;i<600;i++){
  malemod::surface::Frame frame;
  if(!trapped&&i<240){frame.pitchForce=.7f*std::sin(i*.13f);frame.yawForce=.7f*std::cos(i*.17f);}
  session.Step(frame);
 }
 auto after=session.Read();auto mid=after.shaftGuide[6];
 float center=.5f*(after.lobeCenters[0].x+after.lobeCenters[1].x),lobeDrift=0;
 for(int s=0;s<2;s++){
  auto a=before.lobeCenters[s],b=after.lobeCenters[s];float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;
  lobeDrift=std::max(lobeDrift,std::sqrt(x*x+y*y+z*z));
 }
 bool finite=true;for(auto p:after.anatomy.positions)finite=finite&&std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);
 bool pass=finite&&(fixtureEnvelopeEnabled?(mid.x-center>2&&lobeDrift<2):(mid.x<center));
 std::printf("{\"enabled\":%s,\"trappedFixture\":%s,\"shaftAnteriorDistance\":%.6f,\"lobeDrift\":%.6f,\"finite\":%s,\"passed\":%s}\n",
 fixtureEnvelopeEnabled?"true":"false",trapped?"true":"false",mid.x-center,lobeDrift,finite?"true":"false",pass?"true":"false");
 return pass?0:1;
}
