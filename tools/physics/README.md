# Physics regression and replay tools

Use an x86 MSVC installation and the June 2010 DirectX SDK. Adjust local SDK/compiler paths in the build commands if necessary.

From this directory:

```bat
build-regression.cmd
contact-regression.exe
build.cmd
verified-harness.exe --throb-test
verified-harness.exe --glans-controls-test
verified-harness.exe --surface-limit-test
python compare.py candidate
python stress.py
```

Python analysis requires NumPy and SciPy. `compare.py` defaults to `verified-harness.exe`; set HARNESS to compare a separately built baseline. The harness needs Windows Direct3D 9 and creates a hidden rendering window. It writes local test settings/captures beside the executable; it does not use the game's installation folder. Replay outputs contain exact refined geometry and per-frame solver/mesh metrics. Material coordinates are not calibrated medical units. See the update document and delivered reports for measured results and skin-fold limitations.

The production DLL is built separately by running `build.cmd` in `src/runtime`.

`python verify_performance.py OUTPUT_DIRECTORY` builds reference 1.2 and the
candidate, compares every frame's mesh/body/physics hashes in eight scenarios,
and records CPU timings. `--case N --skip-build` reruns one scenario after a
completed build. See `docs/PREPARED-SHAPE-PERFORMANCE.md` for cache ownership and
the distinction between replay CPU timing and gameplay FPS.


The compressed geometry update adds `build-geometry-regression.cmd` and
`geometry-regression.exe` for randomized scalar equivalence of prepared triangle
limits, rotations and four-vertex contact projection. Run
`python bake_geometry_bindings.py --check` to verify the generated refinement
frames and affected-face lists; regenerate after changing source mesh bindings.
See `docs/GEOMETRY-PASSES.md`. Use `--reference 97a1a37` for comparisons against
the preceding full-rate prepared-shape runtime.
