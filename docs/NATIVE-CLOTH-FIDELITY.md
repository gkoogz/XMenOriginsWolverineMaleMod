# October 8: residual white/grey switching and curved pouch detail

The user reported recurrent white/grey jockstrap changes and jagged wrapping
around the testicles after the earlier material repair. The follow-up candidate
keeps 32 rays, 24 render rows, 2,471 garment vertices and 4,816 triangles, with
geometry evaluated once per render frame and reused across native light passes.

The shared walker now refines the two measured testicle ovoids with 48 virtual
longitudinal intervals and redistributes surface rows toward local turns. The
rest of the collision chain still participates in whole-triangle certification.
No extra proxy geometry is rendered. See Base docs/MERIDIAN-CURVATURE.md for the
SDK-free contract, regression proof and adoption requirements. The optional
last ClearMeridians argument is 7 here because this observed rig has seven
penile solids followed by its two testicle solids; that index is not universal.

The material now sets its complete sampler contract: clamp U/V/W, zero maximum
mip level and LOD bias, anisotropy one, explicit filters and linear sampling.
The old draw set filters and sRGB but inherited other sampler values from native
body materials. The pixel shader handles visible back faces using VFACE and
guards interpolated-normal normalization. Native lights and SH remain active;
no fixed ambient floor, unlit override or update-rate reduction is introduced.

Gameplay native shader disassembly ends with inverse scene depth in output
alpha: MinZ_MaxZRatio.x / projected.w + MinZ_MaxZRatio.y. The native vertex shader
passes the same clip position into that varying. The follow-up cloth implements
this measured gameplay contract as well as the title contract, instead of
writing gameplay opacity one. This supersedes the earlier gameplay exception
in NATIVE-MATERIAL-REPAIR.md. Additive RGB-only passes retain native depth.
The title highlight shoulder is unchanged. State-block restoration includes
all sampler states; opt-in diagnostics compare them before and after the draw.

These are identified rendering-contract defects. They do not establish a single
cause for every user-observed white/grey transition; normal diffuse shading
must still change with pose and light. Human-play confirmation remains pending.
Source compilation alone is not evidence that the visual bug is eliminated.

The source-only CPU profiler accepts either the prior or refined exact clearance
call, rejects unknown signatures and checks the selected build's input hashes.
The native command writer also recognizes a completed native capture receipt
as an acknowledgement. One run had the completed PNG receipt but lacked its
earlier queued-capture log line, stopping the matrix after five cases. That
partial sweep is retained separately; the complete rerun is the acceptance
sample. No host keyboard, mouse or focus is used in either native run.

Exact build, native evidence, update and rollback results are recorded in the
newest HANDOFF checkpoint. The full attachment/campaign/LOD gate remains false.
No normal runtime promotion, release, commit or push is implied.

## Installed sampled checkpoint

Installed developer SHA256: 25dc08422d5bc6585d704ccd937a648356d3d0e10b134ba59bd94dbaff6786f9.
The 18-case room run completed without reported draw rejection; sampled front,
side, oblique and movement captures were reviewed. The full attachment gate
remains false. A separate wider title view audited 7,921 passes with zero state
mismatches; its original camera file was restored by matching SHA256. The
overlay still obscures cloth, so no complete tank-cloth visual pass is claimed.
Exact 29ebe88c rollback: MeridianPrototype/rollback-performance-20261008-183418.
Current settings and retail 4f4900a5 runtime are preserved. No commit or push.
