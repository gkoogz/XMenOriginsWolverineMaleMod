# Existing Alkali clothing refit

Current correction: tank20 adds local measured armhole clearance through
pinned Base cbe542f, plus a folded lower hem with original fabric donors.
The jockstrap cotton is calibrated from the original tank diffuse median;
ribbing and red/blue trim remain. Reproduce with prepare_matched_fabric.py
using the licensed T_Wolverine_Shirt_d.dds and matched_fabric_data.h output.
The original tank18 pec apex and remote chart remain
unchanged. Original stock maps and original-sheet topology, UVs, source pairs and skin weights
remain. See PROJECT_HANDOFF.md for exact identities and 61 native samples;
full attachment acceptance remains false and this candidate is uninstalled.

## Historical correction: tank18 and jeans6

The tank now follows closest points on the complete measured body surface,
with 10 passes relaxing its three-dimensional displacement. Original geometry
topology, fold chart, UVs, normal aliases, source pairs and skin weights remain
the input; the cut positions follow the body and donor bindings are measured
again. The offset-envelope fit was rejected for raised pec pockets. Source
face orientation is preserved; relaxed clearance is not certified. Base pin
014e57a2d659298cd3126482a2573b2087cabaa6 supplies the shared algorithm.
Native front/side/moving oblique and tank-scene captures were inspected.
Jeans6, original maps, protected cut mask and shader state restoration remain.
See PROJECT_HANDOFF.md for exact identities, 57 samples and acceptance limits.
No replacement installation, push or publication was performed.

## Historical tank12 and jeans6 attempt

Base pin8f45949f146f1ce866fc8ac35682be3f76ef585a supplies shallow garment
shells, a localized height/lateral chest offset, bounded stock-cut refinement
and conservative body masking at open cuts. Tank12 retains original corners,
folds, UVs and source-pair/skin lineage; shading normals follow the posed mesh.
The previous tank11 body mask was rejected after a native close-up revealed
black triangular holes at the exposed neck. The new mask protects a measured
2.5-unit cut band and checks edge/interior witnesses before removing a body
face. It preserves the original body positions, topology and UVs.

The black title-scene anatomy had a separate cause: deferred dog-tag rendering
left upper vertex constants, including LocalToWorld, holding the tag transform.
MenuNecklace explicitly preserves/restores all256 vertex and224 pixel float
constant registers with the native state block. Original skin maps, native
depth/culling and lighting remain. Full opt-in state auditing and captures are
diagnostic only; recorded source/native evidence is separate from acceptance.

Jeans6 retains the original trouser mesh, boots, original leather chart and
rigid buckle. The fly keeps its narrower155-degree fold and restrained motion.
Inner faces and finished cut walls add0.12 measured units of depth. The original
leather strip follows the whole waist at0.22 thickness/0.24 outset, splitting
at the open fly. Depth layers preserve source donors, UVs, skin and motion
weights; walls have separate shading normals. Denim travel remains3 degrees,
open belt ends2 degrees. No stock pixel assets are replaced or committed.
Other spokes adopt shared helpers through their own pins and measured inputs;
Witcher remains paused. Full body collision, attachment/LOD/campaign acceptance
and UE3 damage/shadow parity remain unverified. See PROJECT_HANDOFF.md for
actual build/native/installed identities and retained rollback.

## Historical tank9 and jeans4 correction

The correction preserves the stock tank/jeans UVs, texture detail and separate
stock belt buckle. The earlier flat procedural cotton/denim was a poor visual
substitute. Tank revision9 fits a smooth front/back section offset field to
measured enlarged torso supports, retaining source Y/Z, neckline, armholes,
hem, folds and UV aliases. Revision8 was rejected during close native review
for a jagged neckline and harsh shoulder folds. Its offline normal comparison
found 509 faces rotated beyond90 degrees; revision9 has zero, with maximum
rotation25.7941 degrees. The exporter refuses overturned source faces.
Dog tags use the same fitted shirt surface.
Jeans revision4 retains the original trouser mesh, boots and buckle; the open
variant applies the existing fly fold and keeps the buckle in one piece on the
right flap using a proper rigid rotation, preserving its original dimensions.
UV donors identify UPK jeans vertices and PSK buckle wedges separately.

Shared fitting lives in Base pind49d42c; asset/material/rig code lives here.
Source package SHA90385a8b1734c3f4b1e1697734d2b6b98a319fec6c6ea27ea906d626f3a06cf6.
Stock PSK SHA3074ccb3a7d2f7fa900f78cd4f27c8111e9afbe1b3eaba01f8b64b9f86cf2f29.
This is a custom renderer using original diffuse/normal/specular maps and
captured native light inputs, not full stock UE3 damage-material parity.

## Reproduce licensed build inputs

Export the developer's licensed CH_Wolverine_Alkali_SF.xxx with UEViewer
`-export -all -noanim -nostat -dds -path=<CookedPC> -out=<private output>`.
Set MALEMOD_STOCK_MAPS to its Startup_int/Texture2D directory. The versioned
tools/costumes/stock_materials.json hashes all eight exact original DDS maps.
prepare_stock_materials.py validates them and generates resource inputs inside
the private build. No stock pixel maps/packages are committed to Git. The
runtime embeds these resources so installation remains atomic with the DLL.
Textures are loaded once, with full mip chains generated at load; draws restore
samplers and GPU state. Geometry follows the existing pose each frame.

Reproduce tank data with prepare_tank_top.py --stock <stock tank JSON>
--stock-psk <Alkali PSK> --base <pinned Base> --output <tank_top_data.h>.
Use the analogous arguments for prepare_jeans.py. Original cut boundaries,
skinning, UV aliases and source donors are retained. Native qualification and
installed identities are recorded separately in PROJECT_HANDOFF.md. Full
attachment/LOD/campaign acceptance is not established by these recipes.

The front/back support fit deliberately excludes grazing and turned-back
cloth faces from its envelope constraints. It does not certify their full
collision clearance. Native cutout, side and motion inspection remain required.
Full stock UE3 damage, shadow receiver and garment shadow silhouette parity
remain unverified. This correction is a development candidate; the retail
runtime is retained until the mandatory attachment gate is complete.
