# 0.7.2 R2 — root ramp

Installed September 18, 2026. F6 identifies `v0.7.2 R2 ROOT RAMP`.

The old shaft-radius projection reached full strength at 1.8% of shaft length, making a raised collar at large sizes. It now eases continuously over 18%. Final rigid-core restoration fades in from 30–40%, matching the end of the pelvic fairing region instead of overwriting it at 4–10%. The Hang slider, moving suspension anchors and weighted neck remain active. Connectivity, UVs and the character package are unchanged.

29 runtime capture cases cover sizes 1, 10, 25, 50, 75, 90 and 100, Hang endpoints, three physics states, gait motion and mixed width/length controls. All emitted finite positions, exact collar welds and no degenerate neck faces. Size 1/25/50/75/90/100 at Hang65 have zero detected nonadjacent neck intersections. Shaft nodes remain identical across Hang settings; proximal surface movement is below 0.0013 model units. This is sampled validation, not proof over every control combination.

Known limits: tight maximum and rigid-tight each retain one detected crossing; the high-angle case has 686, and very short/wide has 5. These poses need further contact tuning. Triangle regularity is not perfect. The screenshots here are Blender renders of actual final runtime coordinates, not live game screenshots. The model is physically clipped from knees to navel for presentation.

`renders/` contains 36 matched before/after images: sizes50/75/100 at Hang65, top/oblique/side views, grey wireframe and original color atlas. The Blender file contains both versions with the same presentation clipping. `installation.json` records verified DLL, package and settings hashes.

The game’s original 0.7.2 DLL and settings are backed up under `WGame/ModBackups/WolverineAnatomyTool-v0.7.2-r2`. Saved settings including Hang65 were preserved byte for byte. The earlier 0.7.2 backup remains intact.

The upgrade ZIP targets an existing v0.7 package. Close the game, extract the ZIP, run Upgrade-0.7.2.cmd. Rollback-0.7.2.cmd restores the preceding DLL without changing settings. Upgrade, repeat-install, rollback, read-only preservation and package/settings preservation were tested in an isolated fixture.

Live verification: the automated launcher did not expose a game window, and the follow-up process/window check found no running game. Installation is hash-verified, but this revision has not been visually verified during live gameplay. Launch through your usual shortcut.
