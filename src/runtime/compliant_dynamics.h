#pragma once
// One XPBD solve for rod strain, suspension and all contacts.
// Positions are integrated once, constraints share generalized masses, and
// velocities are reconstructed once from the accepted configuration.
static const int pdBody0=shaftNodeCount,pdCount=shaftNodeCount+2;
struct PDConstraint {float normal=0.f;V3 tangent{};};
static V3 pdPosition[pdCount],pdOldPosition[pdCount],pdVelocity[pdCount];
static V3 pdOldBasis[2][3],pdThigh[4],pdOldThigh[4];
static float pdInvMass[pdCount],pdInvInertia[2],pdMaxTetherRatio=0.f,pdMinimumGap=1e9f;
static bool pdReady=false;
static float pdPairForce=0.f,pdShaftForce[2]{};
static V3 pdPressureDirection[2]={{0,-1,0},{0,1,0}};
static void ResetCompliantDynamics(){pdReady=false;pdMaxTetherRatio=0.f;pdMinimumGap=1e9f;pdPairForce=0.f;pdShaftForce[0]=pdShaftForce[1]=0.f;}
static void PDRotateBody(int s,V3 rotation){
 float angle=Length(rotation);if(angle<1e-9f)return;V3 axis=rotation/angle;
 for(int j=0;j<3;j++)cpBasis[s][j]=CPRotate(cpBasis[s][j],axis,angle);
 cpBasis[s][2]=Unit(cpBasis[s][2]);cpBasis[s][0]=Unit(cpBasis[s][0]-cpBasis[s][2]*Dot(cpBasis[s][0],cpBasis[s][2]));cpBasis[s][1]=Cross(cpBasis[s][2],cpBasis[s][0]);
}
static void PDSync(){for(int s=0;s<2;s++)ballNodes[s]=pdPosition[pdBody0+s]+cpBasis[s][2]*(CPRadii(s).z*.23f);}
static float PDEffectiveMass(int id,V3 arm,V3 direction){
 if(id<0)return 0.f;float w=pdInvMass[id];if(id>=pdBody0){V3 cross=Cross(arm,direction);w+=pdInvInertia[id-pdBody0]*Dot(cross,cross);}return w;
}
static void PDApply(int id,V3 arm,V3 impulse){
 if(id<0)return;pdPosition[id]=pdPosition[id]+impulse*pdInvMass[id];
 if(id>=pdBody0)PDRotateBody(id-pdBody0,Cross(arm,impulse)*pdInvInertia[id-pdBody0]);
}
static V3 PDPreviousPoint(int id,V3 arm){
 if(id<pdBody0)return pdOldPosition[id]+arm;
 int s=id-pdBody0;V3 local=CPUnorient(arm,s);return pdOldPosition[id]+pdOldBasis[s][0]*local.x+pdOldBasis[s][1]*local.y+pdOldBasis[s][2]*local.z;
}
static void PDContact(PDConstraint& c,int a,V3 ra,int b,V3 rb,V3 fixed,V3 oldFixed,V3 normal,float separation,float compliance,float dt){
 V3 localA=a>=pdBody0?CPUnorient(ra,a-pdBody0):ra,localB=b>=pdBody0?CPUnorient(rb,b-pdBody0):rb;
 V3 oldA=PDPreviousPoint(a,ra),oldB=b>=0?PDPreviousPoint(b,rb):oldFixed;
 float wa=PDEffectiveMass(a,ra,normal),wb=PDEffectiveMass(b,rb,normal),w=wa+wb,alpha=compliance/(dt*dt);
 if(w<1e-9f)return;float next=max(0.f,c.normal-(separation+alpha*c.normal)/(w+alpha)),change=next-c.normal;c.normal=next;
 PDApply(a,ra,normal*change);PDApply(b,rb,normal*(-change));
 if(c.normal<=0.f){c.tangent={};return;}
 // Material-point slip relative to the other body, including moving thighs.
 if(a>=pdBody0)ra=CPOrient(localA,a-pdBody0);if(b>=pdBody0)rb=CPOrient(localB,b-pdBody0);
 V3 da=pdPosition[a]+ra-oldA,db=b>=0?pdPosition[b]+rb-oldB:fixed-oldFixed;
 V3 slip=da-db;slip=slip-normal*Dot(slip,normal);float distance=Length(slip);if(distance<1e-8f)return;
 V3 tangent=slip/distance;float wt=PDEffectiveMass(a,ra,tangent)+PDEffectiveMass(b,rb,tangent);if(wt<1e-8f)return;
 V3 trial=c.tangent-slip/wt;trial=trial-normal*Dot(trial,normal);
 const float staticFriction=.48f,dynamicFriction=.32f;
 float magnitude=Length(trial);if(magnitude>staticFriction*c.normal)trial=trial*(dynamicFriction*c.normal/max(magnitude,1e-8f));
 V3 friction=trial-c.tangent;c.tangent=trial;PDApply(a,ra,friction);PDApply(b,rb,friction*-1.f);
}
static void PDDistance(int a,int b,float rest,float compliance,float& lambda,float dt){
 V3 delta=pdPosition[b]-pdPosition[a];float distance=Length(delta);if(distance<1e-7f)return;
 float alpha=compliance/(dt*dt),dl=(-(distance-rest)-alpha*lambda)/(pdInvMass[a]+pdInvMass[b]+alpha);lambda+=dl;
 V3 impulse=delta*(dl/distance);pdPosition[a]=pdPosition[a]-impulse*pdInvMass[a];pdPosition[b]=pdPosition[b]+impulse*pdInvMass[b];
}
static void PDBend(int i,float compliance,V3& lambda,float dt){
 V3 value=pdPosition[i-1]-pdPosition[i]*2.f+pdPosition[i+1];float alpha=compliance/(dt*dt),w=pdInvMass[i-1]+4.f*pdInvMass[i]+pdInvMass[i+1];
 V3 dl=(value+lambda*alpha)*(-1.f/(w+alpha));lambda=lambda+dl;
 pdPosition[i-1]=pdPosition[i-1]+dl*pdInvMass[i-1];pdPosition[i]=pdPosition[i]-dl*(2.f*pdInvMass[i]);pdPosition[i+1]=pdPosition[i+1]+dl*pdInvMass[i+1];
}
static void PDSuspension(int s,float& lambda,float& stop,float dt){
 int id=pdBody0+s;V3 arm=cpBasis[s][2]*(CPRadii(s).z*.23f),anchor=BallAnchor(s),delta=pdPosition[id]+arm-anchor;float distance=Length(delta);if(distance<1e-7f)return;
 V3 n=delta/distance;float rest=max(2.45f,Length(constraintBallRest[s]-RestBallAnchor(s))),stiff=max(0.f,min(1.f,physUI[4]/100.f));
 float compliance=.00006f*expf(-3.f*stiff),alpha=compliance/(dt*dt),w=PDEffectiveMass(id,arm,n);
 // A suspension transmits tension; it does not push when the sac is slack.
 float next=min(0.f,lambda+(-(distance-rest)-alpha*lambda)/(w+alpha)),dl=next-lambda;lambda=next;PDApply(id,arm,n*dl);
 delta=pdPosition[id]+cpBasis[s][2]*(CPRadii(s).z*.23f)-anchor;distance=Length(delta);n=Unit(delta);
 float hard=CPTetherLimit(s),nextStop=min(0.f,stop-(distance-hard)/max(PDEffectiveMass(id,arm,n),1e-8f));
 PDApply(id,arm,n*(nextStop-stop));stop=nextStop;
}
static V3 PDClosest(V3 p,V3 a,V3 b,float& t){V3 e=b-a;t=max(0.f,min(1.f,Dot(p-a,e)/max(1e-8f,Dot(e,e))));return a+e*t;}
static V3 PDSkinSupport(int s,V3 n){
 V3 r=CPRadii(s),scale=CPDiv(r,s?V3{5.724f,4.86f,7.81f}:V3{5.724f,4.86f,7.93f});
 // Match the enclosing skin used by ApplyPouchSurface, not an enlarged
 // sphere or the smaller internal support alone.
 V3 outer=r+CPMul({.80f,.95f,.65f},scale);
 return CPTransform(CPSupportLocal(CPTranspose(n,s),outer),s);
}
static void PDPair(PDConstraint& c,float dt){
 PDSync();cpNormal=Unit(pdPosition[pdBody0+1]-pdPosition[pdBody0]);V3 n=cpNormal;
 V3 ra=CPSupport(0,n),rb=CPSupport(1,n*-1.f);float gap=Dot(pdPosition[pdBody0+1]+rb-pdPosition[pdBody0]-ra,n);
 PDContact(c,pdBody0+1,rb,pdBody0,ra,{},{},n,gap-.025f,0.f,dt);
}
static void PDBodyCapsule(PDConstraint& c,int s,V3 a,V3 b,V3 oldA,V3 oldB,float radius,float dt){
 int id=pdBody0+s;float t;V3 q=PDClosest(pdPosition[id],a,b,t),n=Unit(pdPosition[id]-q),arm=PDSkinSupport(s,n*-1.f);
 float gap=Dot(pdPosition[id]+arm-q,n)-radius;
 PDContact(c,id,arm,-1,{},q,oldA+(oldB-oldA)*t,n,gap,.000002f,dt);
}
static void PDRodBody(PDConstraint& c,int s,int j,float dt){
 int id=pdBody0+s;float t;V3 q=PDClosest(pdPosition[id],pdPosition[j],pdPosition[j+1],t),n=Unit(pdPosition[id]-q),arm=PDSkinSupport(s,n*-1.f);
 float radius=logicalShaftBodyRadius*(.88f+.12f*min(1.f,float(j)/4.f));float gap=Dot(pdPosition[id]+arm-q,n)-radius;
 V3 local=CPUnorient(arm,s),previousPoint=PDPreviousPoint(id,arm);
 float w0=pdInvMass[j]*(1-t)*(1-t),w1=pdInvMass[j+1]*t*t,body=PDEffectiveMass(id,arm,n),alpha=.000001f/(dt*dt);
 float next=max(0.f,c.normal-(gap+alpha*c.normal)/(body+w0+w1+alpha)),dl=next-c.normal;c.normal=next;
 PDApply(id,arm,n*dl);pdPosition[j]=pdPosition[j]-n*(dl*pdInvMass[j]*(1-t));pdPosition[j+1]=pdPosition[j+1]-n*(dl*pdInvMass[j+1]*t);
 if(next<=0.f)return;
 arm=CPOrient(local,s);
 V3 slip=pdPosition[id]+arm-previousPoint-(pdPosition[j]-pdOldPosition[j])*(1-t)-(pdPosition[j+1]-pdOldPosition[j+1])*t;
 slip=slip-n*Dot(slip,n);float length=Length(slip);if(length<1e-8f)return;V3 tangent=slip/length;
 float wt=PDEffectiveMass(id,arm,tangent)+w0+w1;V3 trial=c.tangent-slip/max(wt,1e-8f);trial=trial-n*Dot(trial,n);
 float mag=Length(trial);if(mag>.48f*next)trial=trial*(.32f*next/max(mag,1e-8f));V3 impulse=trial-c.tangent;c.tangent=trial;
 PDApply(id,arm,impulse);pdPosition[j]=pdPosition[j]-impulse*(pdInvMass[j]*(1-t));pdPosition[j+1]=pdPosition[j+1]-impulse*(pdInvMass[j+1]*t);
}
static void StepConstraintSolver(float dt,float gait,float side){
 if(!constraintSolverReady)InitializeConstraintSolver();CPEnsure();constraintSolverState=physicsState;StepRootSuspension(dt,gait,side);
 // Deformation is frozen during a constraint solve. Updating it after the
 // last contact would reintroduce penetration and force feedback jitter.
 for(int s=0;s<2;s++){
  float blend=1.f-expf(-dt/.12f);cpCompression[s]+=(min(.065f,pdPairForce*.000035f)-cpCompression[s])*blend;
  lobeCompression[s]+=(min(.065f,pdShaftForce[s]*.000025f)-lobeCompression[s])*blend;
  lobePressureNormal[s]=Unit(lobePressureNormal[s]*(1.f-blend)+pdPressureDirection[s]*blend);
 }
 V3 targets[4];CollisionCapsules(targets[0],targets[1],targets[2],targets[3]);
 if(!pdReady){for(int i=0;i<shaftNodeCount;i++)pdVelocity[i]={};for(int s=0;s<2;s++)pdVelocity[pdBody0+s]={};memcpy(pdThigh,targets,sizeof(targets));pdReady=true;}
 memcpy(pdOldThigh,pdThigh,sizeof(pdThigh));float kinematicBlend=1.f-expf(-dt/.035f);
 for(int i=0;i<4;i++)pdThigh[i]=pdThigh[i]+(targets[i]-pdThigh[i])*kinematicBlend;
 float shaftMass=.75f+physValues[1]*.0125f,bodyMass=.75f+physValues[5]*.0125f;
 for(int i=0;i<shaftNodeCount;i++){pdPosition[i]=shaftNodes[i];pdInvMass[i]=i<2?0.f:1.f/shaftMass;}
 for(int s=0;s<2;s++){pdPosition[pdBody0+s]=CPCenter(s);pdInvMass[pdBody0+s]=1.f/bodyMass;pdInvInertia[s]=5.f/(bodyMass*Dot(CPRadii(s),CPRadii(s)));for(int j=0;j<3;j++)pdOldBasis[s][j]=cpBasis[s][j];}
 memcpy(pdOldPosition,pdPosition,sizeof(pdPosition));
 float shaftDrag=.9f+(100.f-physUI[2])*.018f,bodyDrag=1.f+(100.f-physUI[6])*.025f;
 for(int i=2;i<pdCount;i++){
  float response=i<pdBody0?(.65f+physValues[3]*.009f):(.65f+physValues[7]*.009f),gravity=i<pdBody0?ModeValue(5.f,42.f,110.f):72.f;
  V3 acceleration{0.f,side*86.f*response,gait*86.f*response-gravity};float drag=i<pdBody0?shaftDrag:bodyDrag;
  pdVelocity[i]=(pdVelocity[i]+acceleration*dt)*expf(-drag*dt);pdPosition[i]=pdPosition[i]+pdVelocity[i]*dt;
 }
 for(int s=0;s<2;s++){
  V3 torque=Cross(cpBasis[s][2],CPDesiredUp(s))*10.f;cpOmega[s]=(cpOmega[s]+torque*dt)*expf(-6.f*dt);PDRotateBody(s,cpOmega[s]*dt);
 }
 V3 root=ShaftRoot(),direction=LiveRootDirection();float segment=constraintRestLength/(shaftNodeCount-1);
 pdPosition[0]=root;pdPosition[1]=root+direction*segment;
 float lengthLambda[shaftNodeCount]{},tetherLambda[2]{},stopLambda[2]{};V3 bendLambda[shaftNodeCount]{};
 PDConstraint pair,thigh[2][2],pelvis[2],rodContact[2][shaftNodeCount],rodThigh[shaftNodeCount][2];
 float stiffness=max(0.f,min(1.f,physUI[0]/100.f));float bendCompliance=ModeValue(.00000001f,.00008f,.0015f)*expf((.5f-stiffness)*3.f);
 for(int iteration=0;iteration<24;iteration++){
  for(int i=0;i<shaftNodeCount-1;i++)PDDistance(i,i+1,segment,.0000001f,lengthLambda[i],dt);
  // A rod supported at the pelvis has a reinforced proximal section. Its
  // flexural compliance increases continuously toward the free end.
  for(int i=1;i<shaftNodeCount-1;i++){float t=float(i-1)/(shaftNodeCount-2);PDBend(i,bendCompliance*(.02f+.98f*t*t),bendLambda[i],dt);}
  for(int i=0;i<shaftNodeCount;i++)shaftNodes[i]=pdPosition[i];PDSync();
  for(int s=0;s<2;s++)PDSuspension(s,tetherLambda[s],stopLambda[s],dt);
  for(int s=0;s<2;s++){
   for(int j=0;j<2;j++)PDBodyCapsule(thigh[s][j],s,pdThigh[j*2],pdThigh[j*2+1],pdOldThigh[j*2],pdOldThigh[j*2+1],7.2f,dt);
   PDBodyCapsule(pelvis[s],s,{3.f,0.f,70.f},{5.4f,0.f,86.f},{3.f,0.f,70.f},{5.4f,0.f,86.f},6.4f,dt);
   for(int j=1;j<shaftNodeCount-2;j++)PDRodBody(rodContact[s][j],s,j,dt);
  }
  for(int i=2;i<shaftNodeCount;i++)for(int j=0;j<2;j++){
   float t;V3 q=PDClosest(pdPosition[i],pdThigh[j*2],pdThigh[j*2+1],t),n=Unit(pdPosition[i]-q),old=pdOldThigh[j*2]+(pdOldThigh[j*2+1]-pdOldThigh[j*2])*t;
   float gap=Length(pdPosition[i]-q)-7.2f-logicalShaftBodyRadius*.85f;
   PDContact(rodThigh[i][j],i,{},-1,{},q,old,n,gap,.000002f,dt);
  }
  PDPair(pair,dt);
 }
 // One velocity update from accepted positions. No velocity-generating
 // hard projections or displacement caps are layered on afterwards.
 for(int i=2;i<pdCount;i++)pdVelocity[i]=(pdPosition[i]-pdOldPosition[i])/dt;
 for(int s=0;s<2;s++){
  V3 spin{};for(int j=0;j<3;j++)spin=spin+Cross(pdOldBasis[s][j],cpBasis[s][j]);cpOmega[s]=spin*(.5f/dt);
 }
 for(int i=0;i<shaftNodeCount;i++){shaftNodes[i]=pdPosition[i];shaftPrevious[i]=pdPosition[i]-pdVelocity[i]*dt;}
 PDSync();
 for(int s=0;s<2;s++){
  ballPrevious[s]=ballNodes[s]-pdVelocity[pdBody0+s]*dt;V3 oldNeck=neckNodes[s],oldNut=nutNodes[s];
  neckNodes[s]=BallAnchor(s)+(ballNodes[s]-BallAnchor(s))*.43f;nutNodes[s]=pdPosition[pdBody0+s];neckPrevious[s]=oldNeck;nutPrevious[s]=oldNut;
  pdMaxTetherRatio=max(pdMaxTetherRatio,Length(ballNodes[s]-BallAnchor(s))/CPTetherLimit(s));
  // Force-derived bounded yielding, shared by collision and skin transforms.
  pdPairForce=pair.normal/(dt*dt);
  float load=0.f;int contact=1;for(int j=1;j<shaftNodeCount-2;j++)if(rodContact[s][j].normal>load){load=rodContact[s][j].normal;contact=j;}
  float t;V3 q=PDClosest(pdPosition[pdBody0+s],shaftNodes[contact],shaftNodes[contact+1],t);pdPressureDirection[s]=Unit(pdPosition[pdBody0+s]-q);
  pdShaftForce[s]=load/(dt*dt);
 }
 cpNormal=Unit(CPCenter(1)-CPCenter(0));cpLastGap=Dot(CPCenter(1)+CPSupport(1,cpNormal*-1.f)-CPCenter(0)-CPSupport(0,cpNormal),cpNormal);
 cpMinimumGap=min(cpMinimumGap,cpLastGap);pdMinimumGap=min(pdMinimumGap,cpLastGap);
}
