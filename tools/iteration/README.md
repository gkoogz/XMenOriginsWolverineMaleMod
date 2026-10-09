# Wolverine grey room development package

This tracked directory is the complete **delta recipe**, not a game download.
It authors eight tiny native layers and a quiet checkpoint carrier from the
owner's stock Raven v568/licensee101 packages. It preserves real animation,
anatomy, input and physics. The grey walls and fill are not the original jungle.
The native stock `jungle1_p` bootstrap is still required: this is not a cooked
standalone empty map or a claim of cheapest possible engine startup.

## Reproduce from vanilla

Use Windows x64, 64-bit Python 3.10+, PowerShell5.1+, Git, Visual Studio C++
Build Tools with Windows SDK (x86 and x64), DirectX SDK June2010 and7-Zip.
Install the game's legitimate dependencies/PhysX normally. The stock executable
and Raven packages must match `Initialize-VanillaSandbox.ps1`; unsupported
editions are rejected before copying. No bypass of game authentication occurs.

1. Clone this repo and MaleModBase. Download/extract the complete supported mod
   Install release to a separate directory; verify its SHA256SUMS and manifest.
   This is required for the three character/menu deltas, textures and public
   defaults. Never copy private recordings, saves or preferences into a clone.
2. Run from this repository (choose new writable paths with at least10GiB free):

```powershell
./tools/iteration/Initialize-VanillaSandbox.ps1 `
  -GamePath 'D:/Games/Wolverine' -Workspace 'D:/Dev/WolverineGrey' `
  -ReleaseDirectory 'D:/Downloads/WolverineAnatomyTool' `
  -BaseRepository 'D:/Dev/MaleModBase' `
  -VcVars32 'D:/BuildTools/VC/Auxiliary/Build/vcvars32.bat' `
  -VcVars64 'D:/BuildTools/VC/Auxiliary/Build/vcvars64.bat' `
  -DirectXSdk 'C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)'
./tools/iteration/Create-SandboxCheckpoint.ps1 -Workspace 'D:/Dev/WolverineGrey'
```

The latter launches only owned noninteractive children. The engine generates
its own default profile; a small typed numeric recipe seeds the synthetic
developer reference checkpoint, then native Continue proves the room. Default
creation is bounded20seconds and reference verification90seconds. Review its native captures for a complete
character on the grey floor, then freeze the local contract:

```powershell
./tools/iteration/Create-SandboxCheckpoint.ps1 -Workspace 'D:/Dev/WolverineGrey' -VisualReviewed
./tools/iteration/Prepare-NativeControls.ps1 -Workspace 'D:/Dev/WolverineGrey'
./tools/iteration/Install-PlayableSandboxShortcut.ps1 -Workspace 'D:/Dev/WolverineGrey'
./tools/iteration/Play-NativeSandbox.ps1 -Workspace 'D:/Dev/WolverineGrey' -ValidateOnly
```

No checkpoint bytes are distributed. Local hashes vary with the freshly generated
engine profile/compiler; the reviewed contract records them instead of accepting
arbitrary local content. Failure to reach native gameplay is a failed bootstrap,
not approval to import the developer's personal save. Preserve local proof and
inspect native errors. For daily sealed commands, see the adapter sandbox doc.

## Provenance and third-party boundary

The reproducible baseline numerical/runtime source is exact Git
`16affd49da5a122df776969574ca62a984b59425`, Base
`99ff741ea95f18ed84526c35a6c3f38a37d857b6`. New model work must explicitly select
and verify another `build_private_runtime.py --source-ref`; it does not implicitly
include a dirty checkout. Original mesh donors and stock hashes are checked;
all package GUIDs are deterministic. Regenerating the accepted studio yields
SHA256 `41b9764dac2f1e85584fc19bdac00867288237b179f4d4ca31e0ff9c693f5d2a`.

LZO is an external authoring dependency, built from pinned UEViewer Git
`a0bfb468d42be831b126632fd8a0ae6b3614f981` by Build-SandboxDependencies.
Its GPL2-or-later license/copyright are preserved in the fetched source tree.
We do not copy LZO source or binaries into Git, the numerical runtime or release.
MSVC, DirectX, retail assets and profiles are also not distributed. Receipts keep
source/dependency/runtime/stage identities separate from observed acceptance.

## Release boundary

All uploaded release archives must exclude `tools/iteration/`, this README,
`docs/ITERATION-SANDBOX.md` and sandbox-specific evidence. The tracked .gitattributes export-ignore rules also omit the room from Git
tag source archives. Clone the repository for development rooms; our curated
optional Source archive contains the frozen approved runtime implementation. Use
`tools/Assert-ReleaseExclusions.ps1` on staged/expanded upload assets.

## Verification limits

Existing owned native room: grounding/walking, pause/resume/capture, F6 and
Overall50â†’51 observed; human classicD3D9/user input observed. The fresh vanilla
bootstrap has independent compile/authoring/native receipts when executed;
those are not inherited from this earlier room. StaticMesh fluid-world origin,
extreme cloth states and high frame rate remain separate gates. Use the room
for rapid iterations, then verify any campaign-specific integration in retail.

## October 8 recipe inventory and experimental prototype

Run `python tools/iteration/verify_recipe.py` before building. The source-only
inventory now includes the later Meridian prototype, lighting, recovery and
profiling tools. This inventory certifies source identity, not installed or
visual acceptance. Uncommitted local updates must be committed/pushed explicitly
before another machine's clone can inherit them.

The approved baseline recipe remains unchanged. For an explicitly private
Meridian derivative, use `Create-MeridianTestRoom.ps1` with a freshly reproduced
source room and a hashed candidate DLL. `Play-MeridianPrototype.ps1` validates
only an already authored prototype manifest; it is not a universal installer.
Use `Test-MeridianCandidate.ps1` for the shared Base case matrix and
`verify_anterior_recovery.py` for SDK-free recovery proof. See the handoff for
source overlays, identities and known acceptance limits.

To reproduce the newer even-v3 lighting without overwriting a shared room:

```powershell
$env:MALEMOD_SANDBOX_WORKSPACE='D:/Dev/WolverineGrey'
python tools/iteration/author_native_layer.py --name jungle1_zone01a `
  --studio --even-lighting --output-directory 'D:/Dev/WolverineGrey/even-lighting'
python tools/iteration/pack_private_layer.py jungle1_zone01a even-lighting --offline
./tools/iteration/Stage-MeridianLighting.ps1 `
  -Workspace 'D:/Dev/WolverineMeridian' `
  -Layer 'D:/Dev/WolverineGrey/even-lighting/jungle1_zone01a.cooked.xxx' `
  -OverlayDirectory 'D:/Dev/WolverineMeridianCooked'
```

Choose an overlay on the same volume as the source assets. Stage only a closed,
owned Meridian workspace. The tool hard-links immutable inputs and makes the
lighting layer a private regular file; it never writes through the accepted
room's CookedPC junction. Packed even-v3 layer SHA256:
`10adb25651b10b14db1ee490feeb17b0ae300ed558c2b507d16f9c2a5789d5eb`.
A fresh room still needs its own grounding, appearance and motion evidence.

See [measured jockstrap costs and experiments](../../docs/MERIDIAN-PERFORMANCE.md).
The reusable CPU profiler extracts an exact selected build and consumes private
pose inputs; it never launches the host's game or publishes those inputs.
# Tank material diagnosis (October 8)

Use `Open-NativeSandbox.ps1 -TitleOnly -Active` to stay in the floating tank;
omit `-TitleOnly` to enter the grey room. The owned command channel also accepts
`J` for the existing clinical sequence, and capture/pause/garment controls now
work in the title scene. No host input or focus is used. Diagnostic runs can set
`MALEMOD_MERIDIAN_LIGHT_TRACE=1` to audit garment state restoration and capture
native lighting shader declarations. Keep it off for performance measurements.
Normal prototype launches explicitly clear it. See
`docs/NATIVE-MATERIAL-REPAIR.md` for native texture, light and depth contracts.

# Independent wardrobe checks

After the owned startup demo finishes, run Test-Costumes.ps1 with Workspace
and RuntimeProvenance. Add -Jeans for all eight Top/Bottom combinations and
open-fly motion views; otherwise it checks the four Naked/Jockstrap variants.
Captures retain raw hashes and start as visualReviewed=false. Follow with
Test-MeridianCandidate.ps1 for the shared attachment matrix; review actual
front/side/oblique and motion captures before recording any sampled pass.
Full coupled controls, LODs and campaign transitions are a separate gate.
Test-JeansFit.ps1 uses the same Workspace/RuntimeProvenance arguments for
1/25/50/75/100 Overall+Width and maximum open/closed checks. Hide the F6 menu
before the attachment/size matrix. Block is not a dedicated crouch; inspect
the actual captured pose and do not infer crouch recovery from that command.

For revision5 open jeans, run `Test-Costumes.ps1 -Jeans` first, then
`Test-JeansMotion.ps1 -Workspace <room> -RuntimeProvenance <candidate provenance>`.
It records real walking and subsequent settling, source/runtime identities,
native frame/time metadata and bounded hinge samples from that sequence only.
The captures and receipts stay private. Review the frames yourself; scripted
bounds are not cloth collision or attachment visual acceptance. Then run the
attachment and jeans-size matrices before considering any retail installation.
