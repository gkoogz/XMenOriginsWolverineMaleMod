"""Offline stock wardrobe catalog. Run with Blender --background --python.

Input is a private UEViewer -export -all directory from the developer's
licensed game. This tool never reads/writes an installed runtime or settings.
Diffuse/normal materials are a neutral review approximation, not UE3 parity.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import sys

import bpy
from mathutils import Vector


LABELS = {
    'CH_Wolverine_Alkali': 'Alkali',
    'CH_Wolverine_Barfight': 'Barfight',
    'CH_Wolverine_Blob': 'Blob',
    'CH_Wolverine_BonusSkin1': 'Bonus 1 - brown / yellow',
    'CH_Wolverine_BonusSkin2': 'Bonus 2 - blue / yellow',
    'CH_Wolverine_BonusSkin3': 'Bonus 3 - X-Force',
    'CH_Wolverine_Casino': 'Casino',
    'CH_Wolverine_Jungle': 'Jungle',
    'CH_Wolverine_Normal': 'Normal',
    'CH_Wolverine_ThreeMileIsland': 'Three Mile Island',
    'CH_Wolverine_WeaponX': 'Weapon X',
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_psk(path):
    data = path.read_bytes()
    offset = 0
    chunks = {}
    while offset < len(data):
        tag, _, size, count = struct.unpack_from('<20s3i', data, offset)
        offset += 32
        assert size >= 0 and count >= 0 and offset + size * count <= len(data)
        chunks[tag.rstrip(b'\0').decode()] = (data[offset:offset+size*count], size, count)
        offset += size * count
    def rows(name, fmt):
        blob, size, count = chunks[name]
        assert struct.calcsize(fmt) == size, (name, size)
        return [struct.unpack_from(fmt, blob, i*size) for i in range(count)]
    points = rows('PNTS0000', '<3f')
    wedges = rows('VTXW0000', '<I2f2BH')
    if len(points) <= 65536:
        wedges = [(w[0] & 65535, *w[1:]) for w in wedges]
    faces = rows('FACE0000', '<3H2BI')
    materials = [x[0].split(b'\0')[0].decode() for x in rows('MATT0000', '<64s6i')]
    bones = [x[0].split(b'\0')[0].decode() for x in rows('REFSKELT', '<64s3i11f')]
    for w in wedges:
        assert w[0] < len(points)
    for f in faces:
        assert max(f[:3]) < len(wedges) and f[3] < len(materials)
    return points, wedges, faces, materials, bones


def choose(name, paths, preferred):
    candidates = paths.get(name.lower(), [])
    if not candidates:
        return None
    return next((p for p in candidates if preferred in p.parts), candidates[0])


def material(name, package, mats, textures, receipt):
    original = name
    # These exact unresolved imports are named in UEViewer's source log.
    if package == 'CH_Wolverine_Normal_SF' and name in ('material_5','material_6'):
        name = 'MAT_Wolverine_Body'
    if package == 'CH_Wolverine_Jungle_SF' and name == 'material_8':
        name = 'MAT_Wolverine_Bone_Claws'
    mat = bpy.data.materials.new(original)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    bsdf.inputs['Roughness'].default_value = .62
    metallic = any(x in name.lower() for x in ('claws','buckle','dog_tags','adamantium'))
    if metallic:
        bsdf.inputs['Metallic'].default_value = .65
        bsdf.inputs['Roughness'].default_value = .33
    path = choose(name, mats, package)
    fields = {}
    if path:
        fields = dict(line.split('=',1) for line in path.read_text().splitlines() if '=' in line)
    resolved = dict(slot=original, resolvedMaterial=name, materialFile=str(path) if path else None)
    resolved['textures'] = {}
    if original != name:
        resolved['sourceImportRepair'] = 'Known named import from UEViewer log; no guessed costume texture'
    for key in ('Diffuse','Normal'):
        texture = choose(fields.get(key,''), textures, package)
        if texture is None:
            resolved['textures'][key] = None
            continue
        image = bpy.data.images.load(str(texture), check_existing=True)
        node = mat.node_tree.nodes.new('ShaderNodeTexImage')
        node.image = image
        resolved['textures'][key] = dict(path=str(texture),sha256=digest(texture))
        if key == 'Diffuse':
            image.colorspace_settings.name = 'sRGB'
            mat.node_tree.links.new(node.outputs['Color'],bsdf.inputs['Base Color'])
            if 'hair' in name.lower():
                mat.surface_render_method = 'DITHERED'
                mat.node_tree.links.new(node.outputs['Alpha'],bsdf.inputs['Alpha'])
        else:
            image.colorspace_settings.name = 'Non-Color'
            separate = mat.node_tree.nodes.new('ShaderNodeSeparateXYZ')
            combine = mat.node_tree.nodes.new('ShaderNodeCombineXYZ')
            invert = mat.node_tree.nodes.new('ShaderNodeMath')
            invert.operation = 'SUBTRACT'
            invert.inputs[0].default_value = 1
            normal = mat.node_tree.nodes.new('ShaderNodeNormalMap')
            normal.inputs['Strength'].default_value = .65
            links = mat.node_tree.links
            links.new(node.outputs['Color'],separate.inputs[0])
            links.new(separate.outputs['X'],combine.inputs['X'])
            links.new(separate.outputs['Y'],invert.inputs[1])
            links.new(invert.outputs[0],combine.inputs['Y'])
            links.new(separate.outputs['Z'],combine.inputs['Z'])
            links.new(combine.outputs[0],normal.inputs['Color'])
            links.new(normal.outputs[0],bsdf.inputs['Normal'])
    receipt.append(resolved)
    return mat


def look_at(obj, target):
    obj.rotation_euler = (Vector(target)-obj.location).to_track_quat('-Z','Y').to_euler()


def setup_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 24
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 900
    scene.render.resolution_y = 1100
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.view_settings.view_transform = 'AgX'
    world = bpy.data.worlds.new('Neutral review world')
    world.use_nodes = True
    world.node_tree.nodes['Background'].inputs[0].default_value = (.55,.58,.63,1)
    world.node_tree.nodes['Background'].inputs[1].default_value = .5
    scene.world = world
    for name,pos,power,size in [('key',(180,-140,245),2200000,160),('fill',(160,180,160),1600000,180),('back',(-180,40,210),1800000,130)]:
        light = bpy.data.lights.new(name,'AREA')
        light.energy = power
        light.shape = 'DISK'
        light.size = size
        obj = bpy.data.objects.new(name,light)
        scene.collection.objects.link(obj)
        obj.location = pos
        look_at(obj,(0,0,90))
    bpy.ops.mesh.primitive_plane_add(size=2000,location=(0,0,-1))
    floor = bpy.context.object
    floor.name = 'Neutral review floor'
    mat = bpy.data.materials.new('Neutral floor')
    mat.use_nodes = True
    mat.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value=(.24,.27,.31,1)
    mat.node_tree.nodes['Principled BSDF'].inputs['Roughness'].default_value=.9
    floor.data.materials.append(mat)
    cam = bpy.data.cameras.new('Review camera')
    obj = bpy.data.objects.new('Review camera',cam)
    scene.collection.objects.link(obj)
    scene.camera = obj
    cam.type = 'ORTHO'
    cam.ortho_scale = 210
    cam.clip_end = 3000
    return scene


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--export',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--only')
    ap.add_argument('--views',default='front,oblique,back')
    args=ap.parse_args(sys.argv[sys.argv.index('--')+1:])
    args.output.mkdir(parents=True,exist_ok=True)
    mats,textures={},{}
    for pattern,bank in [('*.mat',mats),('*.tga',textures)]:
        for p in sorted(args.export.rglob(pattern)):
            bank.setdefault(p.stem.lower(),[]).append(p)
    records=[]
    for path in sorted(args.export.rglob('*.psk')):
        if args.only and path.stem!=args.only:
            continue
        scene=setup_scene()
        points,wedges,faces,names,bones=parse_psk(path)
        package=path.parent.parent.name
        used=[]
        for f in faces:
            # Gore/inner skeletons are normally covered by the pristine shell.
            # Exclude these explicitly rather than render damage-only internals.
            name=names[f[3]].lower()
            if any(x in name for x in ('innards','skeleton','adamantium')):
                continue
            if len(set(wedges[i][0] for i in f[:3]))<3:
                continue
            used.append(f)
        mesh=bpy.data.meshes.new(path.stem)
        mesh.from_pydata(points,[],[tuple(wedges[i][0] for i in reversed(f[:3])) for f in used])
        mesh.update()
        obj=bpy.data.objects.new(path.stem,mesh)
        scene.collection.objects.link(obj)
        materials=[]
        for name in names:
            mesh.materials.append(material(name,package,mats,textures,materials))
        uv=mesh.uv_layers.new(name='Original UV')
        for polygon,f in zip(mesh.polygons,used):
            polygon.material_index=f[3]
            polygon.use_smooth=True
            for loop,w in zip(polygon.loop_indices,reversed(f[:3])):
                uv.data[loop].uv=(wedges[w][1],1-wedges[w][2])
        label=LABELS.get(path.stem,path.stem)
        record=dict(mesh=path.stem,label=label,package=package,pskSHA256=digest(path),mainOutfit=path.stem in LABELS,points=len(points),sourceTriangles=len(faces),reviewTriangles=len(used),materials=materials,bones=bones,captures=[])
        for view in args.views.split(','):
            if not record['mainOutfit'] and view!='front':
                continue
            angle=dict(front=0,oblique=-35,back=180)[view]*math.pi/180
            scene.camera.location=(340*math.cos(angle),340*math.sin(angle),87)
            look_at(scene.camera,(0,0,82))
            if 'HelmetMesh' in path.stem:
                center=tuple(sum(v[k] for v in points)/len(points) for k in range(3))
                scene.camera.location=Vector(center)+Vector((100,-100,35))
                look_at(scene.camera,center)
                scene.camera.data.ortho_scale=50
            output=args.output/(path.stem+'-'+view+'.png')
            scene.render.filepath=str(output)
            bpy.ops.render.render(write_still=True)
            record['captures'].append(dict(view=view,path=str(output),sha256=digest(output)))
            print('OUTFIT RENDER:',path.stem,view,flush=True)
        records.append(record)
        receipt=dict(schema='wolverine.stock-wardrobe-review/1',offline=True,nativeVerified=False,refitApplied=False,gameMaterialParity=False,renderer='Blender Cycles CPU; neutral diffuse/normal approximation; pristine shell only',outfits=records)
        (args.output/'renders.json').write_text(json.dumps(receipt,indent=2)+'\n')


if __name__=='__main__':
    main()
