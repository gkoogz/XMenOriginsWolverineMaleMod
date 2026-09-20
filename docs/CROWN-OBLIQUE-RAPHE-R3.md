# Longer crown, defined raphe and converging V — R3

Extends crown-oblique-raphe-r2 with the four requested changes:

- Another 1.02 model units of distributed axial length toward the tip, with a smooth transition from the crown.
- Raphe height envelope increased from 0.79 to 1.03 and proximal width parameter from 1.08 to 1.24. A rounded shoulder profile and shallow 0.065-unit flanking relief define its lengthwise edges. The edges remain rounded.
- Another 0.68-unit ventral coronal sweep envelope, broadly blended so the lower margin moves forward while the upper margin stays substantially fixed.
- Paired underside valleys converge toward the tip, framing a narrow raised frenular bridge that continues from the raphe and tapers into the terminal channel. The terminal recess is rebuilt as one smooth depression over a cap envelope fitted from unaffected lateral points, avoiding the raised pole artifact from overlapping earlier recesses.

These are authoring parameters, not physical measurements or medical validation. The slit remains a closed surface recess rather than an internal urethral lumen. R2's smooth outer cap and modest flare are retained. Some local fine faceting remains visible under cavity lighting, especially at large size, as does the retained original shaft faceting.

The size mapping is unchanged: 0 = 0.85x, 50 = 1.00x, 100 = 1.60x. Both models are compared at identical physical scales. The comparison has five rendered settings, four views and three shading styles (120 images total); Blender has a continuous control. OBJ is exported at slider 50.

Topology is unchanged: 10,554 vertices / 21,086 triangles. Forty-one size checks pass finite/nondegenerate geometry, ordered body rings and uniform scaling of fully weighted crown vertices. Terminal rows are excluded from the axial-order assertion because the closed recess turns inward. The edited region has no open/nonmanifold edges. Five BVH checks find no nonadjacent intersections involving edited faces. Both before/after Blender evaluated positions are verified against their expected sample meshes. See geometry-audit.json, intersection-audit.json, blender-control-check.json, before-control-check.json, change-audit.json and preview-ui-check.json. These checks do not cover animation, physics or clinical accuracy.

The final terminal fitting step supersedes the inherited meatal displacement recipe: future edits should use the final positions and the R3 reconstruction code rather than assume that subtracting the original 0.30-unit meatal field exactly restores the cap. The fitted region is restricted to the central terminal surface; no global crown replacement is performed.

Previous artifacts are preserved. Source scripts in sources/ require the existing workspace inputs and dependencies. model-data.json contains the resulting mesh positions/topology. The game was not launched or modified. Final UV/rig/physics transfer and game integration remain unfinished; this is an authoring preview pending user review.
