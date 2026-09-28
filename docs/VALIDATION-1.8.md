# 1.8 release validation

Runtime SHA-256: ED0BCB2297791101F0E31BEDCBBB5315E0476B1878E95301DE7B52ECABC4C58C. This is the exact installed performance candidate; packaging makes no runtime behavior change.

The preceding performance pass compiled production and regression binaries and passed all three full sequence states, world-space and GPU origin capture, menu return, render-state restoration/device reset, idle animation and audio-cue checks. Additional hidden-D3D checks passed loading framebuffer identity, camera matrix guards/restoration, movie whitelist safety and cached collision-topology equivalence/invalidation. Both DDS caches match every texel of every generated source mip level.

Release-specific validation uses Test-Release18.ps1 for original package reconstruction and approved-package upgrade, payload/DDS verification, exact rollback, legacy camera migration and preservation of existing fluid/camera settings. Test results are recorded in the release handoff. Fixtures use placeholder executables and mock only the process query; the production game-running guard remains unchanged.

Performance figures are local CPU/texture-load fixture measurements, not overall FPS or total game-loading guarantees. Live loading rectangle and all-level rendering coverage remain unverified. Mesh fidelity, solver iterations, existing settings and private audio are preserved.
