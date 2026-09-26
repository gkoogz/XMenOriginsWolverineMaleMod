// Compile against both the reference and candidate runtime. The runner compares
// every frame, including packed lighting/UV/skin attributes and body attachment.
#ifndef RUNTIME_SOURCE
#define RUNTIME_SOURCE "../../src/runtime/d3d9_proxy.cpp"
#endif
#include RUNTIME_SOURCE
#include "../r14/cpu_buffer.h"
static uint64_t Hash(const void* raw,size_t size){
  const auto* bytes=static_cast<const unsigned char*>(raw);uint64_t h=14695981039346656037ull;
  for(size_t i=0;i<size;i++){h^=bytes[i];h*=1099511628211ull;}return h;
}
static void InitializeBuffer(){
  graftBuffer=new CpuVertexBuffer(50915*32);graftOffset=47050*32;
  void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);memset(raw,0,50915*32);
  for(UINT i=0;i<collarNormalTriangleCount*3;i++)memcpy((char*)raw+collarNormalTriangleIndices[i]*32,collarNormalTriangleBasePositions+i*3,12);
  for(UINT i=0;i<pelvisControlCount;i++)memcpy((char*)raw+pelvisControlIndices[i]*32,pelvisControlBasePositions+i*3,12);
  for(UINT i=0;i<graftCount;i++)memcpy((char*)raw+(47050+i)*32,morph_base+i*3,12);
  graftBuffer->Unlock();
}
int main(int argc,char** argv){
  if(argc<3)return 2;int scenario=atoi(argv[2]),frames=argc>3?atoi(argv[3]):240;
  ResetStudyControls();InitializeBuffer();
  if(scenario==1||scenario==4){sliderUI[0]=100;sliderUI[1]=100;sliderUI[2]=100;sliderUI[3]=100;sliderUI[4]=10;hangUI=100;}
  if(scenario==2){for(int i=0;i<7;i++)sliderUI[i]=1;hangUI=1;glansUI=0;}
  if(scenario==3){physicsState=0;sliderUI[4]=27;}
  if(scenario==5){physicsState=1;hangUI=1;}
  ApplyControlMapping();ApplyShape();
  FILE* out=nullptr;fopen_s(&out,argv[1],"w");if(!out)return 3;
  fprintf(out,"frame,mesh,body,physics,ms\n");LARGE_INTEGER frequency;QueryPerformanceFrequency(&frequency);
  float minArea=1e30f;unsigned invalid=0;float maxSeam=0;
  for(int frame=0;frame<frames;frame++){
    float fps=scenario==7?15.f:60.f,time=frame/fps;
    if(scenario==6){
      // Reuse a shape, change every key input separately, reset controls and
      // recreate CPU/GPU state. Changing only glans/physics must remain live.
      int phase=frame/12;
      if(frame%12==0){
        if(phase<7)sliderUI[phase]=phase%2?100.f:1.f;
        else if(phase==7)hangUI=1;
        else if(phase==8)physicsState=0;
        else if(phase==9)physicsState=1;
        else if(phase==10)physicsState=2;
        else if(phase==11)glansUI=0;
        else if(phase==12)glansUI=100;
        else if(phase==13)for(int i=0;i<8;i++)physUI[i]=i%2?100.f:1.f;
        else if(phase==14)ResetStudyControls();
        else if(phase==15){
          ReleaseR14();graftBuffer->Release();graftBuffer=nullptr;
          constraintSolverReady=false;shaftRestFrameReady=false;constraintAccumulator=0;constraintSolverState=-1;
          InitializeBuffer();
        }
      }
    }
    throbMode=scenario==4?3:scenario==5?1:scenario==7?2:0;
    throbSizePulse=ThrobEnvelope(fmodf(time,3.f),.20f,1.05f,false);
    float cycle=time<5.75f?time:1.15f+fmodf(time-1.15f,4.6f);
    throbTwitchPulse=ThrobEnvelope(cycle,1.15f,4.6f,true);
    throbAngleSizePulse=ThrobEnvelope(cycle,1.15f,1.05f,true);
    collisionCapsuleOverride=true;float shift=.8f*sinf(time*1.1f);
    overrideLeftA={2.f,-7.8f+shift,79.f};overrideRightA={2.f,7.8f+shift,79.f};
    overrideLeftB={1.f,-8.2f-shift*.35f,43.f};overrideRightB={1.f,8.2f-shift*.35f,43.f};
    LARGE_INTEGER begin,end;QueryPerformanceCounter(&begin);
    ApplyControlMapping();UpdateConstraintSolver(1.f/fps,.7f*sinf(time*5),.7f*sinf(time*7));ApplyShape();
    QueryPerformanceCounter(&end);
    V3 dynamics[shaftNodeCount*2+12];size_t offset=0;
    auto append=[&](const V3* p,size_t count){memcpy(dynamics+offset,p,count*sizeof(V3));offset+=count;};
    append(shaftNodes,shaftNodeCount);append(shaftPrevious,shaftNodeCount);
    append(ballNodes,2);append(ballPrevious,2);append(neckNodes,2);append(neckPrevious,2);append(nutNodes,2);append(nutPrevious,2);
    void* raw=nullptr;graftBuffer->Lock(0,0,&raw,0);
    fprintf(out,"%d,%016llx,%016llx,%016llx,%.9f\n",frame,Hash(rsPacked,sizeof(rsPacked)),Hash(raw,50915*32),Hash(dynamics,sizeof(dynamics)),double(end.QuadPart-begin.QuadPart)*1000/double(frequency.QuadPart));
    graftBuffer->Unlock();
    for(unsigned i=0;i<rsCount;i++)if(!std::isfinite(rsPositions[i].x)||!std::isfinite(rsPositions[i].y)||!std::isfinite(rsPositions[i].z))invalid++;
    if(frame%12==0){
      for(unsigned i=0;i<rsIndexCount;i+=3)minArea=min(minArea,Length(Cross(rsPositions[rsIndices[i+1]]-rsPositions[rsIndices[i]],rsPositions[rsIndices[i+2]]-rsPositions[rsIndices[i]])));
      for(unsigned i=0;i<paSeamCount;i++)maxSeam=max(maxSeam,Length(r14Positions[paSeamR14[i]]-paBodyBefore[paSeamBody[i]]));
    }
  }
  fclose(out);printf("frames=%d invalid=%u min_area=%g max_seam=%g\n",frames,invalid,minArea,maxSeam);
#ifdef PREPARED_SHAPE_TEST
  printf("prepared_builds=%u prepared_hits=%u\n",preparedShapeBuilds,preparedShapeHits);
  if(preparedShapeBuilds==0||preparedShapeHits==0)return 5;
#endif
  return invalid||minArea<1e-10f||maxSeam>1e-4f?4:0;
}
