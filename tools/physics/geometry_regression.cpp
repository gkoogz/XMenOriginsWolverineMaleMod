// Exercise the compressed operators against their scalar definitions,
// including contact penetration cases absent from a settled replay.
#define main GeometryReplayMain
#include "performance_replay.cpp"
#undef main
static uint32_t randomState=0x5ead1234;
static float Random(float extent=1.f){randomState=randomState*1664525u+1013904223u;return (float(randomState>>8)/16777215.f*2.f-1.f)*extent;}
static V3 RandomPoint(float extent=1.f){return {Random(extent),Random(extent),Random(extent)};}
static void ScalarKeepOutside(){
  for(unsigned k=0;k<cpActiveCount;k++){
    unsigned id=cpActive[k];V3 p=rsPositions[id];
    for(int side=0;side<2;side++){
      V3 local=CPLocalPoint(p,side);if(CPPouchLevel(local,cpRenderRadii[side])>=.018f)continue;
      for(int j=0;j<5;j++)local=local*(1.020f/max(1e-7f,1.f+CPPouchLevel(local,cpRenderRadii[side])));
      p=CPWorldPoint(local,side);
    }
    rsPositions[id]=p;
  }
}
int main(){
  unsigned failures=0;
  for(unsigned i=0;i<10000;i++){
    V3 a=RandomPoint(20),b=RandomPoint(20),c=RandomPoint(20),da=RandomPoint(10),db=RandomPoint(10),dc=RandomPoint(10);
    if(i%5==0)c=a+(b-a)*.5f;
    if(i%7==0)da=db=dc={};
    PreparedSurfaceLimit prepared;prepared.Prepare(a,b,c);
    float expected=SurfaceCorrectionLimit(a,b,c,da,db,dc,.35f),actual=prepared.Evaluate(da,db,dc,.35f);
    if(memcmp(&expected,&actual,sizeof(float)))failures++;
    V3 from=RandomPoint(),to=RandomPoint();
    if(i%11==0)to=from;if(i%13==0)to=from*-1.f;
    V3 oldRotation=RotateFromTo(a,from,to),newRotation=GeometryRotation(from,to).Apply(a);
    if(memcmp(&oldRotation,&newRotation,sizeof(V3)))failures++;
  }
  ResetStudyControls();InitializeBuffer();
  static V3 input[rsCount],expected[rsCount];
  for(unsigned pose=0;pose<24;pose++){
    sliderUI[0]=1.f+float(pose%3)*49.5f;sliderUI[2]=1.f+float(pose%4)*33.f;
    sliderUI[3]=1.f+float(pose%5)*24.75f;hangUI=1.f+float(pose%3)*49.5f;physicsState=pose%3;
    ApplyControlMapping();ApplyShape();UpdateConstraintSolver(1.f/30.f,Random(),Random());ApplyShape();
    for(unsigned k=0;k<cpActiveCount;k++){
      int side=k%2;V3 local=RandomPoint(1.5f);
      local={local.x*cpRenderRadii[side].x,local.y*cpRenderRadii[side].y,local.z*cpRenderRadii[side].z};
      rsPositions[cpActive[k]]=CPWorldPoint(local,side);
    }
    memcpy(input,rsPositions,sizeof(input));ScalarKeepOutside();memcpy(expected,rsPositions,sizeof(expected));
    memcpy(rsPositions,input,sizeof(input));CPKeepSkinOutside();
    for(unsigned i=0;i<rsCount;i++)if(memcmp(expected+i,rsPositions+i,sizeof(V3)))failures++;
  }
  printf("10000 area bounds, 10000 rotations, %u penetrated/exterior skin samples: failures=%u\n",24*cpActiveCount,failures);
  return failures?1:0;
}
