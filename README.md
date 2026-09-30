# Wolverine Anatomy Tool 2.0 Beta 1

Prerelease tag: `v2.0.0-beta.1`. See [beta release notes](releases/2.0.0-beta.1.md) and [project handoff](docs/PROJECT_HANDOFF.md). The preceding local snapshot is tagged `release-2026-09-29-compact-runtime`.

The installed anatomy render skin and fluid collider share **35,000 triangles / 17,528 vertices**, reduced from 61,378 triangles. The rounded supporting surface has 32,000 triangles. Bindings, joined collar seams, safety metadata and fluid references are compacted together. No full-resolution anatomy fallback or runtime decimator remains.

The release includes the prior surface/worker-pool optimizations, corrected collar lighting winding, stronger upright support and a shaft contact envelope covering the rendered ventral bulge. The established 240 Hz physics step and 24 coupled constraint iterations remain. The disabled clinical pipeline and its stale tables are removed.

Build from this directory with `build.cmd` using the configured Visual C++ and DirectX SDK. The source includes generated geometry headers and the resource binary. `tools/remesh` documents the offline reduction inputs and rebaking process; it does not regenerate every original authored asset from source meshes. Linker metadata can make a rebuilt DLL differ byte-for-byte without a source change.

The `tools/neck` tests compile from that directory using their command files. They cover static/moving/pulsing geometry, the J timeline and hitches, collision skinning, fluid budgets, upright support and perched recovery. Offline pose banks in `captures` and some earlier review inputs in `outputs` are local fixtures, not runtime dependencies or distributed game assets.

Detailed reduction and shading audits live in `tools/remesh`; hanging-contact results and actual before/after mesh renders are in the task's `outputs/hanging-contact-review-20260929` folder. CPU-stage measurements are not whole-game FPS. The user's persistent discoloration report remains unconfirmed after the shading repair; see release limitations.
