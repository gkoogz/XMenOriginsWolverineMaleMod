# Prepared geometry and compressed live passes

This update starts from the full-rate prepared-shape build (`97a1a37`, runtime
equivalent to `2c1deee`). It preserves the authored mesh, effective controls,
240 Hz coupled solver, every-frame surface updates and contact thresholds.
It does not incorporate the rejected later collar geometry or surface cadence.

## What is baked or prepared

- `geometry_binding_data.h` is a build-time asset. The 16,196 refined vertices
  share 2,228 unique parent triangle frames. Its coarse correction face closure
  contains 2,479 of 21,086 faces, and its fine closure contains 34,620 of 53,444.
  These closures include all faces incident to potentially displaced vertices,
  including seam and axial support corrections. Faces outside them have zero
  correction. Source face order is preserved.
- Immutable attachment profiles, lobe ownership masks, neighbor rows, normal
  incidence, body index mappings, UV edge differences and smoothing strengths
  are prepared once. The original profile functions are not evaluated at every
  triangle corner on every frame.
- Anatomical lobe offsets, axial material positions and flex coordinates are
  cached by the prepared rest-shape revision. Changing dimensions, Hang, mode,
  or resetting the rest frame rebuilds the dependent data. Solver interpolation
  does not become the cache key. Short-frame previous-length dependencies remain.
- R14 growth positions and interpolation coefficients are prepared when the
  effective glans growth changes. Live deformation only accumulates cage deltas.

Regenerate the topology asset after changing any binding/mesh input:

```bat
python tools/physics/bake_geometry_bindings.py
python tools/physics/bake_geometry_bindings.py --check
```

## What is compressed

- One topology-bound XYZ SIMD operator replaces scalar collar, raphe and
  lobe-transition fairing loops. It traverses writable rows, retaining boundary
  values, pass counts, neighbor order and full-precision division.
- Shared pose rotations are prepared once per lobe instead of repeatedly
  rebuilding anchors and normalizing identical axes per vertex.
- The oblique correction scans its incident faces, reuses triangle area
  coefficients between iterations, and exits once a complete sweep changes
  nothing. The web pass computes only normals needed by writable rows.
- Neck fairing computes vertex normals only in the tangential phase and face
  normals only in the curvature phase. Immutable group mappings/counts are
  prepared once. Only active rows are copied between iterations.
- Pouch contact projection processes four independent vertices together with
  the original five projections, exact divisions and square roots. Harmonic
  transfer shares XYZ arithmetic, builds only templates in its dependency
  closure, and removes unused full-mesh snapshots.
- Refinement evaluates each unique triangle frame once and applies its baked
  per-vertex coefficients. Safety passes traverse the baked affected faces.

## What remains live, and why

The final pouch still responds to changing support orientation, pressure,
separation, and contact. Its field evaluation, interleaved fairing/projection,
and the attachment's pose-dependent corrections remain live. They cannot be
replaced by one static wireframe while keeping the existing response exactly.
The 60 pouch fairing passes retain their interleaved outside-core projections.

Dimension pulses can still rebuild the prepared rest shape: multiple nonlinear
authoring stages couple the shaft and collar. Those rebuilds now use prepared
operators, and repeated downstream rest/material fitting is cached. There is no
quantization, held animation frame, lower physics rate, or lower mesh resolution.

Intermediate lobe geometry was not blindly discarded merely because the final
pouch overwrites some positions; it still feeds harmonic boundaries and safety
fractions. This change reduces those passes' work while preserving dependencies.
Existing skin folds are consequently preserved, not repaired by this update.

## Verification

`tools/physics/verify_performance.py OUTPUT --reference 97a1a37` compares complete
packed mesh, body buffer and solver-state hashes on every frame across eight
240-frame scenarios. `build-geometry-regression.cmd` builds randomized scalar
equivalence tests for prepared area limits, rotations and batched skin projection.
The existing contact, controls, pulse, surface-limit and D3D replay checks apply.
See the delivered report for results from the actual final build. Replay CPU
timings exclude the game/GPU/HUD and do not establish gameplay FPS.
