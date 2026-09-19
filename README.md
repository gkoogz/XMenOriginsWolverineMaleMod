# Wolverine Anatomy Tool 0.7.1 - scrotal junction R2

This revision builds on the original 0.7.1 pelvic ramp. It replaces the pointed upper scrotal transition with a broader, smoother attachment, a gradual neck taper, and a rounded transition into the lobes. Vertex redistribution improves triangle regularity in the sampled neck region. It retains the original mesh connectivity, UV atlas, rig, character package and physical solver.

## Upgrade from 0.7 or original 0.7.1

Close Wolverine, extract the whole archive, and run **Upgrade-Junction.cmd**. The default location is `C:\Games\X-Men Origins Wolverine`. For another location, run `Upgrade-Junction.cmd -GamePath "D:\Games\Wolverine"`.

The upgrade verifies the released 0.7 or original 0.7.1 DLL and the exact character package, saves the previous DLL, and preserves settings. **Rollback-Junction.cmd** restores whichever supported version was present. Backups are under `WGame\ModBackups\WolverineAnatomyTool-v0.7.1-junction-r2`. F6 displays **v0.7.1 SCROTAL JUNCTION R2**.

For a supported unmodified game installation, use `Install.cmd` and `Uninstall.cmd`. The older `Upgrade-0.7.1.cmd` remains available for 0.7 only; prefer the Junction pair above.

## Implementation

`FinishScrotalJunction()` runs after the existing final-pose pelvic ramp and before normal/tangent reconstruction. A constrained biharmonic operator covers 902 existing welded graft groups, including the adjacent ventral shaft rows. It retains the outer body weld and the ends of the patch. Tangential redistribution improves vertex spacing; limited high-curvature relaxation reduces folded fans. A uniform displacement bound and finite/area checks guard unusual settings. The process is recomputed from the current posed surface each frame, without accumulating drift.

`tools/Generate-ScrotalJunction.py` regenerates `src/runtime/scrotal_junction.h` using Python, NumPy and SciPy. Build `src/runtime/build.cmd` with x86 Visual C++ BuildTools and the June 2010 DirectX SDK. The deterministic harness is in `tools/`.

## Validation and limits

Eleven matched 240-frame captures cover default/maximum in all three physics states, small size, two mixed-size combinations and low/high angle. All have finite positions, zero degenerate neck triangles, exact body-weld coincidence, unchanged body donor positions and identical physics-chain nodes. Both median and fifth-percentile neck triangle quality improve in all eleven cases.

In the default capture, median quality improves from 0.774 to 0.842; maximum improves from 0.768 to 0.859. The metric is 4*sqrt(3)*area / sum(edge length squared), where 1 is equilateral. The measured region contains 940 triangles.

Nonadjacent triangle-crossing checks involving the neck find 16 to 0 pairs at default, 36 to 0 at maximum, and 36 to 0 at maximum shaft/default scrotum. This is an intersection test on the emitted graft surface, excluding shared-edge/vertex and coplanar cases; it is not a proof of global collision-free geometry. The high-angle case still has 252 crossing pairs, down from 351. Maximum size also retains three neck edge dihedrals above 90 degrees. This is an improved existing mesh, not a uniform retopology or a claim of zero distortion across the entire slider range.

The final compiled DLL passed an isolated D3D9 smoke test. Upgrade/idempotency/rollback tests passed for both supported starting DLLs, with settings, package and read-only attributes preserved. Mean extra source-harness time was approximately 4.5 ms per frame; that is not an in-game FPS benchmark.

The game installation was not changed. **Live-game visual, animated-contact and performance playtests remain pending.** The renders use emitted runtime coordinates and the original diffuse atlas in Blender studio lighting, not the game's shaders.
