# Current: rejected lighting fix removed; R14 capture build installed

User feedback 2026-09-20: the skin-basis shader fix reduces camera-dependent sheen; dark scribble remains with fix both ON and ORIGINAL. Treat it as unsuccessful. Original constants staying unchanged did not prove equivalent appearance. The old replay omitted environment cubemaps and was insufficient to validate full shading. Do not reuse this override as a solved fix.

Verified game closed; installed R14 original-shading diagnostic at 02:34 EDT. Runtime SHA256 041300B923F00CD25B1D98D871BA311A9D7171DB78E2401DEC44CE05A32D4767. Backup of the rejected runtime and live package/INI: C:\Games\X-Men Origins Wolverine\WGame\ModBackups\R14-render-capture-20260920-023404. Package and saved settings hashes unchanged. Game not launched.

The runtime draw calls origDIP again with no shader override. F10 captures one frame (max16 R14 draws): CPU-mirror geometry (no read-locking WRITEONLY buffers), original shader code/constants, layout, states, all textures including cubemap faces, per-draw RT textures and before/after RT images, screenshot. Binaries/R14Capture_timestamp/complete.txt marks completion. Menu says CAPTURE SAVED. F9 no longer switches shading; F6 menu still works.

Tests: x86 build; actual saved-draw capture yields PNG byte-identical to original replay (DD71EF1C6583F5600A893F2301D2E516223854C6DF136D72D07C68377CFC1764). All12 textures saved, six cube faces roundtrip exact, original shaders/PS constants unchanged, capture completion tested. A first test accidentally loaded adjacent proxy DLL; reran from tests/ with system D3D9 to eliminate this contamination. Initial overconservative texture budget skipped4096maps; corrected and all12 now saved. Build/test sources in tools/render-capture and work/r14-render-capture. No game key injection.

Next action requires user: launch, frame dark scribble and press F10, report CAPTURE SAVED. Asked asynchronously. Then inspect fresh R14Capture folder/log; replay exact current draw INCLUDING real cubemaps and all lighting. Investigate dark mark separately from old collar highlight. Capturing before/after RT and final frame should distinguish material/base pass from later contributions. Do not claim fixed yet. No automatic game launch.

