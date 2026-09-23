# Wolverine Anatomy Tool v0.9

Version 0.9 packages the current R36-based runtime, necklace contact correction, surface refinements, and two skin texture variants.

## Install or upgrade

Close Wolverine, extract the entire archive, and run **Install.cmd** for a clean installation or **Upgrade-0.9.cmd** for an existing installation. Both use the same checksum-verified installer. The original supported Natural package or the existing Revision 161 package is required. WGame must be the supported original or this release's patched version. Unsupported packages are rejected before changes.

The default game path is `C:\Games\X-Men Origins Wolverine`. For another location run `powershell -ExecutionPolicy Bypass -File Install.ps1 -GamePath "D:\Games\Wolverine"`.

**Uninstall.cmd** or **Rollback-0.9.cmd** restores the files present before installation. Settings are preserved. Backups are retained in `WGame\ModBackups\WolverineAnatomyTool-v0.9`. F6 opens the controls; F8 resets controls. The existing R36 overlay label is retained.

## Changes since 0.8

- R36 attachment shaping and the existing midline continuation.
- Stronger root damping and bounded shaft motion.
- Oblique resting lobe shape and a shallower central skin dip.
- Necklace clearance measured against the animated chest surface.
- Earlier chest collider and necklace collision-flag adjustments.
- Subtle diffuse skin detail and a stronger vascular-color variant for the erect state.
- Unified installation and upgrade with package deltas, payload verification, and rollback.

## Validation and known limitations

Clean installation and upgrade fixtures pass package reconstruction, payload checks, settings preservation, existing-texture backup, and exact rollback. Runtime build, recorded necklace contact checks, and deterministic rendering/state restoration/device reset checks passed during development.

The newly reported underside indentation remains unresolved. Extreme poses can still produce contact or collision artifacts. Necklace correction is a rendered contact adjustment, not a repair of native rigid-body self-collision. Texture changes affect diffuse color; they do not add normal-map relief. Live visual validation is incomplete. This release is not medical validation.

## Build

Build `src/runtime/build.cmd` using x86 Visual C++ BuildTools and the June 2010 DirectX SDK. The payload DLL is the exact current installed build; `manifest.json` records every payload hash. Cooked game packages are distributed as WBX1 deltas requiring the supported local game files.
