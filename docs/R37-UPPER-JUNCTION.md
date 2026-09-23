# R37 upper junction

User clarified the marked area: a huge upper scrotal connection blending upward into the shaft, rather than enlarging the pouch below it. Abandoned previous experiments; restored and verified R36 before this revision.

This revision fills the lateral upper undercut, broadens the shared cuff and fairs its transition into the shaft. It uses a fixed reference-space mask on the R14 render mesh with pelvic boundary locks, medial raphe exclusion for fairing, and updated geometric normals. The R36 cage and earlier R35 solver remain intact. Generate-UpperJunction.py reproduces the support data. Runtime geometry runs 60 concavity-filling iterations and 40 finishing iterations across 946 active vertices; lateral width is expanded with smooth material-space fades.

At the saved settings, the upper two measured bands are 1.89 and 1.91 times R36 width. This is not a measurement of doubled total surface area. Seven pose/motion cases are finite; cage/pelvis positions, shaft/lobe solver trajectories and all 20 measured render-body weld vertices match R36 exactly. Harness draw, buffer restoration, device reset and recreation checks passed in all seven cases. The game was not launched.

Saved pose intersection audit: 10 existing pairs before and after, no new pairs. Other tests have changed contacts and some increases: maximum erect 101 to 137, maximum floppy 52 to 69, maximum semi 30 to 26, long hang 183 to 167, neutral 133 to 145, smallest 1616 to 1418. Counts cover the union of edited regions and are not a collision-free guarantee. Gameplay appearance and performance still require review. The visible compressed crease is softened but not completely erased in the floppy preview.

Installed SHA256: 57AFF334ACF5424BF56FF91A33E85AD5E91ECD02C60D35CD3A9A379C816E88B3
R36 backup: C:\Games\X-Men Origins Wolverine\WGame\ModBackups\R37-upper-junction-20260922-162808
Settings and character package are byte-preserved. Rollback-R37.ps1 restores R36 with the game closed and preserves current settings.
