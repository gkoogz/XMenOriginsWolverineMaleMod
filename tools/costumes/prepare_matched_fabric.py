"""Measure licensed tank fabric for the jockstrap's shared FabricStyle contract.

Only the original cloth UV footprint is sampled. The stock sampler decodes
sRGB, so its robust median is converted to linear RGB before shader upload.
No texture, garment geometry, stripes or installed file is modified.
"""
import argparse, hashlib, json, re
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw


def array(text, name, width):
    block = re.search(r'\b'+name+r'\[\](?:\[\d+\])?\s*=\s*\{(.*?)\};', text, re.S)
    if not block:
        raise ValueError(name)
    return np.array([float(x) for x in re.findall(r'-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?', block[1])]).reshape(-1, width)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--diffuse', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    repo = Path(__file__).resolve().parents[2]
    recipe = repo/'src/runtime/tank_top_data.h'
    text = recipe.read_text()
    count = int(re.search(r'sourceSheetVertices=(\d+)', text)[1])
    uv = array(text, 'uv', 2)[:count]
    triangles = array(text, 'triangles', 3).astype(int)
    triangles = triangles[np.all(triangles < count, axis=1)]
    if not np.isfinite(uv).all() or np.any((uv < 0)|(uv > 1)):
        raise ValueError('Expected measured unit-island stock UVs')
    image = Image.open(args.diffuse).convert('RGB')
    mask = Image.new('1', image.size)
    draw = ImageDraw.Draw(mask)
    xy = uv*np.array(image.size)
    for t in triangles:
        draw.polygon([tuple(q) for q in xy[t]], fill=1)
    samples = np.asarray(image)[np.asarray(mask, dtype=bool)]
    if len(samples) < 100:
        raise ValueError('Empty stock cloth footprint')
    median = np.median(samples, axis=0)/255.
    linear = np.where(median <= .04045, median/12.92, ((median+.055)/1.055)**2.4)
    rows = ['#pragma once', '// Licensed stock cotton median, linear RGB; see matching JSON receipt.',
            '#include <malemod/garments/meridian_material.hpp>', 'namespace WolverineFabricPalette {',
            'inline constexpr malemod::garments::meridian::FabricStyle tankMatched{',
            ' {'+','.join(f'{v:.9f}f' for v in linear)+'}', '};', '}']
    args.output.write_text('\n'.join(rows)+'\n')
    receipt = dict(schema='wolverine.matched-fabric/1', revision=1,
                   method='Per-channel median of original cloth UV footprint, sRGB decoded to linear RGB',
                   diffuseSHA256=hashlib.sha256(args.diffuse.read_bytes()).hexdigest(),
                   tankHeaderSHA256=hashlib.sha256(recipe.read_bytes()).hexdigest(),
                   imageSize=list(image.size), originalSheetVertices=count, originalSheetTriangles=len(triangles),
                   clothTexels=len(samples), medianEncodedRGB=(median*255).tolist(), linearRGB=linear.tolist(),
                   sharedContract='malemod::garments::meridian::FabricStyle',
                   appliesTo='Jockstrap pouch, waistband and straps; original red/blue stripes and rib detail retained',
                   originalTextureModified=False, geometryModified=False,
                   headerSHA256=hashlib.sha256(args.output.read_bytes()).hexdigest())
    args.output.with_suffix('.json').write_text(json.dumps(receipt, indent=2)+'\n')
    print(json.dumps(receipt))


if __name__ == '__main__':
    main()
