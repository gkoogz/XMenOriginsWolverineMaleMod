# Current-pose garment following

The developer meridian adapter now tries Base's `SurfaceFollower` before the
full walker. Seven measured circular sections and two current physics testicle
frames supply position, orientation and scale. The existing physics solver is
unchanged. This is direct current-frame deformation, without another cloth
simulation, integration step or rate reduction.

Shared cached bindings and bounded contact projection live in Base's
`meridian_follow.hpp`; see Base docs/MERIDIAN-FOLLOW.md. Wolverine retains bone
palettes, actor-space transforms, donor skinning, UI/state/scene invalidation,
lighting, buffers and installation. The current topology is binding revision6,
64 columns, 40 rows, 2561 cloth vertices, 4279 total garment vertices and 8368
triangles. The numerical guides are not rendered. A successful full walk stores
new material bindings; subsequent frames move from those bindings without drift.

UI morphology/state or scene/clothing epochs clear the follower. Ordinary pose
motion does not. Current support-plane certificates and bounded contact refits
decide whether a new full walk is necessary. A certified follower skips repeat
fairing. Distant face/solid pairs are culled with current exact support bounds.
A failed sewn-edge fit returns before the fallback walk because that path keeps
the raw edge fixed. Other rejected walks retain the continuity/repair fallback;
uncertified contact is reported honestly.
Every native lighting pass still reuses one current-frame geometry upload.
Once a cached wrap exists, Update/Draw skip the old interior cloth donors that
neither direct deformation nor rebuilding uses. Private raw capture deliberately
retains those donors for reproducible CPU inputs.

The adapter now skips those interior donors even before a cached wrap exists.
The walker seeds every interior point from the current outline and pole; only
the seam, support controls and trim need posed donors. Raw diagnostic capture
still samples the full recipe. This removes work from frames where a wrap is
not yet available without changing the solve or its contact acceptance rules.

The historical `profile_meridian_cpu.py` splice currently targets an older
adapter block and refuses revision6; do not use it to claim current performance.
The October 9 investigation used exact clean native builds, an owned grey room
and separate private C++ replays. Synthetic replay is not native animation,
visual acceptance or FPS. Discontinuous poses can still require repeated walks.

Build with the existing explicitly hashed private builder and committed recipe. Use the
repository-owned grey room, complete Test-MeridianCandidate, then review native
front/side/oblique attachment and cloth plus walk/jump/stop. Normal and accepted
runtime installation remains unchanged until its complete acceptance gate.
Updating only the unaccepted developer prototype uses Update-MeridianPrototype
with matching reviewed native evidence and its exact rollback/settings checks.
Never replace its DLL while the human game is open.

This is an adoption of committed shared Base development through the explicit
`dependencies/base.lock.json` pin, not a divergent copied library. The pinned
branch is local pending synchronization. Future ports use a tested pin and measured
frames/bindings; Witcher remains paused. Current build/test/native/install
identities and limitations are recorded in PROJECT_HANDOFF.
