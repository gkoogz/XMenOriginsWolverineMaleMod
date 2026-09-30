# Project handoff — 2.0 Beta 1, 2026-09-29

Release target: `gkoogz/XMenOriginsWolverineMaleMod`, tag `v2.0.0-beta.1`, GitHub prerelease. The source snapshot derives from local release commit `65a1983`; the beta metadata commit does not alter the runtime binary. This tag contains the canonical runtime source snapshot rather than a merge into the older repository default branch.

The currently installed and packaged DLL has SHA-256 `FED0695979D6DA9D0C7B21466FF9E1F36009E496687E89A552AA443737C1B71D`. It uses 35,000 anatomy triangles and 32,000 rounded support triangles. Required pressure-field support points and the original coarse motion cage remain; no 61,378-face runtime anatomy fallback remains.

Observed/reproduced: the reduction initially reversed joined-collar lighting normals; remapping now preserves opposite-to-render winding. All 88 packed collar normal sets agree with their posed geometry. A post-movement floppy fixture had 37 crossing triangle pairs on 21 shaft faces before the larger existing contact envelope and zero afterward. Six support-overturn and 18 perched/launch cases pass on the final build.

Installed for user testing: the final contact build was copied to the game's Binaries directory with a verified matching hash and a backup of the preceding DLL. Settings and other assets were untouched. The last fixes have offline regression coverage; user-confirmed elimination of clipping in every game pose is not established.

Open: the user reported a base discoloration artifact, then said “Not working” after the winding repair without clarifying whether it was still discoloration or a load problem. Subsequent requests addressed physics. Do not describe all live shading issues as resolved. Geometry captures are not full game-material validation, and CPU-stage benchmarks are not whole-game FPS.

Distribution: this beta is a DLL-only update requiring the existing 1.9 assets. It is not a clean-install package. It includes no full game packages, private audio, user settings, backups or raw session logs. Generated source/data and offline remapping inputs are versioned; capture banks and some earlier review fixtures remain local-only.

See `releases/2.0.0-beta.1.md`, `releases/2026-09-29-compact-runtime.md`, `tools/remesh/README.md` and `tools/remesh/WORKLOG.md` for build commands, validation details and limitations. The earlier public handoff history remains available under the 1.9 tag.
