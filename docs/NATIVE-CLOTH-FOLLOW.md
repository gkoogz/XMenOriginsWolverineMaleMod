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
Once a cached wrap exists, Update/Draw skip the old interior cloth donors that
neither direct deformation nor rebuilding uses. Private raw capture deliberately
retains those donors for reproducible CPU inputs.

The adapter now skips those interior donors even before a cached wrap exists.
The walker seeds every interior point from the current outline and pole; only
the seam, support controls and trim need posed donors. Raw diagnostic capture
still samples the full recipe. This removes work from frames where a wrap is
not yet available without changing the solve or its contact acceptance rules.

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
