# Shared surface regression

Run build.cmd and harmonization-test.exe from this directory. Put the two
SharedBody-*.dds files from the deliverable beside the executable for material tests.
No game process is required or started. The D3D test window stays hidden.

The binary fixtures are packed 32-byte vertices exported read-only from the
installed CH_Wolverine_Natural and WolverineNudeMenuMesh assets on 2026-09-28.
They are the same geometry used by the current local mod, including its edits.
The palette fixture maps tank compact bones to gameplay bone names.

Tests cover 18 scene/control/deformation cases, all 30,717 final anatomy vertices and
20,783 body vertices, 60 synthetic bone poses at each of 20 collar pairs, an
idempotent body field, the production collision consumer, all 16 rigid electrode
islands, adaptive body-patch density, and real-device shader/texture state restore.
The synthetic bone poses are not a live animation capture.

The sharedBodyDeform hook receives the reset, full model-space donor and final
anatomy buffer. A future field must deform both affected surfaces, including
their tangent bases, with a continuous transition. The final boundary weld is
reasserted before publishing body output. Native scene bone palettes remain
separate adapters, so the two scenes retain their own animation and cameras.

Electrode anchors transport the original rigid geometry with a body triangle's
barycentric point and tangent frame. Native skin weights and materials remain
unchanged. Rest placement is byte-identical. This is a morph adapter, not new
electrode rigid-body physics. The separate tank_hose_wx static mesh and its 11
instances retain native placement; endpoint rebinding is still required with
future chest/glute changes. No new chest/glute slider is introduced here.
