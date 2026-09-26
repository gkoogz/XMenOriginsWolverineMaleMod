# Wolverine Anatomy Tool 1.4

Release 1.4 packages the current installed build: the 20-second teaching tracer demonstration, enlarged blue fluid tracers, demonstration pulses, and relaxed lateral suspension. Runtime behavior is unchanged from the last installed update.

## Install or upgrade

Close Wolverine, extract the entire archive, and run **Install.cmd** or **Upgrade-1.4.cmd**. Default location: `C:/Games/X-Men Origins Wolverine`. For another location:

```powershell
powershell -ExecutionPolicy Bypass -File .\Install.ps1 -GamePath "D:\Games\Wolverine"
```

The installer verifies supported original or Revision 161 Natural packages and original/patched WGame packages by SHA-256. Cooked assets are WBX1 deltas requiring local game files. Saved controls are preserved. All 22 existing audio clips and both skin textures are included. F6 opens controls; F8 resets them. Period starts or cancels the teaching demonstration; see [teaching documentation](docs/TEACHING-FLUID-POC.md) for cancellation and configuration details.

**Uninstall.cmd** or **Rollback-1.4.cmd** restores previous files and preserves saved controls. Backups are retained in `WGame/ModBackups/WolverineAnatomyTool-v1.4.0` and never overwritten.

## Validation and limits

Frame-rate, fluid lifetime/detachment, saved-control, cancellation, integrated mesh/physics, shader reflection, D3D state restoration and device-reset checks passed for this build. Installer/rollback fixture validation is provided in tools/Test-Release14.ps1.

The teaching sequence is a visual prototype without medical calibration. Detached tracers remain in character component coordinates; demonstrate stationary. Reference-plane/lifetime cleanup is not terrain collision. No new ground splats or vocalizations are included. Existing folds/intersections remain in some poses. Offline replay/render checks do not establish live game FPS or all-chapter coverage. Final appearance remains pending user verification.

## Source and payload

Source and build scripts are included. Runtime SHA-256: `09AA98C9293573398E0A44CCD2FAEB0A00FBD79520C40CF572361F4D6871D05F`.
