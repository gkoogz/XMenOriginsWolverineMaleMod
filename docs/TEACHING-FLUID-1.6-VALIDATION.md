# Teaching Fluid 1.6 validation

- x86 runtime built from `src/runtime/build.cmd`; packaged `payload/d3d9.dll` SHA-256: `0F5AE2A27416B3AEA24D6CAAED7D128E971BE8C06641E323818ADDDD9489C3A7`.
- `tools/fluid/thread-test.exe variance` passed seeded reproducibility, configured bounds, zero-variation behavior, a guaranteed long feed when settings permit it, gap-free overlap flow, and per-pulse angle peak checks.
- The 20-second simulation passed with randomized pulses at 30, 60, and 120 Hz. Each cadence emitted the same total sampled material, `836.624820` game-units³, with volume error below `0.000004`.
- Representative CPU update/mesh timings on this host: default 60 Hz median 0.552 ms and p95 0.700 ms; 600-unit pulse stress median 0.891 ms and p95 1.298 ms; 2000-unit long-feed stress median 1.784 ms and p95 2.319 ms. These are offline harness timings, not game frame time.
- Viscous energy dissipation, momentum preservation, ballistic motion, breakup volume, and 30/120 Hz fixed-step agreement passed.
- `tools/teaching/sequence-test.exe --camera-motion` passed 90 camera orbit/pre-view translation/rotation frames and D3D state/reset checks. The 20-second animation passed in all three tested physical states.
- Existing local audio pools passed format/count checks (13 Phase1, 11 Phase2). User study recordings remain outside the repository and release archive.
- `tools/Test-Release16.ps1` passed clean-install and v1.5-upgrade fixtures, including manifest validation, exact rollback, and custom `TeachingFluid.ini` preservation. Fixtures use reconstructed game data and a placeholder executable, and never launch the game.

Live gameplay appearance and frame rate have not been verified. This remains an educational visual approximation, not a calibrated physiological or full free-surface fluid solver.
