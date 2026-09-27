# Wolverine Anatomy Tool 1.6

Release 1.6 keeps the v1.5 character package, clothing, textures, saved controls, and 22 existing idle clips. It adds independently randomized volume and feed duration to each of the four main pulses, smooth overlap when a pulse runs into its successor, and varied per-pulse angle intensity. No pec morph or sculpt is included.

## Install and rollback

Close Wolverine, extract the full archive, then run `Install.cmd` for a clean install or `Upgrade-1.6.cmd` for the existing supported v1.4/v1.5 setup. The installer checks supported game packages and payload hashes, preserves saved settings and any existing `TeachingFluid.ini`, and writes a rollback backup under `WGame/ModBackups/WolverineAnatomyTool-v1.6.0`. Run `Rollback-1.6.cmd` to restore the pre-install files. Run `Uninstall.cmd` for the same restoration action.

F6 opens the existing anatomy controls; F8 resets them. The period key starts or cancels the 20-second teaching sequence. The simulation settings live in `Binaries/TeachingFluid.ini`, which the installer adds only if one is not already present. The file can be edited between demonstrations.

## Teaching sequence

The first phase shows a short clear, attached strand that sags before release. Each demonstration samples a new, repeatable-within-that-run set of per-pulse volumes, durations, and angle strengths. Long feeds merge with the next pulse and taper across the handoff without a gap. The main pulses use a continuous indexed surface with white material. Cross-section changes, thinning, bending, and occasional breakup produce a stream instead of separate bead particles. Camera transforms are applied at render time so camera motion does not deform stored simulation positions.

The two vocalization cues select one random local WAV per phase: Phase 1 at 2.5 seconds and Phase 2 at 7 seconds. To use recordings, place mono PCM 16-bit 44.1 kHz WAV files in `Binaries/TeachingAudio/Phase1` and `Binaries/TeachingAudio/Phase2`. The folders are optional and may be empty. Audio supplied for this user's study is not included in the release; neither are any private source recordings. The installer leaves these folders alone.

## Fluid settings

All volumes and distances are illustrative game units; they are not millilitres or calibrated physiology. `Volume` is the base amount per main pulse. `Duration` sets base feed time. `PulseVolumeVariation` and `PulseDurationVariation` set the per-pulse relative variation; a duration above the 1.5-second pulse interval merges with the next feed. `AngleVariation` changes the angle-pulse intensity per throb. Set a variation to 0 for a fixed value. `Viscosity` controls resistance to relative movement, `SurfaceTension` controls illustrative necking, `Breakup` scales stream separation, and `ThreadSpacing` changes material injection density. `MeshSides` selects cross-section detail. `DropVolume`, `DropDuration`, `DropHold`, and `DropLength` configure the initial clear strand. `Lifetime`, `CatchPlane`, and `CatchDepth` control retirement and the optional demonstration plane. Valid ranges are enforced by the parser in `src/runtime/teaching_fluid_render.h`; unsupported or out-of-range values are rejected.

See [fluid implementation and limits](docs/TEACHING-FLUID-1.6.md) for the authoring details. The CPU thread/mesh solver is a reduced-dimensional visual approximation. It is not a complete Navier-Stokes or Discrete Viscous Threads solver and is not medically calibrated. Self-contact, splash sheets, refraction, whole-body/terrain collision, and ground accumulation are not implemented. Large settings can increase CPU and mesh cost. The camera fix has offline movement regression coverage; live gameplay appearance and FPS still require confirmation.

## Build and validation

Build `src/runtime/build.cmd` with x86 MSVC and the June 2010 DirectX SDK. `manifest.json` records the release payload hashes and source/runtime provenance. Run `tools/Test-Release16.ps1` against the supported game package inputs for clean-install and rollback fixture checks. Build `tools/fluid/thread_test.cpp` with `tools/fluid/build_thread.cmd`, then run `thread-test.exe variance` from `tools/fluid` to validate pulse bounds, repeatable seeding, overlap continuity, and angle peaks. The audio, camera-motion, and render-state test sources are in `tools/teaching`; their build commands are included. The release does not contain local patient audio.



