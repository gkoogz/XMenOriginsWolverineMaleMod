# Runtime performance investigation for 1.9

The user reported a severe frame-rate regression after the recent anatomy changes. The captured live log from the preceding installed build recorded about 64 ms for the mod overlay/update, including 27 ms of physics and 36 ms of surface evaluation. These are inclusive CPU timings, not GPU times; nested scopes must not be added.

A fixed-work offline replay reproduced the expensive surface pipeline. The structural skin took approximately 6 ms, the new neck render binding about 3 ms, and the older pouch surface approximately 6 ms. These are distinct stages; the pouch and structural skin are included in the broader rounded-surface timer. Longer live frames also trigger more 240 Hz physics catch-up steps, increasing the frame cost further.

## Changes

- Run independent per-vertex skin, projection and binding calculations in synchronous batches on a scheduler limited to four concurrent contexts. Sixteen batches distribute the edited region among workers. All batches finish before dependent operations, collision publication or rendering. Constraints, shared reductions and D3D operations remain sequential.
- Retain per-vertex arithmetic order and copy the caller's SSE control state to workers, restoring each worker's state afterward.
- Evaluate XYZ surface bindings and four nearest curve segments together with full-precision SSE arithmetic.
- Inline small vector operations, use hardware square root and packed vector division, and remove redundant floor calls when packing clamped nonnegative normal components.
- Decode each possible packed normal byte once. Prepare invariant bending coefficients once per physics substep and reuse rotation trigonometry across each basis.
- Add separate live timers for pouch, structural skin and neck rendering.

The approved upright neck, mesh density, UVs, shader/material mapping, collision surface, 240 Hz physics step, 24 constraint iterations and 60 pouch fairing passes are retained. No approximate reciprocals, global fast-math setting, simulation throttling or reduced geometry quality is used.

## Reproduction and limits

Build `src/runtime/build.cmd`, then `tools/neck/build_performance.cmd`. Run the performance executable from its own directory. It evaluates three physics states with fixed controls and continuously varying size, 90 frames each, at a fixed 1/60-second input step. It separately records physics and surface CPU times and can save packed geometry and motion samples every 15 frames.

The original reference is the installed D0AF67110816F419AA25B034EF92A2C4F7EACB5F2D3A5E766A30214E98B8D4D4 build's source. The reference and optimized replay samples are compared byte-for-byte, including positions, normals, tangents, UVs, bone data and shaft motion. Measurements are made on the local Ryzen 7 5800X; first-use scheduler costs and total game/GPU frame time are separate. The benchmark results are summarized in VALIDATION-1.9.md.

This reduces CPU cost but is not a guarantee of 60 FPS. Dynamic rest-shape authoring, smoothing, physics catch-up, skinned collision refits and engine-owned vertex-buffer synchronization remain costs. Live frame-rate verification is reported separately from the offline tests.
