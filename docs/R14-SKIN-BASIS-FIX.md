# R14 skin basis correction — installed; gameplay confirmation pending

2026-09-20: User reports persistent bright collar glyph and dark distal scribbles, fixed to surface positions as camera rotates. Current R14 gameplay confirmed by screenshots. User confirmed game closed; verified process absent and installed the tested candidate at 02:23 EDT. Did not launch game.

Observed: old Capture168_20260916_055428 draw_023 replay reproduces the collar glyph. Replacing only the skin-lobe basis calculation removes it with c21.y=15 unchanged. Original shader reconstructs a missing basis row using interpolated normal.w handedness; discontinuities in that calculation are the supported cause of the reproduced mark. Current distal mark remains unconfirmed.

Compiled candidate: outputs/r14-skin-basis-fix/d3d9.dll SHA256 60AF0CD90EA456FCE3012481AC1AD3142DE70679E13F3FD076998052BD739A84. Previous baseline R14 SHA256 43103535473C9215137C173D3478E00BE8F7C4B8105BB4332421020D04204E7F. No package, geometry, texture, UV, physics or saved settings changes.

Implementation: skin_basis_fix.h exact-matches both original shaders, carries three transformed TBN vectors in additional VS outputs, and computes the skin-lobe dot product in world space. Strength/exponent/normal texture unchanged. Only R14 draw uses it; unmatched shader pairs pass through. Restores shaders immediately. Reset releases resources/cache. F9 toggles fix/original, default enabled; F6 menu retained.

Verification: x86 runtime build passed. Actual hook replay checks exact matching, F9-equivalent off bypass, unknown pair bypass, original shader restoration, every VS/PS float constant, all 16 texture bindings, resource release/recreation. All passed. Runtime-hook output PNG hash equals experimental world-space shader PNG. No geometry tests repeated because geometry code/assets unchanged. Replay excludes unavailable captured environment cubemaps equally in both images; not full-scene visual proof.

Artifacts: outputs/r14-skin-basis-fix/{comparison.html,verification.json,candidate.json,Install-Fix.ps1,Rollback-Fix.ps1}. Local repro source tools/skin-basis; needs game capture. Candidate source is in repo src/runtime and isolated work/r14-skin-basis-fix. Historical R166–R178 at 2026-09-11/i-j ruled out many texture/damage/geometry fixes; c21.y=0 worked but flattened shine and was rejected. This fix leaves that strength unchanged.

Installed and hash verified: 60AF0CD90EA456FCE3012481AC1AD3142DE70679E13F3FD076998052BD739A84. Backup: C:\Games\X-Men Origins Wolverine\WGame\ModBackups\R14-skin-basis-20260920-022356. Live settings and package hashes verified unchanged. Settings SHA256 26777CBA8842EC5AB84E79F16F00F0D4CA5E52EAE8E3A333E16E5DFA21F88C9F. See outputs/r14-skin-basis-fix/installation.json. Next: user launches and compares collar/tip with F9. Do not claim gameplay solved until confirmed. Public release payloads untouched.

