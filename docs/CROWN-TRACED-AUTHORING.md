# Traced crown boundary — authoring prototype

Matched the supplied screenshot to the scale1 lateral render. The fitted screenshot scale is0.858203 with offset(8.921,73.919) pixels; mean absolute grayscale fit error0.24 levels. The red polyline was extracted and smoothed to remove pixel stair-stepping, then projected into the neutral model's lateral view. The curve is extended through the lateral depth of the mesh; a single image cannot define the unseen side independently.

The scaling mask is zero on the shaft side of the traced line and reaches full influence in a0.10-model-unit crown-side strip. Two local subdivision passes and three boundary/support cuts increase the raw graft from2388 vertices/4596 triangles to12645 vertices/25034 triangles. Added vertices retain sparse barycentric correspondence and interpolated UVs. A narrow0.16-unit shoulder and constrained local fairing make the crown-side rise steeper and reduce unevenness. The shaft-side coordinates remain exactly fixed.

The Blender model has a working Glans Size1-100 property; its five samples match generated geometry within0.0001 model units. Five samples have finite coordinates, no zero-area faces at the1e-10 threshold, and zero shaft-side displacement. Small sliver triangles and visible creasing remain at maximum size. Self-intersection freedom, rig deformation and in-game performance are not validated. This is a reviewable authoring prototype, not a finished game-ready mesh.

The larger topology has NOT been integrated into the runtime/package. No new DLL is supplied for it. Existing runtime tables assume2388 graft vertices; integrating this authoring mesh requires regenerating its mesh package, morph/physics/collar/normal tables and draw signatures as a matched set. The installed0.7.2R2 and latest prior compiled candidate remain untouched. model-data.json and vertex-correspondence.json preserve geometry, samples and original-vertex mapping for that work.

Before/after views compare the previous undercutR2 at matching slider values. New renders are authoring-model studio renders, not runtime captures. Only the local graft is included; the upper body is absent.
