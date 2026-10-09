# Wolverine grey development room

The cohesive tracked recipe lives in [tools/iteration](../../tools/iteration/README.md).
Use it by default for native physics, mesh, cloth, material and live-control
iterations, then verify campaign-specific behavior separately.

The package is source/deltas only. It reconstructs native layers from supported
stock packages, builds pinned private runtime/dependency sources, and generates
the owner's own fresh engine checkpoint in an isolated clone. It ships no game
assets, saves, audio, captures or compiled runtime. Reviewed local acceptance
and hashes remain outside Git.

Exclude this directory and tools/iteration from uploaded Install/Source release
assets. Git tag source snapshots exclude the room through .gitattributes export-ignore.

## Current extensions

The recipe includes the even-lighting option, isolated Meridian stage transaction,
prototype validation, native regression matrix and SDK-free diagnostics. Run
`python tools/iteration/verify_recipe.py`; see the main recipe README for commands.
Performance findings are in `docs/MERIDIAN-PERFORMANCE.md`. Source inventories
are distinct from native acceptance receipts and runtime dependency pins.
