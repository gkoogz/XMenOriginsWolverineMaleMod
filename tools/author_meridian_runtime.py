"""Author measured donor bindings for a private native meridian experiment.

Generated capture-dependent recipes remain in the requested output directory.
Nothing is installed. Numerical deformation belongs to the supplied Base.
"""
import argparse,hashlib,json,sys,time
from pathlib import Path
import numpy as np
from scipy.spatial import cKDTree,ConvexHull


def obj(path):
    v=[];f=[]
    for line in path.read_text().splitlines():
        s=line.split()
        if not s:continue
        if s[0]=='v':v.append(list(map(float,s[1:])))
        elif s[0]=='f':f.append([int(i.split('/')[0])-1 for i in s[1:]])
    return np.array(v),np.array(f)


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--base',type=Path,required=True)
    ap.add_argument('--inspection',type=Path,required=True);ap.add_argument('--lod',type=Path,required=True)
    ap.add_argument('--body-input',type=Path,required=True);ap.add_argument('--anatomy',type=Path,required=True)
    ap.add_argument('--columns',type=int,default=80)
    ap.add_argument('--output',type=Path,required=True);args=ap.parse_args()
    sys.path.insert(0,str(args.base.resolve()))
    from malemod_base.surface_guides import project_surface,fixed_ribbon,bezier
    args.output.mkdir(parents=True,exist_ok=True)
    j=json.loads((args.inspection/'collision-model.json').read_text())
    lod=np.load(args.lod);p=lod['points'];f=lod['faces'];cloth_count=len(p);columns=args.columns;rows=(cloth_count-1)//columns
    if not 8<=columns<=256:raise ValueError('Unsupported column count')
    if 'columns' in lod and int(lod['columns'])!=columns:raise ValueError('LOD column metadata differs')
    if columns*rows+1!=cloth_count or rows<4:raise ValueError('Unsupported meridian grid')
    # Keep the existing waistband/glute mesh; resample only the new thin hems.
    trim,tf=obj(args.inspection/'fixed-straps.obj')
    retained=j['trim']['bandVertices']+j['trim']['strapVertices'];tf=tf[tf.max(1)<retained];trim=trim[:retained]
    for proof in j['tautGuides']['fixedHemStraps']['curveProofs']:
        h,hf=fixed_ribbon(bezier(proof['controls'],33),.30,.04,[1.,0.,0.])
        tf=np.vstack([tf,hf+len(trim)]);trim=np.vstack([trim,h])
    f=np.vstack([f,tf+len(p)]);p=np.vstack([p,trim]);alias_start=len(p)
    # Duplicate the longitudinal UV seam only in the render mesh. Collision
    # topology keeps its welded grid; aliases copy final positions and normals.
    aliases=np.arange(rows)*columns;p=np.vstack([p,p[aliases]]);render_count=len(p)
    seam=(f<cloth_count-1)&(f%columns==0)
    crossing=((f<cloth_count-1)&(f%columns==columns-1)).any(1)
    for index in np.flatnonzero(crossing):
        for corner in range(3):
            if seam[index,corner]:f[index,corner]=alias_start+f[index,corner]//columns
    # Collision vertices are numerical only. Keep their nine convex groups and
    # bind them to anatomy so the current pose, morphology and motion drive them.
    groups=[];current=[]
    for line in (args.inspection/'collision-model.obj').read_text().splitlines():
        tokens=line.split()
        if not tokens: continue
        if tokens[0]=='o':
            if current: groups.append(np.array(current));current=[]
        elif tokens[0]=='v':current.append(list(map(float,tokens[1:4])))
    if current:groups.append(np.array(current))
    if len(groups)!=9:raise ValueError('Expected six links, a dome and two lobes')
    proxy_ranges=[]
    for group in groups:
        proxy_ranges.append((len(p),len(group)));p=np.vstack([p,group])
    axis=np.array(j['tautGuides']['meridians']['polarAxis']);axis_anchor=cloth_count-1
    p=np.vstack([p,p[axis_anchor]-axis*np.linalg.norm(p[axis_anchor]-p[:columns].mean(0))])
    body=json.loads(args.body_input.read_text());bv=np.array([v['position'] for v in body['bodySurface']]);bf=np.array(body['bodyTriangles']);bn=np.array([v['normal'] for v in body['bodySurface']])
    body_ids=np.array([v['lineage'][0]['vertex'] for v in body['bodySurface']])
    av=np.fromfile(args.anatomy,dtype='<f4').reshape(-1,3);af=np.fromfile(str(args.anatomy)+'.indices',dtype='<u2').reshape(-1,3)
    an=np.zeros_like(av)
    for k in range(3):np.add.at(an,af[:,k],np.cross(av[af[:,1]]-av[af[:,0]],av[af[:,2]]-av[af[:,0]]))
    an/=np.maximum(np.linalg.norm(an,axis=1)[:,None],1e-12)
    # Exact nearest triangles within a local spatial candidate bank. Preserve
    # the barycentric donor IDs, and encode the remaining offset in its frame.
    valid_faces=[faces[np.linalg.norm(np.cross(v[faces[:,1]]-v[faces[:,0]],v[faces[:,2]]-v[faces[:,0]]),axis=1)>1e-8] for v,faces in [(av,af),(bv,bf)]]
    trees=[cKDTree(v[faces].mean(1)) for v,faces in zip([av,bv],valid_faces)]
    recipe=[];max_error=0
    for index,point in enumerate(p):
        options=[]
        for surface,(v,faces,normals) in enumerate([(av,af,an),(bv,bf,bn)]):
            ids=trees[surface].query(point,k=32)[1];candidate=valid_faces[surface][ids]
            q,_,face_id,bary=project_surface([point],v,candidate,normals)
            face=candidate[face_id[0]];options.append((np.linalg.norm(q[0]-point),surface,face,bary[0],q[0]))
        # Boundary/trim must follow the body. The rest uses the nearest actual
        # source surface, avoiding a full 20k-body conversion every frame.
        force_body=(cloth_count<=index<render_count) or index<columns or index==len(p)-1
        distance,surface,face,weights,q=options[0] if render_count<=index<len(p)-1 else (options[1] if force_body else min(options,key=lambda x:x[0]))
        v=bv if surface else av;tri=v[face]
        e=(tri[1]-tri[0]);e/=np.linalg.norm(e);n=np.cross(tri[1]-tri[0],tri[2]-tri[0]);n/=np.linalg.norm(n);b=np.cross(n,e)
        offset=np.array([np.dot(point-q,e),np.dot(point-q,b),np.dot(point-q,n)])
        restored=q+offset[0]*e+offset[1]*b+offset[2]*n;max_error=max(max_error,float(np.linalg.norm(restored-point)))
        if not np.isfinite(offset).all():raise ValueError('Nonfinite donor frame')
        recipe.append(dict(source=(body_ids[face] if surface else face).tolist(),weights=weights.tolist(),offset=offset.tolist(),surface=surface))
    # Hide only identified anatomy faces, leaving the recruited pelvic collar.
    regions=Path(__file__).resolve().parents[1]/'src/runtime/jockstrap_regions_data.h'
    import re
    mask=np.zeros(len(av),bool)
    for chunk in re.findall(r'\[\]\s*=\s*\{([^}]+)\}',regions.read_text()):
        ids=np.array([int(x) for x in re.findall(r'\d+',chunk)]);mask[ids]=True
    visible=af[~mask[af].all(1)]
    header=['#pragma once','#include <malemod/garments/meridian_runtime.hpp>','namespace MeridianRecipe {',
        f'static constexpr unsigned contractRevision=5,columns={columns},rows={rows},aliasStart={alias_start},count={render_count},sampleCount={len(p)},faceCount={len(f)},clothCount={cloth_count},clothFaceCount={len(lod["faces"])};',
        'static const malemod::garments::meridian::Binding bindings[]={']
    band_vertices=j['trim']['bandVertices']
    if band_vertices%14:raise ValueError('Expected observed two-layer seven-row waistband')
    header.insert(4,f'static constexpr unsigned bandVertices={band_vertices},bandColumns={band_vertices//14};')
    number=lambda v:format(float(v),'.9g')+'f' if '.' in format(float(v),'.9g') or 'e' in format(float(v),'.9g') else format(float(v),'.9g')+'.0f'
    for r in recipe:header.append('{{'+','.join(map(str,r['source']))+'},{'+','.join(map(number,r['weights']))+'},{'+','.join(map(number,r['offset']))+'},'+str(r['surface'])+'},')
    header+=['};','static const malemod::garments::meridian::Face faces[]={']
    header+=['{'+','.join(map(str,t))+'},' for t in f];header+=['};','static const malemod::garments::meridian::Face clothFaces[]={'];header+=['{'+','.join(map(str,t))+'},' for t in lod['faces']];header+=['};','static const unsigned short uncoveredIndices[]={']
    header+=[','.join(map(str,t))+',' for t in visible];header+=['};','static const unsigned proxyRanges[][2]={']
    header+=['{'+str(first)+','+str(count)+'},' for first,count in proxy_ranges]
    header+=['};']
    all_normals=[];normal_ranges=[];frames=[]
    i=np.arange(64);z=1-2*(i+.5)/64;a=i*2.39996322973;r=np.sqrt(1-z*z)
    uniform=np.column_stack([r*np.cos(a),r*np.sin(a),z])
    for start,count in proxy_ranges:
        points=p[start:start+count];normals=uniform.copy()
        values=p[:cloth_count]@normals.T-(points@normals.T).max(0)
        bad=np.flatnonzero(values[lod['faces']].min(1).max(1)<.001)
        planes=np.unique(np.round(ConvexHull(points).equations,9),axis=0)
        if len(bad):
            values=p[:cloth_count]@planes[:,:3].T+planes[:,3]
            selected=np.unique(values[lod['faces'][bad]].min(1).argmax(1))
            normals=np.vstack([normals,planes[selected,:3]])
        # Store directions in an observed three-point frame, so the tight
        # support planes rotate with the collision object rather than the world.
        origin=0;far=int(np.linalg.norm(points-points[origin],axis=1).argmax())
        e=points[far]-points[origin];e/=np.linalg.norm(e)
        third=int(np.linalg.norm(np.cross(points-points[origin],e),axis=1).argmax())
        n=np.cross(e,points[third]-points[origin]);n/=np.linalg.norm(n);v=np.cross(n,e)
        frames.append((start+origin,start+far,start+third))
        normal_ranges.append((len(all_normals),len(normals)));all_normals.extend(normals@np.array([e,v,n]).T)
    header+=['static const malemod::garments::meridian::Vec supportNormals[]={']
    header+=['{'+','.join(map(number,n))+'},' for n in all_normals]
    header+=['};','static const unsigned normalRanges[][2]={']
    header+=['{'+','.join(map(str,r))+'},' for r in normal_ranges]
    header+=['};','static const unsigned proxyFrames[][3]={']
    header+=['{'+','.join(map(str,r))+'},' for r in frames]
    header+=['};','struct SeamFollower {unsigned a,b;float fraction,weight;};','static const SeamFollower trimFollowers[]={']
    starts=p[:columns];ends=np.roll(starts,-1,axis=0);edges=ends-starts
    for point in p[cloth_count:alias_start]:
        t=np.clip(((point-starts)*edges).sum(1)/(edges*edges).sum(1),0,1)
        distance=np.linalg.norm(starts+t[:,None]*edges-point,axis=1);a=int(distance.argmin())
        weight=float(np.clip((1.-distance[a])/.7,0,1))
        header.append('{'+str(a)+','+str((a+1)%columns)+','+number(t[a])+','+number(weight)+'},')
    header+=['};','static const float rowHeights[]={']
    pole=p[cloth_count-1];axis=np.array(j['tautGuides']['meridians']['polarAxis'])
    for row in range(rows):
        for col in range(columns):
            height=float((p[row*columns+col]-pole)@axis/((p[col]-pole)@axis))
            header.append(number(height)+',')
    header+=['};']
    ring_controls=[[proxy_ranges[0][0]+k for k in (0,16,32,48)]]
    ring_controls += [[first+64+k for k in (0,16,32,48)] for first,count in proxy_ranges[:6]]
    lobe_controls=[[first+k for k in (1200,1201,576,600,588,612)] for first,count in proxy_ranges[7:]]
    apex=proxy_ranges[6][0]+1536
    samples=set(range(columns))|set(range(cloth_count,alias_start))|{cloth_count-1,len(p)-1,apex}
    for controls in ring_controls+lobe_controls:samples.update(controls)
    for name,controls in [('ringControls',ring_controls),('lobeControls',lobe_controls)]:
        header+=['static const unsigned '+name+'[]['+str(len(controls[0]))+']={']
        header+=['{'+','.join(map(str,r))+'},' for r in controls];header+=['};']
    header+=['static const unsigned domeApex='+str(apex)+';','static const unsigned runtimeSamples[]={'+','.join(map(str,sorted(samples)))+'};','}']
    (args.output/'meridian_recipe.h').write_text('\n'.join(header)+'\n')
    (args.output/'binding.json').write_text(json.dumps(recipe)+'\n')
    np.savez(args.output/'reference.npz',points=p,faces=f,anatomy=av,body=bv,bodyIds=body_ids)
    record=dict(schema=2,geometryBindingRevision=5,sources={str(path.resolve()):hashlib.sha256(path.read_bytes()).hexdigest() for path in [args.anatomy,Path(str(args.anatomy)+'.indices'),args.body_input,args.lod,args.inspection/'collision-model.json',args.inspection/'collision-model.obj',args.inspection/'fixed-straps.obj']},vertices=render_count,renderSeamAliases=rows,columns=columns,rows=rows,collisionSamples=sum(count for _,count in proxy_ranges),runtimePoseSamples=len(samples),triangles=len(f),clothTriangles=len(lod['faces']),clothVertices=cloth_count,
        referenceReconstructionError=max_error,hiddenAnatomyTriangles=len(af)-len(visible),retainedCollarTriangles=len(visible),
        dynamics='Live analytic circular-chain/dome/physics-ovoid wrapping, whole-triangle support certificate and welded render UV aliases; adaptive origin redistribution pending',installed=False)
    (args.output/'recipe-checks.json').write_text(json.dumps(record,indent=2));print(json.dumps(record))


if __name__=='__main__':main()
