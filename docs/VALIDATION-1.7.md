# Release 1.7 validation — 2026-09-28

The packaged DLL is byte-identical to the user-approved installed runtime:
A0EACEAB5DAB3F8EC8892B4388FC4EDCF4436FA74400DBB6CD5D0B951A38E8E2.
Runtime source and generated geometry headers were copied from the corresponding authoring tree. The regression executable compiled successfully from this release checkout.

Passed:
- World-space: 360 moving-camera/moving-actor projection frames; declaration/transform rejection; 1:50 and 1:1 scale calibration; distinct-reference requirement; stacked floors; exact ledge rejection; scene reset.
- World-origin GPU: real D3D geometry/shader capture; generic zero CameraPosition cannot replace or refresh origin.
- Menu return: visible tank mesh updates with a retained gameplay buffer.
- Render: captured shader reflection, emitter transforms, pipeline restoration, duplicate-pass suppression, device reset/recreation.
- Clean installer fixture: all three stock packages reconstructed to manifest hashes, runtime/textures/idle payloads verified, camera defaults seeded, existing settings preserved and managed files restored exactly.
- Upgrade fixture: all three already-approved packages accepted; runtime upgrade and rollback succeed.
- Existing custom fluid settings preserved through installation and rollback in both fixtures.

Installer fixtures use a placeholder executable and mock only the process lookup; the production process guard remains intact. They do not modify the live game. Camera pose records are retained settings and are deliberately not removed on rollback.

Live evidence: user confirmed return-to-menu and approved this build. The latest game log records floor scale 0.02000 and verified world contact at (1548.40,29193.41,3144.00). This is evidence from the tested scene, not all chapters or surfaces. Missing supported BSP/collision reference data can pause fluid or withhold contacts. Performance across all hardware and levels is unverified.

No private phase audio, raw diagnostic logs, full game packages or prior release archives are included in the new bundle. The existing 22 public idle clips are unchanged. This is an illustrative visual model without clinical calibration.
