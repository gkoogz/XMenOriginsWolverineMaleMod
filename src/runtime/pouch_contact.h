#pragma once
// Geometry shared by the compliant dynamics solver and enclosing skin.
static bool cpReady=false;
static V3 cpBasis[2][3],cpOmega[2],cpNormal{0,1,0};
static float cpCompression[2]{},cpMinimumGap=1e9f,cpLastGap=0.f;
static V3 CPMul(V3 a,V3 b){return {a.x*b.x,a.y*b.y,a.z*b.z};}
static V3 CPDiv(V3 a,V3 b){return {a.x/b.x,a.y/b.y,a.z/b.z};}
static V3 CPRadii(int s){return CPMul(eggRadii[s],s?V3{5.724f/5.6240826f,4.86f/3.8670442f,7.81f/6.129288f}:V3{5.724f/5.6093187f,4.86f/3.871375f,7.93f/6.1588774f});}
static V3 CPRotate(V3 v,V3 axis,float angle){float c=cosf(angle),s=sinf(angle);return v*c+Cross(axis,v)*s+axis*(Dot(axis,v)*(1.f-c));}
// Positive rotation about lateral Y brings the upper poles forward (+X).
static V3 CPDesiredUp(int s){return CPRotate(Unit(BallAnchor(s)-ballNodes[s]),{0,1,0},.436332313f);}
static void CPEnsure(){if(cpReady)return;for(int s=0;s<2;s++){cpBasis[s][2]=CPDesiredUp(s);cpBasis[s][0]=Unit(Cross({0,1,0},cpBasis[s][2]));cpBasis[s][1]=Unit(Cross(cpBasis[s][2],cpBasis[s][0]));cpOmega[s]={};cpCompression[s]=0;}cpNormal={0,1,0};cpReady=true;}
static V3 CPOrient(V3 v,int s){return cpBasis[s][0]*v.x+cpBasis[s][1]*v.y+cpBasis[s][2]*v.z;}
static V3 CPUnorient(V3 v,int s){return {Dot(v,cpBasis[s][0]),Dot(v,cpBasis[s][1]),Dot(v,cpBasis[s][2])};}
static V3 CPPairPressure(V3 v,int s,bool inverse=false){float a=1.f-cpCompression[s],t=1.f/sqrtf(a);if(inverse){a=1.f/a;t=1.f/t;}return v*t+cpNormal*(Dot(v,cpNormal)*(a-t));}
static V3 CPTransform(V3 v,int s){return LobePressureOffset(CPPairPressure(CPOrient(v,s),s),s);}
static V3 CPInverse(V3 v,int s){return CPUnorient(CPPairPressure(LobePressureOffset(v,s,true),s,true),s);}
static V3 CPTranspose(V3 v,int s){return CPUnorient(CPPairPressure(LobePressureOffset(v,s),s),s);}
static V3 CPCenter(int s){return ballNodes[s]-cpBasis[s][2]*(CPRadii(s).z*.23f);}
static V3 CPSupportLocal(V3 n,V3 r){
 float h=sqrtf(n.x*n.x*r.x*r.x+n.y*n.y*r.y*r.y),v=n.z*r.z;
 if(h<1e-8f)return {0,0,copysignf(r.z,v)};
 float lo=-.999999f,hi=.999999f;
 for(int i=0;i<22;i++){float z=(lo+hi)*.5f,s=sqrtf(max(1e-12f,1.f-z*z)),d=h*(-.13f*s-(1.f-.13f*z)*z/s)+v;if(d>0)lo=z;else hi=z;}
 float z=(lo+hi)*.5f,section=(1.f-.13f*z)*sqrtf(max(0.f,1.f-z*z));return {r.x*r.x*n.x/h*section,r.y*r.y*n.y/h*section,r.z*z};
}
static V3 CPSupport(int s,V3 n){return CPTransform(CPSupportLocal(CPTranspose(n,s),CPRadii(s)),s);}
static float CPLevel(V3 p,int s,V3 radius){V3 a=CPDiv(CPInverse(p-CPCenter(s),s),radius);float t=1.f-.13f*max(-1.f,min(1.f,a.z));a.x/=t;a.y/=t;return Length(a)-1.f;}
static void CPShift(int s,V3 shift){ballNodes[s]=ballNodes[s]+shift;ballPrevious[s]=ballPrevious[s]+shift;nutNodes[s]=nutNodes[s]+shift;nutPrevious[s]=nutPrevious[s]+shift;neckNodes[s]=neckNodes[s]+shift*.5f;neckPrevious[s]=neckPrevious[s]+shift*.5f;}
static float CPTetherLimit(int s){
 // Derive reach from the authored suspension, including size and Hang.
 // Contact is allowed modest slack but cannot indefinitely lengthen it.
 return max(2.45f,Length(constraintBallRest[s]-RestBallAnchor(s)))*1.12f+CPRadii(s).y*.15f;
}
