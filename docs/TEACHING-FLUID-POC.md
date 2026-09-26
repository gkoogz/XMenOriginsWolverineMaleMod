# Teaching fluid proof of concept on 1.3

This is a local development candidate based on tag `v1.3.0`, commit
`9a9c4f27f5ab2eb4db251c3b516ee4d85ce9cffe`. It adds an illustrative, non-erotic
reproductive-physiology demonstration using a blue tracer. The existing model,
geometry preparation, contact solver, textures, and audio assets remain the base.

## Controls and timeline

Press `.` once while the character is visible to start. Press it again to cancel.
F8 reset, changing a study control, losing focus, a rendering gap over 250 ms,
or a graphics-device reset also cancels. Existing settings are not replaced by
sequence state. The existing ambient throb fades out and back in; the selected
firmness remains stored while the solver temporarily approaches a firm state.

The sequence lasts 20 seconds. Four preparatory contractions peak at 1.5, 2.5,
3.5, and 4.5 seconds. Two small attached tracer samples occur at 2.5 and 4.5.
Four short emissions occur at 7, 8.5, 10, and 11.5 seconds. Each emission lasts
about 0.13 seconds. Recovery occupies the latter part of the timeline. These
timings and dimensions are illustrative design choices, not physiological data.

The local default and user input files have no gameplay `Period` or `Slash`
binding. They do accept these characters in text fields: use the trigger during
the demonstration, not while typing in a console or game menu. Other keyboards
or remapped configurations may differ. To change the trigger, add to the existing
`Binaries/WolverineLive.ini` (do not replace that file):

```ini
[Teaching Sequence]
TriggerKey=191
```

190 is the Windows OEM period key; 191 is OEM slash on a US keyboard. Restart
the game after editing. F6, F8, F9 and Shift are reserved by this candidate.

## Implementation

`teaching_sequence.h` contains the deterministic timeline and bounded 120 Hz
position-based filament simulation. Short connected segments use extension
constraints and relative axial velocity damping to suggest viscosity. An
attachment constraint holds the most recent node at the live emitter until a
time or separation threshold releases it. Detached segments retain momentum,
fall under gravity, fade, and expire. Two sample events and four emission events
are fixed for reproducible classroom comparisons. This is not a Navier-Stokes
solver and the viscosity coefficient is not calibrated in Pa s.

`teaching_fluid_render.h` samples the final model's outer opening (ring 86) and
mid-glans ring to derive the emitter direction, then applies the live pelvis bone
palette. The innermost recess rings and cap cannot provide an outward tangent;
using them initially reversed the emitter and was corrected after visual review.
It reuses reflected LocalToWorld and ViewProjectionMatrix constants, renders
depth-tested tracer tubes in the character scene pass, and restores render state
and the full vertex constant bank. Shader reflection is cached with the existing
shader lifetime. Simulation is bounded to eight filaments of 32 nodes each; only
six are emitted per demonstration. Device reset releases the resources.

`d3d9_proxy.cpp` blends restrained size/angle envelopes into the existing throb
mapping, changes the solver's continuous firmness target, polls the configurable
key only in the foreground process, and displays progress in the F6 panel.

## Current limits

- Fluid is simulated in character component space after pelvis skinning. It
  detaches from the animated tip, but character root travel still transports it.
  Evaluate the proof of concept with a stationary character. Correct world-space
  travel needs a verified world origin independent of UE3 pre-view translation.
- Cleanup uses lifetime and a reference component-space z=0 plane. This is not
  terrain detection. Slopes, platforms, body contact, ground splats and vocals
  are not implemented in this candidate.
- The blue tracer is a diagnostic material. Refraction, physically calibrated
  rheology, volume conservation and medical validation remain future work.
- Full live gameplay, transparency sorting against other scene objects,
  different chapters/shaders, and gameplay FPS have not been verified. Existing
  1.3 surface folds are not addressed by this change.
- The underlying stored physics mode/texture choice remains unchanged during
  the sequence; continuous solver firmness is the temporary state being driven.

## Reproduce validation

Use the repository's x86 MSVC and June 2010 DirectX SDK paths, or adjust the
three build scripts for the local installation. Build `src/runtime/build.cmd`,
`tools/teaching/build.cmd`, and `tools/physics/build.cmd` in their own directories.
Run `sequence-test.exe` and `sequence-test.exe --render` from `tools/teaching`.
The render test uses the local captured game shader named in its source; it
does not launch the game. Run the existing `--throb-test` and
`--glans-controls-test` options of `tools/physics/verified-harness.exe`.

The CPU tests cover 15/30/60/120/144 FPS emission and cleanup, 30 versus 120 FPS
fixed-step agreement, stalls/teleports, and a full surface/solver replay from all
three starting states. The render test checks the real shader's reflected
constants, transformed emitter, draw call, duplicate-pass suppression, pipeline
restoration, and release/recreation. See the delivered logs for actual results.

The installer is tested against a separate fake game directory. That fixture
stubs only the process query because the real game is running; the production
installer retains its process guard. No live game files are changed by testing.
