# Local pelvic ramp update

Installed and checksum-verified. Settings and all22audio files preserved. See installation-receipt.json for the exact payload and backup.

Broadens the existing lateral pelvic ramp and slightly increases forward prominence at large diameters. Baked material weights ease the attachment into shaft motion instead of abruptly rotating/projecting proximal rows. Additional recruitment fades to zero below the neutral diameter. No new mesh vertices, triangles, live passes, physics steps or solver iterations. Root ownership and rest tangents are cached instead of recomputed every frame. The raphe, glans, pouch sculpt, settings and audio assets were not edited.

The intermediate-size regression from the first candidate is fixed. Captured neutral and minimum cages, body patches and packed render meshes are byte-identical to the installed baseline. Strict local triangle-interior crossing pairs, beforeâ†’after: saved large1â†’0; maximum90â†’0; moving maximum89â†’0; intermediate21â†’21; minimum278â†’278; firm216â†’179. Shared boundary contacts are excluded. Existing neighboring skin folds remain in some poses; this is not an all-pose/global intersection-free claim.

Six final D3D replays passed geometry/contact/tether/state restoration/reset/recreation checks. Six150-frame performance scenarios and240-frame control/state/reset transitions passed finite-position, sampled triangle-area and paired-seam checks. Default-motion, minimum and firm/default benchmark meshes and body buffers matched the baseline for150frames. The wider firm replay uses different dimensions and improves its existing crossings. Pulse, glans/menu/persistence,10000surface-limit tests and both binding-generator checks passed.

No added per-frame pass or mesh density; repeated CPU timings are within observed variability. Alternating default-motion median24.59â†’24.36ms; maximum-motion24.91â†’25.28ms, with overlapping run ranges. First-pass maximum measured25.74â†’25.33ms. These measurements do not establish a real slowdown or speedup; no in-game FPS claim. Raw timings are included.

Update.ps1 installs the hash-verified DLL and preserves an exact rollback. Update.ps1 -Rollback restores the prior 8E44 suspension-relaxation build. Before/after renders use captured runtime positions and normals, with identical cameras and no additional smoothing/subdivision.
