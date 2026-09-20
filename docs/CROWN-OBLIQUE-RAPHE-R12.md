# Rounded side transition and defined V crest — R12

Retry from the actual R10 mesh after the user rejected the earlier R11 reconstruction. R12 is a local correction: a rounded positive lip and shallow proximal return follow the existing continuous crown path. The path is smoothed around the circumference and the offset blends into the original upper crown. The contribution retains its strength down the inner arms and fades near parameter5.40–5.80 at the tip; there is also a narrow centerline taper. The slit and terminal rows remain unchanged.

The side lobe receives 32 restrained arc-length-weighted fairing iterations, localized away from the upper rim and terminal slit. This rounds the transition introduced by earlier lateral compression. The underlying R10 convergence, overall proportions and mesh connectivity are retained. The result is intentionally a restrained refinement; subtle triangular highlights remain under strong cavity lighting.

Same10,554 vertices /21,086 triangles. Geometry checks pass at41 sizes; no edited open/nonmanifold edges. Five BVH samples contain no nonadjacent intersections involving edited faces. Shaft, terminal region and maximum axial extent are unchanged. See change-audit.json for measured displacement. ActualR10 Blender model is the comparison baseline; five sizes, four views, three shading modes produce120 renders. Control coordinate checks and the preview's60 combinations/reset pass. Blender has a continuous size control; OBJ is default50. Mapping remains0=.85x,50=1x,100=1.60x.

Preserve custom normals in Blender/OBJ or reconstruct with the included renderer. The JSON alone lacks fitted shaft normals. Source scripts require existing workspace inputs. The two user sketches are included.

Authoring revision pending user review, not a medically validated asset. R11 remains rejected; R10 is preserved. No game launch, installation or game-file modifications. UV/rig transfer, physics and runtime integration remain unfinished.
