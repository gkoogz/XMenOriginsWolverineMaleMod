# Installed fast baseline — 2026-09-29

This is the canonical source snapshot for the DLL installed at `C:\Games\X-Men Origins Wolverine\Binaries\d3d9.dll` after the conservative physics contact precheck. The installed DLL SHA-256 is `5B43DC725ACFF13788684D6A1361206053E359489A0AE23A0FB519CF6E64628F`.

Build from this directory with `build.cmd`. It enables `NO_CLINICAL_COLLAR` and `NO_CLINICAL_NECK`; the older sculpt and neck filter are deliberately absent from the live pipeline. It retains the newer unified collar, the full physics constraint count and 240 Hz step, and the complete render mesh. A clean build succeeds on the configured Windows toolchain. The PE file may differ byte-for-byte from the installed DLL because the linker embeds build metadata and the output filename differs from the earlier build.

The source includes generated geometry/data headers and the resource binary. The original asset-authoring scripts are spread across earlier work directories and have not yet been consolidated here. This snapshot reproduces the runtime build, but does not yet regenerate every authored asset from source meshes.

`tools/harmonization/audit-gameplay-base.bin` is the captured reference buffer for offline tests. The `tools/neck` tests compile with the existing DirectX SDK and Visual C++ command files. `build_unified_fast.cmd` exercises 48 static controls and motion/pulse cases; `build_j_sequence_fast.cmd` exercises the J timeline and simulated hitches. `physics_step_profile.cpp` is the diagnostic for fixed-step CPU cost. Generated executables and object files are ignored by Git.

Performance measurements and optimization priorities are in the audit report next to this project’s `outputs` directory. CPU times are not GPU or whole-game frame times.
