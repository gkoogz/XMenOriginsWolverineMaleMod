# Contact physics update — September 25, 2026

Installed in `C:/Games/X-Men Origins Wolverine/Binaries/d3d9.dll`. Settings and all 22 audio files were preserved and verified by SHA-256. This is a local development update based on 1.1 Beta 1, not a new public release.

The lobe simulation is substantially calmer in matched replays. **This is not yet a fully resolved all-pose visual solution:** tight non-pulsing poses can expose more upper-attachment and shaft skin folds than the previously installed build. Live gameplay feel and frame rate have not been verified.

## What changed

The existing experimental solver now solves the shaft, supports, suspension and thigh/body contacts together at 240 Hz, with 24 positional iterations and a separate 10-iteration contact-velocity solve. Suspension forces react back onto the shaft. Material shear and torsional elasticity oppose unsupported orbital motion; curvature damping dissipates bending motion without damping every movement indiscriminately.

Static friction holds material contact points within a load-dependent friction cone; kinetic friction handles sliding relative to moving surfaces. The static/kinetic coefficients are 0.48/0.32. Contact normals follow the convex tapered support surfaces. Morph dimensions, rest frames and thigh motion are interpolated over physics steps so periodic size changes do not arrive as frame-sized jumps. A startup safeguard waits for authored rest geometry.

These changes replace the force/contact behavior itself. The existing controls and connected surface mesh remain. The solver uses an analytic attachment Jacobian, with a numerical fallback at the rotation singularity, and an accurate Newton solve for the support function.

## Measured motion

Compared with the exact DLL that was installed at the start of this task:

| Replay | Reduction in lobe motion above 6 Hz |
|---|---:|
| Saved large setting, Gentle periodic motion | 96.9% |
| Tight suspension, slow weight shift, periodic motion off | 82.9% |
| Tight suspension, slow weight shift, Gentle | 95.3% |
| Default-size rest, periodic motion off | 59.9% |

Each replay lasts 20 seconds at 30 FPS. The metric is RMS displacement after a third-order high-pass filter above 6 Hz, over seconds 6–19. It measures high-frequency motion rather than anatomical accuracy. Shaft metrics are mixed: they improve in the large/Gentle cases but increase in the tight non-pulsing and default rest cases. Full measurements are in `motion-verification.json`.

`motion-comparison.gif` shows the actual refined runtime meshes from matched saved-large-setting replays, seconds 4–12, rendered in Blender at 10 FPS. Left: previous installed DLL's source. Right: updated solver. This is an offline diagnostic rendering, not footage captured from the game.

## Verification and remaining limits

Four matched motion replays and nine stress replays completed. Stress cases cover maximum dimensions, strongest periodic motion, strong drive, 15/60 FPS, semi/raised states, minimum dimensions, stopping and restarting movement, and 45-second rest. All nine passed finite/nondegenerate geometry, seam, suspension-bound, core-separation and D3D draw/state-restoration/reset/recreation checks. Maximum recorded suspension ratio was 0.883. Sampled core clearances remained positive. Rest-to-motion recovery passed, so the calmer result is not a permanent sleep/freeze.

Regression checks passed for static hold, dissipative sliding, moving surfaces at 120/240/480 Hz, unequal-mass momentum, attachment derivatives, support-function accuracy, initialization, periodic-motion controls, glans controls and 10,000 surface-limit samples. The final initialization guard reproduced all 600 frames of the saved-large replay byte-for-byte.

The refined skin still has folds and triangle crossings. At the final samples, crossing-pair counts changed from 92 to 51 (large), 395 to 51 (tight with Gentle), 132 to 151 (default rest), and **156 to 755 (tight without periodic motion)**. The latter is better than the unfinished Beta 1 solver's 1,326, but worse than the previously installed runtime. Counts describe intersecting triangle pairs, not distinct visible defects. All four updated samples had zero lower-core/core or core/transition crossings and no skin vertices inside the support surfaces. These are sampled tests, not continuous collision-free guarantees; some refined faces also oppose their coarse parent normals. The collision report records this openly.

The contact solver costs more computation. No gameplay FPS or real-time performance guarantee is established by the replay timings. Educational anatomical accuracy has not been independently validated.

## Rollback

Close the game and run `Rollback.cmd`, or:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Update.ps1 -Rollback
```

The original DLL is backed up at `C:/Games/X-Men Origins Wolverine/WGame/ModBackups/CoherentPhysics-20260925/d3d9.dll`. The updater validates both DLLs and preserves settings/audio. `Install.cmd` reapplies this update after rollback. Both repeated install and repeated rollback were tested in an isolated fixture. These scripts target this existing local installation; they are not standalone mod installers.

Updated DLL SHA-256: `42CAD24ADAF62892B5C7A0D348867D3C13CAD3B969BECF53DC0AF2A6758326D2`

Original DLL SHA-256: `A388C7C92D66136878ADA90333C91F1AE315210144933ED781343D92BEA6B9DF`

## Source and reproducibility

`runtime-source.zip` contains the runtime, required harness headers, regression/replay tools and handoff notes. `changes.patch` contains changes relative to repository commit `1fe283a`. Build with MSVC x86 and the June 2010 DirectX SDK; paths are in the included `.cmd` files. Run builds from their containing directories. Python replay analysis needs NumPy and SciPy. `tools/physics/README.md` gives commands. The source snapshot is suitable for reproducing the runtime and tests, but excludes unrelated game/audio assets and the full mod installer.

No GitHub push or release was made. User-confirmed in-game results remain pending.
