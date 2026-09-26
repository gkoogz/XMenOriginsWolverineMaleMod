# Prepared shape and surface execution

This development branch starts from the restored 1.2 source, commit `21275c3`.
It separates reusable shape preparation from live deformation. The later,
rejected collar candidate is not included.

## Work removed from ordinary frames

`prepared_shape.h` owns the rest-stage pipeline previously embedded in
`ApplyShape`: morph sampling, pelvic recruitment and fairing, rest centerline
fitting, radial/ridge construction, support fitting, neck preparation and
material bindings. Its cache key contains all seven **effective** shape values,
Hang, physical state and graft offset. Pulse-driven dimensions invalidate it in
exactly the same way as sliders. The glans and physics controls remain evaluated
downstream; they do not require rebuilding the rest cage.

Unchanged dimensions reuse the prepared cage, body positions, rest centerline,
support centers/radii and length. Only positions owned by the rest stage are
copied into the body buffer. Live transport and collision still follow the
current solver state. Rest-frame invalidation on device reset forces preparation
again. Degenerate centerline fallbacks retain their dependence on previous rest
length rather than incorrectly treating it as constant.

Other fixed material data is prepared once: centerline fitting weights, packed
UV/bone attributes, tangent UV differences and smoothing-row constants. These
arrays belong to the compiled mesh and survive GPU resource recreation.

## Work accelerated while remaining live

The pouch field evaluates four independent material rays together using SSE.
It retains all 18 bisection steps, full-precision square roots/division and the
original operation order within each ray. The 60-pass skin fairing evaluates XYZ
together, preserving neighbor order, Jacobi snapshots, contact projection every
five passes, and the final outside-support projection. No mesh simplification,
physics iteration reduction, pulse quantization, lower update rate or relaxed
surface safety threshold is introduced.

The coupled contact solver, suspension, static/sliding friction, authored mesh
topology, shape/control ranges, sound, material assets and all live shape safety
corrections retain their 1.2 behavior. This also preserves its existing upper
attachment folds; this performance change does not repair those visual issues.

## Verification

Run `tools/physics/verify_performance.py OUTPUT_DIRECTORY`. It builds the original
1.2 and current runtime with x86 MSVC `/O2`, then runs them sequentially. Every
frame compares hashes of the complete packed render mesh, original body vertex
buffer, and current/previous simulation particle positions. Eight scenarios
cover small/maximum dimensions, all physical states, all active pulse levels,
moving contacts, individual control changes, control reset, buffer recreation
and 15 FPS input. Geometry finiteness, sampled triangle areas and body welds are
also checked. Timing covers control mapping, solver and geometry only; hashing,
validation and output are outside the timer. Startup is excluded from medians.

This is a CPU replay measurement, not gameplay FPS or a full GPU/frame profile.
Changing dimensions still executes rest preparation. The nonlinear live skin
passes remain a significant cost; replacing them with baked linear operators
would require a separate geometry/contact validation effort.
