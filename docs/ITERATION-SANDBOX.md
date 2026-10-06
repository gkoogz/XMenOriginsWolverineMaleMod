## Reproducible tracked delta package, October5

Start with dev/sandbox/README.md and tools/iteration/README.md. The complete
source-only recipe now initializes a supported vanilla owned clone, compiles
its external authoring dependency and private runtime, regenerates native
layers, and seeds an engine-generated local reference checkpoint using typed
numeric controls. No earlier developer save is required or distributed.
A clean27-source recipe copy passed final native startup and whole-body grey
floor visual review; human storage and exact local contract preflight pass.
See SANDBOX-REPRODUCTION.md for failure history, proof and limits. Older notes
below describe the original workspace, not an additional prerequisite.

# Wolverine native iteration sandbox

## Current checkpoint, 2026-10-05

A real Raven UE3 gameplay prototype runs on an owned, noninteractive Windows
station. Wolverine stands and walks on an authored flat grey native floor under
one directional key and a soft native hemispherical fill. Five simple grey
panels replace the black background. Native pawn animation, the installed anatomy solver,
prototype garment, F6 overlay and live Overall control have been observed.
Normal retail installation, settings, audio and saves are untouched.

Workspace: `E:/MaleModBuilds/wolverine-sandbox-capability-20261004`.
The executable and authored packages reside in `owned-game`; no cooked game
assets, profiles, captures or compiled binaries belong in Git.

This is a reusable **native prototype**, not a standalone cooked empty map.
It retains the stock `jungle1_p` checkpoint bootstrap and its required streaming
definitions. Authored replacements remove the loaded environment, sky and NPC
actors; 812 native sequence input links are disabled. The floor/light layer has
32 exports, 48 floor triangles and five backdrop panels reusing that geometry,
inside a 32 KB native compressed envelope. Seven
other loaded visual layers are 32 KB empty actor layers. They preserve the
native Level/World envelope and hard-reference indices.

## Repeat an iteration

### Studio lighting update

The installed native studio layer has shadowless upper/lower SkyLight fill at
0.35, using measured Raven Engine classes/properties, and retains the original
0.12 directional key with one grounding shadow. Five neutral grey native mesh
panels enclose the observed player/camera views. The actor/collision/mesh
bindings of the original floor remain intact; the backdrop never enters pawn
or rigid-body collision. No anatomy, garment or numerical solver code changed.

Native back/underside renders improved and the black background disappeared.
The private test also walked the actual grounded pawn, closed its exact child
with status zero and restored its exact audio session with HRESULT zero.
Captures `studio-verification-20261005-090751/native-frame-606.png` and
`native-frame-799.png` live under the C: state directory. Present samples were
51–64 ms against earlier approximately 59–63 ms; these were different animation
samples, not a controlled GPU benchmark. A full front orbit was not established
in this test, so no universal lighting claim is made.

Root visual review accepted this candidate. It was installed only after both
owned human/sealed processes were absent. Native stage SHA-256 is
`41B9764DAC2F1E85584FC19BDAC00867288237B179F4D4CA31E0FF9C693F5D2A`.
The previous layer is preserved on C: as `stage-before-studio.xxx`, SHA-256
`C84DF9CE0D73089C51F2A4DD9BAE49EFD04D60F5C73AA6C03085678623A5046B`.
The same Desktop shortcut validates the active stage contract. Its guarded
pending-stage application refuses a live owned child, checks before/after
hashes, and does not touch retail assets or preferences.

`author_native_layer.py --name jungle1_zone01a --studio --output-directory <C-path>`
and `pack_private_layer.py jungle1_zone01a <C-path> --offline` reproduce the
accepted 32 KB bytes without installation. The accepted package GUID is frozen;
future scene versions must choose a new explicit GUID and verification contract.
`Prepare-NativeSandbox.ps1` now defaults to this studio recipe. Avoid running
its broader reauthoring process on nearly-full E:; the small C: offline recipe
is sufficient for lighting iteration.

### Play it yourself

The Desktop shortcut **Wolverine Grey Sandbox** opens the owned stage with real
keyboard, mouse and controller input. Press Enter at the title, then Continue.
The launcher restores only the owned test checkpoint before each launch. Normal
retail saves/settings remain separate. The game opens in a 1280×900 window;
close it through its menu or window. There is no automatic pause, lifetime timer,
debugger or audio mute in this mode.

F6 expands/collapses the installed mod menu. Up/Down select, Left/Right adjust,
and Shift uses larger steps. WASD, mouse and the normal gamepad use native input;
Space jumps. The owned camera extras are R/T orbit, Y/U pitch and V zoom. Naked
and Jockstrap are the installed prototype controls, not the unfinished dirty
cloth implementation. Settings changes remain in the owned Binaries directory.

```powershell
./tools/iteration/Install-PlayableSandboxShortcut.ps1
./tools/iteration/Play-NativeSandbox.ps1 -ValidateOnly
./tools/iteration/Play-NativeSandbox.ps1
```

The direct launch clears all sealed-driver environment flags. The same verified
runtime then uses its original classic D3D9 path; the private input shim,
activation, capture and pause hooks are inactive. Human launch is separate from
the sealed Open/Close tools below. Concurrent use of the owned executable is
refused. Ordinary game audio is initialized and no mute is applied; private
voice-pool assets were not copied.

The human executable, DLLs, configs, private saves, caches and logs now live in
`C:/Users/Administrator/AppData/Local/MaleMod/WolverineSandbox/human-game`.
`Prepare-PlayableSandboxStorage.ps1` copies approximately 309 MB of binaries and
small config trees; large package/movie/shader-source directories are junctions
to the existing owned assets on E:. No game package or original retail file is
changed. Human SaveData/Logs are independent directories on C:, separate from
the sealed driver's mutable outputs. At least 4 MiB free is required on C:
before launch; human mode has no E: free-space prerequisite. Receipts `storage.json`,
`shortcut-verification.json` and `launch-validation.json` live in that C: state
directory; each actual human launch also writes `human-play.json`. A launcher
failure persists `launch-error.json` and displays that exact error to the user.

The shortcut target, working directory, icon and visible window style were read
back and checked, and the exact target Windows PowerShell validates the assets.
No interactive game window was opened over the user during this setup. Prior
sealed runs establish the actual native stage/player/installed controls; normal
desktop performance and physical interaction await user play. The 15.8 FPS
sealed measurement below is not a prediction or verification of human mode.

The first shortcut launch failed **before CreateProcess**, because free E:
space had dropped below its former 4 MiB guard. No human launch receipt or new
runtime log existed, and invoking validation reproduced the exact guard error.
That guard was a launcher failure, not an observed native CTD. The C: runtime
replacement was tested on a sealed station using the same installed runtime:
actual native floor/player, walking, F6 and Overall 50→51 were observed. Native
capture `verification-20261005-083720/native-frame-1200.png` and guard receipt
live under the C: state directory; exact child PID23508 exited with status zero.
This native test uses private D3D9Ex; the human route uses unchanged classic
D3D9/input. At 12:45:06 UTC the user opened the corrected shortcut: exact human
PID22672 initialized classic D3D9 at 1280×900, transitioned to anatomy gameplay
scene0, and connected the real character graft buffer at vertex47050. Its native
physics/render log continued advancing. The agent did not launch over the user
or send physical input. The private diagnostic child had already exited;
the live human process was left running for the user.

`launch_capability.cpp` now retains exact audio-session COM identities it changes
from unmuted and restores them on both success and exception cleanup. The updated
helper compiled on C: as `launch_capability_restore.exe`; the older E: helper
was not overwritten. Sealed Start now defaults to that C: restoration helper,
not the older E: executable. Native gameplay is verified with the previous
helper; the restoration revision is compiled, while root separately observed
exact-session restoration. Human PID22672 inherited the earlier private test's
cached mute: root restored only its verified path/PID/creation audio session,
then independently confirmed that session was unmuted. Receipts
`human-audio-verification.json` and `human-audio-after.json` record one owned
session and no endpoint/other-application change. Never change endpoint/master
volume or reuse the older unbalanced helper for human-runtime diagnostics.

### Sealed agent iteration

Run these from the canonical repository. Open starts a hidden launcher that
owns only its exact private child. It restores the private checkpoint, warms
the real simulation for 90 gameplay frames, then pauses rendering/simulation.
It closes after ten minutes unless closed sooner; `-Active` requests continuous
simulation. The limit is configurable up to thirty minutes.

```powershell
./tools/iteration/Open-NativeSandbox.ps1
./tools/iteration/Get-NativeSandboxStatus.ps1
./tools/iteration/Set-NativeSandboxInput.ps1 -Key Resume
./tools/iteration/Set-NativeSandboxInput.ps1 -Key W -Action Hold
./tools/iteration/Set-NativeSandboxInput.ps1 -Key W -Action Release
./tools/iteration/Set-NativeSandboxInput.ps1 -Key TurnLeft -Action Hold
# Release TurnLeft after the desired orbit; LookUp/LookDown adjust native pitch.
./tools/iteration/Set-NativeSandboxInput.ps1 -Key TurnLeft -Action Release
./tools/iteration/Set-NativeSandboxInput.ps1 -Key Overall -Value 100
./tools/iteration/Set-NativeSandboxInput.ps1 -Key Jockstrap
./tools/iteration/Set-NativeSandboxInput.ps1 -Key Capture
./tools/iteration/Set-NativeSandboxInput.ps1 -Key Pause
./tools/iteration/Close-NativeSandbox.ps1
# Reset restores the private checkpoint and requests installed defaults.
./tools/iteration/Reset-NativeSandbox.ps1
```

Supported keys include W/A/S/D, Space, Shift, F6, F8 and arrows. TurnLeft,
TurnRight, LookUp and LookDown feed Wolverine's actual native gamepad camera axes.
Zoom uses an observed native camera input binding. Defaults, Overall, Naked and
Jockstrap call the installed control implementation, preserving its mappings.
Commands are read by
the private engine driver from its own INI; they do not use SendInput, switch
desktops, or mirror the host keyboard. Input is gated on observed gameplay.
Captures, runtime logs, process receipt and timeout diagnostics land in
`runs/<timestamp>`. `-AcceptanceControls` runs the observed walk/menu/size sequence.
The command helper waits for the previous acknowledgement before replacing the
single command slot. Paused Capture saves the current actual backbuffer; changes
to size/clothing while paused appear after Resume. Test-NativeSandbox runs the
bounded walk, camera, size, clothing, F6 and pause/resume cases and stores receipts.
Timeout status 124 is expected scope cleanup, not an engine crash.

The bootstrap advances the native title and the **owned test profile's** grounded
checkpoint. Its historical environment label `new-game` does not imply the
user's ordinary Continue/save. The test profile lives exclusively in
`owned-game/WGame/SaveData`; no normal profile was copied.

## Author/rebuild

`Prepare-NativeSandbox.ps1 -Workspace <workspace>` reconstructs the small native
layers from measured Raven v568/licensee101 exports and checks exact source
hashes. It refuses a running owned child and writes only the private clone.
It requires the existing initialized clone, original donor backups under
`native-stream-room`, and private `lzo_read.dll` used by the parser. That helper
was compiled from the read-only UEViewer LZO reader source; it is not shipped
as a production mod dependency. Offline parse and native gameplay are separate
verification stages.

`build_private_runtime.py --output <fresh-path> --base <Base-repo>` exports clean
canonical HEAD and its pinned Base include files. It builds a private graphics,
focus, capture and control derivative. Dirty garment development is excluded.
The output provenance records all source/helper/build/runtime hashes.
`launch_capability.cpp` builds with MSVC x64 and dbghelp/user32/ole32/uuid.
It creates and cleans an exact noninteractive station/child, traps native
errors, and mutes only that child's audio sessions. It never mutes an endpoint.

Runtime source is `afe8647fdce7fa4c0f0b593cf48e61ed72156535`, whose implementation
is byte-for-byte the installed source at `16affd49da5a122df776969574ca62a984b59425`.
Base is `99ff741ea95f18ed84526c35a6c3f38a37d857b6`. Installed retail DLL SHA-256:
`4F4900A5E70CE9BB127B46C3D7FE57EF74FFEACE8A5AE704D3DA1EFF6E2BA9A3`.
The private renderer uses D3D9Ex because classic D3D9 reports NOT_AVAILABLE on
noninteractive stations. Managed-pool allocations are translated only there.
No verification bypass or production graphics change was made.

## Evidence and limits

Observed captures: `native-final-motion-frame-409.png` (walking),
`native-final-motion-frame-545.png` (F6, Overall 51), and
`native-final-motion-frame-1200.png` (settled native garment/player).
Observed source states: Full Floppy=2, Clothing=1, Overall 50→51.
First observed gameplay was 18.656 seconds after runtime attachment.

The installed high-resolution solver remains CPU heavy. Initial default-size
samples were approximately 15–17 ms physics and 55–57 ms inclusive overlay.
Earlier daily runs measured roughly 8 ms physics, 89 ms inclusive overlay and
native frame intervals of 168–284 ms. Reasserting native WM_ACTIVATEAPP and
WM_ACTIVATE on the observed gameplay transition improved the final daily9
interval to **63.22 ms (15.8 FPS)**. Activation is sent only to the verified
owned noninteractive HWND; no host foreground/window is touched. These
overlapping CPU timings must not be added. Original Present cost was 0.6–2.4 ms
and the private INI poll 0.11–0.14 ms. The installed implementation remains
heavy; no 60 FPS or GPU timing claim is made.

Pause reduced exact-child CPU from 1.08–1.09 CPU seconds per wall second to near
zero in two-second samples. Open's automatic warmup pause and exact-handle Close
make idle use inexpensive. File-I/O debugger tracing was accidentally enabled by
the literal environment value `0`; it now requires exactly `1` and is disabled
for ordinary runs. The small authored scene does not remove native startup
dependencies: the existing owned clone is 7,538,436,249 bytes across 1,038 files;
the eight tiny scene layers plus story/persistent carrier total 15,597,568 bytes.

Daily evidence: `runs/20261005-055821-8f210c/verification.json` contains walk,
camera, clothing, size endpoints, live menu and CPU cases. Its frame393 shows a
changed native camera angle. `runs/20261005-060427-ae5c45/native-frame-372.png`
shows the native front view. `runs/20261005-062147-b1b2ca` verifies deterministic
reset, automatic pause, paused Capture, Resume and Close with the final daily
runtime. Its first observed player was **19.262 seconds after exact child
creation**. Paused capture returned HRESULT zero, and Close exited the child
with status zero. Current build provenance is
`private-runtime-daily9/provenance.json`; final runtime SHA-256 is
`02a654ba91b7aac7be3a8e9ef958ef61b178210a028bb50224efc626ad693fde`.

The floor is a native StaticMesh, whereas the existing fluid world-origin
observer derives camera/pre-view origin from native BSP. Consequently this
stage has **not** verified fluid floor contacts or the newer dirty persistent
cloth implementation. A future adapter change must observe the exact authored
floor transform or supply real BSP, not invent camera/world coordinates.
Run, extreme rest-angle cases, all cloth collisions and the full shared
scene metre/camera contract remain unverified. The original convex collision
cache is retained alongside flattened triangles; observed grounding/movement
does not establish exact surface contact at every point.

Earlier standalone custom-map loading stalled in native LoadingMovie lifecycle;
the retail `CookPackages` commandlet was not available. An initial unguarded
clone emitted R6025; its specific cause is unproven. Current successful guarded
bootstrap handles error UI privately. Runtime `SET` visibility experiments were
rejected before execution and were removed; authored package loading supplies
the scene instead. Do not revive those rejected experiments.
