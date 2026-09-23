# R36 - broader hourglass attachment

Broadens the upper scrotal attachment sideways and forward into a rounded hourglass transition. The blend is authored in rest coordinates with smooth endpoint fades and continuous triangle-area limits. Support volumes are fitted before reshaping; the broadened shaft surface is then captured for live transport. The R35 pelvic anchoring and R34 raphe continuation are retained.

At the saved controls, the middle attachment bands are approximately 17-19% wider, while lower lobe width changes by approximately 0.54%. Matched offline before/after renders are in comparison.html; these are CPU mesh previews, not screenshots from the game.

Validation: seven deterministic pose/motion cases, successful build, and passing harness draw, buffer restoration and reset checks. Shaft and lobe solver trajectories match R35 exactly in all seven cases. Pelvic and distal surface positions match R35 exactly. Settled pelvic motion is zero except for 0.00000763 units of floating-point noise in the smallest case. Current saved pose has no detected edited-region intersections or reversed face orientation relative to R35.

Limitations: other stress poses retain surface overlaps and changed contact pairs; some faces reverse orientation relative to baseline in stress configurations. This is not a guarantee of collision freedom across controls, increased physical solver stability, or anatomical validation. In-game appearance and motion still need user review. The game was not launched.

Installed SHA256: 2AB61776898D80F0D57B90606C9E81AF2C7A5D2080985329D8F63F1036279E5C
R35 backup: C:\Games\X-Men Origins Wolverine\WGame\ModBackups\R36-hourglass-attachment-20260922-151701
Settings and character package hashes were preserved. Rollback-R36.ps1 restores the backed-up R35 runtime with the game closed and preserves current settings.
