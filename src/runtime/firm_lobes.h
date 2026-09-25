// Each lower lobe retains its authored ovoid and a bounded pressure transform.
// The solid core preserves volume; the attachment and central skin can yield.
static float firmLobeMask[r14Count];
static void PreserveRigidLobeSurfaces(){
  if(!constraintSolverReady||!eggRestReady)return;
  float scale=sqrtf(max(.35f,BallShapeScale()));
  for(UINT i=0;i<r14NewStart;i++){
    V3 rest{r14Base[i*3],r14Base[i*3+1],r14Base[i*3+2]};float membership=0.f;
    for(UINT k=r14Offsets[i];k<r14Offsets[i+1];k++){
      UINT j=r14Sources[k];float w=r14Weight[k];
      rest=rest+(firmLobeRestSkin[j]-V3{r14Reference[j*3],r14Reference[j*3+1],r14Reference[j*3+2]})*w;
      membership+=w*phys_scrotum_weight[j];
    }
    float lower=Smoother01((81.f-r14Base[i*3+2])/5.f);
    float mask=lower*Smoother01((membership-.55f)/.35f);
    firmLobeMask[i]=mask;if(mask<=0.f)continue;
    float right=Smoother01(.5f+r14Base[i*3+1]/1.4f);V3 target{};
    for(int side=0;side<2;side++){
      V3 offset=EggSurface(rest-constraintBallRest[side],eggRadii[side]);
      V3 originalOffset=offset;
      // A fuller shared skin envelope fills the medial hollow without
      // changing either rigid support or moving the lateral lobe surfaces.
      float medial=1.f-Smoother01((fabsf(r14Base[i*3+1])-.20f)/1.40f);
      float pouch=Smoother01((78.f-r14Base[i*3+2])/3.f);
      float fill=.80f*medial*pouch;
      V3 envelope=EggSurface(V3{offset.x,0.f,offset.z},eggRadii[side]);
      offset.x+=(envelope.x-offset.x)*fill;
      offset.z+=(envelope.z-offset.z)*fill;
      // No wraparound scrotal ridge: the shaft owns the axial continuation.
      float forward=(side?16.f:18.f)*.01745329252f,lateral=(side?10.f:-9.f)*.01745329252f;
      V3 up=Unit(V3{sinf(forward),sinf(lateral),cosf(forward)*cosf(lateral)});
      offset=RotateFromTo(offset,{0,0,1},up);
      offset.y=RotateFromTo(originalOffset,{0,0,1},up).y;offset.z+=(side?.05f:-.15f)*scale;
      V3 restAxis=Unit(constraintBallRest[side]-RestBallAnchor(side));
      V3 liveAxis=Unit(ballNodes[side]-BallAnchor(side));
      target=target+(ballNodes[side]+LobePressureOffset(RotateFromTo(offset,restAxis,liveAxis),side))*(side?right:1.f-right);
    }
    r14Positions[i]=r14Positions[i]*(1.f-mask)+target*mask;
  }
  static V3 next[r14NewStart];
  for(int pass=0;pass<24;pass++){
    for(UINT i=0;i<r14NewStart;i++){
      next[i]=r14Positions[i];float m=firmLobeMask[i];
      if(m<=0.f||m>=.99999f)continue;
      V3 mean{};UINT first=undersideRows[i],last=undersideRows[i+1];
      for(UINT k=first;k<last;k++)mean=mean+r14Positions[undersideNeighbors[k]];
      next[i]=next[i]+(mean/float(last-first)-next[i])*(.45f*4.f*m*(1.f-m));
    }
    memcpy(r14Positions,next,sizeof(next));
  }
}
