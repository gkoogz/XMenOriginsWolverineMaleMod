# Wolverine Anatomy Tool v0.8

This release packages the current R33 anatomy runtime and the established Revision 161 character package. It adds the integrated R14 surface, expanded size controls, continuous fixed-step dynamics, ovoid internal supports, constrained shared-sack motion, and distinct floppy, semi, and erect response profiles.

## Install or upgrade

Close Wolverine and extract the whole archive. For an unmodified game, run **Install.cmd**. For an existing v0.7-series installation, run **Upgrade-0.8.cmd**. The default location is `C:\Games\X-Men Origins Wolverine`; pass `-GamePath` from a command prompt for another location.

The installers verify checksums, preserve settings, and make version-specific backups. **Rollback-0.8.cmd** restores the runtime replaced by the upgrade. F6 displays **R33 DISTINCT STATES**.

The project is intended for adult anatomical visualization, postgraduate medical-education demonstration, geometric analysis, and biomechanical experimentation. It is not a diagnostic or clinical-decision system.

## Major changes

- Integrated high-density R14 surface with regenerated runtime correspondence.
- Remapped Glans Size range with a smaller lower half while retaining the previous range above midpoint.
- Expanded low-end shaft Length range while preserving the crown at small values.
- Fixed-step shaft and suspended-mass solver with continuous state transitions.
- Ovoid support volumes and closer, constrained independent motion inside the shared sack.
- Separate floppy, semi, and erect stiffness, damping, weight, and root-spring profiles.
- Captured-material lighting correction with original texture bindings.

Build `src/runtime/build.cmd` with x86 Visual C++ BuildTools and the June 2010 DirectX SDK. The release payload is the exact installed and verified R33 DLL.

## Validation and limits

Deterministic state-profile tests, continuity tests, long physics regressions, render-path checks, device-reset checks, installer tests, and checksum verification pass. The three states produce distinct measured bend and root-swing behavior.

This is a research and visualization tool, not medical validation. Extreme-pose collision clearance remains imperfect, texture fidelity work is unfinished, and the newly reported ventral junction notch is documented but not repaired in this release.
