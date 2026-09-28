# Upright neck render surface

The approved junction is a local implicit trumpet union remeshed inside a bounded patch and stitched into the unchanged surrounding skin. The established dynamic surface remains the deformation cage. neck_render_data.h publishes a final render surface for both gameplay and tank; fluid collision uses this same final surface.

Run `python export_neck.py` with NumPy and SciPy to regenerate the header from authoring inputs. The input bindings reproduce the approved rest mesh with affine coordinates and an exact original-surface border. Original retained vertices copy all packed attributes exactly; a continuous boundary-constrained UV solve uses the deformed source UV guide. Zero-weight bone slots must name a valid palette entry.

Build with `build.cmd`, then run `neck-test.exe --test`. Run the harmonization test after changes to verify tank/gameplay/body-field/seam/material parity and final collision positions. No model or fluid compiler runs at game startup.

Authoring experiments are not runtime code. The installed shape must be identified by the DLL receipt, not an executable or preview left in the study directory. Offline tests do not prove live performance or appearance.
