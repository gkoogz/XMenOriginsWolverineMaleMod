## October 10 - user accepted the sewn-boundary correction

The user confirmed the installed correction is liked and requested a GitHub
push. This checkpoint commits the exact installed runtime source overlays and
Base pin 47023a7, together with the build and focused verification tools.
The Base pin is published on codex/pouch-cage. The runtime SHA and evidence
scope below remain unchanged. This is a source push, not a new release package.
Unrelated experimental working changes remain outside this checkpoint.

## October 10 - sewn boundary and saddle correction installed

Installed production 25ca1204061a0a7c0bcdc7943031d8ac8263a1d12c9b4c588efb64d3339ba3fa
to C:/Games/X-Men Origins Wolverine/Binaries/d3d9.dll; hash verified.
Base pin 47023a7f53d794cff84f2f423ba6a7d1fb98ec0f. Source identity
10ea90218b16ac10c5d7ac93fc272fa2bd327b5746172a2aac6064302192c1d7.
Only shared pouch_cage.hpp changed among 566 compiled inputs.
The fitter now derives the cage boundary from the actual sewn polygon and
propagates exact seam detail, replacing the former immediate elliptical base.
Removed the upper local-radius release responsible for the saddle depression.

Focused source regression passes and fails against the previous oval-base code.
Reviewed 20 native control cases, 22 orbit/size views, three exposed attachment
views, 13 selected motion frames and four campaign captures. No fitting rejects.
Representative room preparation means remained around 2.9-3.1 ms. No new paired
FPS benchmark is claimed. Room run 20261010-150456-a7043c and campaign run
20261010-151021-5a10b3 closed; room prior DLL restored; no host input/focus used.
This is the requested focused development update. Full attachment certification
is false: exhaustive all-LOD/body-resource/motion combinations were not rerun;
prior source-identical body evidence is retained separately. No continuous
collision or campaign reload-cycle claim. Private receipt:
D:/MaleModBuilds/jockstrap-boundary-20261010/install-evidence.json.

Exact rollback to the rejected 6fb31e30 build is retained at
C:/Games/X-Men Origins Wolverine/WGame/ModBackups/
Meridian-development-20261010-151424-022be3/Restore.ps1; the earlier accepted
9919da09 rollback chain remains intact. Installer verified protected files and
rollback. Saves/settings/audio preserved. No push or release; canonical main
remains 68 commits ahead of origin with pre-existing and current working edits.
User acceptance of the replacement appearance remains separate from this review.

## October 10 - oval attachment and saddle correction in progress

The user rejected installed 6fb31e30: its oval shoulder remained and the upper
local-radius release introduced a saddle. Previous visual acceptance below is
superseded for garment shape. The authored seam is already the actual waistband
arc plus side hems. FitPouchCage replaced its adjacent rows with an ellipse.
New Base pin 47023a7f53d794cff84f2f423ba6a7d1fb98ec0f instead seeds each cage ray
from its intersection with the sewn polygon, preserves nonplanar seam detail
continuously, pins cage row zero, and removes the upper local-radius release.
A nonelliptical outline regression fails the old code and passes the correction.
Source identity 10ea90218b16ac10c5d7ac93fc272fa2bd327b5746172a2aac6064302192c1d7:
565/566 compiled inputs unchanged; only shared pouch_cage.hpp differs.
Native review and installation remain pending. Private build/evidence directory:
D:/MaleModBuilds/jockstrap-boundary-20261010. No complete attachment gate claim.

## October 10 - waistband cone correction installed

Installed 6fb31e30af86dd88ec8b32f3d5b7ab0e341a1c48d6477238f5415fc09b6fb4de
to C:/Games/X-Men Origins Wolverine/Binaries/d3d9.dll. Base pin
add4d255ce9faae485c1f9502e296e132c681b10; source identity
870e94086e24554186e9d606052d71ad9c36c8ca233aa692b912c848537a9bca.
Exactly one compiled input changed: shared garments/pouch_cage.hpp.
Upper-waist rays blend local contact into the original distal envelope,
allowing a curved waistband transition while retaining the rounded underside.
Removing the whole envelope made the pouch lumpy and was rejected.
565 anatomy/physics/material/control/binding inputs remain byte-identical.

Focused acceptance: shared seam/covariance/finite/cadence test, source provenance,
20 native control cases, 22 views, 3 exposed attachment views and reviewed motion
samples; original campaign front/side running with tank and pouch. No fitting
rejections; representative preparation remains around 2.8-3.0 ms. Prior broad
unchanged-body numerical/LOD evidence retained by source identity, not rerun
exhaustively. No continuous collision or fresh reload-cycle claim.
Room run 20261010-143041-fa3d43 and campaign 20261010-143835-02b23a closed.
Room prior DLL restored. No host input/focus. Private evidence:
D:/MaleModBuilds/jockstrap-waist-20261010/install-evidence.json.

Exact rollback: C:/Games/X-Men Origins Wolverine/WGame/ModBackups/
Meridian-development-20261010-144251-69d5b0/Restore.ps1. Prior runtime9919da09
retained; installer verified rollback and protected saves/settings/audio.
Canonical c718b1b plus named overlays; Base compatibility pins local. No push
or release. Existing uncommitted work retained. Other spokes adopt the shared
pin with their own binding/visual checks.

# Playable pouch acceptance - October 10, 2026

Status: installed October 10 at 13:41 EDT after the required sampled attachment, actual supported LOD, native motion and campaign checks. Production SHA-256 and exact rollback verified. This is a local development update, not a published release.

## Repairs

The new pouch uses a 24 by 17 control cage and local contact corrections. It preserves the original sewn boundary, UV lineage and display mesh. The expensive walking, repeated following and repair path is no longer the default. CPU source caching removes repeated GPU reads, and covered contents use 120 Hz integration with alternate-frame fine skin updates. Controls and clinical timing bypass the fine-skin cadence limit.

Campaign validation found a separate lifetime error: a new gameplay character vertex buffer after checkpoint reload did not replace the retained old buffer. Verified character sections now rebind after the old owner is absent, resetting dependent caches. This was observed working after an actual death/reload in the owned native campaign clone.

The first detailed motion review rejected exaggerated garment swings despite zero fit rejection counters. Shared covered-content support now reduces motion transfer to 0.18, increases root support/damping, and adds drag. Naked mechanics retain their original parameters. Native walking and landing captures showed a substantially more compact pouch; the final sampled native review is recorded below.

The combined smallest controls exposed a second visual failure: the Length mapping extrapolated to 0.40, below the authored 0.60 morph endpoint. Its lower range now uses the existing coherent 1.0 endpoint. Default 50 and the upper range are unchanged; saved numeric controls are preserved, but short settings intentionally produce a less collapsed shape.

The uncovered extreme-angle check then exposed a root fold: the old +120 degree endpoint pointed the root back through the pelvis. The adapter now maps the full Angle UI to -65 through +75 model degrees while preserving neutral 50. This is a measured-character calibration, not a universal angle recommendation for other adapters. The rejected view remains recorded in visual-review-rejected-short.json.

## Comparison

| Aspect | Original expensive walker | Optimized walker | Pre-shaped pouch |
|---|---|---|---|
| Fit CPU, original paired replay | 139.876 ms | 36.044 ms | Separate paired comparison below |
| Fit CPU, same 14-pose alternating comparison | Not rerun | 37.8749 ms | 2.1098 ms |
| Relative fit cost | Repeated walking and repairs | Same output with less repeated work | 94.43% below optimized walker, about 18 times faster |
| Fit behavior | Follows local detail closely | Preserves walker output | Smooth envelope bridges small recesses; deliberately looser |
| Contact guarantee | Sampled acceptance | Same sampled acceptance | Sampled local contact, not continuous triangle certification |
| Motion | Expensive fit follows contents | Same basic motion | Garment support damps the covered contents |
| Integration | Old resource owner could survive reload | Same unrelated lifetime defect | Reload ownership repaired and observed |

These percentages measure cloth fitting CPU, not whole-game FPS. An earlier exact-source reload-fixed candidate averaged 30.979 ms / 32.28 FPS across 1,080 campaign frames at actual 1280 by 720 resolution. The final supported-arc candidate measured 32.124 ms / 31.13 FPS over 1,080 active campaign frames at 1280x720, wearing the tank and jockstrap; different scene activity prevents treating the two runs as a paired FPS comparison. High-resolution grey-room captures are visual checks, not a comparable performance benchmark.

## Verification identity

Normal production candidate: `D:/MaleModBuilds/jockstrap-acceptance-20261010/production-supported-arc/d3d9.dll`
SHA256: `9919da09202c9e339db79a4b38df95eeaa0118055dc121df3759279f8c829cb8`.

Sealed native derivative: `D:/MaleModBuilds/jockstrap-acceptance-20261010/native-supported-arc/d3d9.dll`
SHA256: `fa450a2f509dc048a3bfff2a5e057b468d0e7c1b5de7a010c81cec0f2b894c6e`.

Source: canonical Wolverine c718b1b15cbe16ca1cb309e57770cca9ccfd244d plus named, hashed overlays; Base pin 3ee59844520e9e4ca6ad949594df9e6c9b2a297c. Source identity 37e6b6afca3d61f4b49105fa0197e8bd4bba4f93731821abbaebb327b28672dd. All 566 compiled source/header inputs match across builds after reversing the eight documented sealed-driver insertions. Production contains no private factory/input hooks or sandbox world origin.

Final numerical pass: 147 static cases, 540 covered/uncovered motion frames, 480 pulse frames; finite geometry, normalized weights, fixed exterior and exact welded constraints. This does not mathematically certify triangle orientation or shading. The x86-host compiler hit C1001 while building the enlarged test; the x64-host compiler targeting x86 built and ran it successfully. Runtime build passed normally.

Installed supported Natural character package has one skeletal LOD with 50,915 vertices, including both body resources. Package inventory and hash are in character-lods.json. No extra LOD or other costume support is inferred. Nine installed package/material hashes match the accepted manifest.

Evidence root: `D:/MaleModBuilds/jockstrap-acceptance-20261010`. The first native4 visual review is explicitly rejected in visual-review-rejected-native4.json. Preserve it as failure evidence. Final room run: `D:/MaleModBuilds/jockstrap-cadence-20261009/room32/runs/20261010-130654-6b0170`.

All native automation uses the owned engine channel; no host keyboard, mouse or focus input. Campaign clone uses original installed campaign packages and its own generated profile, never personal saves. Room tests do not prove every campaign level, other costumes, every possible pose, or human interactive feel.

Shared algorithms remain in Base and are pinned by Wolverine. Other spokes must adopt the pin with their own measured bindings and repeat native gates; no Witcher adoption is claimed. Full Base verification still has the pre-existing motion-header extraction drift, and the sandbox recipe verifier has the existing sealed-driver hash mismatch; neither unrelated manifest was silently refreshed.

## Final room continuation after host restart

The October 10 Windows stop was recorded as MEMORY_MANAGEMENT 0x1A/0x41792; an earlier October 8 event has the same subtype. Windows reported MEMORY.DMP, but the file was absent on inspection. The events do not establish a cause. No RAM/driver repair is claimed. Crash event receipts remain private under the evidence root.

The interrupted run preserved both 20-case matrices and 21 uncovered static attachment views. The continuation 20261010-132740-abab5e completed 22 full-orbit/size views, 25 covered motion samples and 25 uncovered motion samples, then closed with exit 0. Runtime identity is the final fa450a2f derivative. 133 native images have retained SHA-256 records in room-visual-review.json. The earlier root collapse and extreme-angle fold were absent. Broad native shadowing and ribbing remain visible; sampled review does not certify continuous contact or every possible pose.

At actual 1280x960, 30 eligible 120-frame windows averaged 29.861 ms (33.49 FPS), including capture and control-change overhead. Garment preparation windows were approximately 2.7-3.0 ms with zero reported rejection. This is room cadence, not the campaign result. Campaign and installation receipts follow separately.

## Final campaign and installation

Campaign run 20261010-133115-7b36e6 used the exact fa450a2f native derivative with original licensed campaign assets. Observed skydive/landing, damage, front and side running with Tank Top + Jockstrap, death and checkpoint reload. Log confirms resourceReplaced=1 and the reloaded outfit is visible. The final post-reload front movement attempt met the 360-second bounded timeout; rear outfit and tutorial captures establish reloaded visibility only. No post-reload motion claim. The child exited 124 as designed, not a crash. No reported fitting rejection.

Nine active 120-frame timing windows between tutorial dismissal and the side-running capture averaged **32.124 ms / 31.13 FPS at 1280x720**. Death/loading/tutorial windows are excluded. See campaign-supported-arc.json. Human play feel, every campaign level, other costumes and continuous contact remain outside this sampled acceptance. The Windows BSOD cause remains undiagnosed; successful short tests do not establish machine stability.

Installer preflight and installation both passed without bypassing the attachment gate. The recorded gate covers the required sampled cases, with numerical, LOD, native and visual evidence recorded separately in install-evidence-final.json. It is not mathematical proof of all possible configurations.

Installed: `C:/Games/X-Men Origins Wolverine/Binaries/d3d9.dll`
SHA-256: `9919da09202c9e339db79a4b38df95eeaa0118055dc121df3759279f8c829cb8`.

Exact rollback: `C:/Games/X-Men Origins Wolverine/WGame/ModBackups/Meridian-development-20261010-134102-530873/Restore.ps1`. Retained prior DLL is ab1f8d8e028a1239474465aa5f98faedea45860e1547aff21ad4db8d4087a1a7; rollback ValidateOnly passed. All protected saves/settings/audio hashes remained unchanged, as did the nine managed character/material assets. The existing tank/jockstrap selection remains selected. No retail process was launched over the user. Normal production has no private test hooks.

The room closed with exit 0, its prior DLL was restored to 74ec982f1add7c5f0e053df938549eca6cbb645bcd0b42cba60b5745aa56b83f, and no owned game remains running. Campaign test material remains private outside Git and release payloads. No commit, push or release was requested or performed. Canonical Wolverine remains 68 commits ahead of origin/main with named current overlays and pre-existing dirty work; compatibility Base pin is local.
