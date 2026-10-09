# Independent tank-top costume

Current correction: tank18 follows the complete measured pec/body surface
using pinned Base 014e57a and a relaxed displacement field. Original mesh/UV/
source-pair/skin lineage is retained; cut positions follow the body and measured
body bindings are regenerated. Tank12's raised pec pockets are rejected;
the bounded tank13 attempt is superseded. The body mask and native shader fix
remain. See PROJECT_HANDOFF.md for 57 sampled native captures and exact source/
build identities; full attachment acceptance remains false and no install ran.

Historical correction: tank12 uses localized expansion, bounded cut rounding,
posed alias normals and a protected body mask at the neckline/armholes. The
deferred title dog-tag draw explicitly restores full native shader constants
to fix black anatomy. See STOCK-CLOTHING-REFIT.md and PROJECT_HANDOFF.md for
current source/build/native evidence; full attachment acceptance remains false.

Historical correction: tank revision9 preserves the original shirt cut, folds,
UVs and licensed diffuse/normal/specular maps while refitting the enlarged
chest with Base's smooth section offset. Revision8's jagged neckline was
rejected in native review. See STOCK-CLOTHING-REFIT.md; the older revision6
verification below is historical and does not qualify the new candidate.

The F6 menu has independent TOP (Naked / Tank Top) and BOTTOM (Naked /
Jockstrap / Jeans / Jeans open) rows. Both values persist in WolverineLive.ini. Legacy Clothing/Style
only migrates the bottom. Changing the top does not restart anatomy physics.
F8 resets both selections to Naked. Existing preferences survive installation.

Base owns the SDK-free preference contract, measured torso fitting, edge
refinement, chart relaxation and regression cases. The adapter consumes an exact
Base lock and owns stock extraction, observed palette mappings, native buffers,
lighting passes, the menu and installation. Witcher remains deferred.

The licensed stock Alkali shirt supplies the cut, original UV seams, source
vertex lineage and skin weights. One subdivision produces 2341 vertices and
4364 triangles. New vertices retain source pairs and blended weights; the maximum
discarded fifth weight is recorded rather than described as lossless. Fitted
geometry follows current body displacement through measured barycentric donors.
The shirt is CPU-skinned once per current render frame. It has no cloth walking
solver and no reduced update cadence.

Dog-tag surface contact unions the current fitted shirt triangles with the bare
body. Top removal invalidates the cache even at an identical pose. This updates
rendered contact clearance; it does not modify native PhysX accessory actors.
The title scene queues its HDR tag draw until both current body palettes arrive,
then restores the original tag material and current body state. Joint mappings
are observed by exact joint name, not inferred from matching integer IDs.

Reproduce from your own licensed stock package and external UPK Explorer tools:

```powershell
pwsh -File tools/costumes/Export-StockTank.ps1 `
  -InputPackage '<vanilla>/WGame/CookedPC/CH_Wolverine_Alkali_SF.xxx' `
  -UpkDirectory '<UPK Explorer>' -Output '<private>/stock-tank.json'
python tools/costumes/prepare_tank_top.py --stock '<private>/stock-tank.json' `
  --stock-psk '<private>/CH_Wolverine_Alkali.psk' `
  --base '<exact pinned Base>' --output src/runtime/tank_top_data.h
python -m unittest tests.test_torso_garment_fit # in Base
```

Stock package SHA256 is
90385a8b1734c3f4b1e1697734d2b6b98a319fec6c6ea27ea906d626f3a06cf6.
Body/header hashes, source pairs and numerical fit settings accompany the
generated header. Stock exports, packages, captures and settings stay private.

Native iteration uses tools/iteration/Test-Costumes.ps1 for eight independent
selections, motion and front/side/rear views, followed by the shared attachment
matrix. Wait for the startup demo to finish; otherwise its F6/shape controls can
race the tests. Captures are evidence only after visual review, not on creation.

## Current verification boundary

Current source027628d pins Base45ca775. Tank binding revision6 keeps2341
vertices/4364 triangles. Native iteration rejected pec clipping, a bad native
non-HDR pass, inflated section fitting and inappropriate bare-body overwrites.
Chart relaxation, measured smooth normals, clockwise face convention and
coverage masks improve the close fit. Mask a bare-body triangle only when all
three corners are covered, retaining tissue along neckline/armhole cutouts.
Material roles/winding use TEXCOORD data rather than color interpolation.
Cotton owns its diffuse/SH/light-pass state and excludes the bare-chest receiver
map. New garments restore GPU state after every draw. Their own engine shadow
silhouettes and receiver-shadow fidelity remain unverified.

Stock skin weights remain original, including their animation deformation;
this is a fitted fixed costume rather than a dynamic cloth solver. Front,
side, oblique, rear and stride/jump/landing views are sampled. Some directional
shading/faceting remains at edges; do not describe this as universal cloth
quality or campaign/LOD acceptance. Installation/evidence identity is recorded
in PROJECT_HANDOFF.md after validation.

The full attachment gate remains FALSE. A sampled room pass does not certify all
coupled controls, resources/LODs, campaign transitions or gameplay performance.
Retain the accepted runtime and exact rollback. A shirt shadow silhouette and
native accessory rigid-body collision are not certified by this renderer.

Jeans and Jeans (open) are implemented alongside this costume. See
JEANS-COSTUMES.md for their measured stock section, fly design and limits.
