> Release note: this audit documents the performance candidate now packaged as 1.8.0. Candidate installer names and unpublished status below describe the earlier audit delivery; use the root 1.8 installer and release validation for the published package.

# Performance pass on 1.7 — findings, changes and validation

The performance candidate preserves the approved model, texture pixels, mesh resolution, contact solver iterations, fluid settings and sequence behavior. The 1.7 release/tag and its distribution remain unchanged. This candidate is installed locally with rollback; gameplay loading time and the screenshot artifact still need a live retest.

## What caused the strongest measured stall

The runtime loaded two 4096 x 4096 PNG skin textures and generated their mip chains synchronously during the first character draw. On this machine the same D3D9 load calls took 2951.2 ms and 2965.9 ms. Prebuilt, uncompressed DDS textures loaded in 42.9 ms and 40.0 ms. All texels in every one of the 13 mip levels were compared and matched exactly. This moves almost six seconds of decoding/filtering out of the rendering path in the fixture. It is not a measurement of total game load time.

The runtime now prefers the native DDS files and retains PNG fallback. The tradeoff is approximately 171 MiB of additional disk storage; resolution, format and GPU texture memory are unchanged. The updater requires matching original PNG hashes, preventing a cache from replacing a differently authored skin.

## Runtime and transition changes

| Area | Finding | Change |
| --- | --- | --- |
| Startup settings | Each launch scanned a 119,255,580-byte diagnostic log to locate eight camera poses. | Store poses separately in TankCameraPresets.txt. Migrate legacy records once; retain the old log. |
| Tank initialization | A screenshot was automatically read back and PNG-encoded after 90 tank draws. | Remove automatic capture. Explicit F10 diagnostic capture remains. |
| Loading frames | Anatomy, physics and the controls panel ran without a character draw, including retained resources from the old scene. | Skip them on absent-character frames. Brief culling pauses preserve a sequence; absence over 250 ms cancels it. |
| Presentation | Multiple final EndScene calls or nested device/swap-chain Present paths could repeat overlay work. | One overlay update per presented frame; nested presentation is guarded. |
| HUD | 97 gameplay/loading or 104 tank draw submissions, repeated states and per-label allocation. | Reuse one vertex batch, preserve order, draw once, restore device state. Loading submits zero HUD draws. |
| Camera | The title camera override could affect any shader exposing a view/projection constant. | Require a local scene transform and a perspective matrix; reject stale scene ownership. Cache pose matrices until the pose changes. |
| Movies | The attract blocker rejected all Bink opens while the title-scene timeout remained active. | Reject only the verified filename-mode attract movie fmv3_1000.bik. Pass other filenames, memory streams and unknown modes through. |
| Idle fluid hooks | Camera discovery, shader reflection and body palette reads ran even without active fluid or marks. | Gate fluid capture/render work on actual consumers. Keep the gameplay input readiness heartbeat. |
| Shader reflection | Vertex layout lookup linearly scanned retained shaders. | Index retained layouts by shader pointer; clear ownership on reset. |
| Body collision | Triangle connectivity was rebuilt alongside animated vertices every frame. | Retain connectivity until participating sections change. Continue skinning vertices and refitting bounds. |
| Remaining marks | Static floor marks could trigger a full skinned-body collision update. | Skip body collision when only world marks remain; preserve current world projection. |
| Diagnostics | Repeated audio messages and append-only session history increased file I/O and log size. | Limit audio diagnostics, use a bounded per-session WolverineRuntime.log, and record inclusive CPU phase timings every 300 visible frames. |

The loading rectangle has two concrete interference paths addressed here: broad movie blocking and camera overrides on non-scene draws. A screenshot alone cannot establish which path caused the live artifact. The fixture verifies that an absent-character loading target remains pixel-for-pixel unchanged even while old menu resources exist.

## Measurements and their limits

The isolated fixture's HUD/loading CPU submission fell from about 0.5–0.9 ms and 97 draws to about 0.001–0.002 ms and zero draws. Visible tank HUD submission fell from 104 draws to one. An idle, unrelated draw-hook microbenchmark went from roughly 0.12–0.16 microseconds to 0.057–0.059 microseconds. These are synthetic CPU measurements, not a game FPS estimate.

The isolated tank overlay plus full anatomy simulation/deformation measured approximately 32 ms before and 29–30 ms after. That loop uses elapsed time and has scheduling/solver feedback, so it is directional evidence rather than a rigorous fixed-work comparison. CPU phase measurements show the remaining major cost is anatomy deformation (about 19–21 ms per update in this fixture), including rounded skin/contact refinement (about 10–11 ms), plus the fixed-step physical solve. Nested timings must not be added together.

## Architecture review and remaining costs

WBX delta files are used only by the installer to reconstruct game packages. The runtime does not apply deltas per frame. Replacing the distribution format would not address these stalls.

The authored rest-shape cache, SIMD geometry passes, dynamic DISCARD uploads for replacement meshes, bounded fluid reference queries, and BVH refit structure were already useful. They are retained. The collision topology cache now complements animated vertex updates rather than duplicating immutable work.

The anatomy solver runs at 240 Hz with 24 constraint iterations; the pouch surface has 60 fairing passes interleaved with contact projection. These dominate remaining CPU work. Reducing those counts or throttling geometry would trade responsiveness or surface/contact fidelity; this candidate does not silently lower them. The supplied CPU telemetry separates physics, deformation, refinement and collision so a live trace can guide the next change.

Gameplay still modifies the engine-owned character vertex buffer. A private staged/dynamic replacement could reduce synchronization stalls, but must preserve engine edits, gore, section identity and device lifecycle. It requires targeted live profiling and broader geometry coverage before replacing that ownership model. Cold shader compilation for small effect shaders, optional audio pool discovery and device resource re-creation remain potential smaller startup costs. No claim is made that all loading stalls or all chapters are fixed.

## Validation

- Production x86 DLL and regression tools compile successfully.
- All three complete 20-second sequence replays pass finite geometry, control persistence, firmness, cancellation and reset checks.
- World-space and actual D3D origin capture, menu return, render-state restoration, duplicate-pass suppression, device reset/recreation, idle animation and audio cues pass.
- Loading framebuffer remains exactly unchanged with retained menu resources.
- Orthographic/UI camera matrices remain untouched; valid perspective overrides are applied and restored; stale scene overrides are rejected.
- Attract filename is recognized; unrelated loading filenames, memory mode, null and invalid pointers are not blocked.
- Retained collision connectivity matches rebuild; moved vertices continue to update; changing participating sections invalidates topology.
- Both DDS textures match every source mip texel exactly.
- Update/rollback fixture verifies runtime/payload hashes, pre-existing cache restoration, removal of newly installed cache, exact prior runtime restoration, and retained settings/audio/logs.

Build tools: tools/performance/build.cmd and tools/teaching/build.cmd. The performance fixture uses a hidden test device and never launches the game. Runtime logs describe inclusive CPU wall time, not GPU completion or overall frame time.

## Installation and next live check

Update.ps1 installs only the candidate DLL, the two native texture caches and separate camera settings. Rollback.cmd restores the exact previous runtime and cache state. Existing controls, fluid settings, phase audio and legacy log are preserved. Source is archived alongside the candidate. Nothing has been pushed or published as a new release.

Restart Wolverine, compare startup-to-tank and tank-to-gameplay, and check whether the loading rectangle is gone. Test camera rotation with fluid, then return to the tank. The new diagnostics are in Binaries/WolverineRuntime.log.

The implementation follows Microsoft's guidance to [minimize state changes and batch draws](https://learn.microsoft.com/en-us/windows/win32/direct3d9/performance-optimizations) and accounts for [SetRenderTarget resetting the viewport](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setrendertarget). The timing claims above come from local fixtures, not those documents.
