"""Compile the actual support/cache functions and check all cache-key inputs."""
from pathlib import Path
import argparse,subprocess
ap=argparse.ArgumentParser(__doc__);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
root=Path(__file__).resolve().parents[2]/'src/runtime'
def function(file,name,text=None):
    s=(root/file).read_text() if text is None else text;start=s.index('static ',s.rfind('\n',0,s.index(name+'('))+1);begin=s.index('{',start);depth=1;i=begin+1
    while depth:
        depth+=(s[i]=='{')-(s[i]=='}');i+=1
    return s[start:i]+'\n'
source='''#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
using std::max;using std::min;
struct V3{float x,y,z;};
V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V3 operator*(V3 a,float s){return {a.x*s,a.y*s,a.z*s};}
float Dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V3 eggRadii[2],cpBasis[2][3],cpNormal,lobePressureNormal[2];
float cpCompression[2],lobeCompression[2];
'''
source+=function('d3d9_proxy.cpp','LobePressureOffset')
for f in ['CPMul','CPDiv','CPRadii','CPOrient','CPUnorient','CPPairPressure','CPTransform','CPTranspose','CPSupportLocal']:source+=function('pouch_contact.h',f)
for f in ['PDSkinSupport','PDSkinExtents']:source+=function('compliant_dynamics.h',f)
reference=subprocess.check_output(['git','show','HEAD:src/runtime/pouch_contact.h'],cwd=root.parents[1],text=True)
source+=function('pouch_contact.h','CPSupportLocal',reference).replace('CPSupportLocal(','ReferenceSupport(')
source+='''int main(){
 unsigned random=941;auto sample=[&](){random=random*1664525u+1013904223u;return float(random>>8)/16777216.f;};
 for(int i=0;i<1000000;i++){V3 n={sample()*2-1,sample()*2-1,sample()*2-1},r={.01f+sample()*40,.01f+sample()*40,.01f+sample()*40};V3 a=ReferenceSupport(n,r),b=CPSupportLocal(n,r);if(memcmp(&a,&b,sizeof(a))){printf("FAIL support %d\\n",i);return 1;}}
 puts("PASS 1000000 exact support comparisons against committed reference");
 for(int s=0;s<2;s++){eggRadii[s]={5,4,6};cpBasis[s][0]={1,0,0};cpBasis[s][1]={0,1,0};cpBasis[s][2]={0,0,1};lobePressureNormal[s]={1,0,0};}cpNormal={0,1,0};
 for(int n=0;n<20000;n++){int s=n%2,k=(n/2)%20;float delta=(n%7==0?-.0003f:.0001f);
  float* key[20];for(int i=0;i<3;i++)key[i]=((float*)&eggRadii[s])+i;for(int i=0;i<9;i++)key[i+3]=((float*)cpBasis[s])+i;for(int i=0;i<3;i++)key[i+12]=((float*)&cpNormal)+i;key[15]=&cpCompression[s];for(int i=0;i<3;i++)key[i+16]=((float*)&lobePressureNormal[s])+i;key[19]=&lobeCompression[s];*key[k]+=delta;
  V3 expected={max(PDSkinSupport(s,{1,0,0}).x,-PDSkinSupport(s,{-1,0,0}).x),max(PDSkinSupport(s,{0,1,0}).y,-PDSkinSupport(s,{0,-1,0}).y),max(PDSkinSupport(s,{0,0,1}).z,-PDSkinSupport(s,{0,0,-1}).z)};
  for(int repeat=0;repeat<4;repeat++){V3 got=PDSkinExtents(s);if(memcmp(&expected,&got,sizeof(V3))){printf("FAIL input %d iteration %d\\n",k,n);return 1;}}
 }puts("PASS 80000 exact comparisons including all 20 cache inputs and both bodies");}
'''
cpp=a.output/'cache_test.cpp';cpp.write_text(source)
cmd=a.output/'test.cmd';cmd.write_text(f'@echo off\ncall "C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat" >nul\ncl /nologo /O2 /MT /EHsc /std:c++17 "{cpp.resolve()}" /Fo"{a.output.resolve()}/test.obj" /Fe"{a.output.resolve()}/test.exe"\nif errorlevel 1 exit /b 1\n"{a.output.resolve()}/test.exe"\n')
subprocess.run(['cmd','/d','/c',str(cmd.resolve())],check=True)
