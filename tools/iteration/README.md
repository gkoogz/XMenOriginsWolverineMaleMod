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
Overall50→51 observed; human classicD3D9/user input observed. The fresh vanilla
bootstrap has independent compile/authoring/native receipts when executed;
those are not inherited from this earlier room. StaticMesh fluid-world origin,
extreme cloth states and high frame rate remain separate gates. Use the room
for rapid iterations, then verify any campaign-specific integration in retail.
