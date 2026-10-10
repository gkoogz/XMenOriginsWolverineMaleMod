# Current-pose garment following

The developer meridian adapter now tries Base's `SurfaceFollower` before the
full walker. Seven measured circular sections and two current physics testicle
frames supply position, orientation and scale. The existing physics solver is
unchanged. This is direct current-frame deformation, without another cloth
simulation, integration step or rate reduction.

Shared cached bindings and bounded contact projection live in Base's
`meridian_follow.hpp`; see Base docs/MERIDIAN-FOLLOW.md. Wolverine retains bone
palettes, actor-space transforms, donor skinning, UI/state/scene invalidation,
lighting, buffers and installation. The topology remains binding revision5,
32 columns, 24 rows, 769 cloth vertices, 2471 total garment vertices and 4816
triangles. The numerical guides are not rendered. A successful full walk stores
new material bindings; subsequent frames move from those bindings without drift.

UI morphology/state or scene/clothing epochs clear the follower. Ordinary pose
motion does not. Current support-plane certificates and bounded contact refits
decide whether a new full walk is necessary. A rejected full walk retains the
previous continuity/repair fallback and reports uncertified contact honestly.
Every native lighting pass still reuses one current-frame geometry upload.

The current dense revision6 candidate now distinguishes a whole-cloth contact
certificate from a safe movable-interior follow. When prescribed seam faces
prevent a whole-cloth certificate, the adapter may use the current rig's cached
wrap only if every movable face passes current-pose contact projection and the
relative deformation budget. It keeps the full anatomy draw in that case and
reports the frame as `interiorOnly`, not certified. Larger motion or failed
interior checks still rebuild. A followed wrap skips the local four-sweep
fairing; its reference wrap already passed the full taut solve. The fixed seam,
pole, UV aliases and trim still update from the current pose each render frame.
The shared Base contact pass also uses conservative current-pose support bounds
to reject remote triangle/solid pairs before its full test. Native visual and
performance acceptance for this candidate remains separate from source tests.
Once a cached wrap exists, Update/Draw skip the old interior cloth donors that
neither direct deformation nor rebuilding uses. Private raw capture deliberately
retains those donors for reproducible CPU inputs.

`profile_meridian_cpu.py` supports both previous and current named builds,
checks their input hashes, and reports follow/bind/rebuild phases. Optional
`--interpolate 30` synthesizes intermediate physics inputs; receipts label this
explicitly. Synthetic replay is not native animation, visual acceptance or FPS.
Discontinuous historical poses can still require a walk every time and add
cache-check overhead; do not present the synthetic improvement as universal.

Build with the existing explicitly hashed private builder and recipe32. Use the
repository-owned grey room, complete Test-MeridianCandidate, then review native
front/side/oblique attachment and cloth plus walk/jump/stop. Normal and accepted
runtime installation remains unchanged until its complete acceptance gate.
Updating only the unaccepted developer prototype uses Update-MeridianPrototype
with matching reviewed native evidence and its exact rollback/settings checks.
Never replace its DLL while the human game is open.

This is an adoption of shared Base development through a hashed private overlay
on the historical release pin, not a divergent copied library or released Base
pin update. Future ports must use a tested committed pin and their measured
frames/bindings; Witcher remains paused. Current build/test/native/install
identities and limitations are recorded in PROJECT_HANDOFF.
