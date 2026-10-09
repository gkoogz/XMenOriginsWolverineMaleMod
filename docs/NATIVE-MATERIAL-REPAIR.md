# October 8: native garment lighting and tank materials

The 32-ray developer prototype could reuse stale lighting constants, omit the
tank's additive spotlights, and write opacity into UE3's scene-depth alpha.
These were adapter defects, not a reason to reduce the cloth update rate.

`meridian_material.h` now captures a complete current-pass snapshot. Ambient,
directional, spherical-harmonic and point/spot lighting are recognized separately.
The spot response follows the captured native shader: radius/falloff, squared
cone, projected attenuation texture and light color. Missing bindings invalidate
the pass instead of mixing values from different frames. The cloth receives each
native lighting pass; `meridian_adapter.h` skins, wraps and uploads it once per
render frame. Pixel shaders are cached by the four lighting capabilities; the
compiler removes inactive light paths and their optional samplers. The 32
columns, 24 rows and anatomy solver are unchanged. An opt-in flat-color probe
requires both the trace environment variable and MeridianFlat.request; ordinary
prototype play clears trace and cannot activate it.

Tank base passes write the native inverse depth to scene alpha using captured
`MinZ_MaxZRatio.x / clip.w + MinZ_MaxZRatio.y`. Additive passes write RGB only,
preserving depth for the native compositor. Gameplay retains the prior opaque
alpha behavior. Earlier captures showed incomplete-looking coverage, but the
flat-color probe demonstrated that the standing pouch geometry was present.
Those captures did not isolate depth alpha as the cause. Do not generalize the
tank compositor contract to gameplay. All changed shader, texture, sampler,
geometry and render state is restored. Opt-in `MALEMOD_MERIDIAN_LIGHT_TRACE=1`
compares before/after state, records native pass identities and dumps shader
disassembly. Normal prototype launches clear that environment variable.

The isolated D3D9Ex tank also reproduced the misplaced face/hair appearance.
Both R14 color maps and both shared body maps failed to load from the managed
pool, leaving native tank textures bound to the replacement body's different UV
layout. Loading these static maps into the default pool succeeded and restored
the face in native captures. Existing reset handlers release/reload all four.
This is an observed failure in the sealed D3D9Ex environment; it does not prove
every reported human-play texture symptom had that same cause.

One captured tank spotlight measured RGB (4.3904, 11.8488, 17.5028). White cotton
therefore needs controlled highlight headroom. A smooth scalar shoulder preserves
hue and gradients instead of hard clipping: per-pass cotton headroom is 0.22;
tank liquid surface headroom is 0.65. The old liquid shader also wrote device
depth instead of inverse depth in the tank; this is corrected. Gameplay radiance and the engine's scene
bloom settings are unchanged. Deposited-fluid shaders retain their prior response.
These scene-specific HDR choices belong in Wolverine. Base geometry, bindings,
physics and fabric design remain shared; other spokes must map their own native
light/depth/exposure contracts rather than inherit D3D9 registers or tank values.

## Reproduction

Build with `tools/iteration/build_private_runtime.py`, the accepted source
`16affd49da5a122df776969574ca62a984b59425`, and the private 32-ray recipe. Named
overlays and the exact resulting DLL are recorded in the generated provenance.
Use a repository-owned room built from the licensed vanilla installation.
`Open-NativeSandbox.ps1 -TitleOnly -Active` stays in the native tank; omitting
`-TitleOnly` enters the room. The sealed command channel supports capture/pause,
garment switches and the existing J teaching-sequence key in both scenes, without
host keyboard, mouse or focus changes. Commands and captures are private data.

Run `Test-MeridianCandidate.ps1` for the sampled 18-case matrix and review native
front/side/oblique attachment, movement and title captures. State-audit success
does not imply visual acceptance. Full campaign transitions, every LOD/resource
and all control combinations remain untested. Do not promote the normal retail
runtime from these developer samples. Preserve exact prototype rollback/settings.

Private working evidence: Base `build/lighting-20261008/`. The developer update
receipt records final native runs, reviewed image hashes and installed identity.

## Native investigation and limits

The first tank baseline reproduced a blank face, misplaced hair and black cloth.
The initial lighting repair then exposed excessive white glare. Default-pool
skin maps restored the face; captured native spot response plus the smooth HDR
shoulder restored a blue-lit cloth surface. State snapshots found no leaked
shader/texture/geometry/render state in the audited runs.

Do not interpret the early coverage experiments as an isolated depth-alpha bug.
Both global inverse-depth and gameplay-opacity candidates produced incomplete
looking pouch captures after control changes. A flat-color probe and subsequent
lit frame showed a complete standing pouch, but switching the probe required an
unpaused render and advanced the surface. They are not identical-frame proof
that shading alone caused the earlier missing coverage. Cached static shader
variants likewise do not establish that inactive samplers caused that symptom.
Transient coverage and uncertified contact remain unresolved developer limits.

The 17:26 trace sweep completed 18 cases with zero reported draw rejections.
The flat probe was enabled during its final mechanical-2 case, so that capture
cannot validate lit cloth. A separate final sweep runs with trace/probe disabled.
Native front and side anatomy inspection showed a continuous attachment at the
sampled default pose. Diagonal input attempts still yielded side views; the
final run obtained actual oblique views through the owned TurnRight camera
channel. The native tank supplies another scene, but is not
a campaign traversal or full body-resource/LOD acceptance result.

The final tank run triggered the existing J clinical timeline and submitted
254 fluid updates/draws without a reported fluid draw failure. The native title
overlay and glass obscured the stream in reviewed images; this proves execution,
not visual fluid-bloom acceptance. Deposited-fluid shader bytecode was unchanged.
Further clear-view fluid and campaign checks are still required.

## Installed checkpoint

User requested continuation of gameplay lighting flashes, black tank cloth,
misplaced face/hair textures and excessive white-material bloom. The installed
prototype is now 29ebe88c81e10fd95a26f81ec0e1c9950ab1dd1ae0fe84f0a4ce977b73e4a6c7,
only in %LOCALAPPDATA%/MaleMod/MeridianPrototype. The missing Wolverine Jockstrap
Prototype desktop shortcut was recreated against the canonical Play script.
WolverineLive.ini and TeachingFluid.ini were preserved by exact SHA256. Previous
01446089 build and manifests are retained in rollback-performance-20261008-174541;
Restore-PreviousPrototype.ps1 -ValidateOnly passed. Retail remains 4f4900a5.

The Wolverine adapter now captures coherent current-pass ambient/SH/directional/
point/spot lighting, caches pixel shaders by native light capabilities, and skins,
wraps and uploads garment geometry once per render frame across lighting passes.
Tank additive passes preserve scene-depth alpha; tank opaque cloth and fluid use
native inverse depth with smooth HDR highlight shoulders. Default-pool skin maps
fix a reproduced sealed D3D9Ex loading failure that left native textures mapped
onto replacement UVs. All four maps have existing reset/reload handlers. The
32 rays, 24 rows, fixed straps, shared geometry and anatomy cadence are unchanged.

Final diagnostics-disabled native run 20261008-173708-c20829 completed 18 cases
with zero reported draw rejections and exited 0. Front, side, actual camera-orbit
oblique, and jump/stop captures were reviewed. Last cumulative garment sample:
721 attempts, 673 wraps, 48 transported, 43 uncertified; mean Draw 5.3736 ms.
This is CPU submission timing, not game FPS. Separate traces audited 1,201
room and 3,241 tank passes with zero state mismatches. Tank captures show aligned
face maps and blue-lit cloth without the former black/white-glare extremes.
The clinical timeline submitted 254 fluid updates/draws without reported draw
failure, but overlay/glass obscured the stream: fluid-bloom visual acceptance is
still pending. Deposited-fluid shaders retain their previous response.

Full attachment gate remains incomplete. Transient incomplete pouch coverage
after control changes, extreme fit, uncertified contact, campaign traversal,
all body resources/LODs and complete control combinations remain unresolved.
Human play of this exact revision is pending. Neither the flat probe nor shader
variants prove the cause of every missing-coverage capture; do not claim that.
One final shutdown capture was truncated and excluded from reviewed evidence.
All private runs ended; host input/focus were never driven. No commit or push.

Private exact build, native receipt and install receipt: Base
build/lighting-20261008/{probe,native-evidence.json,install-receipt.json}.
Implementation and investigation: Wolverine docs/NATIVE-MATERIAL-REPAIR.md.
These graphics contracts belong in the spoke. Other ports should reuse Base's
fabric/geometry and implement their engine's current light/depth/HDR lifecycle;
never copy Wolverine registers or tank headroom values into shared headers.


The updater now backs up/restores the prior native evidence alongside the DLL
and manifests, and validates its original manifest hash. This update's previous
receipt was recovered byte-for-byte from its matching private Base evidence;
rollback validation passed. A requested capture must finish before closing the
owned run; the truncated shutdown image was excluded from evidence. The desktop
shortcut invokes the canonical Play-MeridianPrototype.ps1, which validates the
runtime contract and clears all private input/lighting-trace flags for user play.

## Residual white/grey follow-up

The user confirmed whole-garment switching. The installed 25dc0842 follow-up
sets all relevant sampler states, handles visible cloth faces and implements
measured native gameplay inverse-depth alpha. This supersedes the gameplay
opacity exception above. The previous title images did not clearly expose the
pouch: tank shader execution/state audits are proven, but tank cloth visual
acceptance remains pending. See NATIVE-CLOTH-FIDELITY.md and the newest HANDOFF.
