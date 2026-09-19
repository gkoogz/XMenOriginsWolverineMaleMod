# Glans Size study

The head has a larger, broad-shouldered, rounded spade silhouette. This is a neutral speculative-anatomy study built on551b6c7, not a validated reconstruction of the photographed individual. The supplied photograph is not embedded or used as a texture.

GLANS SIZE appears after WIDTH in the runtime menu. Range1-100, default50. Left/right changes one unit, Shift changes five; F8 resets to50 with the other controls. Settings save as Shape / Glans Size in WolverineLive.ini. Older settings without this key load50 and retain their other values.

Size1 retains the preceding contour-study head. At50, fully influenced head vertices use1.40x transverse width,1.26x depth and1.60x axial scale about the attachment frame. At100 the scales are1.60x,1.39x and1.90x. The short attachment transition blends between authored coordinates .79 and .845; shaft vertices at or below.79 and all suspended scrotal skin are excluded. This is a visual shape control; it does not increase physical mass, solver-node length or collision-rig dimensions. Bounds were reduced from an initial2x transverse scale after detecting new crossings at extreme combinations.

In Blender, select "Glans Size - adjustable study", then Object Properties > Custom Properties > Glans Size. The1-100 property drives the shape key and the wire overlay. Driver results at1/50/100 were checked against runtime captures. The OBJ is a static default50 export. No skeletal rig is included in the Blender study.

The browser preview uses five actual runtime samples (1/25/50/75/100); the game and Blender controls accept intermediate values. Three matching angles, solid/wireframe,36renders. Controls other than Glans Size are50, physics state2,240deterministic frames. Renders use Blender lighting and are not gameplay screenshots. The source photograph is not part of the deliverable.

Validation:27cases combining overall-size presets1/50/100, states0/1/2 and Glans Size1/50/100. All positions finite,0degenerate distal triangles,0added detected intersection pairs relative to Glans Size1 in each matched pose. Shaft<=.79, pelvis and physics nodes are unchanged; measured head width grows monotonically. Existing intersections/faceting remain and some triangle-quality metrics worsen as the head stretches. These checks do not cover every independent slider combination or prove medical accuracy. UI tests cover all18rows,clamping,save/reload,missing-key migration and reset.

The compiled candidate is runtime-preview/d3d9.dll, labelled0.7.2R2 GLANS STUDY. Not installed or live-game tested. Installedgame remains accepted0.7.2R2; package/settings unchanged. build.json contains hashes, geometry-audit.json has case metrics, and glans-size.patch contains runtime/harness changes.
