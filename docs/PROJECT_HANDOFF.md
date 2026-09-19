# Current: rounded raphe relief and recessed meatal slit

User explicitly said DO NOT LAUNCH GAME. No game launch or modification performed. outputs/crown-raphe-meatus builds on crown-anatomical-rebuild: rounded ventral ridge tapers toward frenulum; subtle central continuation toward a rounded-end terminal meatal divot. Closed recess only, not internal urethra. Crown profile/size response retained. Joined 7 near-duplicate midline seam vertices, removed 14 collapsed faces; 10,554 vertices / 21,086 triangles. 41 size checks pass; five BVH detail-region checks zero overlaps after seam cleanup; no open/nonmanifold detail edges; Blender samples match. 4 views x 5 sizes x 3 styles x before/after = 120 renders. See CROWN-RAPHE-MEATUS.md; work/sculpt_raphe_meatus.py, render_raphe_meatus.py, check_raphe_meatus.py, audit_raphe_meatus_bvh.py, package_raphe_meatus.py. Still authoring only; UV/rig/physics/game integration incomplete. Raphe prominence is conceptual design, not medically measured.

# Current: local crown reconstruction delivered for review

User requested a stronger correction using their contour and additional anatomical references. outputs/crown-anatomical-rebuild replaces only the failed distal patch from crown-rounded (cut at authored flex .74), preserving the retained proximal source poses. 21,100 fixed triangles / 10,561 vertices; rounded rolled corona, convex cap, inward ventral notch and paired surfaces. Soft Y/delta remains stylized. Removed ring-order reversal causing an artificial transverse fold. 41 size samples ordered/nondegenerate, uniform crown scaling; five Blender samples agree; five BVH checks zero nonadjacent overlaps involving new patch; no new open/nonmanifold edges. Three-view five-size solid/wire/color comparison rendered from actual before/after objects. See CROWN-ANATOMICAL-REBUILD.md and REFERENCES.md. No game install/runtime changes. New UV patch needs atlas and rig/physics transfer. Meatal/internal anatomy omitted; not medically validated. Prior accepted artifacts preserved.

# Current: corrected lobe silhouette received; trial failed visual review

User supplied a red side-profile outline requesting a fuller rounded lobe, tucked-under transition, and retained underside Y. Saved outputs/crown-rounded-lobe/corrected-silhouette.png. Current experiments did not achieve it; no new comparison published, accepted crown-lower-rim left unchanged, game unchanged. See ROUNDED-LOBE-FAILED-TRIAL.md. Do not claim trial is complete or use its mixed-size diagnostic renders as a validated comparison.
# Current: identified lower underside rim blend

User answered clarification: LOWER RIM across underside, not shaft junction. outputs/crown-lower-rim retains reversed Y and targets authored flex .94 using .65/120 local rounding plus a compact .25*scale outward blend onto lower dome (.932–1). Prior shaft-junction seam modifier removed. Same 10% reduction/size control, five evaluated finite/nondegenerate samples. Lower edge softened but remains visible; surrounding corrugation not fixed. No runtime/install changes. See CROWN-LOWER-RIM.md; work/blend_lower_rim.py, render_lower_rim.py, package_lower_rim.py.

# Current: Y reversed; transverse crease unresolved, clarification pending

User said Y upside down and horizontal medial line. outputs/crown-ventral-y-r2 reflects relief parameter v=1.75-t on chosen crown-rounded base; local seam relaxation attempted but horizontal crease persists. Five sampled controls/evaluated geometry checks pass, 22530 triangles. Asked whether offending line is shaft-junction crease or lower rim of broad underside; await answer before guessing further. Do not claim line removed. No runtime/install changes. See CROWN-VENTRAL-Y-R2.md. Scripts sculpt_ventral_y_r2.py, prepare_y_r2_renderer.py, render_ventral_y_r2.py, package_y_r2.py.

# Current: ventral Y junction sculpt on requested rounded base

User specified two rounded lobes and soft V/Y central fold, then authorized implementation. outputs/crown-ventral-y adds bilateral broad lobe/branch/stem/recess relief to crown-rounded source, retaining rounded-R2 modifiers and 10% reduction. NOT the rejected loft or failed R3 fits. Five control samples unchanged outside patch; five evaluated samples finite/nondegenerate, 22530 triangles. Shape control continuous. Y junction visible, but preexisting corrugation/broad fan silhouette remain; this is local sculpt, not completed local retopology or medical validation. No runtime/install changes. See CROWN-VENTRAL-Y.md; scripts sculpt_ventral_y.py, render_ventral_y.py, package_ventral_y.py.

# Latest feedback: underside unacceptable; R3 trials not delivered

User says underside looks awful and unlike supplied images. Two limited shape-fit trials still fail visual review. See UNDERSIDE-R3-FAILED-TRIAL.md. Keep requested crown-rounded base and existing R2 unchanged; do not call experimental R3 a fix. No game changes.

# Current: user restored crown-rounded base; modest reduction + underside pass

IMPORTANT: User rejected clean-loft direction and explicitly requested crown-rounded base, slight poly reduction, underside refinement. outputs/crown-rounded-r2 uses EXACT work/crown-rounded/mesh.json control samples, with masked underside Smooth .25/12 and Decimate .90, evaluated 11298 verts/22530 triangles. Five evaluated finite/nondegenerate samples; original controls match. Existing corrugation still visible. New loft/lowpoly-loft outputs are superseded and NOT the chosen base. Preserve the rounded anatomy and do not substitute the loft again. Authoring modifiers only, no runtime/game installation; decimation per-pose correspondence unverified. See CROWN-ROUNDED-R2.md. Scripts prepare_rounded_r2.py/render_rounded_r2.py/package_rounded_r2.py.

# Current: clean distal loft shape study

User supplied stylized AI references; asked less sharp, smoother model. Rebuilt distal surface above flex .70 with 105x96 regular loft, clipped/zipper-connected to original shaft. outputs/crown-clean-loft: new neutral Blender/OBJ, continuous size property, comparison vs crown-rounded, audits, source scripts. Eliminates previous crown corrugation but simplifies/omits detailed frenulum, raphe and meatus; shaft join still visible. UVs placeholders; no physics or runtime source correspondence. This is a PRIMARY-FORM AUTHORING STUDY, NOT medically validated or game integrated. Do not describe source images as anatomical evidence or imply exact silhouette tracing. Five finite/nondegenerate samples; new edges manifold/closed; rows axially ordered; Blender control samples agree. See CROWN-CLEAN-LOFT.md for limitations and next steps. No installation changes.

# Current: rounded prominent ridge authoring pass

User requested round rather than sharp ridge, retaining prominence. outputs/crown-rounded broadens C2 support 1.85 to 2.8, reduces amplitude .42 to .35, adds ridge-only Smooth .5/60 iterations with matching wire geometry. Five control sample checks pass; editable size property retained. Default/max visual review shows softer outline, but inherited seam corrugation remains substantial. Authoring only, no runtime or installation changes. See CROWN-ROUNDED.md; scripts work/round_crown_ridge.py, render_rounded_crown.py, package_rounded_crown.py.

# Current: restored crown ridge authoring pass

User requested more prominence after smoothing. outputs/crown-ridge-refined adds compact radial ridge projection (0.42 to 0.672 model units with size), reduces final smoothing and uses area-weighted normals. Five sample control geometry checks pass; shaft-side control coordinates unchanged. Existing seam corrugation still visible; not a completed topology cleanup or game build. Continuous Blender size control, three-view solid/wire comparisons against crown-smoothed. Scripts work/refine_crown_ridge.py, render_refined_ridge.py, package_refined_ridge.py. See CROWN-RIDGE-REFINED.md. No installation changes.

# Current: smoothed crown authoring preview

User requested smoothing of crown-traced. See CROWN-SMOOTHED-AUTHORING.md. outputs/crown-smoothed contains constrained fairing plus crown-weighted Blender smoothing, continuous size control, OBJ, five-sample three-view solid/wire comparison. Adjacent shaft band now relaxes through -0.65 signed distance; farther shaft control vertices fixed exactly. Finite/no degenerate control faces; five Blender control samples match. Residual seam irregularity remains. No runtime source or installation changes; still authoring-only, not game ready. Scripts work/smooth_traced_crown.py and render_smoothed_crown.py reproduce.

# Current: screenshot-traced higher-density authoring prototype

User supplied red line and requested sharp mask boundary, steeper slope and higher local poly count. Matched screenshot to scale1 lateral (s.858203,offset8.921/73.919,fit.24gray). work/trace_crown_line.py extracts line; build_traced_crown.py refines mesh locally, clips support rows, preserves barycentric UV/source mapping, adds shoulder and constrained fairing; render_traced_crown.py builds editable Blender size control and30renders. Raw12645verts/25034tris; shaft-side fixed at5sizes, finite/no zeroarea threshold1e-10; Blender5samples agree. Visible maximum-size creasing and sliver triangles remain. This is an AUTHORING PROTOTYPE, not finished/runtime integrated. No runtime source or game files modified. ExistingDLL still undercutR2 candidate, installedR2hash verified. outputs/crown-traced contains model, OBJ, before/after, source scripts, trace, model-data/source mapping, validation/limitations. Need cleanup creases, robust topology checks, then regenerate matched game mesh and all runtime tables/draw constants before claiming game ready. Do not claim new topology has been installed or compiled.

# Current: deeper underside cut R2

User requested further underside exclusion. Extends4d8d3cc: cutIn strength.085->.115 and lateral falloff.90->1.10; central boundary now.895. Other scale behavior retained; scale1unchanged. 77-81 underside vertices fixed in9poses; freecrownuniform/frenulumpinned. 23 of 27 sampled cases have no added intersections; 4 extreme-size cases have up to 2 added crossing pairs each. Existing folds and faceting remain. outputs/crown-undercut-r2 contains underside-first comparison, Blender/OBJ, compiled uninstalled DLL/audits. Scripts work/check_crown_undercut_r2.py, check_crown_undercut_r2_mask.py, crown_undercut_r2_render.py reproduce. R2 installation unchanged, user review pending.

# Current: inward underside crown boundary

User accepted isolated crown but notes lower boundary cuts inward and includes underside shaft incorrectly. Extends7ceae01. Added cutIn=.085*Smoother01((ventral-.25)/.60)*(1-Smoother01(seamDistance/.90)); blend starts at.78+cutIn, width.010. Narrow frenulum mask retained.42-43 underside shaft vertices fixed across9poses; central frenulum pinned and freecrownuniform; size1unchanged.27cases finite/no degenerate; same4 extreme-size cases2crossings each. outputs/crown-undercut has underside-first comparison, Blender/OBJ, compiled uninstalled DLL and audits. work/check_crown_undercut.py, check_crown_undercut_mask.py, crown_undercut_render.py reproduce. R2 installation unchanged; visual acceptance pending.

# Current: isolate crown scaling from distal shaft

User accepted uniform crown growth but reports shaft dragged into it. Extends0a4ad1b. Moved selection start from.70 to.78 and blend from.70-.74 to.78-.79; shaft<=.78 is exactly fixed. Full crown scalar and narrow frenulum mask retained. Scale1unchanged. 23 of 27 sampled cases have no added intersections; 4 extreme-size cases have up to 2 added crossing pairs each. Existing folds and faceting remain. Allfinite/no degenerate; pelvis/nodes unchanged. outputs/crown-isolated contains matched before/after, Blender/OBJ, compiled uninstalled DLL and audits. work/check_crown_isolated.py, check_crown_isolated_mask.py, crown_isolated_render.py reproduce. InstalledR2hash verified; visual approval/live-game testing pending.

# Current: narrower attachment mask for Glans Size

User again reports crown shape loss; confirmed Glans Size preview, not overall controls. Extends9db1ddf. Found prior angular fold mask excluded broad underside of crown. Changed lateral falloff to abs(offset dot lateral)/logicalShaftBodyRadius: pinned.055, fullscale.180. Retained ventral/longitudinal gates and pivot/blend. Tests:22 central attachment vertices fixed,538-540 crown vertices uniformly scaled in9poses, scale1byte-identical. 27geometrycases finite/no degenerates; same2crossingpairs at size100/state1/g50 remain;26cases no additions. No installed changes. outputs/crown-shape comparison/Blender/OBJ/compiledcandidate/audits; work/check_crown_shape.py, check_crown_shape_mask.py, crown_shape_render.py reproduce. Original facets remain; user visual acceptance pending.

# Current: preserve rim during crown enlargement

User likes size1 but reports crown dissolves when enlarged. Extends edec109. Previous .82-.855 blend left authored rim underscaled. New pivot.76 and blend.70-.74 finish behind folded rim; full uniform scale applies to rim and cap outside pinned frenulum region. Wider angular fold transition .28+.40 protects central frenulum without sharp neighboring displacement. Scale1 unchanged. 27case audit:26cases no added intersections vs each pose size1; size100/state1/g50 adds2 folded-rim crossings, unresolved, no degenerates, finite; shaft<=.70/pelvis/nodes unchanged. Scale-mask audit tests full rim/cap and pinned central frenulum. outputs/crown-preserve has comparison, Blender/OBJ and compiled uninstalled candidate. Existing faceting remains, live-game testing pending. Scripts work/check_crown_preserve.py, check_preserved_crown_mask.py, crown_preserve_render.py, package_crown_preserve.py.

# Current: uniform crown scaling with pinned frenulum

User requested uniform scaling only from the crown forward, excluding frenulum. Extends cfdbb98. ScaleGlansIndependently now uses a single scalar 1/1.4/1.6 about .82, attachment blend .82-.855. Angular central ventral mask pins frenulum through .91 and fades by .935; shaft<=.82 and suspension excluded. 27cases pass no added crossings vs size1, finite/no degenerates, shaft/pelvis/nodes unchanged. Separate work/check_crown_mask.py verifies44-45 protected vertices exactly fixed and194 distal vertices uniformly scaled within8e-6 across9poses. Blenderdriver1/50/100matchesruntime. outputs/crown-uniform has matched old/new previews, editable model, compiled DLL, source/patch/audits. Not installed; R2hash verified. Existing facets remain; user review pending. Reproduce with work/check_crown_uniform.py, crown_uniform_render.py and package_crown_uniform.py.

# Current: shallow dorsal coronal approach

User requested a slight groove in the upper shaft leading to the crown. Extends77825ab on glans-size-study. SculptVentralContourStudy now adds an 8% radius dorsal relief centered.700, proximal width.055/distal.040, underside faded. Direct relief across the existing folded seam (.75-.78) introduced crossings; discarded. Final.645-.740 approach passes27 matched cases against77825ab: no added crossings/degenerates, finite, proximal<=.645/pelvis/nodes unchanged. Compiled candidate not installed; game R2 hash verified. outputs/coronal-groove contains matched-size before/after previews, editable size-controlled Blender model, OBJ, DLL/source/patch/audit. work/check_coronal_groove.py and coronal_groove_render.py reproduce results. Visual review pending; existing faceting remains.

# Current: independent Glans Size study

User requested broad spade-shaped head and separate size slider, supplied adult anatomical photo. New branch glans-size-study extends551b6c7. ScaleGlansIndependently afterSculptVentralContourStudy beforecontact/normals, uses liveframe anchor.79; authored.79-.845 blend. Range1..100 default50, width1/1.4/1.6, depth1+.65growth,axial1+1.5growth. Earlylarger2x/1.8x ranges and altered transitions introducedcrossings; final1.6bound passes27cases. No originalimageembedded/textured; neutral anatomical design study.

GLANS SIZE row3 afterWIDTH; oldrows remapped through controlindex. AdjustStudyControl/ResetStudyControls are used by realinput and harness tests. Save/load Shape/Glans Size, default50 ifmissing, reset50. Harness optional finalarg (argv11)glansUI and --glans-controls-test. Tests runonly from work/glans-size-study/settings-test so liveINI untouched. All18rows,bounds,persistence,missingkey/resetpass.

work/check_glans_size.py:27cases sizes1/50/100 xstates0/1/2 xglans1/50/100. Allfinite,0degenerate,0addedintersectionpairs,shaft<=.79/pelvis/nodesunchanged,headwidthmonotonic. Existingintersections/faceting remain; no allpose or clinicalaccuracy claim. SourceDLLcompiled, HUDGLANS STUDY, notinstalled/livetested. InstalledR2hashB835ABE6... verified. outputs/glans-size-study has36renders,5samplebrowsercontrol,Blenderreal1..100customproperty/morph testedagainstcaptures1/50/100,staticdefaultOBJ,DLLpreview/source/audit/notes. Meshcorrespondence in Blenderkey generation usesnearest default runtimevertex because bmesh welding order differs acrosscaptures. Image renders areactualruntimepositions. work/glans_size_render.py packagesmodel; userreviewpending.

# Current: user-authorized ventral contour study from restored R2

User supplied stylized AI references; assistant described neutral geometric cues (tapered coronal lip, narrowing central ridge, localized frenular fold), and user saidYes. New branch r2-ventral-contour-study starts20f25dc, acceptedR2. No supplied image reproduced/used as texture; only agreed neutral contour intent. SculptVentralContourStudy applies angular taper/sweep to coronal addition, narrower distalcrest and flanking relief, author t>.40 and<.97 excluding suspension. Semi/floppy axial-offset transport correction retained; no surface dome replacement/smoothing/retopology. HUD CONTOUR STUDY.

work/check_ventral_contour.py:10cases versusR2,1860triangles authored>.40. Finite,0degenerate,0addedintersectionpairs; existingcrossings neutral94→46,maxmovingstate2 152→68. Proximal<=.40,pelvis,nodesidentical. Some quality metrics/state0 sharpedges worsen; visible faceting remains. Reviewed three neutral solid views. Candidatecompiled, not installed/live-game-tested; installedR2hash B835ABE6... confirmed. outputs/ventral-contour-study contains12renders/comparison,staticBlender/OBJ,DLLpreview,sourcepatch/audit/build/README. work/ventral_contour_render.py and captures reproduce. Study awaiting user review, not clinically validated or accepted release.

# Current: user requested restoration to 0.7.2 R2 again

The user requested "revert to7.1.2R2", interpreted consistently as accepted0.7.2R2. Active branch restored to version-0.7.2, source baseline79bc194. All subsequent enhancements are preserved on enhance-072-r2 at22e43da and are no longer active. InstalledDLL verified SHA256 B835ABE6DE44C4F67A9CD63805897BBB347FA5EB4489343260E3BA82D2F573FE; it already matched R2, so no game files/settings were changed. Local ignored src/runtime/d3d9.dll refreshed from that installedR2binary, and deterministic_harness.exe rebuilt from restored source to remove the experimental executable. Original outputs/v0.7.2-root-ramp-r2/comparison.html reopened. Do not resume sculpt/enhancement candidates without a new request.
# User-directed restoration to 0.7.2 R2

User rejected the subsequent anatomy studies and explicitly requested return to 0.7.2 R2. Active branch restored to version-0.7.2; source baseline d36f840. Installed DLL already matched R2 SHA256 B835ABE6DE44C4F67A9CD63805897BBB347FA5EB4489343260E3BA82D2F573FE, so no game files or settings were rewritten. Harness rebuilt from restored source. Experiments preserved separately on anatomy-glans-study and in prior outputs; they are not active. Do not resume those experiments without a new request.

# Current state — September 18, 2026: installed 0.7.2 R2

Supersedes older candidate-only notes below. Branch version-0.7.2. User requested Hang, soft scrotal neck suspension from a firm shaft, then reported a large-size root shelf. 0.7.2 was installed earlier; this revision is installed now, SHA256 B835ABE6DE44C4F67A9CD63805897BBB347FA5EB4489343260E3BA82D2F573FE. INI preserved exactly including Hang65 and size100. Package unchanged.

0.7.2 adds harmonic suspension_weights.h (1590 fixed,319 neck,479 lobe vertices), Hang1..100/default50 after rest-frame capture, shaft-carried BallAnchor at .12, independently moving weighted skin with follow=w/sqrt(.06+.94*w), shaft contact clearance and no ball impulse back into core. Junction generator fixes zero-suspension shaft vertices. Full radial construction remains important: minimum-only projection caused neck collisions and was rejected.

Root R2 changes LogicalShaftOwner takeover from .018 to .18, and post-fairing core restoration .04–.10 to .30–.40. Merely moving the latter did not remove shelf; expanding the former did. Both use quintic easing. Do not restore the abrupt tube root. No topology/package edits.

See docs/RELEASE-0.7.2-R2.md for checks and limits. work/root-ramp-fix has exact original072 before.cpp/.exe/.dll, captures and render logs; work/run_root_suite.py, audit_root_suite.py, intersect_root_suite.py reproduce29cases. outputs/v0.7.2-root-ramp-r2 has36croppedrenders and Blender comparison. All finite/weld0/neckdegenerate0. Ordinary size sweep and max/Hang65 neckcrossings0. Extreme angle686,shortwide5,tightmax1 remain; don't claim all poses perfect. Shaft nodes identical over Hang; proximal core variation max.00124. Automated sky.launch_app returned no targetable window; refreshed window/process inventory confirmed no running game. No live gameplay verification completed. User should launch through usual shortcut; do not claim live visual acceptance.

Installation backup is separate: C:/Games/X-Men Origins Wolverine/WGame/ModBackups/WolverineAnatomyTool-v0.7.2-r2. Older072backup preserved. Upgrade-0.7.2.ps1 now installs R2 and accepts072 in addition to prior versions; version and fullinstaller hashes updated. User closed game during work. No forced termination used.

# Shared project context

## September 18, 2026 - scrotal junction R2 candidate

User rejected the jagged scrotal attachment and asked for a broad attachment,
gradual neck and even triangles. Latest work is branch
`version-0.7.1-scrotal-junction`. Original 0.7.1 output ZIP remains untouched.
New outputs: `outputs/v0.7.1-scrotal-junction-r2` in this workspace.

`FinishScrotalJunction` follows FinishPelvicRamp before final normals. Generated
operator covers 902 graft normal groups; exact body welds are fixed. Includes
adjacent ventral shaft rows, which were essential to remove rear intersections.
Do not revert to the narrow 678-group prototype: it left 16 crossings at max.
Python generator is tools/Generate-ScrotalJunction.py. No package/UV/connectivity
changes, and no changes to physical solver. Source capture scripts and audits
are in work/neck*.py, work/run_neck_cases.py and work/neck-capture.

All eleven cases improve neck median/p05 quality, finite with no degenerate neck
faces, weld error zero, body positions and shaft nodes unchanged. Default/max
nonadjacent neck crossing counts are zero. High-angle still has 252 (previously
351); maximum has 3 sharp edge dihedrals >90 degrees. Do not call it fully free
of intersections/distortion at all controls. Some face normals rotate >90
degrees during repair of old folds; that alone is not a new inversion test.

Compiled DLL smoke passed; Upgrade-Junction and Rollback-Junction tested against
both release0.7 and original0.7.1, preserving settings/package/read-only bits.
The installed game has NOT been changed. All new visuals crop knees to navel;
32 Blender runtime-coordinate renders are not game shader screenshots. Remaining
work for acceptance: user visual review and live-game contact/performance testing.
The older reference screenshot/version was queried but not identified by user.


## Version 0.7.1 â€” current candidate, September 18, 2026

User authorized collar/root/pelvis cleanup, stronger large-size recruitment,
matched grey-wireframe and color renders cropped knee-to-navel, and an in-game
build named 0.7.1. This supersedes the inspection-only scope immediately below.

Implemented in the new workspace clone on branch `version-0.7.1`:
`FinishPelvicRamp()` runs after final physics surface construction and before
normal/tangent reconstruction. A generated constrained spline uses the existing
weld groups, measured angular shaft radius, bounded donor displacement,
tangential redistribution and a final orientation line search. Topology and the
0.7 package are unchanged. Source generator is `tools/Generate-PelvicRamp.py`;
header regeneration was byte-identical. No inherited experiment was installed.

Candidate DLL SHA256: 08DFE9F322CD7E6B071DAD0E0D24F7718811D629E689787CF00EAB5915F2D520.
Ten deterministic geometry cases: finite positions, zero added face reversals,
zero weld separation. Some triangle aspect statistics increase; do not describe
this as a complete retopology or uniformly better triangle quality. Existing
degenerate body faces persist. Extreme poses automatically weaken the edit.
Isolated released-DLL D3D9 smoke test and both installer paths passed.
Live game and settings were not changed; in-game playtest remains pending.

Deliverables are in the new workspace's `outputs/v0.7.1`. `work/v0.7.1-capture`
contains deterministic before/after dumps, nodes, geometry audit and timing.
`work/final_renders.py` renders the emitted coordinates and original diffuse UV
atlas in Blender; it clips the presentation only, welds coincident render vertices,
and computes smooth normals. Renders are not game-shader screenshots. Four views
at default (all50) and maximum (O/L/W/S100, others50), full-floppy state, 240 fixed
60-Hz frames are matched between versions. Uncropped original work remains in
the earlier inspection outputs; all NEW render deliverables are cropped.

Next: review the matched images and playtest the packaged candidate using the
Upgrade/Rollback pair. Do not call the appearance user-approved until feedback.

## September 18, 2026 â€” independent v0.7 inspection in a new workspace

The sections below this entry are historical v0.6/161 notes, not the current
authorization or a complete description of v0.7. Current user request: inspect
v0.7, parse its implementation, open and understand the 3D model before future
physics/model improvements. No modifications or installation were requested.

Working copy: `C:/Users/Administrator/Documents/Codex/2026-09-18/i-am-an-educator-working-as/work/wolverine-v0.7`,
tag v0.7 / commit f7811dc24cfb0e3ce1f2ab1c91a9c800a1b15bc5. The installed DLL and
package match this release's manifest (DLL 8658C9FC..., package 7C5CE1F5...).
The released source matches the local t70 provenance archive after newline
normalization. No source rebuild-to-binary equivalence was claimed.

Observed: UModel exported the actual installed package successfully. Blender
import contains 47,030 vertices, 87,854 triangles, 128 bones, and 116 vertex
groups; section 7 has 4,596 triangles matching graft_normals.h. The runtime
still addresses 2,388 graft slots starting at GPU vertex 47,050 in a 50,915
vertex buffer. Do not confuse imported PSK indices with GPU indices.

An isolated shape harness, compiled with its synthetic keyboard events removed,
ran the unchanged release DLL at all UI sliders 50 / state 2 for 240 frames.
It captured 2,844 positions with HRESULT 0; no animated bone input was supplied.
Its graft and package-rest previews were visually inspected. Triangle checks
show finite coordinates and very thin triangle outliers, but this is not an
in-game shading, collision or full-envelope validation. No fixes were attempted.

Openable Blender scene, inspection renders and detailed source map are in the
new workspace's `outputs/`; scripts, logs, geometry-audit.json, exported PSK and
the isolated harness are in `work/`. The Blender file holds both the rigged
package mesh and a disabled collection with an exact-order runtime graft dump.
Installed game and settings were not modified. Next: obtain the concrete first
model/physics change, then make a separate candidate from the verified v0.7.

Updated 2026-09-15. Both agents/accounts must update this document when handing
off. Relative workspace paths below are relative to the parent of this Git repo.

## State and user intent

- Historical Git `617e815`, tag v0.5: package revision154 (Weapon X safe), runtime157.
- User authorized packaging and committing restored Revision161 as a release.
  Release v0.6 contains character161/runtime161; payload DLL and reconstructed
  package match the restored installed hashes below. Revision162/163 excluded.
  Local release ZIP: `release/BigDickLoganMod-v0.6.zip` (ignored by Git).
  Published publicly on GitHub as release `v0.6` on 2026-09-15 at 18:18 local,
  with the ZIP attached. Remote `main` and tag `v0.6` point to commit
  `194ce9d5bfa0c6d09265eb5113dede2c7face380`.
  `tools/Test-Release.ps1` passed in an isolated fixture: package reconstruction,
  matching DLL, existing settings preservation, both checkpoint config edits,
  and byte-exact uninstall restoration. Known collar shimmer is disclosed in
  README/CHANGELOG. Remote publication was explicitly requested and completed
  after release validation.
- Functional reference: `work/hd/revision154-weaponx-safe/atlas/mesh.json`,
  its `final/WGame/CookedPC/CH_Wolverine_Natural_SF.xxx`, and `work/hd/runtime157`.
- Installed at last verification: **Revision161 restored on 2026-09-15 at
  17:33 local**, after the user closed the game. Package SHA256 remains
  `970D5F99F3DE8E9EA2EC4D4EEECFB0E430C21018BB449B486D6586B59F1E76E6`;
  DLL SHA256 is
  `72B5ABD7BA007C3F207BE0EAF1130F4964DE2E59720AAA3CAD22CC631C2C648E`.
  Live INI SHA256
  `14F4A8686B9B8BEFA529B4418385E819534A7EAC4171CA60D1EB28DA85E38DB8`
  and the package were verified unchanged during the DLL restore.
- Game: `C:/Games/X-Men Origins Wolverine`; runtime in `Binaries/d3d9.dll`.
- User's 06:09â€“06:10 screenshots confirm 161 STILL shimmers/crinkles, including
  Overall55/Width57 and Overall100/Width100. User explicitly says 157 also fails.
- Current authorization: fix the Revision161 collar shimmer surgically. Do not
  broaden the donor field, reshape the silhouette, or rearchitect dilation.
  Avoid excessive token expenditure and broad tests.
- Revision162 failed its first in-game playtest and is **not installed**.

## Revision163 surgical tangent fix

- **REJECTED by playtest 2026-09-15 17:20; reverted at 17:33 after game exit.**
  Rejected DLL, current INI and package were preserved at
  `C:/Games/X-Men Origins Wolverine/_ModBackups/Rejected_Revision163_20260915-173333`.
  User reports worse rendering; screenshot shows conspicuous shaft highlights.
  The earlier attribution of torso damage to tangent corruption was unverified
  (the game also renders body damage). Do not retry this change unchanged.
- Source: `work/hd/runtime163`, copied from runtime161. The package, topology,
  morphs, physics, vertex positions, collar fairing, and Weapon X materials are
  byte-for-byte Revision161. Only `d3d9_proxy.cpp` tangent selection changed.
- Diagnostic rationale: Revision161 introduced `collarTangentSums`, which welded
  tangents by geometric position across duplicated collar UV vertices. Normals
  should be shared across coincident weld copies, but tangents belong to each UV
  chart. Revision161 also retained each vertex's original packed W handedness;
  assigning a position-averaged XYZ tangent with a different orientation can
  invert the normal map on isolated triangles, matching the reported white
  shimmer. Revision160/157 used per-vertex UV tangents instead.
- Fix: retain Revision161's unified geometric normals, but use the final-position
  UV derivative tangent accumulated per graft vertex. Orthogonalize it against
  the unified normal, fall back to the original runtime tangent if degenerate,
  and sign-align it to that original tangent so the untouched packed W
  handedness remains valid. No vertex is moved by Revision163.
- Verification: runtime163 builds cleanly. Its D3D9 proxy harness exits0 with
  correct Revision161 50915-vertex buffer detection, shaft and ball motion,
  lobe RMS0.00000 and maximum relative distortion0.00001.
- Revision161 backup before the DLL-only install:
  `C:/Games/X-Men Origins Wolverine/_ModBackups/Revision161_before_Revision163_20260915-110826`.
- Restore completed and DLL hash verified against the exact Revision161 backup.
  Exact next step: await user direction; the current request was only to revert.
- Failure lesson: using the already-skinned runtime packed tangent as a sign
  reference is unsafe here; its basis/packing convention is not equivalent to
  the reconstructed object-space UV derivative, and preserving W did not make
  them compatible. The mismatch corrupted normal mapping across the character.

## Revision162 implementation and evidence

- **REJECTED:** At Overall100 / Width50 / other controls50, the user's 10:10
  screenshots show the collar expanding into a huge, nearly horizontal pelvic
  apron/torus spanning the entire crotch. This is substantially worse than
  Revision161's localized shimmer. The exact rejected installation is backed up
  at `C:/Games/X-Men Origins Wolverine/_ModBackups/Rejected_Revision162_20260915-101843`.
- Root cause: the runtime's final shaft-ring cylinder and broad pelvis-control
  recruitment were validated only for local triangle validity. Those checks
  proved watertightness/non-inversion but placed no bound on the collar's global
  silhouette, thickness, or geodesic donor extent. Overall alone drives
  `PelvisCollarGrowth()` through the multiplicative diameter ratio, while the
  6-11 unit body radius plus the 53-lane outer footprint turns the collar into
  a broad annular shelf. This candidate must not be reinstalled unchanged.
- Any successor must add silhouette/envelope assertions: bound displacement of
  every body donor from its original surface, bound collar X/Z/Y extents relative
  to the live shaft radius, and explicitly test Overall100/Width50 (the failure
  case), not just max/max and moderate pairs. Prefer a much narrower donor field
  or leave the pelvis mesh fixed and solve only the immediate seam normals/ring.

- Sources: `work/hd/build_revision162_collar.py`,
  `work/hd/runtime162/d3d9_proxy.cpp`, and generated files under
  `work/hd/revision162-parametric-collar/build` / `work/hd/runtime162`.
- Topology: graft base47090, graft count2567, total VB50955, section7 index
  start249924. The replacement is an open anatomical collar with 53 matched
  lanes and six longitudinal rows (outer boundary, four interior bands, shaft
  boundary). Forty body-side edge-split vertices preserve exact chunk-local
  skin palettes. This is a modest replacement, not a high-poly radial shell.
- Both endpoints use one fixed spatial correspondence for positions, UVs,
  morphs, skin weights, physics weights, normals and runtime support. The old
  Revision161 zipper patch is removed. Alternating band diagonals replace star
  or fan topology; the lower inner-thigh sector remains outside the donor field.
- Runtime evaluates bounded Hermite lanes from the final body boundary to the
  final shaft boundary before rest-frame construction, after tube preservation,
  and again after physics. The shaft-side ring is forced to the live cylinder;
  neighboring shaft vertices follow a compact smooth support field. Final
  normals/tangents are rebuilt after the last position solve.
- Shaft-side angular positions are regularized 15% toward even spacing while
  preserving 85% of the anatomical pelvis correspondence. Raw pelvis angles
  had nearly coincident lanes that created skinny shimmering wedges; stronger
  (45%) regularization folded one maximum-width outer transition, so it was
  rejected. This 15% version has zero opposed or degenerate patch faces in all
  sampled cases.
- Physical angle is clamped to -20..+55 degrees via `EffectiveShaftAngle()`;
  rotation occurs at the pubic root and the collar is regenerated around the
  posed shaft. This is the user-authorized inward endpoint clamp.
- Production-position harness cases: saved preset50/50/50, reported
  Overall55/Width57/Angle52, Overall100/Width100/Angle52, maximum width at each
  clamped angle endpoint, and Overall10/Width10. All six have zero degenerate
  faces, zero opposed adjacent patch pairs, and weld coincidence error <=
  1.53e-5. See `build/final-audit.json` and `build/runtime-collars.png`.
  Neighbor face-angle maxima are still 59-74 degrees, concentrated mostly at
  the two open splice ends and one high-angle inner band; dynamic welded
  normals reduce shading discontinuity, but only the game can decide whether
  those local transitions are visually acceptable.
- The actual D3D9 proxy harness passed with exit0: correct 50955-vertex buffer
  detection, base47090 attachment, shaft/ball motion, lobe RMS0.00000,
  lobe maximum relative distortion0.00001. UModel exported the injected mesh
  successfully (its missing-import/TFC warnings are the known standalone-package
  baseline). Package validation confirms both Weapon X materials still parent
  `MAT_Gore_WolverineBase` with permutation1.
- Revision161 backup:
  `C:/Games/X-Men Origins Wolverine/_ModBackups/Revision161_before_Revision162_20260915-093413`.
  `work/hd/revision162-parametric-collar/backup-path.txt` records the same path.
- Exact next step: begin from restored Revision161, not Revision162. Diagnose
  Revision161's small shimmer locally. Do not reuse Revision162's broad pelvis
  recruitment without strict envelope constraints and an Overall100/Width50
  regression test.

## Architectural dependencies

- Mesh package and proxy-generated tables are a matched set. 161 uses graft
  first vertex47050, count2388, stride32, full VB50915 vertices, section7 index
  start249804. 157 uses count2304. Changes to topology require regenerating morph,
  physics, graft normals, pelvis control and collar headers plus updating runtime
  and harness buffer/count/signature constants. Hardcoded draw signatures have
  previously broken character transform detection.
- UE3 uses chunks with local bone-index palettes. Moving vertices between
  chunks requires palette translation. Indices are 16-bit (<65536 vertices).
- Body sections4/6 and graft7 have duplicated weld vertices. Coincidence alone
  does not ensure tangent continuity, valid faces, UV continuity, or smooth light.
- Runtime changes positions after skeletal buffer preparation; normals/tangents
  must be rebuilt from FINAL positions, including physics. Geometry passes before
  physics are insufficient evidence of the final surface.
- Weapon X overrides MAT_Electrodes and MAT_Wolverine_TankMarkings must retain
  MAT_Gore_WolverineBase parent/permutation1. Start package injection from154.
- UI1â€“100 is mapped piecewise around the user's saved preset at50. Shape neutral
  is [1.2,1.6,1.59,1.53,30,-0.7,0.400001]; angle UI52 =33.6 physical degrees.
  Morph-grid midpoint is a DIFFERENT legacy reference (Overall1.5/Width1.15).
- F6 toggles Big Dick Logan Mod menu, F8 resets all to50. Preserve live INI.
  Older NORMALIZED-CONTROLS.md incorrectly says Home; source uses F8.

## Failed approaches and diagnostic caveats

- User rejected ultra-high-poly and radially symmetrical collars. Rings mean
  connected bands draped over actual anatomy. Inner thighs are not general donor
  material; dilation should favor the upper pelvis. Avoid broad body resculpting.
- 161 removed97 faces and inserted221 using4x21 new vertices in reserved slots
  49354â€“49437. Outer boundary13 / inner42 are zipper-stitched. Lower sector retained.
- Reparameterizing each morph separately caused lanes to slide/twist; keep fixed
  neutral correspondence for ALL geometry and weight samples. Scalar interpolation
  in build_revision161_round_weld.py independently reverses based on scalar values:
  this is unsafe correspondence, not a valid spatial orientation test.
- 160â€“280 uniform Laplacian passes shrink the collar; later tube preservation can
  undo its shape. Ring-only Taubin passes cannot ensure longitudinal smoothness.
- Full-graph Taubin and mean-value geometry smoothing were tried unsuccessfully.
  Normal smoothing or texture paint cannot fix folds, degenerate faces or overlaps.
- The Python runtime161 audit is only an approximation: defaults use weighted
  geometry smoothing and zero ring passes, unlike installed runtime. Match with
  COLLAR_WEIGHT_MODE=uniform, COLLAR_RING_FAIR_PAIRS=4 and physical angle.
  It omits tube preservation, full secondary sliders, and final physics. Its
  'flips' count compares to neutral normals, not a robust inversion proof.
- At O2.5/W2/angle33.6 that PARTIAL audit gave seam0, max weld42.7deg,
  raw normal coherence min0.055. These diagnose concerns; they do not prove the
  exact in-game culprit. Earlier reports overstated certainty about these metrics.
- Fixed-reference normal hemisphere flips can hide invalid geometry and can be
  inappropriate after large rotations. Validate the final surface itself.

## Build / verification / install

- Python with numpy: `C:/Users/Administrator/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`.
- Git: `C:/Program Files/Git/cmd/git.exe`.
- Runtime build: build.cmd uses x86 VC BuildTools and DirectX SDK June2010.
- Injector: work/hd/inject_hd_mesh_anatomy.ps1 -InputPackage <154 package>
  -MeshJson <new mesh> -OutputPackage <candidate>. Validate with validate_package.ps1;
  export/readback with work/tools/umodel/umodel.exe -export -game=xmen -all -dds.
- Existing proxy harness tests detection and motion/ball rigidity, not a complete
  collar render. It sends keyboard input: only run with game closed.
- Build isolated candidates; inspect a few moderate/extreme size and angle cases.
  Prefer actual runtime final-position dumps over reimplementing its math in Python.
- Before installing check Wolverine process, back up package/DLL and live INI;
  copy only intended package/DLL, verify hashes, leave saved settings intact.
- Existing backups recorded under work/hd/revision161-round-weld/*backup-path.txt.
  Experimental backups use game/_ModBackups; release installer uses
  WGame/ModBackups/BigDickLoganMod-v0.5 (different systems).
- Public repo contains a WBX1 delta requiring original game package, not the
  original package. Update payload/manifest only when preparing an authorized release.

## Maintenance requirement

Next agent: read this first, verify current disk/process state, update this file
with what you learn and the exact next action before handing off. Do not repeat
failed approaches without a specific new reason. Do not mark a candidate visually
fixed until in-game evidence supports that claim.


