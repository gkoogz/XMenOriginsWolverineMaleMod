#pragma once
#include <malemod/garments/meridian_runtime.hpp>
#include <cstdint>
#include <cstring>
// Engine-owned preparation gate. Compare actual posed numerical inputs rather
// than draw-pass IDs or palette addresses. A late changed palette/actor input
// must get a new attempt, even within the same render frame.
class MeridianAttemptGate {
 using Vec=malemod::garments::meridian::Vec;
 std::int64_t frame_=-1;
 std::uint64_t epoch_=0;
 std::vector<Vec> inputs_;
public:
 void Reset(){frame_=-1;inputs_.clear();}
 bool Begin(std::int64_t frame,std::uint64_t epoch,const std::vector<Vec>& points,const unsigned* ids,unsigned count){
  bool same=frame_==frame&&epoch_==epoch&&inputs_.size()==count;
  for(unsigned k=0;k<count;k++){
   if(ids[k]>=points.size())throw std::runtime_error("Invalid preparation input index");
   if(same&&std::memcmp(inputs_[k].data(),points[ids[k]].data(),sizeof(Vec)))same=false;
  }
  if(same)return false;
  inputs_.resize(count);for(unsigned k=0;k<count;k++)inputs_[k]=points[ids[k]];
  frame_=frame;epoch_=epoch;return true;
 }
};
