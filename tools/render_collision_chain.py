"""Wolverine offline adapter for the geometry-only collision inspection.

Consumes an existing paired anatomy/mechanics export and a saved fixed garment.
No pouch is imported and no fabric solver is initialized or called.
"""
import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
import numpy as np
base_parser=argparse.ArgumentParser(add_help=False)
base_parser.add_argument('--base',type=Path,required=True)
base_args,_=base_parser.parse_known_args()
ROOT = base_args.base.resolve()
sys.path.insert(0, str(ROOT))
from malemod_base.meridian_surface import cloth_surface
from malemod_base.taut_guides import adaptive_meridian_paths, lift_embedded_anchors, sample_polyline
from malemod_base.surface_guides import bezier, project_surface, smooth_clearance_curve, fixed_ribbon
from malemod_base.collision_chain import chain, ovoid, edge_arc, unit, closed_link, enclosing_tip, hemisphere_dome
from malemod_base.garment_regions import reference_anatomy_semantics, full_source_root_boundary


def fit_circle(points):
    origin = points.mean(0)
    _, _, axes = np.linalg.svd(points-origin, full_matrices=False)
    a, b = axes[:2]
    xy = np.column_stack([(points-origin) @ a, (points-origin) @ b])
    x, y, _ = np.linalg.lstsq(np.column_stack([2*xy, np.ones(len(xy))]), (xy*xy).sum(1), rcond=None)[0]
    center = origin+a*x+b*y
    return center, float(np.linalg.norm(xy-[x,y], axis=1).max())


def read_trim(path):
    raw = path.read_bytes()
    count, faces = struct.unpack_from('<II', raw)
    if len(raw) != 8+count*352+faces*16:
        raise ValueError('Unexpected saved garment ABI')
    vertices = np.ndarray((count,3), '<f8', raw, 8, strides=(352,8)).copy()
    uv = np.ndarray((count,2), '<f8', raw, 8+72, strides=(352,8))
    triangles = np.frombuffer(raw, '<u4', faces*4, 8+count*352).reshape(-1,4)
    pouch_ids = np.unique(triangles[triangles[:,3] == 0,:3])
    rows = len(np.unique(uv[pouch_ids,1]))
    band_end, strap_start = int(pouch_ids.min()), int(pouch_ids.max())+1
    hem_start = count-rows*4*2
    keep = np.all(triangles[:,:3] < band_end,axis=1) | np.all((triangles[:,:3]>=strap_start)&(triangles[:,:3]<hem_start),axis=1)
    # The saved source layout is two seven-row layers with a repeated seam.
    columns = band_end//14-1
    if band_end != 14*(columns+1):
        raise ValueError('Unexpected saved waistband topology')
    starts = strap_start+np.flatnonzero((uv[strap_start:hem_start,0]==0)&(uv[strap_start:hem_start,1]==0))
    return vertices, triangles[keep,:3], vertices[:columns], dict(strapRanges=[list(map(int,pair)) for pair in zip(starts,list(starts[1:])+[hem_start])], bandVertices=band_end, strapVertices=hem_start-strap_start, discardedPouchVertices=len(pouch_ids), discardedHemVertices=count-hem_start)


def fit_converging_trim(trim, proof, arc, body_input, mechanics):
    body = np.asarray([v['position'] for v in body_input['bodySurface']],float)
    body_normals = np.asarray([v['normal'] for v in body_input['bodySurface']],float)
    body_faces = np.asarray(body_input['bodyTriangles'],int)
    # Actual measured pelvis/upper-thigh triangles, never inferred anatomy.
    triangles = body[body_faces]
    selected = (triangles[:,:,2].min(1)>67)&(triangles[:,:,2].max(1)<97)
    region = body_faces[selected]
    def project(points,clearance=.12):
        return project_surface(points,body,region,body_normals,clearance)
    lobe_center=np.asarray(mechanics['lobeCenters']).mean(0)
    # The illustrated junction is posterior and superior to the lobe centers.
    # Find its exact measured groin surface, then share it between both straps.
    seed=lobe_center+np.array([-4.5,-lobe_center[1],5.5])
    junction,junction_norm,_,_=project([seed]);junction=junction[0]
    updated=trim.copy();paths=[]
    shared_width=float(np.median(np.concatenate([np.linalg.norm(trim[a:b].reshape(-1,4,3)[:,1]-trim[a:b].reshape(-1,4,3)[:,0],axis=1) for a,b in proof['strapRanges']])))
    for first,last in proof['strapRanges']:
        rows=trim[first:last].reshape(-1,4,3)
        retain=int(len(rows)*.44)
        count=len(rows)-retain
        old=rows[retain:].copy()
        start=old[0].mean(0);side=np.sign(start[1])
        seeds=bezier([start,[-5,side*3.3,75.5],[2.5,side*1.8,76.5],junction],count)
        mid,norm,_,_=project(seeds)
        mid[-1]=junction
        tangents=np.gradient(mid,axis=0)
        widths=np.cross(norm,tangents)
        widths/=np.maximum(np.linalg.norm(widths,axis=1)[:,None],1e-12)
        if widths[0]@(old[0,1]-old[0,0])<0:widths*=-1
        for i in range(1,len(widths)):
            if widths[i]@widths[i-1]<0:widths[i]*=-1
        width=float(np.median(np.linalg.norm(rows[:,1]-rows[:,0],axis=1)))
        edge0,n0,_,_=project(mid-widths*width/2)
        edge1,n1,_,_=project(mid+widths*width/2)
        replacement=np.stack([edge0,edge1,edge1-.12*n1,edge0-.12*n0],axis=1)
        # Both end cross-sections use the exact same two body-bound edges.
        common,ncommon,_,_=project([junction+[0,-shared_width/2,0],junction+[0,shared_width/2,0]])
        if (common[1]-common[0])@widths[-1]<0:common=common[::-1];ncommon=ncommon[::-1]
        replacement[-1]=[common[0],common[1],common[1]-.12*ncommon[1],common[0]-.12*ncommon[0]]
        blend=np.clip(np.arange(count)/5,0,1);blend=blend*blend*(3-2*blend)
        rows_new=rows.copy();rows_new[retain:]=old*(1-blend[:,None,None])+replacement*blend[:,None,None]
        updated[first:last]=rows_new.reshape(-1,3)
        paths.append(rows_new.mean(1))
    junction=updated[proof['strapRanges'][0][1]-4:proof['strapRanges'][0][1]-2].mean(0)
    # Orange hems are conceptual body-surface traces, not sewn fabric.
    hems=[];hem_donors=[]
    for endpoint in [arc[0],arc[-1]]:
        side=np.sign(endpoint[1])
        seeds=bezier([endpoint,[10.5,side*6.5,85],[8.5,side*3.2,80],junction],81)
        line,_,ids,bary=project(seeds)
        line[0]=endpoint;line[-1]=junction
        hems.append(line)
        hem_donors.append(dict(regionFace=ids.tolist(),barycentric=bary.tolist()))
    end0=updated[proof['strapRanges'][0][1]-4:proof['strapRanges'][0][1]]
    end1=updated[proof['strapRanges'][1][1]-4:proof['strapRanges'][1][1]]
    distance=max(min(np.linalg.norm(a-b) for b in end1) for a in end0)
    if distance>1e-8:raise ValueError('Strap ends do not share their junction edge')
    return updated,hems,dict(junction=junction.tolist(),strapPaths=[p.tolist() for p in paths],hemGuides=[p.tolist() for p in hems],hemDonors=hem_donors,sharedEndError=float(distance),bodyClearance=.12,conceptualOnly=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--base',type=Path,required=True,help='Explicit Base source checkout for this offline inspection')
    parser.add_argument('--baseline',type=Path,required=True)
    parser.add_argument('--trim',type=Path,required=True)
    parser.add_argument('--body-input',type=Path,help='Measured identity-pose body for converging straps and conceptual hems')
    parser.add_argument('--output',type=Path,required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    source = args.baseline/'surface.xyz'
    vertices = np.fromfile(source,'<f4').reshape(-1,3).astype(float)
    faces = np.fromfile(str(source)+'.indices','<u2').reshape(-1,3)
    mechanics = json.loads((args.baseline/'mechanics.json').read_text())
    bank = np.load(ROOT/'assets/wolverine-reference/geometry.npz')
    direct = bank['neck_render_data__nrDirect'].ravel()
    # Source R14 has 96 vertices per axial glans ring. Select the narrow
    # transition before the crown, then measure the crown maximum itself.
    measured = {}
    for ring in range(10,90):
        ids = np.flatnonzero((direct>=1721+ring*96)&(direct<1721+(ring+1)*96))
        if len(ids)>=8:
            measured[ring] = fit_circle(vertices[ids])
    connection = min(range(12,19),key=lambda k:measured[k][1])
    crown = max(range(connection+1,41),key=lambda k:measured[k][1])
    tip = 82
    junction, junction_radius = measured[connection]
    guide = np.asarray(mechanics['shaftGuide'],float)
    best = None
    for i,(a,b) in enumerate(zip(guide[:-1],guide[1:])):
        t = np.clip((junction-a)@(b-a)/np.sum((b-a)**2),0,1)
        distance = np.linalg.norm(a+t*(b-a)-junction)
        if best is None or distance<best[0]:best=(distance,i,t)
    path = np.vstack([guide[:best[1]+1],junction])
    length = np.r_[0,np.cumsum(np.linalg.norm(np.diff(path,axis=0),axis=1))]
    centers = np.column_stack([np.interp(np.linspace(0,length[-1],6),length,path[:,k]) for k in range(3)])
    root_ids = full_source_root_boundary(bank['derived__final_reference_positions'],faces)
    root_center,root_radius = fit_circle(vertices[root_ids])
    centers[0]=root_center
    centers = np.vstack([centers,measured[crown][0],measured[tip][0]])
    glans_axis=unit(centers[7]-centers[5])
    centers[6]=centers[5]+glans_axis*((centers[6]-centers[5])@glans_axis)
    directions = np.array([unit(v) for v in np.diff(centers,axis=0)])
    normals = np.array([directions[0]]+[unit(a+b) for a,b in zip(directions[:-1],directions[1:])]+[directions[-1]])
    fields=reference_anatomy_semantics(bank)
    axial=fields[:,:2].sum(1);lobes_weight=fields[:,2:].sum(1)
    selected=faces[(axial[faces].mean(1)>.001)&(axial[faces].mean(1)>=lobes_weight[faces].mean(1))]
    triangle_points = vertices[selected]
    radii = []
    for index,(center,normal) in enumerate(zip(centers[:5],normals[:5])):
        if index==0:
            radii.append(root_radius*1.01)
            continue
        signed = (triangle_points-center)@normal
        hits = {}
        for a,b in [(0,1),(1,2),(2,0)]:
            take = (signed[:,a]*signed[:,b]<0)
            fraction = signed[take,a]/(signed[take,a]-signed[take,b])
            for face_id,point in zip(np.flatnonzero(take),triangle_points[take,a]+fraction[:,None]*(triangle_points[take,b]-triangle_points[take,a])):
                hits.setdefault(int(face_id),[]).append(point)
        # The plane can also cut a lobe. Select the connected section nearest
        # the mechanical shaft centerline instead of using tissue weight masks.
        nodes,adj={},{}
        for pair in hits.values():
            if len(pair)!=2:continue
            keys=[tuple(np.round(p,5)) for p in pair]
            for k,p in zip(keys,pair):nodes[k]=p;adj.setdefault(k,set())
            adj[keys[0]].add(keys[1]);adj[keys[1]].add(keys[0])
        components=[];seen=set()
        for key in nodes:
            if key in seen:continue
            pending=[key];group=[]
            while pending:
                k=pending.pop()
                if k in seen:continue
                seen.add(k);group.append(nodes[k]);pending.extend(adj[k]-seen)
            if len(group)>=8:components.append(np.asarray(group))
        if not components:raise ValueError(f'Measured shaft section {index} has no connected contour')
        hits=min(components,key=lambda p:np.linalg.norm(p.mean(0)-center))
        fitted,radius=fit_circle(hits)
        centers[index]=fitted
        radii.append(radius*1.01)
    radii += [junction_radius*1.01,measured[crown][1]*1.01,measured[tip][1]*1.01]
    old_tip=centers[7].copy()
    centers[7],radii[7],tip_proof=enclosing_tip(vertices,centers[6],radii[6],old_tip,glans_axis)
    tip_proof['advance']=float(np.linalg.norm(centers[7]-old_tip))
    directions=np.array([unit(v) for v in np.diff(centers,axis=0)])
    normals=np.array([directions[0]]+[unit(a+b) for a,b in zip(directions[:-1],directions[1:])]+[directions[-1]])
    # The closely spaced glans neck/crown sections share its measured axis;
    # independently tilted circles here would cross one another at the rim.
    normals[5:]=glans_axis
    rings, chain_faces, normals = chain(centers,radii,normals=normals)
    dome_points,dome_faces,dome_proof=hemisphere_dome(rings[6],centers[6],centers[7])
    from scipy.spatial import ConvexHull
    dome_planes=ConvexHull(dome_points).equations
    distal=vertices[((vertices-centers[6])@glans_axis)>=0]
    max_distance=max(float((chunk@dome_planes[:,:3].T+dome_planes[:,3]).max()) for chunk in np.array_split(distal,16))
    if max_distance>1e-7:raise ValueError('Glans wireframe escapes dome mesh')
    dome_proof.update(distalVertices=len(distal),maximumPlaneDistance=max_distance)
    rings=rings[:7]
    chain_faces=chain_faces[chain_faces.max(1)<len(rings)*rings.shape[1]]
    lobes = [ovoid(c,a,r) for c,a,r in zip(mechanics['lobeCenters'],mechanics['lobeAxes'],mechanics['lobeRadii'])]
    trim, trim_faces, bottom, trim_proof = read_trim(args.trim)
    arc = edge_arc(bottom,np.array([1,0,0]),np.array([0,-1,0]))
    hems=[];routing=None
    if args.body_input:
        trim,hems,routing=fit_converging_trim(trim,trim_proof,arc,json.loads(args.body_input.read_text()),mechanics)
    meshes=[closed_link(rings[i],rings[i+1],centers[i],centers[i+1]) for i in range(6)]+[(dome_points,dome_faces)]+lobes
    original_hems=[h.copy() for h in hems]
    hem_proofs=[];hem_straps=[]
    if hems:
        from scipy.spatial import ConvexHull
        proxy_planes=[ConvexHull(points).equations for points,_ in meshes]
        def clearance_values(points):
            return np.concatenate([(points@p[:,:3].T+p[:,3]).max(1)-.20 for p in proxy_planes])
        smooth_hems=[]
        for hem in hems:
            smooth,proof=smooth_clearance_curve(hem,lambda points:lift_embedded_anchors(points,meshes,[1.,0.,0.],surface_margin=.20),[1.,0.,0.],clearance_values=clearance_values)
            smooth_hems.append(smooth);hem_proofs.append(proof)
            strap_points,strap_faces=fixed_ribbon(smooth,.30,.04,[1.,0.,0.])
            hem_straps.append((strap_points,strap_faces))
            trim_faces=np.vstack([trim_faces,strap_faces+len(trim)])
            trim=np.vstack([trim,strap_points])
        hems=smooth_hems
        outline=np.vstack([arc,hems[1][1:],hems[0][-2::-1]])
    else:outline=arc
    starts=sample_polyline(outline,41)[:-1] if hems else sample_polyline(outline,40)
    original_starts=starts.copy()
    starts=lift_embedded_anchors(starts,meshes,[1.,0.,0.])
    pole=centers[7].copy()
    polar_axis=unit(pole-starts.mean(0))
    rays,ray_receipts,starts,meridian_proof,density_proof=adaptive_meridian_paths(outline,pole,polar_axis,meshes)
    anchor_order_proof=density_proof['anchorOrderRepair']
    distances=np.r_[0.,np.cumsum(np.linalg.norm(np.diff(outline,axis=0),axis=1))]
    start_distances=np.array(density_proof['originFractions'])*distances[-1]
    order=np.argsort(np.r_[distances,start_distances],kind='stable')
    outline_draw=np.vstack([outline,starts])[order]
    targets=np.tile(pole,(40,1))
    if len(rays)!=40:raise ValueError('Expected exactly forty taut guides')
    cloth_points,cloth_faces,cloth_proof=cloth_surface(rays,outline,density_proof['originFractions'],pole,polar_axis,glans_axis,meshes)
    with (args.output/'white-cloth.obj').open('w') as out:
        out.write('o meridian_white_cloth\n')
        for point in cloth_points:out.write('v '+' '.join(map(str,point))+'\n')
        cols=cloth_proof['angularColumns'];rows=cloth_proof['rows']
        for row in range(rows):
            for col in range(cols+1):out.write(f'vt {col/cols} {row/rows}\n')
        out.write('vt 0.5 1\n')
        for face in cloth_faces:
            seam=any(i%cols==cols-1 for i in face if i<len(cloth_points)-1)
            aliases=[]
            for i in face:
                if i==len(cloth_points)-1:uv=rows*(cols+1)+1
                else:
                    row,col=divmod(int(i),cols)
                    uv=row*(cols+1)+(cols if seam and col==0 else col)+1
                aliases.append(f'{int(i)+1}/{uv}')
            out.write('f '+' '.join(aliases)+'\n')
    circle_error = max(float(np.max(np.abs(np.linalg.norm(r-c,axis=1)-radius))) for r,c,radius in zip(rings,centers,radii))
    if circle_error>1e-10:raise ValueError('Collision sections are not perfect circles')
    record = dict(schema=2,mode='offline meridian cloth reference mesh; no gameplay claim',whiteCloth=cloth_proof,cylinderCount=6,shaftCylinders=5,glansCylinders=1,domeCount=1,dome=dome_proof,circleCount=7,
        centers=centers.tolist(),radii=radii[:7],normals=normals[:7].tolist(),circleError=circle_error,
        glansSourceRingRecipe=dict(connection=connection,crown=crown,historicalNearTipFit=tip,sharedAxis=glans_axis.tolist()),ovoidExpansion=1.03,
        lobeCenters=mechanics['lobeCenters'],lobeAxes=mechanics['lobeAxes'],lobeRadii=mechanics['lobeRadii'],
        historicalTipFrustumFit=tip_proof,tautGuides=dict(count=40,anchorBoundary='entire orange outline',targets=targets.tolist(),meridians=meridian_proof,adaptiveDensity=density_proof,anchorOrderRepair=anchor_order_proof,endpointPolicy='One shared pole; fixed distinct longitude half-planes; no fixed lateral constraint',paths=[p.tolist() for p in rays],receipts=ray_receipts,method='Shortest convex-turn visible route in each outward meridian half-plane; no global 3D geodesic claim',fixedHemStraps=dict(count=len(hem_straps),width=.30,thickness=.04,curveProofs=hem_proofs,mode='Static fixed mesh, no fabric simulation')),
        waistbandArcDegrees=80,waistbandArc=arc.tolist(),trim=trim_proof,
        sources={str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in [source,Path(str(source)+'.indices'),args.baseline/'mechanics.json',args.trim]},
        observations='Static offline matched anatomy/mechanics export. Fixed trim retained from identity-pose capture. No runtime physics/collision acceptance.')
    record['tautGuides']['orangeOutline']=outline_draw.tolist()
    record['tautGuides']['additionalSampleLift']=float(anchor_order_proof['maxAnchorAdjustment'])
    if routing:
        routing['bodyHemGuides']=routing['hemGuides']
        routing['hemGuides']=[p.tolist() for p in hems]
        routing['hemGuideStatus']='Smooth endpoint-fixed cubic with obstacle clearance; .30-wide fixed straps centered on hem guides'
        record['trimRouting']=routing
        record['sources'][str(args.body_input.resolve())]=hashlib.sha256(args.body_input.read_bytes()).hexdigest()
    (args.output/'collision-model.json').write_text(json.dumps(record,indent=2)+'\n')
    with (args.output/'collision-model.obj').open('w') as out:
        offset=0
        links=[(f'link_{i+1}',*closed_link(rings[i],rings[i+1],centers[i],centers[i+1])) for i in range(6)]
        for name,points,triangles in links+[('glans_dome',dome_points,dome_faces)]+[(f'ovoid_{i}',p,f) for i,(p,f) in enumerate(lobes)]:
            out.write('o '+name+'\n')
            for p in points:out.write('v '+' '.join(map(str,p))+'\n')
            for f in triangles:out.write('f '+' '.join(str(int(x)+offset+1) for x in f)+'\n')
            offset+=len(points)
    with (args.output/'fixed-straps.obj').open('w') as out:
        used=np.unique(trim_faces);remap=np.full(len(trim),-1,dtype=int);remap[used]=np.arange(len(used))
        for point in trim[used]:out.write('v '+' '.join(map(str,point))+'\n')
        for face in trim_faces:out.write('f '+' '.join(str(remap[i]+1) for i in face)+'\n')
    with (args.output/'taut-guides.obj').open('w') as out:
        offset=0
        for i,path in enumerate(rays):
            out.write(f'o taut_ray_{i+1}\n')
            for p in path:out.write('v '+' '.join(map(str,p))+'\n')
            out.write('l '+' '.join(str(offset+j+1) for j in range(len(path)))+'\n')
            offset+=len(path)
    render(args.output,vertices,faces,trim,trim_faces,rings,chain_faces,lobes,arc,centers,hems,rays,outline_draw,dome_points,dome_faces)
    render(args.output,vertices,faces,trim,trim_faces,rings,chain_faces,lobes,arc,centers,hems,rays,outline_draw,dome_points,dome_faces,cloth_points,cloth_faces)
    print(json.dumps({k:record[k] for k in ['cylinderCount','glansSourceRingRecipe','radii','circleError','trim']},indent=2))


def render(output,vertices,faces,trim,trim_faces,rings,chain_faces,lobes,arc,centers,hems,rays,outline_draw,dome_points,dome_faces,cloth_points=None,cloth_faces=None):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    from matplotlib.collections import LineCollection,PolyCollection
    edges = np.unique(np.sort(np.vstack([faces[:,[0,1]],faces[:,[1,2]],faces[:,[2,0]]]),axis=1),axis=0)
    views = [('Oblique',np.array([.85,1,.16])),('Front',np.array([1,0,0])),('Side',np.array([0,1,0]))]
    fig,axes = plt.subplots(1,3,figsize=(21,10),facecolor='#f5f7fa')
    for view_index,(ax,(name,camera)) in enumerate(zip(axes,views)):
        camera=unit(camera);right=unit(np.cross([0,0,1],camera));up=unit(np.cross(camera,right))
        def project(p):return np.stack([np.asarray(p)@right,np.asarray(p)@up],axis=-1)
        ax.set_facecolor('#f5f7fa')
        ax.add_collection(PolyCollection(project(trim[trim_faces]),facecolors='#cbd0d7',edgecolors='#717987',linewidths=.16,alpha=.30))
        ax.add_collection(LineCollection(project(vertices[edges]),colors='#394657',linewidths=.22,alpha=.22))
        for p,f in [(rings.reshape(-1,3),chain_faces),(dome_points,dome_faces)]+lobes:
            ax.add_collection(PolyCollection(project(p[f]),facecolors='#2c8fff',edgecolors='none',alpha=.055))
        for ring in rings:
            q=project(np.vstack([ring,ring[0]]));ax.plot(q[:,0],q[:,1],color='#0875dc',linewidth=1.15)
        for j in range(0,rings.shape[1],8):
            q=project(rings[:,j]);ax.plot(q[:,0],q[:,1],color='#1686e8',linewidth=.7,alpha=.85)
        dome_grid=dome_points[:-2].reshape(24,64,3)
        for row in range(0,24,4):
            q=project(np.vstack([dome_grid[row],dome_grid[row,0]]));ax.plot(q[:,0],q[:,1],color='#0875dc',linewidth=.65,alpha=.85)
        for col in range(0,64,8):
            q=project(np.vstack([dome_grid[:,col],centers[7]]));ax.plot(q[:,0],q[:,1],color='#0875dc',linewidth=.65,alpha=.85)
        for p,f in lobes:
            grid=p[:25*48].reshape(25,48,3)
            for row in range(0,25,3):
                q=project(np.vstack([grid[row],grid[row,0]]));ax.plot(q[:,0],q[:,1],color='#0875dc',linewidth=.65,alpha=.85)
            for col in range(0,48,6):
                q=project(grid[:,col]);ax.plot(q[:,0],q[:,1],color='#0875dc',linewidth=.65,alpha=.85)
        if cloth_points is not None:
            triangles=cloth_points[cloth_faces]
            normals=np.cross(triangles[:,1]-triangles[:,0],triangles[:,2]-triangles[:,0])
            normals/=np.maximum(np.linalg.norm(normals,axis=1)[:,None],1e-15)
            light=unit(np.array([.7,-.4,1.]))
            shade=.74+.24*np.abs(normals@light)
            colors=np.column_stack([shade,shade,shade,np.ones(len(shade))])
            order=np.argsort(triangles.mean(1)@camera)
            ax.add_collection(PolyCollection(project(triangles[order]),facecolors=colors[order],edgecolors='none',zorder=10))
        for ray in (rays if cloth_points is None else []):
            q=project(ray);ax.plot(q[:,0],q[:,1],color='#c66019',linewidth=.65,alpha=.18 if cloth_points is not None else .72,zorder=11)
        q=project(arc);ax.plot(q[:,0],q[:,1],color='#df8700',linewidth=3)
        q=project(outline_draw);ax.plot(q[:,0],q[:,1],color='#df8700',linewidth=2,zorder=12)
        if hems:
            q=project(hems[0][-1]);ax.scatter(*q,s=26,color='#df8700',zorder=13)
        c=project(centers);ax.plot(c[:,0],c[:,1],':',color='#075eab',linewidth=1);ax.scatter(c[:,0],c[:,1],s=15,c='#075eab',zorder=10 if cloth_points is None else 2)
        if name!='Front' and cloth_points is None:
            for i,point in enumerate(c):
                offset={5:(16,10),6:(-20,-10)}.get(i,(5,5))
                ax.annotate(str(i+1),point,xytext=offset,textcoords='offset points',fontsize=10,color='#004e99',weight='bold',arrowprops=dict(arrowstyle='-',color='#075eab',lw=.5) if i in (5,6) else None)
        bounds=project(np.vstack([trim[np.unique(trim_faces)],vertices]));low=bounds.min(0);high=bounds.max(0)
        ax.set_xlim(low[0]-2,high[0]+2);ax.set_ylim(low[1]-2,high[1]+2);ax.set_aspect('equal');ax.axis('off');fig.text((view_index+.5)/3,.87,name,ha='center',fontsize=16,color='#293442')
    fig.suptitle('White cloth surface from the adaptive meridians' if cloth_points is not None else 'Collision scaffold — six circular links + rounded glans dome + two ovaloids',fontsize=22,y=.97,color='#1e2f43')
    fig.text(.5,.925,'Gray: original anatomy wireframe   |   Blue: inspection collision geometry   |   Orange: smooth fixed hem straps + 40 adaptive meridians' if cloth_points is None else 'White: new cloth reference mesh   |   Orange: attachment seam   |   Gray: retained fixed waistband and straps',ha='center',fontsize=12,color='#445266')
    fig.text(.5,.045,('White: sewn reference surface lofted between rays; complete triangles cleared against all blue proxies. Tip has a small cloth-thickness offset.\nOffline geometry only; no cloth dynamics or in-game acceptance.' if cloth_points is not None else 'Sections 1–6: shaft   •   7: measured crown   •   8: dome apex   •   Ovaloids: same physics centers/axes/shape, enlarged 3%\nCrown circle retained; final circle replaced by a dome ending at its former center. Smooth thin fixed hem straps join waistband to under-straps. Adaptive meridians share one tip pole. Offline inspection; no fabric simulation.'),ha='center',fontsize=12,color='#445266',linespacing=1.8)
    fig.subplots_adjust(left=.025,right=.975,bottom=.13,top=.88,wspace=.04)
    fig.savefig(output/('white-cloth-inspection.png' if cloth_points is not None else 'collision-inspection.png'),dpi=140)
    plt.close(fig)


if __name__=='__main__':main()
