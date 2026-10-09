# Stock wardrobe candidates - offline review, October9

The user requested outfit renders, then separate new Top/Bottom candidate
lists beyond the installed Naked/Tank Top/Jockstrap/Jeans/Jeans (open).
Private Base build/outfits-20261009/review contains self-contained galleries,
labeled PNG sheets, exact source hashes and37 selected reference renders.
These are stock meshes/UV textures in bind pose with neutral Blender lighting;
no new costume integration, enlarged-body fit or UE3 material parity is claimed.
The installed483a555e DLL remains unchanged. This is offline reference work,
not a mod edit or a new attachment acceptance pass.

| Top candidate | Authoring implication |
| --- | --- |
| Barfight button-up shirt | Rolled sleeves; shirt/pants share a material, so extract geometry regions |
| Casino leather jacket | Separate jacket section; stock tank underneath |
| Jungle field tank | Worn/blood-stained appearance variant of existing tank |
| Bonus1 brown/yellow top | Author a waist boundary; mask/gloves are shown as optional upper details |
| Bonus2 blue/yellow top | Author a waist boundary and preserve UV aliases |
| Bonus3 X-Force top | Author a waist boundary and preserve UV aliases |

| Bottom candidate | Authoring implication |
| --- | --- |
| Barfight brown trousers and boots | Extract from combined clothes material |
| Jungle cargo trousers and boots | Separate pants section, including field gear |
| Bonus1 brown/yellow bottoms | New waist boundary, tall boots |
| Bonus2 blue/yellow bottoms | New waist boundary, tall boots |
| Bonus3 X-Force bottoms | New waist boundary, tall boots |

Alkali/Blob/ThreeMile/WeaponX mostly repeat tank-and-jeans geometry; Normal is
shirtless jeans. These do not imply eleven distinct new toggle pieces. Eleven
main outfit meshes use the same128 named joints in the same order; this is not
proof of native pose/material compatibility. The Jungle no-claws variation
also uses that skeleton. Two Bonus2 drafts and the unused WeaponX helmet have
only Bone01/Bone02; they are auxiliary assets, not additional playable outfits.
Natural is the anatomy source body, not another garment candidate, and is omitted.

UEViewer initially exported64px resident mips for Barfight diffuse, jacket and
Jungle clothes because Textures_P0.tfc was absent. A private hardlink alias to
the unchanged coalesced.tfc recovered full1024px compressed mip chains. The
three affected looks and no-claws variant were rerendered; earlier low-mip
iterations remain private. Known named unresolved body/claw imports are repaired
from exported reference materials and recorded. No game packages were edited.
Damage/gore internals are excluded; native damage behavior is not reconstructed.

Reproduce from your licensed installation (all output stays private):

```powershell
pwsh -File tools/costumes/Export-StockOutfits.ps1 -InputDirectory '<game>/WGame/CookedPC' -OutputDirectory '<private>/wardrobe' -UModel '<UEViewer>/umodel.exe'
blender --background --factory-startup --threads 6 --python tools/costumes/render_stock_outfits.py -- --export '<private>/wardrobe/export' --output '<private>/wardrobe/renders'
python tools/costumes/build_outfit_gallery.py --renders '<private>/wardrobe/renders' --output '<private>/wardrobe/review'
python tools/costumes/build_toggle_candidates.py --catalog '<private>/wardrobe/review/catalog.json' --output '<private>/wardrobe/review'
```

Shared refit-factor math belongs in Base. Use body-relative displacement
bindings, garment-specific ease and protected details, not uniform whole-model
scale. Full suits require new coupled boundary bindings and compatible coverage
masks before mix-and-match toggles. Cache authoring bindings; reuse current
animation/body deformation each frame. Boots/masks/buckles should preserve
shape where body expansion is irrelevant. No general factor is implemented by
this review. Future adoption must pin tested Base revisions and complete the
native attachment gate in every spoke; Witcher remains deferred.
