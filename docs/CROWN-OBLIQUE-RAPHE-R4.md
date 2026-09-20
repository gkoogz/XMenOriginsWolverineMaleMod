# Curved ventral folds and structured raphe — R4

Changes requested from the delivered R3: replace the straight-looking underside V with organic sweeping curves, define the raphe as a specific rounded structure, and add slight ripples at the shaft-to-glans transition.

- Bowed paired valleys use a shape-preserving spline: their half-width expands from 1.25 to 1.72 model units around the shoulders, then narrows to zero at the tip. The groove cross-section is broadened to keep the curves soft. Limited local fairing reduces accumulated transverse corrugation before the new folds are added.
- The raphe has a rounded crest and clearer longitudinal shoulders. Its free-shaft height envelope is 1.10, width parameter 1.20, profile exponent 2.8, with 0.085 shallow flanking relief. The established proximal attachment is preserved; the new profile fades in over four model units to avoid nearby source-surface intersections.
- The narrower frenular bridge has a 0.31 height envelope and continues into the same terminal channel. R3's terminal cap fit/recess reconstruction is retained.
- Three shallow, curved folds near the crown use unequal spacing and amplitudes (0.10, 0.12, 0.085 authoring units). They fade across the central attachment to avoid a horizontal ring across the raphe. Their relief is intentionally subtle.
- R3's length, coronal advance, outer cap and flare are retained. The topology is unchanged at 10,554 vertices and 21,086 triangles. Local shading facets remain visible at the largest size under cavity lighting, and retained source-shaft faceting remains.

The latest supplied stylized reference guided curves qualitatively, not anatomical measurements. These are design parameters, not physical units or clinical validation. The slit is still a closed surface recess without an internal urethral lumen.

Size mapping remains 0 = 0.85x, 50 = 1.00x, 100 = 1.60x. The page compares actual delivered R3 with R4 at five matched physical scales, four views and three shading styles (120 images). The editable Blender file has a continuous 0–100 Glans Size property; OBJ is the default size 50.

Validation: 41 intermediate size checks pass finite/nondegenerate faces, ordered body rings and uniform fully weighted crown scaling. The intentional inward terminal recess is excluded from axial ring ordering. No edited open/nonmanifold edges; five BVH checks find no nonadjacent overlaps involving edited faces. Both Blender control evaluations match their expected sample meshes. The actual page script is checked at all 60 setting combinations and reset. See the JSON audit files for results. Animation, UV/rig transfer, physics and game integration are not covered.

Construction: R4 recomputes the R3 sculpt recipe from R2 with revised fields, instead of stacking another V cut on R3. Its comparison baseline is the actual R3 blend; detail_displacement records differences from R3. Sources require existing workspace inputs. Future edits should use final positions or the reconstruction script, not assume inherited meatal metadata alone reverses the terminal recess.

Previous outputs are preserved. No game launch, installation or game-file modification was performed. This remains an authoring preview pending user review.
