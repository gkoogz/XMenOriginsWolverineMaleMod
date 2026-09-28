# Release 1.9 validation

Runtime SHA256: `25E6538A4227F97A9FC9CDDEA060B60E7068AB0D5271E9C349A40941992ADCBF`.

## Fixed-work performance comparison

Measured on a Ryzen 7 5800X using the same 540-frame input replay. Each row contains 90 updates. “Varying” changes the size control each frame and exercises rest-shape invalidation. Times are mean surface CPU milliseconds per update, not whole-game FPS.

| Physics state | Controls | Previous installed build | 1.9 | Reduction |
|---|---|---:|---:|---:|
| 0 | Fixed | 29.776 | 20.353 | 31.6% |
| 0 | Varying | 35.494 | 25.955 | 26.9% |
| 1 | Fixed | 28.952 | 20.087 | 30.6% |
| 1 | Varying | 35.330 | 25.842 | 26.9% |
| 2 | Fixed | 28.571 | 19.484 | 31.8% |
| 2 | Varying | 34.644 | 25.308 | 26.9% |

Physics at a fixed 1/60-second input fell from 5.815–5.926 ms to 5.453–5.554 ms. Live physics can cost more because it catches up after longer frames. The original live trace showed roughly 64 ms in the mod update. No post-fix whole-game FPS measurement is claimed here.

All 36 saved replay samples match byte-for-byte: 35,391,168 bytes of final packed vertices and shaft-node positions. Snapshot SHA256: `8688dfd6cce4601a56b1ad7733238d5f7a663e6e2e75cbb83e0a0588052fb116`. The previous installed reference was `D0AF67110816F419AA25B034EF92A2C4F7EACB5F2D3A5E766A30214E98B8D4D4`. Full machine-readable results are in `validation/performance-1.9.json`.

## Regression checks

- Runtime and test executables build with x86 MSVC `/O2`, ordinary precise floating point, and the June 2010 DirectX SDK. The fast-math experiment was discarded.
- Nine neck state/size cases and 270 motion frames: finite mesh, 16-bit indices, same 54-edge attachment boundary, valid bone palettes, normalized skinning weights, unchanged retained vertices and deterministic repeat evaluation.
- Eighteen shared-surface cases: tank/gameplay geometry, normals, tangents, UVs and body-field parity; zero animated attachment gap; rigid electrode transport.
- Real D3D material contracts: captured gameplay and tank base/spotlight bindings; all touched textures, constants and shaders restored; unknown passes retained; device reset/recreation.
- Three complete 20-second integrated sequence replays: finite surfaces, firmness transitions, control persistence and cancellation/reset.
- Contact regression: ovoid support, static hold, dissipative sliding, moving surfaces at 120/240/480 Hz and unequal-mass momentum.
- Production surface contacts: deferred crossing recovery, 512 simultaneous deposits without duplicate/lost volume, zero queries for settled samples; curved chest anchors and reordered/rotated/vertical receivers.
- World projection and D3D origin capture: moving actor/camera, calibration, stacked floors, ledges and scene reset; return to tank selects the visible surface.
- Loading/render contracts: retained-scene loading framebuffer unchanged, orthographic/UI camera preserved, scene override restored, unrelated movie streams retained, collision topology reused and invalidated correctly.
- Clean-stock installation and approved-package upgrade fixtures: all payload hashes, local package reconstruction, preservation of existing settings, exact rollback of prior runtime/textures/normal map, removal of newly installed specular map, and camera/fluid settings retained.

## Scope and live status

The approved preview shape is retained. Existing solver step/iteration counts and mesh density are unchanged. The shared maps and native tank-pass material fixes are included; future chest/glute controls and hose endpoint rebinding are not implemented by this release.

The local DLL was installed after confirming the game was closed; settings and shared maps were hash-verified unchanged. A live recheck has been requested. Offline tests establish geometry and material contracts, not final appearance, universal scene coverage, or a guaranteed frame rate. See PERFORMANCE-1.9.md for remaining costs and the distinction between inclusive CPU scopes and complete game frames.
