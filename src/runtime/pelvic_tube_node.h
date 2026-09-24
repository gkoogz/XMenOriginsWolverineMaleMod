// Render-only posterior tube attachment. Does not change the solver supports.
static float PelvicAttachmentFullness(){
  // Keep the compact attachment when the shaft folds down into this region.
  return Smoother01((Unit(shaftNodes[1]-shaftNodes[0]).z+.55f)/.65f);
}
static V3 PelvicNodeOffset(V3 p){
  float x=(p.x-6.2f)/5.0f,y=p.y/5.4f;
  float profile=expf(-(x*x*x*x+y*y*y*y))*(1.f-Smoother01((p.x-8.5f)/2.3f));
  profile*=Smoother01((p.z-70.4f)/7.6f)*(1.f-Smoother01((p.z-81.f)/5.f));
  float scale=max(.65f,min(1.6f,logicalShaftBodyRadius/4.f));
  V3 enlarged=V3{.25f,0.f,-.97f}*(4.5f*scale*profile);
  float oldX=(p.x-6.8f)/3.8f,oldY=p.y/3.1f;
  float compact=expf(-(oldX*oldX*oldX*oldX+oldY*oldY*oldY*oldY))*(1.f-Smoother01((p.x-8.5f)/2.3f));
  compact*=Smoother01((p.z-75.8f)/1.2f)*(1.f-Smoother01((p.z-81.f)/3.f));
  V3 previous=V3{.48f,0.f,-.88f}*(2.5f*scale*compact);
  return previous+(enlarged-previous)*PelvicAttachmentFullness();
}
static void SculptPelvicTubeNode(unsigned char* controlled,UINT graftFirstVertex){
  // A folded-back shaft occupies this space; retract the added node smoothly.
  float fraction=Smoother01(Unit(shaftNodes[1]-shaftNodes[0]).x/.35f);
  for(UINT t=0;t<collarNormalTriangleCount;t++){
    V3 p[3],d[3];
    for(UINT c=0;c<3;c++){
      UINT k=t*3+c,id=collarNormalTriangleIndices[k];
      p[c]=ReadAnyCollarPosition(id,k,controlled,graftFirstVertex);
      V3 base{collarNormalTriangleBasePositions[k*3],collarNormalTriangleBasePositions[k*3+1],collarNormalTriangleBasePositions[k*3+2]};
      d[c]=PelvicNodeOffset(base);
      if(id>=graftFirstVertex){UINT j=id-graftFirstVertex;if(j<graftCount&&phys_scrotum_weight[j]>.55f&&graftCollarGroups[j]==65535u)d[c]={};}
    }
    fraction=min(fraction,SurfaceCorrectionLimit(p[0],p[1],p[2],d[0],d[1],d[2],.45f-.20f*PelvicAttachmentFullness()));
  }

  for(UINT i=0;i<pelvisControlCount;i++){
    V3 base{pelvisControlBasePositions[i*3],pelvisControlBasePositions[i*3+1],pelvisControlBasePositions[i*3+2]};
    V3 shift=PelvicNodeOffset(base)*fraction;unsigned char* v=controlled+(pelvisControlIndices[i]-pelvisControlFirstVertex)*graftStride;
    V3 p;memcpy(&p,v,12);p=p+shift;memcpy(v,&p,12);
  }
  for(UINT i=0;i<graftCount;i++){
    if(phys_scrotum_weight[i]>.55f&&graftCollarGroups[i]==65535u)continue;
    V3 base{morph_base[i*3],morph_base[i*3+1],morph_base[i*3+2]};
    graftDeformedPositions[i]=graftDeformedPositions[i]+PelvicNodeOffset(base)*fraction;
  }
}
