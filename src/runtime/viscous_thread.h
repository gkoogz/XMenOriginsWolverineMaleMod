#pragma once
// MIT. Reduced-dimensional viscous stream, not a full Navier-Stokes or DVT solver.
// Segment volume is conserved. Internal velocity diffusion dissipates deformation
// while preserving mass-weighted translation; there are no elastic rest lengths.
namespace teaching {
struct ThreadNode {V3 p,v;float born=0;};
struct ThreadLink {float volume=0,initial=0,neck=1;bool broken=false;unsigned label=0;};
struct ViscousThread {
 std::vector<ThreadNode> nodes;std::vector<ThreadLink> links;
 std::vector<float> diagonal,upper,mass;std::vector<V3> rhs;
 double volume=0,retiredVolume=0;unsigned nextLabel=0;bool live=false,feeding=false;int breaks=0;
 void Reset(){nodes.clear();links.clear();volume=retiredVolume=0;nextLabel=0;live=feeding=false;breaks=0;}
 void Feed(float amount,V3 tip,V3 dir,V3 velocity,float time,float dt){
  if(amount<=1e-9f)return;
  if(nodes.empty())nodes.push_back({tip+velocity*dt,velocity,time});
  V3 p=tip;
  float distance=Length(p-nodes.back().p);
  if(distance<.002f)p=nodes.back().p-dir*.002f;
  links.push_back({amount,max(.002f,Length(p-nodes.back().p)),1,false,nextLabel++});nodes.push_back({p,velocity,time});volume+=amount;live=feeding=true;
 }
 void Step(float dt,float time,const volumeFluid::Settings& cfg,const volumeFluid::F4* colliders,V3 origin){
  size_t n=nodes.size();if(!live||n<2)return;
  size_t expired=0;
  while(expired+1<n&&(time-nodes[expired].born>cfg.lifetime||nodes[expired].p.z<origin.z-180||Length(nodes[expired].p-origin)>1000))expired++;
  if(expired){for(size_t i=0;i<expired;i++)retiredVolume+=links[i].volume;links.erase(links.begin(),links.begin()+expired);nodes.erase(nodes.begin(),nodes.begin()+expired);n=nodes.size();}
  if(n<2){live=false;return;}
  mass.assign(n,0);diagonal.assign(n,0);upper.assign(n,0);rhs.resize(n);
  for(size_t i=0;i<links.size();i++){mass[i]+=links[i].volume*.5f;mass[i+1]+=links[i].volume*.5f;}
  for(size_t i=0;i<n;i++){mass[i]=max(mass[i],1e-8f);diagonal[i]=mass[i];rhs[i]=(nodes[i].v+V3{0,0,-cfg.gravity}*dt)*mass[i];}
  for(size_t i=0;i<links.size();i++)if(!links[i].broken){
   float length=max(.03f,Length(nodes[i+1].p-nodes[i].p));
   // Symmetric implicit diffusion is unconditionally dissipative and O(n).
   float weight=dt*cfg.viscosity*.22f*links[i].volume/(length*length+.08f);
   diagonal[i]+=weight;diagonal[i+1]+=weight;upper[i]=-weight;
  }
  for(size_t i=1;i<n;i++){float w=upper[i-1]/diagonal[i-1];diagonal[i]-=w*upper[i-1];rhs[i]=rhs[i]-rhs[i-1]*w;}
  nodes[n-1].v=rhs[n-1]/diagonal[n-1];for(int i=(int)n-2;i>=0;i--)nodes[i].v=(rhs[i]-nodes[i+1].v*upper[i])/diagonal[i];
  int below=0;
  for(size_t i=0;i<n;i++){
   V3 old=nodes[i].p;nodes[i].p=old+nodes[i].v*dt;
   float radius=sqrtf(max(.000001f,mass[i])/(3.14159265f*max(.1f,i<links.size()?Length(nodes[i+1].p-old):Length(old-nodes[i-1].p))));radius=min(cfg.nozzle*1.5f,radius);
   if(cfg.catchPlane&&nodes[i].p.z<origin.z-cfg.catchDepth+radius){nodes[i].p.z=origin.z-cfg.catchDepth+radius;nodes[i].v.z=max(0.f,nodes[i].v.z);float friction=expf(-dt*4);nodes[i].v.x*=friction;nodes[i].v.y*=friction;}
   for(int j=0;j<8;j++)if(colliders[j].w>0){V3 center{colliders[j].x,colliders[j].y,colliders[j].z},delta=nodes[i].p-center;float d=Length(delta),r=colliders[j].w+radius*.7f;if(d>1e-6f&&d<r){V3 normal=delta/d;nodes[i].p=center+normal*r;float inward=Dot(nodes[i].v,normal);if(inward<0)nodes[i].v=nodes[i].v-normal*inward;}}
   if(nodes[i].p.z<origin.z-180||Length(nodes[i].p-origin)>1000)below++;
  }
  if(below==(int)n){live=false;return;}
  // A finite-wavelength capillary instability transfers volume into its neighbors
  // before a link separates. It cannot delete material or break every sample.
  for(size_t i=2;i+2<links.size();i++)if(!links[i].broken){
   float age=time-nodes[i].born,length=Length(nodes[i+1].p-nodes[i].p);
   float radius=sqrtf(links[i].volume/(3.14159265f*max(.001f,length)));
   bool neckSite=(links[i].label%29)==17;float stretch=length/max(.01f,links[i].initial);
   if(neckSite&&age>.65f&&length>radius*1.6f&&stretch>1.15f&&cfg.breakup>0){
    float rate=cfg.breakup*cfg.tension/(cfg.viscosity+8.f)*2.f;
    float lost=links[i].volume*(1-expf(-dt*rate));links[i].neck*=expf(-dt*rate);
    bool cut=links[i].neck<.08f&&length>radius*4;if(cut)lost=links[i].volume;
    // Advected material carries its momentum into the neighboring sections.
    float transfer=lost*.25f;
    nodes[i-1].v=(nodes[i-1].v*mass[i-1]+nodes[i].v*transfer)/(mass[i-1]+transfer);
    nodes[i+2].v=(nodes[i+2].v*mass[i+2]+nodes[i+1].v*transfer)/(mass[i+2]+transfer);
    mass[i-1]+=transfer;mass[i+2]+=transfer;mass[i]-=transfer;mass[i+1]-=transfer;
    links[i].volume-=lost;links[i-1].volume+=lost*.5f;links[i+1].volume+=lost*.5f;
    if(cut){links[i].broken=true;++breaks;}
   }
  }
 }
 void Append(LiquidMesh& mesh,int sides,std::vector<volumeFluid::Particle>& bounds)const{
  if(!live||nodes.size()<2)return;
  size_t first=0;
  while(first+1<nodes.size()){
   size_t last=first;while(last<links.size()&&!links[last].broken)last++;
   if(last>first){
    std::vector<V3> points;std::vector<float> radii;float amount=0;
    for(size_t i=first;i<last;i++)amount+=links[i].volume;
    for(size_t i=first;i<=last;i++){
     float vol=0,length=0;
     if(i>first){vol+=links[i-1].volume;length+=Length(nodes[i].p-nodes[i-1].p);}
     if(i<last){vol+=links[i].volume;length+=Length(nodes[i+1].p-nodes[i].p);}
     float radius=sqrtf(max(1e-9f,vol)/(3.14159265f*max(.005f,length)));
     points.push_back(nodes[i].p);radii.push_back(radius);bounds.push_back({nodes[i].p,1,{},radius});
    }
    // Unresolved terminal wisps retract into the rounded end cap. This mesh-only
    // regularization retains the component's complete volume in Tube(). Never
    // shorten the feeding end, which must remain connected to the outlet.
    if(points.size()>10){
     size_t span=min(size_t(24),points.size()/3),trim=0;float largest=0;
     for(size_t k=0;k<span;k++)largest=max(largest,radii[k]);
     while(trim+2<span&&radii[trim]<largest*.28f)++trim;
     if(trim){points.erase(points.begin(),points.begin()+trim);radii.erase(radii.begin(),radii.begin()+trim);}
     span=min(size_t(7),points.size()/4);
     float head=radii[span]*.7f,tail=radii[radii.size()-1-span]*.6f;
     for(size_t k=0;k<span;k++){
      float t=float(k)/span,w=1-t;radii[k]=max(radii[k],head*w+radii[k]*t);
      size_t j=points.size()-1-k;radii[j]=max(radii[j],tail*w+radii[j]*t);
     }
    }
    mesh.Tube(points,radii,sides,amount);
   }
   first=last+1;
  }
 }
};
}
