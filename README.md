# Wolverine Anatomy Tool 1.3

Packages the currently installed runtime with prepared shape caching, baked geometry bindings, faster live surface evaluation, slightly relaxed lateral suspension, and a wider pelvic attachment at large dimensions.

## Install or upgrade

Close Wolverine, extract the entire archive, and run **Install.cmd** or **Upgrade-1.3.cmd**. Default game location: `C:/Games/X-Men Origins Wolverine`. For another location:

```powershell
powershell -ExecutionPolicy Bypass -File .\Install.ps1 -GamePath "D:\Games\Wolverine"
```

The installer verifies supported original or Revision 161 Natural packages and original/patched WGame packages by SHA-256. Cooked assets are distributed as WBX1 deltas requiring local game files. Saved controls are preserved. All 22 audio clips and both skin textures are included. F6 opens the controls; F8 resets them.

**Uninstall.cmd** or **Rollback-1.3.cmd** restores the previous files and preserves saved controls. Backups are retained separately in `WGame/ModBackups/WolverineAnatomyTool-v1.3.0` and never overwritten.

## Changes since 1.2

Prepared rest geometry and shared refinement frames reduce repeated live work. Recorded performance comparisons measured approximately 30–36% lower whole-update CPU costs at 60 FPS input, with identical mesh/body/physics output in 1,920 matched frames for the optimization stage. Physics remains 240 Hz and surfaces update every presented frame. Details: docs/GEOMETRY-PASSES.md and tools/physics/README.md.

Lateral suspension compliance increases slightly. The pelvic attachment recruits more width and a little forward prominence at large diameters; baked proximal skin weights smooth its transition into shaft motion. Neutral and minimum captured meshes remain identical to the preceding installed build. Sampled local crossings were removed in large static/moving cases; the broad firm sample improved. Local-ramp timings were within observed variability and add no mesh density or live pass.

## Known limits

Neighboring skin folds and intersections remain in some poses. Replay timings are CPU measurements, not in-game FPS. Live gameplay quality/performance of the final build has not been independently verified. This release does not claim an all-pose collision-free surface.

## Source and payload

Source and build scripts are included. The exact installed runtime SHA-256 is `2B9341CF48EEC9F2AEDBC23F3F5E29249E46A83A614E1AEA812859A5E20C81FF`. Installation and upgrade/rollback fixtures: tools/Test-Release13.ps1. Generated bindings can be checked with tools/physics/bake_geometry_bindings.py --check and bake_pelvic_root.py --check.
