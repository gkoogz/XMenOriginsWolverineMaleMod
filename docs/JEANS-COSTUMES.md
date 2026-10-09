# Jeans and open fly

The independent Bottom selector includes Naked, Jockstrap, Jeans and Jeans
(open). Closed jeans suppress the anatomy and clinical-fluid draw. Open jeans
retain the full anatomy surface and its current physics. Neither option runs
the jockstrap walking solver. Top selection remains independent.

The licensed Alkali material3 section supplies 888 vertices/1478 triangles,
original UVs and canonical bone weights. It includes its stock footwear. The
adapter uses a dark denim weave, antialiased waistband/fly/pocket stitching and dark leather foot region, with the current
native diffuse/SH lighting; original stock texture reproduction is not claimed.
Source units are measured export coordinates, not asserted centimeters.

Base `fly_panels.py` partitions triangles at the front fly envelope, preserves
source-face barycentric donors, then folds the two panels145 degrees outward
around their attached outer edges. The measured adapter envelope is Z70..97,
half-width14 at its top, front +X. Skin and UV fields interpolate from the same
donors. Maximum discarded skin weight is recorded in jeans_data.json. Folded
panels are fixed rest geometry driven by the existing skeleton each frame;
there is no second cloth simulation or update-rate reduction.

Both measured body palettes are required in the current render frame: the
stock left and right leg bones live across the two body resources. Gameplay
compact slots are derived from the measured body export and exact retarget
palette, with consistency assertions. No new joint names are invented. Body
coverage indices preserve the existing resource boundary and leave the open
fly's pelvic patch visible. These coverage masks require native visual review.

Reproduce offline from your licensed vanilla package:

```powershell
pwsh -File tools/costumes/Export-StockTank.ps1 -Garment Jeans `
  -InputPackage '<vanilla>/WGame/CookedPC/CH_Wolverine_Alkali_SF.xxx' `
  -UpkDirectory '<UPK Explorer>' -Output '<private>/stock-jeans.json'
python tools/costumes/prepare_jeans.py --stock '<private>/stock-jeans.json' `
  --base '<exact pinned Base>' --output src/runtime/jeans_data.h
```

Package SHA256:
90385a8b1734c3f4b1e1697734d2b6b98a319fec6c6ea27ea906d626f3a06cf6.
The export is read-only. Packages, captures and generated sandbox outputs stay
private. Grey-room testing uses Test-Costumes.ps1 -Jeans for all8 wardrobe
combinations plus oblique/side/rear/motion, then the shared attachment matrix.

Native sampled fit is recorded separately in PROJECT_HANDOFF.md. Do not infer
success from the generated header or compilation. Full attachment/LOD/campaign acceptance is
separate. Witcher remains deferred; future spokes adopt the shared fly and
preference contracts through a pinned Base revision and their measured assets.

Observed final source027628d room run20261009-033739-611f95 completed9 specific fit
captures:1/25/50/75/100 Overall+Width, all-max open/closed, Block command and
recovery. The Block samples remained standing; largest-size crouch recovery
is not certified by that command. Ordinary stride/jump/landing views are
separate. Earlier sourcee1ba307 fit checks used the same jeans geometry,
material and anatomy solver; final027628d keeps tissue at shirt cutout edges.
Open-fly panels are fixed designed folds, not physically simulated denim.
