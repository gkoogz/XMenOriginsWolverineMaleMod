# Wolverine Anatomy Tool 1.2

Restores and packages the September 25 coupled-contact solver update (source commit f8d354f). This is the exact earlier solver-overhaul DLL. Later raphe/frenulum/collar modeling candidates are excluded.

## Install or upgrade

Close Wolverine, extract the entire archive, and run **Install.cmd** or **Upgrade-1.2.cmd**. Default game location: `C:/Games/X-Men Origins Wolverine`. For another location:

```powershell
powershell -ExecutionPolicy Bypass -File .\Install.ps1 -GamePath "D:\Games\Wolverine"
```

Supported original or Revision 161 Natural packages and original/patched WGame packages are checked by SHA-256. Cooked packages are distributed as WBX1 deltas requiring supported local game files. Saved controls are preserved. All 22 audio clips and both skin textures are included unchanged. F6 opens the controls; F8 resets them.

**Uninstall.cmd** or **Rollback-1.2.cmd** restores the prior files and keeps saved controls. A separate backup is retained in `WGame/ModBackups/WolverineAnatomyTool-v1.2.0`. Existing backups are never overwritten.

## Solver changes since 1.1 Beta 1

Coupled shaft/support/suspension/body contacts, static/sliding friction, curvature damping, suspension shear/torsional elasticity, analytic attachment derivatives, convex support normals, and interpolation of changing dimensions and moving thighs over fixed physics steps.

Matched offline replays measured 96.9% less high-frequency lobe motion at the saved large setting, 82.9% less with tight idle weight shifts, and 95.3% less with tight suspension and Gentle periodic motion. These are synthetic replay results, not live gameplay FPS measurements. Full methods and limits are in docs/PHYSICS-CONTACT-UPDATE.md and docs/physics-1.2.

## Known limits

Some tight poses produce worse upper-attachment folds than the preceding installed build. Shaft motion metrics are mixed, and skin intersections remain. Live gameplay performance for this exact restored build has not been verified. The later geometry candidate caused a reported 8 FPS collapse and stretched polygons and is not part of 1.2. The release does not claim those later modeling changes or an all-pose collision-free result.

## Source

Runtime source is based exactly on f8d354f; only packaging, version metadata and documentation change. The shipped DLL SHA-256 is `42CAD24ADAF62892B5C7A0D348867D3C13CAD3B969BECF53DC0AF2A6758326D2`. Runtime build and regression commands are in tools/physics/README.md. Installation fixtures are in tools/Test-Release12.ps1.
