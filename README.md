# Wolverine Anatomy Tool 1.7

This release bundles the approved installed runtime, character revision 161, the menu tank body and camera work, body and floor fluid contacts, and the return-to-menu simulation fix. The gameplay world-space update keeps detached fluid and floor marks registered as the camera and character move. Runtime SHA-256 is recorded in manifest.json.

## Install and rollback

Close Wolverine, extract the full archive, and run Install.cmd (or Upgrade-1.7.cmd). The installer supports the original Natural, WGame and WStart packages or their exact approved modified versions. It verifies all payloads, applies local WBX deltas, and stores pre-install files in WGame/ModBackups/WolverineAnatomyTool-v1.7.0. Unsupported modified packages are rejected before installation. Rollback-1.7.cmd or Uninstall.cmd restores the previous files.

Existing WolverineLive.ini, TeachingFluid.ini and TeachingAudio folders are preserved. A fresh installation receives the approved fluid defaults and eight saved menu camera poses. The existing runtime reads camera poses from WolverineLive.log; the installer appends only the pose records when none exist. These saved camera settings remain after rollback. No diagnostic log, private study audio or source recording is bundled. The 22 established idle WAVs are retained.

## Controls and behavior

F6 opens the controls; F8 resets controls while the panel is open. J starts or cancels the teaching sequence. Menu camera playback and attract-video options are available in the panel. The temporary free-camera editor has been removed. Fluid settings reload between demonstrations from Binaries/TeachingFluid.ini.

The release includes the exact menu body retarget, pelvic weld, necklace contact, skin blend textures and title camera package. Menu-to-gameplay and gameplay-to-menu transitions reset scene-owned simulation and rendering resources while preserving controls. The sequence retains randomized overlapping feeds, illustrative material variation, a clear introductory strand and optional local audio cues. Optional mono PCM 16-bit 44.1 kHz WAVs can be placed in Binaries/TeachingAudio/Phase1 and Phase2.

Gameplay uses recognized baked world geometry to recover the camera origin and verifies the collision scale against rendered floor references. Missing fresh origin data pauses gameplay fluid; missing matching collision references withhold ground contacts. Unsupported surfaces are not inferred. Body contacts and temporary surface marks are visual approximations. This is an illustrative speculative biology tool; distances, volumes and dynamics are not clinically calibrated.

## Build and validation

Build from src/runtime using build.cmd with x86 MSVC and the June 2010 DirectX SDK. The packaged DLL is the exact user-approved installed binary. Build the regression executable from tools/teaching using build.cmd. Run sequence-test.exe --world-space, --world-origin-gpu, --menu-return and --render. Historical implementation documents retain the context of earlier candidates; use manifest.json and this release's validation notes for current identity.

Use tools/Test-Release17.ps1 with OriginalNatural, OriginalWGame and OriginalWStart paths to check clean installation and rollback. Supply -Upgrade -ModifiedNatural for the approved existing character package. The fixture never launches or modifies the live game. See docs/VALIDATION-1.7.md for results and remaining limits.
