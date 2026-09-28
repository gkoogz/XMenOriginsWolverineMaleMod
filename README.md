# Wolverine Anatomy Tool 1.9

Version 1.9 combines the recent shared tank/gameplay anatomy, material corrections, upright flared neck and surface-contact effects with a CPU performance update. The approved shape and existing simulation quality are retained.

## Install and rollback

Close Wolverine, extract the complete release archive and run Install.cmd or Upgrade-1.9.cmd. The installer accepts the original or exact approved Natural, WGame and WStart packages, checks payload hashes, reconstructs the packages locally and backs up replaced files under WGame/ModBackups/WolverineAnatomyTool-v1.9.0. Unsupported package edits are rejected before installation. Rollback-1.9.cmd or Uninstall.cmd restores the pre-install files.

Existing WolverineLive.ini, TeachingFluid.ini and local TeachingAudio folders are preserved. Camera poses live in Binaries/TankCameraPresets.txt. Existing camera settings are retained; otherwise legacy pose lines are migrated from WolverineLive.log, or the supplied defaults are used. Camera settings remain after rollback. The legacy log is not modified. New diagnostics go to the bounded per-session WolverineRuntime.log.

F6 opens the controls; F8 resets controls while the panel is open. J starts or cancels the teaching sequence. Menu camera playback and attract-video options are in the panel. Fluid configuration reloads between demonstrations from TeachingFluid.ini.

## Performance changes

Independent skin calculations now use a bounded worker pool, with full-precision SIMD bindings and cached invariant calculations. In the fixed-work local replay, surface CPU time fell from approximately 29–35 ms to about 19.5–26.0 ms. Saved geometry and motion samples match the preceding installed build byte-for-byte. These are offline CPU measurements, not game FPS. See [the 1.9 investigation](docs/PERFORMANCE-1.9.md).

The two skin textures now include lossless DDS files with prebuilt mipmaps. In a local D3D9 fixture, PNG decode plus mip generation took about 5.9 seconds combined; equivalent DDS loading took about 83 ms. All pixels across 13 mip levels matched exactly. This is texture-load timing, not total game loading time. The caches add approximately 171 MiB on disk; texture resolution and GPU memory are unchanged.

HUD submissions are batched into one draw, loading frames skip absent-character work, duplicate overlay updates are guarded, and automatic screenshot readback is removed. Inactive fluid capture is skipped, shader layouts are indexed, and immutable collision topology is retained while animated vertices and bounds still update. Camera overrides reject UI/orthographic and stale-scene draws. The attract-video blocker is restricted to the known attract movie instead of intercepting loading movies.

See [the earlier loading review](docs/PERFORMANCE-REVIEW.md) and [1.9 validation](docs/VALIDATION-1.9.md). Overall game FPS and the reported loading rectangle still require live confirmation; the release does not claim universal scene coverage. The anatomy solver and surface refinement remain the main measured CPU costs and keep their existing detail/iteration counts.

## Scope and source

WBX deltas are installer inputs; they are not applied during gameplay. Runtime source and regression tools are included. Build from src/runtime using build.cmd with x86 MSVC and the June 2010 DirectX SDK. Build regression tools from tools/teaching, tools/harmonization, tools/neck or tools/performance using their build.cmd files after building the runtime resources. Copy payload/SharedBody-*.dds beside the harmonization test executable before running it.

This is an illustrative speculative-biology visualization without clinical calibration. Optional local audio and user recordings are not included. The 22 established public idle clips are unchanged. No full game packages, raw diagnostic logs, private study audio or older release archives are included in the downloadable bundle.
