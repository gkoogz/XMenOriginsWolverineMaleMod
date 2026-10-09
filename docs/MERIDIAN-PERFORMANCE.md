> October 8 implementation: shared exact-work optimizations and the requested 32-ray recipe are installed in the developer prototype. See the current PROJECT_HANDOFF and Base docs/PERFORMANCE-IMPLEMENTATION.md. Solver cadence remains full-rate.

> Superseded priorities, October 8: the user rejects 30 Hz solving and asks for
> full-quality optimization. Read Base `docs/PERFORMANCE-ARCHITECTURE.md` first.
> Human play is fallback-heavy; the successful-pose replay below is not the full
> workload. Preserve this section as historical measurements, not the work order.

# October 8 jockstrap performance investigation

The user finds the current developer prototype mostly good, but sluggish with
the jockstrap. Preserve that installed revision while evaluating optimizations.
Runtime SHA256: `9e42b4d47c36b972945c4f991e3708cfee78638d75633f0bf6eb6ff1fe98e5a5`.

## Measurements

The exact-build native grey-room run recorded donor Update mean **4.1958 ms**,
max **8.9634 ms**, and Draw mean **14.7202 ms**, max **56.8901 ms**. Draw includes
skinning, chart/collision work, normals, D3D state and draw submission; it is not
GPU timer data. These are separate cumulative counters, not a paired end-to-end
frame sample or an FPS measurement. The room uses sealed presentation; do not
convert its Present timings into normal human gameplay FPS.

An x86 MSVC /O2 replay of the exact build's post-skinning CPU block used 24
historical private pose buffers spread across the existing capture bank, one
warmup sweep and ten measured sweeps (240 evaluations). This is a CPU diagnostic,
not new native gameplay or visual acceptance. There were no outer failures or
fallback calls in this selected subset; it does not measure costly fallback poses.

| Phase | Mean ms | p95 ms |
| --- | ---: | ---: |
| Whole measured CPU block | 11.771 | 14.017 |
| Full triangle collision clearance | 10.197 | 12.146 |
| Sewn seam fit | .759 | .923 |
| Hidden proxy construction and pole preparation | .315 | .345 |
| Analytical support hull construction | .200 | .223 |
| Trim and normal updates | .207 | .250 |
| Save continuity coordinates | .008 | .011 |

Clearance consumes about 87% of this replay's measured CPU block. The garment
has 3,623 rendered vertices and 7,072 triangles; the cloth grid alone is 80 by
24, with 3,760 collision-tested faces against nine solids. Debug guides are
already not rendered, but proxy samples are still rebuilt for pole preparation
and local support frames. Their measured cost is comparatively small. The
covered anatomy remains part of the physical/attachment calculation even when
its interior is omitted from rendering; removing it indiscriminately is unsafe.

## Prioritized experiments

1. **Separate collision and display resolution in Base.** Evaluate 40 by 16 or
   40 by 24 collision grids, preserving the full sewn outline and fixed straps.
   The 40 by 16 grid has 1,240 interior/pole faces, roughly one third of today's
   collision face count. Interpolate a denser display surface if needed. Reduced
   mesh alone is not a collision guarantee: certify every final display triangle
   or use a conservative envelope that provably contains the reconstructed cloth.
   Preserve UV aliases, binding revision, root/collar exclusion and trim followers.
   Native visual comparison and real timing must decide adoption; speedup is not
   assumed to scale linearly with face count.
2. **Reuse a valid previous chart in Base.** Transport the last accepted solution
   into the current body frame, test its retained separating planes against the
   current hulls, and repair only failures. Today's code caches plane IDs but
   regenerates and seeds the whole chart each frame. Cached certificates must be
   rechecked after every pose change; never skip contact checks based solely on
   elapsed time. Large changes must force the full solve. This should be evaluated
   before moving code to another thread.
3. **Full-rate execution is required.** The earlier proposed 30 Hz full-wrap
   experiment is withdrawn per the user. Use current-pose certificate reuse,
   exact arithmetic reduction and parallel jobs with current-frame completion.
   Lower mesh resolution is likewise deferred pending an equivalence argument;
   it is not the default optimization strategy.
4. **Reduce repeated donor preparation in the adapter.** Deduplicate source
   donors/bone transforms across garment bindings, precompute special control IDs
   and reuse workspace allocations. Today's 4.2 ms Update traverses bindings even
   for cloth interiors which a successful seeded wrap overwrites. Keep a lazy or
   cached authored-surface path so the initial/failure frames remain visible.
   Regression tests must cover donor parity and both body resources.

The shared grid/chart/cadence algorithms belong in Base and should be adopted
through pinned dependencies by Wolverine and later Witcher. Engine palette
sampling, graphics upload and timers belong in each adapter. None of these
experiments was installed during this investigation; the current prototype,
settings and rollback remain unchanged. A speedup must retain shaft recovery,
continuous garment visibility, material response and the attachment gate.

## Reproduce the CPU phase diagnostic

Select an exact private build with `provenance.json`, exported `base/include`,
adapter source and its recipe. Supply private `MeridianRaw-<frame>.bin` captures
matching that recipe (8,346 float3 samples for this build). The tool hash-checks
the selected numerical/recipe inputs and extracts the selected adapter block
with strict splice contracts; it does not silently use current dirty source.

```powershell
python tools/iteration/profile_meridian_cpu.py `
  --build 'D:/Dev/private-meridian-build' `
  --poses 'D:/Dev/private-pose-buffers' `
  --output 'D:/Dev/private-meridian-profile' --limit 24 --repeats 10
```

MSVC x86 is required; override `--vcvars` for another installation. Inputs,
generated C++, executable, profile JSON and pose hashes remain private/ignored.
The installed final runtime no longer routinely dumps raw buffers. This tool
does not launch games, generate new private captures or prove rendering success.

Local evidence: Base `build/performance-20261008/profile/profile.json`, exact
build `build/bugs-20261008/fix5/provenance.json`, and native source evidence in
`build/bugs-20261008/room/runs/20261008-080545-d68104/runtime.log`. Paths identify
this machine's evidence only; fresh agents must generate their own measurements.

## Full-quality arithmetic experiment

`tools/iteration/experiment_meridian_distances.py` consumes the exact build and
profile directory described above. It creates isolated baseline/reuse binaries,
runs three alternating paired trials and compares every emitted position and
render vertex attribute byte. Example:

```powershell
python tools/iteration/experiment_meridian_distances.py `
  --build 'D:/Dev/private-meridian-build' `
  --profile 'D:/Dev/private-meridian-profile' `
  --output 'D:/Dev/private-distance-experiment'
```

The reproduced run passed exact output equality. Clearance averaged 9.4084 ms
baseline vs 8.53283 ms reuse (9.3% less), whole post-skin block 10.9338 vs
10.0640 ms (8.0% less). This is the same successful-pose subset, not an installed
improvement or fallback workload. See Base's architectural audit for priorities.
