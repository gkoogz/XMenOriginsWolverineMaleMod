#pragma once
// Native adapter math. Engine palette layout remains outside the shared solver.
#include <malemod/garments/jockstrap.hpp>
#include <memory>
namespace JockstrapKinematics {
namespace G=malemod::garments;
using Matrix=std::array<double,12>;
inline Matrix Identity(){return {1,0,0,0,0,1,0,0,0,0,1,0};}
inline G::Point Direction(const Matrix& a,G::Point p){G::Point q{};for(unsigned i=0;i<3;i++)for(unsigned j=0;j<3;j++)q[i]+=a[i*4+j]*p[j];return q;}
inline G::Point Point(const Matrix& a,G::Point p){auto q=Direction(a,p);for(unsigned i=0;i<3;i++)q[i]+=a[i*4+3];return q;}
inline Matrix Multiply(const Matrix& a,const Matrix& b){Matrix q{};for(unsigned i=0;i<3;i++){for(unsigned j=0;j<3;j++)for(unsigned k=0;k<3;k++)q[i*4+j]+=a[i*4+k]*b[k*4+j];q[i*4+3]=a[i*4+3];for(unsigned k=0;k<3;k++)q[i*4+3]+=a[i*4+k]*b[k*4+3];}return q;}
inline Matrix Inverse(const Matrix& a){auto x=G::Cross({a[4],a[5],a[6]},{a[8],a[9],a[10]}),y=G::Cross({a[8],a[9],a[10]},{a[0],a[1],a[2]}),z=G::Cross({a[0],a[1],a[2]},{a[4],a[5],a[6]});double d=G::Dot({a[0],a[1],a[2]},x);if(!std::isfinite(d)||std::abs(d)<1e-12)throw std::invalid_argument("Garment native transform singular");Matrix q{x[0]/d,y[0]/d,z[0]/d,0,x[1]/d,y[1]/d,z[1]/d,0,x[2]/d,y[2]/d,z[2]/d,0};auto t=Direction(q,{a[3],a[7],a[11]});for(unsigned i=0;i<3;i++)q[i*4+3]=-t[i];return q;}
// Same inverse linear coefficients and division order as Inverse. Normal
// sampling never consumes the inverse translation, so omit only that work.
inline Matrix InverseLinear(const Matrix& a){auto x=G::Cross({a[4],a[5],a[6]},{a[8],a[9],a[10]}),y=G::Cross({a[8],a[9],a[10]},{a[0],a[1],a[2]}),z=G::Cross({a[0],a[1],a[2]},{a[4],a[5],a[6]});double d=G::Dot({a[0],a[1],a[2]},x);if(!std::isfinite(d)||std::abs(d)<1e-12)throw std::invalid_argument("Garment native transform singular");return {x[0]/d,y[0]/d,z[0]/d,0,x[1]/d,y[1]/d,z[1]/d,0,x[2]/d,y[2]/d,z[2]/d,0};}
inline G::Point Normal(const Matrix& a,G::Point p){auto b=Inverse(a);G::Point q{};for(unsigned i=0;i<3;i++)for(unsigned j=0;j<3;j++)q[i]+=b[j*4+i]*p[j];return G::Unit(q);}
inline Matrix ActorFromShader(const float* a){Matrix q{};for(unsigned i=0;i<3;i++)for(unsigned j=0;j<4;j++)q[i*4+j]=a[j*4+i];for(double f:q)if(!std::isfinite(f))throw std::invalid_argument("Nonfinite garment actor transform");Inverse(q);return q;}
struct Pose {std::array<std::array<Matrix,75>,3> skin{},world{};std::array<Matrix,3> actor{},inverseActor{};std::array<unsigned,3> count{};unsigned long long serial=0;bool title=false;};
// Timestamp accumulator for latest-request workers. Calls without a visible
// anatomy surface suspend rather than turning loading/menu wall time into dt.
class Clock {
 std::uint32_t tick_=0;unsigned long long epoch_=0;double active_=0;bool ready_=false,suspended_=false;
 public:
 struct Tick{double activeSeconds=0;bool reset=false;};
 void Reset(){ready_=suspended_=false;active_=0;epoch_=0;}
 void Suspend(std::uint32_t now){if(ready_)tick_=now;suspended_=true;}
 Tick Advance(std::uint32_t now,unsigned long long epoch){if(!ready_||epoch_!=epoch){tick_=now;epoch_=epoch;active_=0;ready_=true;suspended_=false;return {active_,true};}std::uint32_t elapsed=now-tick_;tick_=now;if(suspended_){suspended_=false;return {active_,false};}// A render/driver hitch is not a character or material discontinuity.
 // Preserve active time; loading, pauses and transitions call Suspend/Reset.
 active_+=elapsed*.001;return {active_,false};}
};
}
