# Ridge refinement authoring pass

Restores prominence lost in the smoothing pass with a compact C2 radial profile, maximum 0.42 model units at size 1 and 0.672 at size 100. The profile has zero slope and curvature at its ends and uses the existing crown mask to protect the underside exclusion. Shaft-side control positions remain exactly identical to crown-smoothed. Reduced final Smooth modifier from factor 0.3/five iterations to 0.15/two iterations to retain projection. Area-weighted normals reduce shading sensitivity to narrow triangles.

All five generated control samples are finite, have no zero-area faces at threshold 1e-10 and preserve shaft control positions. Blender shape-key samples agree within 0.0001 units. These checks do not establish self-intersection freedom or animation compatibility. Existing seam corrugation remains visible; this restores the ridge but does not finish the topology cleanup. Preview has three angles, solid/wire and five size samples; Blender retains the continuous size property.

Authoring-only: no runtime source, mesh package or installed game changed. UVs and original source correspondence remain in the control mesh. Existing crown-smoothed and accepted R2 are preserved.
