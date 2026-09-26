#pragma once
// Render detail may run at half cadence; rest inputs and the coupled physics
// still advance on every frame. Never partially update the visible body weld.
static bool fullSurfaceEveryFrame=false,surfaceHoldNext=false;
static unsigned surfaceRefreshes=0,surfaceHolds=0;
static std::vector<unsigned char> surfaceRestBody;
static void FinishShapeRestLength(V3 tipSum,int tipCount){
  if(tipCount){V3 tip=tipSum/(float)tipCount;float newLength=max(8.f,min(60.f,Length(tip-ShaftRoot())));constraintRestLength=(constraintRestLength*.1656f+newLength*.08f)/.2456f;}
}
static void CaptureSurfaceRestBody(const unsigned char* body,UINT first){
  surfaceRestBody.resize(first*graftStride);
  memcpy(surfaceRestBody.data(),body,surfaceRestBody.size());
}
static void ApplyShape();
static void UpdateVisibleSurface(){
  if(!graftBuffer)return;
  struct Controls {float shape[7],physics[8],hang,glans;int state,pulse;};
  static Controls previous{};static IDirect3DVertexBuffer9* previousBuffer=nullptr;
  Controls current{};memcpy(current.shape,sliderUI,sizeof(current.shape));memcpy(current.physics,physUI,sizeof(current.physics));
  current.hang=hangUI;current.glans=glansUI;current.state=physicsState;current.pulse=throbMode;
  const UINT first=graftOffset/graftStride;
  bool forced=fullSurfaceEveryFrame||shapeDirty||!r14Ready||!shaftRestFrameReady||!eggRestReady||!constraintSolverReady
    ||previousBuffer!=graftBuffer||memcmp(&current,&previous,sizeof(current))!=0||surfaceRestBody.size()!=first*graftStride;
  if(surfaceHoldNext&&!forced){
    auto* body=surfaceRestBody.data();ResetPelvicAttachmentBody(body);
    V3 tip{};int count=0;PrepareShape(body+pelvisControlFirstVertex*graftStride,first,tip,count);
    FinishShapeRestLength(tip,count);
    surfaceHoldNext=false;++surfaceHolds;
    return;
  }
  ApplyShape();
  if(!shapeDirty&&r14Ready){previous=current;previousBuffer=graftBuffer;surfaceHoldNext=true;++surfaceRefreshes;}
}
