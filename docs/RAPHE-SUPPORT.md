# Shaft-supported raphe update for 1.0 Beta 1

This build corrects the collapsed shaft-side ridge in the requested default / Semi pose. The shaft-side material strip now follows the shaft's authored rest cross-section instead of blending into the hanging pouch. A precomputed harmonic displacement field spreads the attachment motion into surrounding skin. Scrotal seam relief is retained when the old pose-dependent bridge is inactive. The obsolete final bridge/tube fairing calls are replaced by this support pass.

A final directional ovoid support projection lets the hanging pair yield beneath the shaft and moves its position history consistently. It is an approximate support model, not triangle-exact continuous collision detection. It does not reduce the ridge's strength based on proximity to an enclosing sphere.

## Install

Requires an existing, working **1.0 Beta 1** installation. Close Wolverine, extract the whole update folder, and run **Install.cmd**. Default location is `C:\Games\X-Men Origins Wolverine`.

For a different game folder:

    powershell -NoProfile -ExecutionPolicy Bypass -File Update.ps1 -GamePath "D:\Games\Wolverine"

**Rollback.cmd** restores the original Beta 1 DLL. Use the same GamePath argument if needed. The updater verifies the old and new DLL hashes, retains a backup under `WGame\ModBackups\RapheSupport-20260924`, and leaves settings, character packages, textures and audio files unchanged. Installer and rollback were tested against a separate fixture; the user's game was not modified.

## Renders and validation

The two matched comparison images use geometry and normals captured from the actual baseline and modified runtime. Both are at default controls (50), Semi state (1), after 120 deterministic frames. Each comparison uses identical orthographic camera, scale and lighting on both sides. These are isolated mesh renders, not live gameplay screenshots or generated illustrations. The glans/distal replacement geometry is byte-identical to the baseline in this pose.

- Default Semi: zero detected nonadjacent triangle crossings and zero degenerate triangles.
- Seven sampled configurations: finite geometry, no degenerate triangles, successful D3D draws, render-state restoration, reset and recreation.
- Ridge support is invariant to moving only the lobe proxies. The fixed material strip is restored from the shaft frame.
- 600-frame driven Semi test passed render/reset checks. Its final frame still has 40 detected crossing pairs at the attachment; motion is not collision-free.
- Original area limiter's 10,000-case regression passed.
- Installation and exact rollback passed with settings preservation.

| Sample | Beta 1 crossing pairs | Updated crossing pairs |
|---|---:|---:|
| Default Full Floppy | 186 | 43 |
| Angle 15, Full Floppy | 0 | 0 |
| Default Semi | 0 | 0 |
| Default Erect | 9 | 9 |
| Size 100, Full Floppy | 185 | 108 |
| Size 15, Full Floppy | 221 | 166 |
| Driven Full Floppy, frame 180 | 227 | 74 |

Counts are diagnostic noncoplanar triangle crossings involving the original R14 region, excluding triangles that share positions. They are not counts of anatomical contacts. This audit is not exhaustive continuous collision detection, and the remainder of the game character is not included. Existing motion/extreme-pose attachment artifacts remain; live game validation is pending.

## Source

`source.patch` applies to repository tag `v1.0.0-beta.1` (commit a79357c9ab7215c4d6c73afb8aeb654b6160c434). Apply with `git apply source.patch`, then build `src/runtime/build.cmd` using x86 Visual C++ BuildTools and the June 2010 DirectX SDK. The support generator requires numpy and scipy. Its generated header is included, so regeneration is optional.

Updated DLL SHA-256: `9CDD71CB06BD057B8A23E3ADCBCE4A9BD7ECC8943E5A1D5CE06B84ABFAF310D9`.
Original DLL SHA-256: `19717D7BF796B458E6C64EE7105960654D0B8DBC693D052BAB13A6B037F5375F`.
