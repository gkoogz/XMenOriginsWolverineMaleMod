#define NECK_TEST_MAIN
#include "../raphe/raphe_test.cpp"
int main(int argc,char**argv){
 if(argc!=2)return 2;FILE* file=nullptr;if(fopen_s(&file,argv[1],"wb")||!file)return 3;
 bool visible[nrCount]{};for(auto i:MeridianRecipe::uncoveredIndices)visible[i]=true;
 double sum=0;unsigned count=0;
 for(int state:{0,1,2})for(float size:{1.f,50.f,100.f}){
  if(!RapheInit(state,size))return 1;SetJockstrapStyle(1);
  for(int frame=0;frame<8;frame++){
   UpdateConstraintSolver(1.f/60.f,sinf(frame*.3f)*.3f,cosf(frame*.4f)*.3f);double start=PerfClock();ApplyShape();if(frame){sum+=PerfClock()-start;count++;}
   for(unsigned i=0;i<nrCount;i++)if(visible[i])fwrite(nrPacked+i*32,1,20,file);
   void* data=nullptr;if(FAILED(graftBuffer->Lock(0,0,&data,0)))return 4;
   for(int s=0;s<2;s++)for(unsigned i=0;i<sharedBodyCount[s];i++){fwrite((unsigned char*)data+(sharedBodyFirst[s]+i)*32,1,20,file);fwrite(sharedBodyOutput[s]+i*32,1,20,file);}graftBuffer->Unlock();
  }
 }fclose(file);printf("Exposed anatomy and both body resources captured for 72 frames; surface %.4f ms\n",sum/count);return 0;
}
