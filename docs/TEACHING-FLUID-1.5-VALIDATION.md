# Teaching Fluid 1.5 validation

- Runtime built from `src/runtime/build.cmd`; packaged `payload/d3d9.dll` SHA-256: `8A34EE5AA76735CAEBC459A69A7BBD8001EA6A1C446D410C61888F0278B3D4ED`.
- `tools/Test-Release15.ps1` passed clean-install and upgrade fixtures. It verifies the manifest hashes, exact rollback, preservation of a pre-existing `TeachingFluid.ini`, and installation of the sample config when absent. The fixtures use reconstructed game files and a placeholder executable; they do not launch the game.
- `tools/fluid/thread-test.exe`: watertight topology, two material components, 720.7 game-units³ conserved, 60 Hz simulation. Offline CPU update + mesh path: 0.797 ms median, 1.100 ms p95 on this host (not game frame time).
- `tools/fluid/FluidCheck.exe`: viscous energy dissipation, ballistic translation, necking/breakup volume preservation, 30/120 Hz fixed-step agreement, CPU stream/material, indexed D3D9 mesh draw and device reset passed.
- `tools/teaching/sequence-test.exe --camera-motion`: 90 camera orbit, pre-view translation and rotation frames matched a camera-independent simulation reference; shader, transformed emitter, D3D draw, state restoration, duplicate-pass suppression and device recreation passed.
- Audio pool checks passed against the local 13-clip Phase1 and 11-clip Phase2 WAV folders. These patient recordings are not included in the repository release; empty local pools remain silent.

Live gameplay appearance and frame rate have not been verified. The stream is an educational visual approximation, not a calibrated physiological or full free-surface fluid solver.
