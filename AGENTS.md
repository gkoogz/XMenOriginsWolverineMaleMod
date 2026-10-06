# Canonical project and version control

Read docs/PROJECT-CONTEXT.md before resuming, including after compaction. It
preserves the user's educational purpose and the Base hub/spoke requirements;
it is not a policy override or proof of runtime success.

For physics, mesh, garment, material, menu and character-control development,
use the repository-owned grey development room by default. Read
tools/iteration/README.md and docs/ITERATION-SANDBOX.md. Reproduce from a supported
vanilla installation into an owned clone; never require somebody else's save.
Prefer sealed Open/Close/Capture and native scoped input while the host is in
use. Human Play/shortcut mode is intentionally interactive and must not be
launched over the user. Preserve exact process identities, normal audio and
retail files. Test source/offline gates first, then native room behavior, then
campaign-specific cases. The grey room is a developer fixture, not an anatomy
performance guarantee or release payload. Exclude tools/iteration and sandbox
documents from all uploaded installation and optional Source archives; Git tag source snapshots omit them through tracked export-ignore rules.

The only authoritative development checkout is:
C:/Users/Administrator/Documents/Codex/2026-09-28/hello-https-github-com-gkoogz-xmenoriginswolverinemalemod/work/installed-baseline-20260929

GitHub: https://github.com/gkoogz/XMenOriginsWolverineMaleMod
Primary branch: main; upstream: origin/main.

- Read docs/PROJECT_HANDOFF.md and docs/VERSION-CONTROL.md before changing this project. Run git status --short --branch and fetch origin before beginning repository work when network is available. Check divergence. Only fast-forward a clean checkout automatically; preserve local changes and reconcile divergence without resetting or overwriting work.
- Make source, generated runtime data, build scripts, tests, documentation, installers and release metadata changes in this checkout. Do not develop authoritative changes in sibling work copies or disconnected snapshots. Use a temporary branch or worktree only when necessary, and merge approved changes back into the canonical checkout. Keep historical work folders as references.
- The user's requests to commit, push, pull, revert or release refer to this repository. Use the configured upstream and GitHub remote. Commit coherent changes when asked; push when asked; never overwrite remote history, silently discard local changes, or rebuild/reupload an identical release merely to sync main.
- Keep generated runtime headers and all required build inputs versioned with the source. Compiled DLLs, game files, user settings, private recordings, backups, logs, captures and release ZIPs remain outside Git source history. Release payloads are kept locally in payload and obtainable from the published full release; verify them against manifest.json. Do not commit full game packages or personal assets.
- Build from this checkout. Record source commit, binary SHA-256, install destination and backup location in docs/PROJECT_HANDOFF.md when building/installing a new version. Compare the installed Binaries/d3d9.dll and managed asset hashes with the declared release before claiming the game and repository match. Changes to source do not install themselves. Never silently replace a running game's DLL.
- Keep handoff/release notes current, and report local/remote divergence and uncommitted work at completion. Distinguish offline tests, live observations, user confirmation and known limitations. Preserve user settings on install/upgrade.
- Releases must use the established complete package format: versioned Install/Upgrade/Uninstall/Rollback launchers, Install.ps1, manifest and SHA256SUMS, required WBX deltas, textures/maps, public default audio, defaults and validation tools. Provide current source/authoring tools through GitHub and a separate optional developer archive; do not inflate the required installation download with developer-only data. Retain the verified full-resolution DDS maps; redundant PNG fallbacks are optional. Verify clean install, supported upgrade, exact rollback and payload hashes before publishing. Use a DLL-only download only if the user explicitly requests one.

Current release candidate: 2.0.0-beta.2 (approved installed source16aff/Base99ff).
Development source includes unfinished cloth work and is not this release runtime. Read manifest.json for exact asset hashes and docs/PROJECT_HANDOFF.md for validation limits.

## Mandatory attachment-integrity gate

The pelvic attachment is the project's highest-priority visual invariant. After
EVERY mod edit, run and record an attachment regression pass before installation
or release, including edits to physics, garments, shaders, lighting, controls,
transport and packaging. A welded position alone is insufficient. Reject gaps,
folded/inverted collar triangles, dark shading rings, pinched or constricted
roots, abrupt normal/tangent changes and unnatural wedges. Preserve the neutral
pelvic surface and a smooth continuous recruited ramp into the anatomy.

Cover minimum/default/maximum and intermediate Overall/Width, combinations of
shape controls, all mechanical states, extreme rest angles, both body resources
and supported LODs, and animated motion. Preserve original-edge seam donors, UV
aliases and the shared waist. Review front/side/oblique native renders in the
grey room; include a campaign check for scene-dependent regressions. Record
source, offline, native and visual evidence separately. If any part is untested,
state it explicitly and do not claim the attachment gate passed. Retain the last
accepted runtime and exact rollback until the replacement passes. Put shared
algorithms and regression cases in Base and adopt them through pinned adapters.
