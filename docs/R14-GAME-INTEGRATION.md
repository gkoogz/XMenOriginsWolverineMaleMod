# R14 integrated game candidate

R14 was approved by the user and installed on 2026-09-20 for a play test. The
approved 10,554-vertex / 21,086-triangle mesh replaces the graft section at draw
 time through the D3D9 proxy. The original package, bone palette, materials,
pelvis controls and original 2,388-point shape/physics cage remain in use.

The new surface is transported from FINAL cage positions. One fitted distal
frame preserves the crown; retained parent correspondence preserves the root,
and a short boundary transition joins the two. Glans Size evaluates the R14
shape samples independently: 0 = .85x, 50 = 1x, 100 = 1.6x. F8 resets it to 50.
Old donor glans scaling is bypassed. Final normals/tangents are rebuilt, the
approved shaft-band fitted normals are transported with the frame, and the
source pelvis bone palette and atlas region are retained. Distal UV mapping is
a smooth cylindrical fit to the existing atlas, not a new texture bake.

The exact section draw is replaced in each applicable pass; vertex/index state
is restored afterward. Device reset releases both replacement buffers and
recreates them. Failure to allocate/draw falls back to the original section.
No game launch was performed.

## Evidence

- x86 runtime and source-including deterministic harness compiled successfully.
- 18 menu rows, 0/100 glans endpoints, reset, persistence and missing-key default passed.
- Ten deterministic geometry cases: neutral, both glans endpoints, saved preset,
  saved preset/max glans, small controls, angles25/75, floppy and semi with motion.
- All ten finite/nondegenerate; zero nonadjacent overlap pairs involving the
  rebuilt patch in BVH checks. This does not prove absence of all body collisions.
- Direct D3D9 replacement draw, original buffer restoration, reset and buffer
  recreation passed. Neutral and saved/max silhouettes inspected.
- Existing atlas diffuse rendered through transferred UVs and inspected.
- The first harness camera/depth configuration was incorrect; corrected before
  visual validation. It was not a game runtime geometry defect.
- Installed DLL hash verified; package and live INI hashes stayed unchanged.

Game shader permutations, full gameplay appearance and performance are still
pending the user's play test. Harness rendering is not in-game confirmation.
The previous root/collar physics limitations remain; no new claim is made about
extreme body collisions. Original asset bounds/LOD behavior remain inherited.

## Installation

Runtime SHA256: 43103535473C9215137C173D3478E00BE8F7C4B8105BB4332421020D04204E7F
Previous runtime SHA256: B835ABE6DE44C4F67A9CD63805897BBB347FA5EB4489343260E3BA82D2F573FE
Package SHA256: 7C5CE1F5FD45AB4F7A159F9D5455B1D40D11E72F4C2EFE7D7F18C7391AED2A64
INI SHA256 (unchanged): 55BEC06C4B04444B07931964E2B2962D557306008AE97120F4D079609CE0121D
Backup: C:\Games\X-Men Origins Wolverine\WGame\ModBackups\R14-integration-20260920-014457

Outputs: outputs/r14-game-integration (DLL, manifest, installation record,
Install-R14.ps1, Rollback-R14.ps1, geometry audit and textured harness image).
Rollback restores only the previous DLL and preserves current settings.

Reproduction: work/build_r14_game_asset.py generates r14_asset.h from the
approved authoring JSON, its sparse source parents and the original preview-g1
reference cage. work/r14-game-integration contains the isolated tested source,
build scripts, deterministic harness, final position dumps, rendered captures
and transfer arrays. work/audit_r14_game.py checks the final dumps in Blender.
The repo src/runtime source/header set now matches this integration candidate.
Public release payloads/manifests have not been changed.
