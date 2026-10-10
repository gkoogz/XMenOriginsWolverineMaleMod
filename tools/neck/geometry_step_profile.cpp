#define NECK_TEST_MAIN
#include "../raphe/raphe_test.cpp"
int main(){
 if(!RapheInit(2,50))return 1;SetJockstrapStyle(1);double sum=0,peak=0;
 for(unsigned f=0;f<240;f++){
  UpdateConstraintSolver(1.f/60.f,sinf(f*.05f)*.3f,cosf(f*.08f)*.3f);
  double start=PerfClock();ApplyShape();double ms=PerfClock()-start;
  if(f>=60){sum+=ms;peak=max(peak,ms);}
 }printf("CPU-backed surface mean %.4f ms peak %.4f ms; not native GPU/frame timing\n",sum/180,peak);return 0;
}
