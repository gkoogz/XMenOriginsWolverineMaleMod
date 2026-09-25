# Wolverine Anatomy Tool 1.1 Beta 1

Beta 1.1 freezes the current development snapshot, including the experimental unified physics solver and the accumulated mesh, control, and idle gesture changes since 1.0 Beta 1. It is a prerelease, not a claim that the reported motion issues are fully resolved.

## Install or upgrade

Close Wolverine, extract the complete archive, and run **Install.cmd** or **Upgrade-Beta-1.1.cmd**. The default location is `C:\Games\X-Men Origins Wolverine`. For another location:

```powershell
powershell -ExecutionPolicy Bypass -File .\Install.ps1 -GamePath "D:\Games\Wolverine"
```

The installer accepts the supported original Natural package or the existing Revision 161 package, and the supported original or patched WGame package. It verifies all payload hashes before writing. Cooked packages are distributed as WBX1 deltas requiring supported local game files. Saved controls are preserved. The existing 22 audio clips and two texture variants are included unchanged.

**Uninstall.cmd** or **Rollback-Beta-1.1.cmd** restores the pre-install files, including previous audio. Backups are kept separately in `WGame\ModBackups\WolverineAnatomyTool-v1.1.0-beta.1`. Existing backups are never overwritten. F6 opens the controls; F8 resets them.

## Changes since 1.0 Beta 1

- Accumulated connected-surface and attachment refinements, with refined runtime geometry and shared skin deformation.
- Updated slider mapping and idle gesture support.
- Experimental unified compliant constraint solver with fixed 240 Hz steps, coupled contacts, load-dependent friction, angular response, and bounded suspension.
- Updated release metadata, numerical test harness, and a version-specific installer/rollback directory.

## Validation and limits

The source snapshot builds with x86 Visual C++. Control/persistence tests and the seven-case deterministic motion/rendering suite are recorded in `docs/BETA-1.1-VALIDATION.json`. Clean installation, upgrade, settings preservation, and exact rollback pass in temporary fixtures.

The solver is experimental. Live gameplay feel and performance have not been confirmed for this build. Existing surface folds and contact artifacts remain possible, especially under extreme controls; the checks do not prove continuous collision-free skin. Custom speech does not have matching lip sync. This release is not medical validation.

## Build and test

Run `src/runtime/build.cmd` from its directory using Visual C++ BuildTools and the June 2010 DirectX SDK. `manifest.json` records the shipped DLL and all payload hashes; the DLL is built from this source snapshot, rather than copied from the previous game installation.

The numerical harness is under `tools/beta11`: run `build.cmd` there, then `python validate.py` (NumPy required). Control checks use the executable flags `--glans-controls-test`, `--throb-test`, and `--surface-limit-test`. Installation fixtures are in `tools/Test-ReleaseBeta11.ps1` and require the supported local game packages.
