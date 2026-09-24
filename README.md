# Wolverine Anatomy Tool 1.0 Beta 1

Beta 1.0 packages the currently installed runtime, the 22-clip idle chatter pool, shape and motion refinements, and the two skin texture variants.

## Install or upgrade

Close Wolverine, extract the entire archive, and run **Install.cmd** for a clean installation or **Upgrade-Beta-1.0.cmd** for an existing installation. Both use the same checksum-verified installer. The original supported Natural package or the existing Revision 161 package is required. WGame must be the supported original or this release's patched version. Unsupported packages are rejected before changes.

The default game path is `C:\Games\X-Men Origins Wolverine`. For another location run `powershell -ExecutionPolicy Bypass -File Install.ps1 -GamePath "D:\Games\Wolverine"`.

**Uninstall.cmd** or **Rollback-Beta-1.0.cmd** restores the files present before installation, including any idle WAVs that were already present. Settings are preserved. Backups are retained in `WGame\ModBackups\WolverineAnatomyTool-v1.0.0-beta.1`. F6 shows or hides the controls; F8 resets them.

## Changes since 0.9

- Erection, Throb, and Idle Chatter controls appear above the sliders; F6 is labeled in the header and control legend.
- Four throb settings animate size and angle independently, with pulses available at maximum slider values.
- Idle Chatter switches immediately in-game between standard cues and a pool of the three originals plus 19 supplied clips. It uses the existing spoken idle animations.
- Additional surface and support adjustments in the runtime.
- The installer verifies and backs up the new audio files as well as the runtime and textures.

## Validation and known limitations

Clean installation and upgrade fixtures pass package reconstruction, payload and audio checks, settings preservation, and exact rollback. The runtime build and rendering harness pass. The user heard new idle speech in Weapon X with Idle Chatter on; every clip and later chapters have not been checked in-game.

The reported underside ridge still ends prematurely and remains unresolved. Extreme poses can still produce contact or collision artifacts. The longer custom speech clips reuse the game's existing talking animation and do not have matching lip sync. Texture changes affect diffuse color; they do not add normal-map relief. This release is not medical validation.

## Build

Build `src/runtime/build.cmd` using x86 Visual C++ BuildTools and the June 2010 DirectX SDK. The payload DLL is the exact current installed build; `manifest.json` records every payload hash. Cooked game packages are distributed as WBX1 deltas requiring the supported local game files.
