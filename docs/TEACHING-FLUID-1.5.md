# Teaching fluid implementation and limitations (1.5)

This release uses `viscous_thread.h`, `clear_strand.h`, and `thread_mesh.h` to construct continuous stream components from material volume and centerline state. It advances at 120 Hz, diffuses relative velocity, applies gravity and limited shaft collision proxies, and builds an indexed cross-section mesh with smooth normals. Four feed windows share a stream when their timing overlaps. The main streams render opaque white; the preliminary strand renders clear with the existing transparent material path. Necking moves volume and momentum before separating links. These choices are visual approximations; this is not a full 3D free-surface solver or calibrated rheology.

## Camera-space handling

The stored simulation and collision proxies stay in character component coordinates. Current LocalToWorld and ViewProjection matrices are combined only when drawing. Camera-relative pre-view translation therefore cannot stretch the fluid state. Root travel still follows the character component approximation; terrain collision and detached world-space transport are not implemented.

## Sequence and optional audio

The sequence has four initial contractions at 1.5, 2.5, 3.5, and 4.5 seconds, followed by four main flow pulses beginning at 7, 8.5, 10, and 11.5 seconds. An optional random PCM WAV from `Binaries/TeachingAudio/Phase1` cues at 2.5 seconds; another random file from `Phase2` cues at 7 seconds. Files must be mono PCM 16-bit 44.1 kHz. Empty pools stay silent. Playback is local-only; none of the study recordings is bundled in this release.

## Configuration

`payload/TeachingFluid.ini` is the starter configuration. The installer copies it into `Binaries` only when no file already exists. Main-flow volume and feed duration are per-pulse game units. Viscosity and surface tension are illustrative coefficients, not Pa·s or N/m. `FlowVariation`, `NozzleRadius`, `ThreadSpacing`, `MeshSides`, and `Breakup` control stream delivery and mesh resolution. `DropVolume`, `DropDuration`, `DropHold`, and `DropLength` are separate controls for the initial strand. `Lifetime` and optional catch plane/depth retire material. The parser enforces bounds and rejects excessive peak flow.

## Performance and known limits

The simulation avoids particle-neighbor grids and GPU readback. Mesh construction and D3D9 submission remain CPU work; high volume, fine spacing, many cross-section sides, and long durations increase cost. Offline checks from the matching source cover timeline, volume, finite geometry, cadence, cancellation, drawing state, camera transforms, and reset paths. Those checks do not establish in-game frame rate.

Streams can intersect themselves when folded; they do not generally merge or make splash sheets/pools. Whole-body and terrain contact, refraction, calibrated rheology, and complete world-space transport are absent. The visualization is for teaching and is not a diagnostic or validated physiology model.
