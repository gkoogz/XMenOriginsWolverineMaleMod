# Extended underside groove and fuller rounded crown — R2

Based directly on the user-approved direction in crown-oblique-raphe. This revision makes five local changes while preserving the smooth primary shape and the existing size mapping:

- Extends the ventral V-like depression with a narrowing rounded channel to the lower end of the existing meatal slit. The channel follows the surface meridian normal. Its axial recession is smoothly combined with the existing slit instead of stacking two depressions. Local longitudinal fairing softens the broad-to-narrow junction.
- Advances the ventral coronal margin by up to another 0.78 model units, with a broad onset and distal fade. The upper edge remains substantially fixed. The broader onset avoids ring reversal at the smallest size.
- Increases the raphe relief envelope from 0.58 to 0.79 model units and its proximal width parameter from 0.88 to 1.08. The ridge stays integrated into the surface and retains a long distal taper.
- Adds up to 0.84 model units of axial cap length, distributed gradually from the coronal region toward the tip. This lengthens the cap rather than pulling only the terminal vertices.
- Adds a rounded shoulder flare of up to 0.22 model units, subdued along the central frenular attachment to avoid a transverse lip.

These are design parameters in model units, not physical measurements. The external slit is still a closed recess; this is not an internal urethral reconstruction or a medically validated surface.

The control mapping is unchanged: slider 0 = 0.85x, 50 = 1.00x, 100 = 1.60x. The Blender control is continuous; the comparison provides five rendered settings in four views and three shading modes, totaling 120 images. Both models use matching physical scales. OBJ is exported at the default slider 50.

Topology is unchanged: 10,554 vertices and 21,086 triangles. Forty-one sizes passed finite/nondegenerate geometry, ordered body rings and uniform scaling of the fully weighted crown. Axial ordering excludes the intentionally recessed terminal rows. No open/nonmanifold edges in the edited region. Five BVH checks found no nonadjacent overlap pairs involving edited faces. Both before/after Blender control samples are checked against their expected mesh positions. These checks do not validate skeletal animation or physics.

The previous artifact is preserved. Source scripts in sources/ require the existing workspace dependencies and inputs; model-data.json contains the resulting positions and topology. The game was not launched or modified. This remains an authoring preview; final UV/rig/physics transfer and game integration remain incomplete. Some original shaft faceting persists outside the revised region. Visual acceptance of this R2 pass is pending user review.
