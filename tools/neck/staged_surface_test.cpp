#define NECK_TEST_MAIN
#include "../raphe/raphe_test.cpp"
int main(int argc,char**argv){
 if(argc!=2)return 2;FILE* file=nullptr;if(fopen_s(&file,argv[1],"wb")||!file)return 3;double surfaceMs=0;unsigned samples=0;
 for(int state:{0,1,2})for(float size:{1.f,50.f,100.f}){
  if(!RapheInit(state,size))return 1;
  for(int frame=0;frame<8;frame++){
   UpdateConstraintSolver(1.f/60.f,sinf(frame*.3f)*.3f,cosf(frame*.4f)*.3f);double began=PerfClock();ApplyShape();if(frame){surfaceMs+=PerfClock()-began;++samples;}
   void* data=nullptr;if(FAILED(graftBuffer->Lock(0,0,&data,0)))return 4;
   fwrite(data,32,50915,file);graftBuffer->Unlock();
   fwrite(nrPacked,1,sizeof(nrPacked),file);
   for(int s=0;s<2;s++)fwrite(sharedBodyOutput[s],32,sharedBodyCount[s],file);
  }
 }fclose(file);printf("Captured 72 complete geometry frames, all mechanical states and minimum/default/maximum size; steady surface %.4f ms samples=%u\n",surfaceMs/samples,samples);return 0;
}
