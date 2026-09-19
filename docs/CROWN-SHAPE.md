# Narrow attachment exclusion

User confirmed the problem occurs with Glans Size in the preview. The previous angular frenulum exclusion covered a broad sector of the underside of the crown. This revision measures lateral distance from the central attachment line in shaft-radius units instead: a pinned half-width of .055, blending to full scale at .180. The ventral and longitudinal gates are retained. The full crown receives the same scalar in all axes outside this narrow strip; no overall/width/length control behavior is changed.

Scale-mask checks in nine poses confirm exactly22 central attachment vertices fixed and538-540 rim/cap vertices scaling uniformly, with errors below0.00001 model units. Scale1 captures are byte-identical to the preceding candidate. The rest of the attachment is blended, so this is not a mathematically rigid scaling of every vertex including its fixed attachment.

27 sampled size/state/Glans Size cases are finite with no degenerate distal triangles.26 cases have no added detected intersections versus size1; the existing extreme size100/state1/GlansSize50 issue still has two added crossing pairs in the folded rim. This remains unresolved. Shaft through .70, pelvis and physics nodes remain fixed across the size control. Existing faceting remains.

Comparison uses matched old/new size samples in three views, solid or wireframe. Editable Blender model includes Glans Size, verified at1/50/100 against runtime positions. OBJ is static default50. Compiled candidate not installed or live-game tested; installedR2, package and settings unchanged.
