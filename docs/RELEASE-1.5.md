# Release 1.5.0

Replaces the bead-like teaching tracer renderer with a CPU viscous-thread approximation and continuous indexed D3D9 surface; keeps the initial clear strand and white main-flow material. Camera-relative pre-view translation is excluded from persistent simulation positions, fixing the reported white smear during camera turns. Adds optional random local WAV pools at the first-phase cue (2.5 s) and main-flow cue (7 s). Existing saved `Throb=0`, shape/physics settings, character meshes, skin assets, and idle audio remain preserved.

Redistributable contents exclude all user-provided study recordings. The installer installs the default `TeachingFluid.ini` only when no user file exists and does not change local `TeachingAudio` folders. No chest/pec morphs are present.

Validation: source builds from `src/runtime/build.cmd`; installer fixture covers payload verification, installation, rollback, and preservation of pre-existing settings. The implementation and limitations are documented in `docs/TEACHING-FLUID-1.5.md`; test evidence is in `docs/TEACHING-FLUID-1.5-VALIDATION.md`. Live gameplay/FPS confirmation is not claimed.

