"""Replay an exact built adapter's CPU chart block without a game or graphics SDK.

Private pose buffers are inputs, never repository assets. This measures only
post-skinning CPU phases; it does not measure FPS, GPU work or visual acceptance.
The block is extracted from the selected build, with exact splice checks.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--build', type=Path, required=True)
    ap.add_argument('--poses', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--limit', type=int, default=24)
    ap.add_argument('--interpolate',type=int,default=0,help='Synthetic intermediate physics poses per historical pair, not native evidence')
    ap.add_argument('--repeats', type=int, default=10)
    ap.add_argument('--vcvars', type=Path, default=Path('C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat'))
    args = ap.parse_args()
    if not 1 <= args.limit <= 256 or not 1 <= args.repeats <= 100:
        ap.error('Use limit 1..256 and repeats 1..100')
    build, output = args.build.resolve(), args.output.resolve()
    if output.exists():
        raise SystemExit('Choose a fresh output directory')
    native = build/'source/src/runtime'
    source = native/'meridian_adapter.h'
    provenance = json.loads((build/'provenance.json').read_text())
    for relative, expected in provenance['candidateInputSHA256'].items():
        if relative.startswith('base/') or relative in ('source/src/runtime/meridian_adapter.h', 'source/src/runtime/meridian_recipe.h'):
            if digest(build/relative) != expected:
                raise SystemExit('Build input differs: '+relative)
    text = source.read_text()
    block = text.split(' try{\n  M::CircularSection rings[7];', 1)[1].split(' }catch(const std::exception& e){static LONG reported=', 1)[0]
    block = '  M::CircularSection rings[7];'+block
    def splice(old, new):
        nonlocal block
        if block.count(old) != 1:
            raise SystemExit('Profile splice contract changed: '+old[:70])
        block = block.replace(old, new)
    splice('  std::vector<M::Hull> hulls;', '  Record("proxy", phase); phase=Clock::now();\n  std::vector<M::Hull> hulls;')
    splice('  const auto raw=points;', '  Record("support", phase);\n  const auto raw=points;')
    splice('   M::FitSeam(points,MeridianRecipe::columns,points[MeridianRecipe::clothCount-1],liveAxis,hulls,.12f,6.f);', '   {Timer timer("seam"); M::FitSeam(points,MeridianRecipe::columns,points[MeridianRecipe::clothCount-1],liveAxis,hulls,.12f,6.f);}')
    splice('   M::SeedMeridians(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::rowHeights,liveAxis);', '   {Timer timer("seed"); M::SeedMeridians(points,MeridianRecipe::columns,MeridianRecipe::rows,MeridianRecipe::rowHeights,liveAxis);}')
    lines = [line for line in block.splitlines() if line.startswith('   receipt=M::ClearMeridians(')]
    if len(lines) != 1 or not lines[0].endswith(('&certificates);', '&certificates,7);')):
        raise SystemExit('Profile clearance call contract changed')
    line = lines[0]
    splice(line, '   {Timer timer("clearance");'+line.strip()+'}')
    line = '   continuity.Remember(points,raw,anchors,MeridianRecipe::columns,MeridianRecipe::clothCount);'
    splice(line, '   {Timer timer("remember");'+line.strip()+'}')
    if '  certified=follower.Move(' in block:
        line=next(line for line in block.splitlines() if line.startswith('  certified=follower.Move('))
        splice(line,'  {Timer timer("follow");'+line.strip()+'}')
        splice('   follower.Remember(points,raw,followRig,MeridianRecipe::columns,MeridianRecipe::clothCount);','   {Timer timer("bind");follower.Remember(points,raw,followRig,MeridianRecipe::columns,MeridianRecipe::clothCount);}')
    if '++followed;' in block:
        splice('++followed;', '++followed; ++followCount;')
        splice('++wraps;', '++wraps; ++walkCount;')
    splice('   points=raw;', '   Timer timer("fallback"); ++fallbackCount; points=raw;')
    splice('  std::vector<M::Vec> seamDelta', '  phase=Clock::now();\n  std::vector<M::Vec> seamDelta')
    block += '\n  Record("trim_normals",phase);\n'
    prefix = r'''
#include <malemod/garments/meridian_continuity.hpp>
#include <malemod/garments/meridian_rig.hpp>
#include <malemod/garments/meridian_follow.hpp>
#include "meridian_recipe.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
namespace M=malemod::garments::meridian;
using Clock=std::chrono::steady_clock;
static std::map<std::string,std::vector<double>> samples;
static bool collect=false;
static unsigned fallbackCount=0, failures=0, followCount=0,walkCount=0;
void Record(const char* name,Clock::time_point start){if(collect)samples[name].push_back(std::chrono::duration<double,std::milli>(Clock::now()-start).count());}
struct Timer{const char* name;Clock::time_point start=Clock::now();Timer(const char* n):name(n){}~Timer(){Record(name,start);}};
#define Log(...) ((void)0)
struct Vertex{float p[3]{},n[3]{},uv[2]{},color[4]{};};
int main(int argc,char** argv){
 M::SurfaceContinuity continuity;
 M::SurfaceFollower follower;
 std::vector<unsigned> followCertificates;
 std::vector<unsigned> repairCertificates;
 std::vector<Vertex> vertices(MeridianRecipe::sampleCount);
 std::vector<std::vector<M::Vec>> poses;
 std::ifstream list(argv[1]);std::string path;
 while(std::getline(list,path)){std::ifstream in(path,std::ios::binary|std::ios::ate);if(!in||in.tellg()!=MeridianRecipe::sampleCount*sizeof(M::Vec))return 2;in.seekg(0);poses.emplace_back(MeridianRecipe::sampleCount);in.read((char*)poses.back().data(),poses.back().size()*sizeof(M::Vec));if(!in)return 3;}
 if(poses.empty())return 4;
 unsigned interpolation=std::stoul(argv[3]);
 if(interpolation){auto inputs=poses;poses.clear();for(unsigned k=0;k+1<inputs.size();k++)for(unsigned n=0;n<interpolation;n++){float t=float(n)/interpolation;auto pose=inputs[k];for(unsigned i=0;i<pose.size();i++)pose[i]=M::Add(M::Mul(inputs[k][i],1-t),M::Mul(inputs[k+1][i],t));poses.push_back(std::move(pose));}poses.push_back(inputs.back());}
 unsigned repeats=std::stoul(argv[2]);long renderFrameSerial=0;
 for(unsigned cycle=0;cycle<=repeats;cycle++)for(const auto& pose:poses){
  collect=cycle>0;auto points=pose;Timer total("total");auto phase=Clock::now();++renderFrameSerial;
  try{
'''
    suffix = r'''
  }catch(const std::exception& e){++failures;}
 }
 std::cout<<"{\"nativeFPSMeasured\":false,\"includesSkinning\":false,\"includesGPU\":false,\"repeats\":"<<repeats<<",\"poses\":"<<poses.size()<<",\"fallbackCallsIncludingWarmup\":"<<fallbackCount<<",\"followCallsIncludingWarmup\":"<<followCount<<",\"walkCallsIncludingWarmup\":"<<walkCount<<",\"outerFailuresIncludingWarmup\":"<<failures<<",\"phases\":{";
 bool first=true;for(auto& item:samples){if(!first)std::cout<<",";first=false;auto& values=item.second;std::sort(values.begin(),values.end());std::cout<<"\""<<item.first<<"\":{\"samples\":"<<values.size()<<",\"meanMs\":"<<std::accumulate(values.begin(),values.end(),0.0)/values.size()<<",\"p95Ms\":"<<values[(values.size()-1)*95/100]<<",\"maxMs\":"<<values.back()<<"}";}
 std::cout<<"}}\n";
}
'''
    if not (build/'base/include/malemod/garments/meridian_follow.hpp').exists():
        prefix=prefix.replace('#include <malemod/garments/meridian_follow.hpp>','').replace(' M::SurfaceFollower follower;\n std::vector<unsigned> followCertificates;\n','')
    poses = sorted(args.poses.resolve().glob('MeridianRaw-*.bin'), key=lambda p: int(p.stem.split('-')[-1]))
    if not poses:
        raise SystemExit('No private MeridianRaw pose buffers found')
    poses = [poses[i*(len(poses)-1)//max(1,min(args.limit,len(poses))-1)] for i in range(min(args.limit,len(poses)))]
    output.mkdir(parents=True)
    (output/'profile.cpp').write_text(prefix+block+suffix)
    (output/'poses.txt').write_text('\n'.join(str(p) for p in poses)+'\n')
    command = f'@echo off\ncall "{args.vcvars}" >nul\nif errorlevel 1 exit /b 1\ncl /nologo /O2 /MT /EHsc /std:c++17 /I"{build / "base/include"}" /I"{native}" "{output / "profile.cpp"}" /Fo"{output / "profile.obj"}" /Fe"{output / "profile.exe"}"\n'
    (output/'build.cmd').write_text(command)
    subprocess.run(['cmd','/c',str(output/'build.cmd')],check=True)
    result = json.loads(subprocess.check_output([str(output/'profile.exe'),str(output/'poses.txt'),str(args.repeats),str(args.interpolate)],text=True))
    result.update(runtimeSHA256=provenance['runtimeSHA256'],adapterSHA256=digest(source),recipeSHA256=digest(native/'meridian_recipe.h'),profileSHA256=digest(output/'profile.cpp'),poseInputs=[dict(path=p.name,sha256=digest(p)) for p in poses],syntheticInterpolationSteps=args.interpolate,scope='Exact selected-build post-skinning CPU block replay; historical private pose inputs, no native or visual acceptance')
    (output/'profile.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result['phases'],indent=2))


if __name__ == '__main__':
    main()
