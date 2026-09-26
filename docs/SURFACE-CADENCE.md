# Surface cadence performance tradeoff

Fast mode is the default. It generates the detailed surface on alternating
presented frames, drawing the previous complete mesh/body junction in between.
The game's skeletal animation and the coupled physics solver still advance
every frame. Rest inputs, including pulse dimensions, also update every frame
in a private body buffer, so the simulation does not consume stale dimensions.

The cost is less smooth secondary surface motion and one extra held frame of
surface lag relative to the previous mode. It is a temporal quality tradeoff, not a
measured "10% quality loss". Polygon count, materials, geometry construction and
solver quality are unchanged. Existing skin folds remain.

Press **F10** to toggle full-rate surfaces; the HUD title identifies the mode.
The toggle is session-only, with no settings format changes. User control
changes, initial attachment and device reset force immediate complete surface
updates. Both sides of the body weld are held together between refreshes.

The presentation hooks now update once per presented frame, including repeated
EndScene calls and nested device/swap-chain Present calls. This prevents extra
callbacks from consuming alternate-frame slots or running duplicate updates.

`tools/physics/verify_cadence.py OUTPUT_DIRECTORY` compares against the previous
installed source (2c1deee). It requires identical particle state every frame,
identical packed render meshes and body positions on refreshed frames, and
unchanged complete visible body buffers on held frames. Body tangent bytes
can differ from full cadence because their repeated projection now runs less
often; body positions and welded geometry still match on refreshes. Eight cases
cover size extremes, all physical states, all pulse levels, control changes,
reset/recreation and the fast/full-rate toggle. CPU timing uses the **mean** over
complete alternating cycles after startup, not the median of cheap held frames.

`build-cadence-hooks.cmd` and `cadence-hooks.exe` test repeated EndScene, nested
Present, swap-chain-only presentation and reset using a real D3D9 device.
Replay timings do not establish game FPS; the work remains synchronous and
alternating expensive/cheap frames can affect pacing. F10 permits comparison.
