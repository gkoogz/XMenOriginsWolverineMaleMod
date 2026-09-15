# Big Dick Logan Mod

Version 0.6 for **X-Men Origins: Wolverine — Uncaged Edition** on Windows PC.

This release installs the complete Revision 161 mod: the remodeled Natural body, the Weapon X-safe body/material changes, anatomical mesh, live size and pose controls, saved settings, movement-responsive shaft and scrotum physics, collision constraints, and checkpoint outfit changes that use the Natural body throughout the game.

## One-click installation

1. Close the game.
2. Download and extract the entire release. Do not run the installer from inside a ZIP preview.
3. Double-click **Install.cmd**.
4. If the game is not detected, paste the folder containing `Binaries` and `WGame`.
5. Launch the game normally.

The installer supports the documented original PC Natural package. It verifies the original file, reconstructs the modded package locally, verifies the result, backs up every changed file, and rolls back automatically if installation fails. It does not require Internet access, Python, a mod manager, or developer tools.

## One-click uninstallation

Close the game and double-click **Uninstall.cmd**. The uninstaller verifies and restores the exact files recorded during installation. Backups remain under:

`WGame\ModBackups\BigDickLoganMod-v0.6`

If a managed file was changed after installation, the uninstaller stops instead of silently destroying the newer change.

## In-game controls

- **F6:** show or hide the Big Dick Logan Mod menu
- **Up / Down:** select a control
- **Left / Right:** adjust it
- **Shift + Left / Right:** coarse adjustment
- **F8:** reset all controls to defaults

Settings save automatically to `Binaries\WolverineLive.ini`.

## Compatibility

- Intended for X-Men Origins: Wolverine — Uncaged Edition on Windows PC.
- Other Natural-body replacements conflict with the character package.
- Other Direct3D 9 proxy mods using `Binaries\d3d9.dll` may conflict. The installer backs up an existing DLL and restores it on uninstall.
- The Weapon X handling included here prevents the Natural-body replacement from turning into the level's electrode geometry.
- Uninstall the previous release using its own uninstaller before installing v0.6. Keep each release’s backup folder until you no longer need it.

## Known issue

Revision 161 retains some collar shimmer at certain sizes and viewing angles. This release preserves that accepted baseline; the rejected Revision 162 and 163 experiments are not included.

## Repository layout

- `payload/` contains the verified binary patch and compiled Revision 161 runtime.
- `src/runtime/` contains the runtime source and generated mesh/physics tables used by v0.6.
- `tools/PatchCodec.cs` documents and implements the small WBX1 delta format.
- `Install.ps1` contains the transactional installer and uninstaller.

## Legal

This repository does not contain the original game package, executable, ISO, save, or crack. `Natural.wbx` is a binary delta that requires a supported original game file. X-Men, Wolverine, and original game assets belong to their respective owners. This is an unofficial adult mod and is not endorsed by them.
