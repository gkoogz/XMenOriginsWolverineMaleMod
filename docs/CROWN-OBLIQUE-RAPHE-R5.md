# Outward V, rolled crown and smooth terminal transition — R5

Corrects three issues identified in R4:

- The V opens outward toward the coronal shoulders, following the user's red outline. The paired spline half-widths now decrease from 3.05 proximally through 2.75, 1.95, 1.25, 0.69, 0.35 and 0.18 toward zero at the tip. This reverses R4's inward-curling shoulder interpretation. The lower arms stay nearer the centerline.
- A shallow concavity at the shaft attachment, rounded lip, and slight proximal return produce an inward roll rather than a simple slope. Authoring envelopes: radial tuck -0.18, rounded lip +0.06, proximal return -0.12. The central frenular passage is protected from this roll to avoid a transverse rim across the V.
- The cap-envelope reconstruction now blends over parameter 4.65–5.65 instead of 5.25–5.65, with a wider, softer lateral falloff. This removes the abrupt local turn between the V and terminal slit. On the default-size central underside meridian between parameters 4.65 and 5.70, maximum adjacent-edge turning angle decreased from approximately 28.94 degrees in R4 to 4.52 degrees. This is a local mesh-smoothness measurement, not anatomical validation.

R4's raphe, root protection, shallow transition ripples, length and coronal advance are retained. The slit remains a closed surface recess, not an internal urethral lumen. Local fine faceting remains visible under cavity lighting, especially at maximum size; retained source-shaft faceting is unchanged.

Topology remains 10,554 vertices / 21,086 triangles. Size mapping remains 0 = 0.85x, 50 = 1.00x, 100 = 1.60x. R4 and R5 are compared at identical scales: five settings, four views, three shading modes, 120 images. Blender has a continuous Glans Size property; OBJ is exported at default 50.

Checks: finite/nondegenerate geometry, body-ring order and uniform fully weighted crown scaling across 41 sizes; zero edited open/nonmanifold edges; no nonadjacent overlaps involving edited faces in five BVH checks. Terminal recess rows are excluded from axial ring ordering. Before/after Blender controls are verified against the expected meshes. Preview script, 60 view/size/style combinations, file links and reset are checked. See geometry-audit.json, intersection-audit.json, blender-control-check.json, before-control-check.json, change-audit.json and preview-ui-check.json.

Construction recomputes the R4 recipe from R2, retaining R3's proximal attachment fields. The delivered R4 blend is the actual comparison baseline, and detail_displacement records R5 minus R4. Source scripts require the existing workspace inputs. Preserve the terminal reconstruction code for future edits; inherited meatal metadata alone does not reverse it.

The user's current red outline is preserved as corrected-silhouette.png. This is an authoring preview pending user review. No game launch, installation or game-file modification. UV/rig transfer, animation, physics and game integration remain unfinished.
