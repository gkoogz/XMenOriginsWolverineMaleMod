# Rounded ridge authoring pass

User requested a rounded but prominent rim and a smoother shaft-to-ridge curve. Broadens the compact C2 profile support from 1.85 to 2.8 model units and lowers its radial amplitude from 0.42 to 0.35 at size 1. Amplitude scales proportionally to 0.56 at size 100. A ridge-weighted Smooth modifier (factor 0.5, 60 iterations) rounds the crest without globally smoothing the crown. Solid and wire overlays use identical modifier parameters and shared weights. Area-weighted normals retained.

All five control samples are finite, have no zero-area faces at threshold 1e-10, and preserve shaft-side coordinates relative to crown-smoothed. Blender control samples match within 0.0001 units. Visual review at default and maximum size shows a softer outline with a raised rim. Significant inherited seam corrugation remains; this pass does not establish a completely smooth anatomical transition. Self-intersections, rig behavior and game performance have not been validated.

Separate authoring preview only, with continuous Blender size control and five-sample comparison against crown-ridge-refined. No runtime source or installed game changes. Previous models preserved.
