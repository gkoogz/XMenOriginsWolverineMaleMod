#define NECK_TEST_MAIN
#include "../raphe/raphe_test.cpp"
static bool CheckRender(){
 for(unsigned i=0;i<nrCount;i++){
  V3 p;memcpy(&p,nrPacked+i*32,12);CHECK(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z));
  if(nrDirect[i]!=65535)CHECK(!memcmp(nrPacked+i*32,rsPacked+nrDirect[i]*32,32));
  unsigned total=0;for(int k=0;k<4;k++){CHECK(nrPacked[i*32+20+k]<5);total+=nrPacked[i*32+24+k];}CHECK(total==255);
 }
 return true;
}
static bool NeckTests(){
 CHECK(nrCount<65536);
 for(unsigned i=0;i<nrCount;i++){double sum=0;for(unsigned j=nrRows[i];j<nrRows[i+1];j++){CHECK(nrSources[j]<rsCount);sum+=nrWeights[j];}CHECK(fabs(sum-1)<.00001);}
 std::map<std::pair<unsigned,unsigned>,int> edges;
 for(unsigned k=0;k<nrIndexCount;k+=3)for(int j=0;j<3;j++){unsigned a=nrIndices[k+j],b=nrIndices[k+(j+1)%3];CHECK(a<nrCount&&b<nrCount);if(a>b)std::swap(a,b);++edges[{a,b}];}
 unsigned boundary=0;for(auto e:edges){CHECK(e.second<=2);if(e.second==1){++boundary;CHECK(nrDirect[e.first.first]!=65535&&nrDirect[e.first.second]!=65535);}}
 CHECK(boundary==54);
 printf("PASS affine bindings, INDEX16, unchanged 54-edge attachment boundary, manifold edge incidence\n");
 for(int state=0;state<3;state++)for(float size:{25.f,50.f,85.f}){
  CHECK(RapheInit(state,size));CHECK(CheckRender());
  auto original=std::vector<unsigned char>(nrPacked,nrPacked+sizeof(nrPacked));UpdateNeckRender();CHECK(!memcmp(original.data(),nrPacked,sizeof(nrPacked)));
  printf("PASS state=%d size=%.0f finite geometry, exact original vertices/materials, normalized bone weights, repeat evaluation\n",state,size);
 }
 for(int state=0;state<3;state++){
  CHECK(RapheInit(state,70));std::vector<double> timing;
  for(int frame=0;frame<90;frame++){
   float t=frame/60.f;UpdateConstraintSolver(1.f/60,sinf(t*4)*.3f,cosf(t*3)*.3f);ApplyShape();CHECK(CheckRender());
   auto start=std::chrono::steady_clock::now();UpdateNeckRender();timing.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());
  }
  std::sort(timing.begin(),timing.end());printf("PASS motion state=%d 90 frames; final render binding median %.3f ms p95 %.3f ms (offline CPU)\n",state,timing[45],timing[85]);
 }
 return true;
}
int main(int argc,char** argv){
 setvbuf(stdout,nullptr,_IONBF,0);
 if(argc>1&&!strcmp(argv[1],"--test"))return NeckTests()?0:1;
 if(!RapheInit(argc>2?atoi(argv[2]):0,argc>3?(float)atof(argv[3]):50))return 1;
 const char* prefix=argc>1?argv[1]:"neck";WriteRapheFixture(prefix);char path[MAX_PATH];FILE* f=nullptr;
 sprintf_s(path,"%s.vertices",prefix);fopen_s(&f,path,"wb");fwrite(nrPacked,32,nrCount,f);fclose(f);
 sprintf_s(path,"%s.indices",prefix);fopen_s(&f,path,"wb");fwrite(nrIndices,2,nrIndexCount,f);fclose(f);
 return CheckRender()?0:1;
}
