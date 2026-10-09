# Existing Alkali clothing refit

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
