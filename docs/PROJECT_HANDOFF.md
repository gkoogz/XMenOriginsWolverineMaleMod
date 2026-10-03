# Project handoff — 2.0 Beta 1, 2026-09-29

Read [PROJECT-CONTEXT.md](PROJECT-CONTEXT.md) for the durable educational purpose
and the Base hub/spoke architecture before resuming from this runtime checkpoint.

Release target: `gkoogz/XMenOriginsWolverineMaleMod`, tag `v2.0.0-beta.1`, GitHub prerelease. The source snapshot derives from local release commit `65a1983`; the beta metadata commit does not alter the runtime binary. This tag contains the canonical runtime source snapshot rather than a merge into the older repository default branch.

The currently installed and packaged DLL has SHA-256 `FED0695979D6DA9D0C7B21466FF9E1F36009E496687E89A552AA443737C1B71D`. It uses 35,000 anatomy triangles and 32,000 rounded support triangles. Required pressure-field support points and the original coarse motion cage remain; no 61,378-face runtime anatomy fallback remains.

Observed/reproduced: the reduction initially reversed joined-collar lighting normals; remapping now preserves opposite-to-render winding. All 88 packed collar normal sets agree with their posed geometry. A post-movement floppy fixture had 37 crossing triangle pairs on 21 shaft faces before the larger existing contact envelope and zero afterward. Six support-overturn and 18 perched/launch cases pass on the final build.

Installed for user testing: the final contact build was copied to the game's Binaries directory with a verified matching hash and a backup of the preceding DLL. Settings and other assets were untouched. The last fixes have offline regression coverage; user-confirmed elimination of clipping in every game pose is not established.

Open: the user reported a base discoloration artifact, then said “Not working” after the winding repair without clarifying whether it was still discoloration or a load problem. Subsequent requests addressed physics. Do not describe all live shading issues as resolved. Geometry captures are not full game-material validation, and CPU-stage benchmarks are not whole-game FPS.

Distribution: corrected to the complete established release format. It includes verified public 1.9 WBX game patches, unchanged body textures/maps and 22 public default idle clips, plus the current DLL, versioned installer/upgrade/rollback wrappers, manifest, current source and regression tools. It supports supported stock packages and exact approved patched packages without requiring a 1.9 download. It includes no full game packages, private audio, user settings, backups or raw session logs. Generated source/data and offline remapping inputs are versioned; capture banks and some earlier review fixtures remain local-only.

See `releases/2.0.0-beta.1.md`, `releases/2026-09-29-compact-runtime.md`, `tools/remesh/README.md` and `tools/remesh/WORKLOG.md` for build commands, validation details and limitations. The earlier public handoff history remains available under the 1.9 tag.

## Canonical repository synchronization, 2026-09-29

The user's request establishes this checkout as the sole authoritative project: C:/Users/Administrator/Documents/Codex/2026-09-28/hello-https-github-com-gkoogz-xmenoriginswolverinemalemod/work/installed-baseline-20260929. main tracks origin/main. Read AGENTS.md and docs/VERSION-CONTROL.md; the desktop shortcut opens this checkout. Historical sibling work folders are references only.

Runtime source release commit: 65a1983; complete published beta source/packaging commit: f5bdef8. Repository synchronization/instruction commits do not alter the runtime. Built and installed DLL SHA-256 remains FED0695979D6DA9D0C7B21466FF9E1F36009E496687E89A552AA443737C1B71D at C:/Games/X-Men Origins Wolverine/Binaries/d3d9.dll. This sync verified that DLL, all eight installed PNG/DDS textures/maps, and the three installed game-package hashes against manifest.json. Existing fluid/shape/audio preferences remain user-managed and outside Git. No new build, game installation, backup or release upload is performed for this sync; previously installed runtime and rollback backups remain in place.

The previous GitHub main tree was older than 2.0 and had unrelated history. Its ancestry is retained through a merge that keeps the current project tree, without force-pushing main or reintroducing stale runtime files and archived ZIPs. Previous branches/tags remain intact. The existing 2.0 prerelease and assets are not duplicated or retagged.

Local ignored payload/ now contains all 36 manifest-verified released files. A fresh clone can run tools/Get-ReleasePayload.ps1, optionally with -ArchivePath pointing to the existing complete ZIP. This avoids putting duplicate compiled binaries/textures/audio/release archives into new Git commits while retaining complete installer support.

## Compact installation packaging

At the user's request, the install download excludes developer source/authoring fixtures (provided in a separate optional Source ZIP and GitHub) and the redundant natural/erect PNG fallbacks. Full-resolution DDS skin maps and all precomputed mipmaps remain byte-identical, as do the DLL, patches, menu PNG blends, normal/specular maps and public audio. No runtime source, mesh, physics, texture pixels or live installed files were changed. Existing fallback PNGs are preserved on upgrade. Complete clean install, supported upgrade, exact rollback, settings preservation and unsupported-input rejection fixtures passed with the smaller 34-file payload. The install archive uses lossless 7z compression and requires extraction with 7-Zip before running the existing launcher. Source remains in the canonical checkout; the published runtime beta tag is not rewritten for this packaging revision.

## Shared garments, October 3, 2026

See GARMENT-ADOPTION.md. Source `7005449a19ccd7095786196abf51e98b1a462dbb` adopts clean Base `c49eea156ab3b6e989f22c52a128123850470dae` and its shape-preserving capsule/strain/sewing repair. Garment output is dynamic (4,068 vertices/7,764 triangles); CPU work remains asynchronous. Shared 15 CTest gates, 96 source states and 41 measured Geralt binding states passed. This adapter's strict production x86 build, immutable worker, actual native donor pose, strict resolver and hidden offscreen production HAL D3D9 draw passed; the latter verifies four materials, final vertex coverage and full binding/constants/state restoration. Installed C:/Games/X-Men Origins Wolverine/Binaries/d3d9.dll is SHA-256 `653D2362822C8F95DB52AB4FF8ACE4554AE325DDA52A71174FCB8306D81D415D` (30,655,488 bytes). The game was closed and only the DLL was replaced; all 48 settings/audio files were hash-checked unchanged. Prior A421 DLL is preserved exactly in `build/installed-backup-contact-20261003-164051/d3d9.dll` with a private local receipt and rollback path. Earlier FED backup remains in `build/installed-backup-20261003-135827/d3d9.dll`. No live Wolverine garment appearance or gameplay performance is claimed. The published beta remains unchanged; unrelated voice documentation/private audio are preserved.
