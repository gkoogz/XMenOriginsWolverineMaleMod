#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <string>
#include "../src/runtime/necklace_contact.h"
static void require(bool ok,const char* msg){if(!ok){printf("FAIL: %s\n",msg);exit(1);}}
static void read(const std::string& path,void* data,size_t bytes){FILE* f=fopen(path.c_str(),"rb");require(f!=nullptr,path.c_str());require(fread(data,1,bytes,f)==bytes,"input length");fclose(f);}
int main(int argc,char** argv){
  require(argc==2,"pass replay data directory");std::string root=argv[1];NcContactSolver solver;NcRig last;
  for(int pose=0;pose<8;pose++){
    NcRig rig;for(bool& v:rig.valid)v=true;read(root+"/pose_"+std::to_string(pose)+".bin",rig.matrix,sizeof(rig.matrix));
    float expected[4];read(root+"/expected_"+std::to_string(pose)+".bin",expected,sizeof(expected));
    float shift[4];NcContactStats stats;require(solver.solve(rig,shift,stats),"captured pose solve");
    for(int i=0;i<4;i++)require(fabsf(shift[i]-expected[i])<.001f,"C++ grid agrees with independent all-triangle Python audit");
    require(stats.remaining<.0001f&&!stats.limited,"all measured contact constraints satisfied");
    NcRig rotated=rig;float a=.81f,c=cosf(a),s=sinf(a);
    for(int b=0;b<128;b++)for(int k=0;k<4;k++){
      rotated.matrix[b][k]=c*rig.matrix[b][k]+s*rig.matrix[b][8+k]+(k==3?61.f:0.f);
      rotated.matrix[b][4+k]=rig.matrix[b][4+k]+(k==3?-23.f:0.f);
      rotated.matrix[b][8+k]=-s*rig.matrix[b][k]+c*rig.matrix[b][8+k]+(k==3?48.f:0.f);
    }
    float other[4];require(solver.solve(rotated,other,stats),"rotated pose solve");
    for(int i=0;i<4;i++)require(fabsf(other[i]-shift[i])<.001f,"contact follows animated chest coordinate system");
    NcRig clear=rig;for(int b=103;b<=106;b++)for(int axis=0;axis<3;axis++)clear.matrix[b][axis*4+3]+=rig.matrix[6][axis*4]*12.f;
    require(solver.solve(clear,other,stats),"free swing solve");
    for(float f:other)require(f<.001f,"unobstructed necklace swing unchanged");
    printf("PASS recorded pose %d shifts %.4f %.4f %.4f %.4f\n",pose,shift[0],shift[1],shift[2],shift[3]);last=rig;
  }
  last.valid[6]=false;float shift[4];NcContactStats stats;require(!solver.solve(last,shift,stats),"missing chest transform bypasses correction");last.valid[6]=true;
  auto start=std::chrono::steady_clock::now();for(int i=0;i<200;i++){last.matrix[127][3]=(float)i;require(solver.solve(last,shift,stats),"benchmark solve");}
  double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/200;
  printf("PASS mean solve time %.3f ms; all replay/contact/transform tests passed\n",ms);
}
