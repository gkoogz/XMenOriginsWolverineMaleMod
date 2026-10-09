# October 9 live-menu candidate

The overlay consumes Base `controls/menu_theme.hpp`: charcoal background,
ivory text, muted brass selection/track accents, and an installed condensed
sans-serif font. State, shape, physics and wardrobe groups retain the existing
22 gameplay / 24 title control order and mappings. F6, arrow cycling, Shift
steps, F8 reset, J and persisted settings remain unchanged.

`src/runtime/menu_hud.h` creates one grayscale GDI atlas from the available
Bahnschrift SemiCondensed font, falling back to native Arial. No font asset is
packaged. A failed GDI or texture allocation uses the retained original 5x7
bitmap path. The atlas is released before device reset; HUD sampler, blend,
texture-coordinate and texture-transform state lives within the existing ALL
state block. The panel scales to the available viewport height.

Source/offline evidence: MSVC x86 compilation of the complete runtime source
passes. An ignored preview compiled the actual HUD functions and layout with
representative control values in a hidden owned Direct3D9 device. Gameplay,
title and collapsed panels render; sampled sampler/blend restoration and atlas
recreation pass. The native font resolves to Bahnschrift SemiCondensed. Witcher
uses the same Base theme through its existing native GDI panel.

These previews are renderer evidence, not game observations. The parent
wardrobe/contact candidate must include this source and exact Base pin in its
sealed grey-room run. Native F6/selection/adjustment/collapsed views, device
reset and the mandatory combined attachment regression remain acceptance
requirements. No retail installation, publication or full visual pass is
claimed by this source checkpoint.

## Scoped input isolation

Final typography source6158957 was observed in two sealed runs: 22 gameplay
rows at1920x1440 and24 title rows at1280x720. Ten reviewed captures show clean
text, one/five-step adjustment, reset and collapse. The rejected garment
solver was not accepted by this menu test; Jeans/Naked views isolate the UI.
Both children exited0; no owned audio sessions were muted. Retail was intact.

The title run exposed the existing arrows reaching the stock menu behind the
F6 panel. `menu_input.h` now intercepts only this process's exact game HWND
and its observed DirectInput keyboard devices. While the visible panel is open,
arrows, Shift and F8 reach panel polling while the engine receives neutral
events/state. F6 belongs to the panel. Unrelated keys, mouse devices and closed
bindings remain available. Background panel polling returns0. No OS keyboard
global hook or host focus operation is used; unload restores owned procedure/patches.
Buffered events retain count/peek semantics while neutralizing panel presses.

The licensed executable actually imports Win32 message/GetKeyState and
DirectInput8Create paths; it does not import GetAsyncKeyState directly. Native
diagnostics identify keyboard creation and which intercepted state/buffered
paths execute. The scoped hidden-window/fake-device test verifies open/closed,
unrelated keys, shared mouse tables, buffered peek, background and detach without
reading hardware keys or changing host focus. Neither this source nor menu
observations establish garment or attachment acceptance.

The first input candidate fba369a failed native title isolation: panel arrows
also moved Continue to Options. Its HWND subclass was bypassed in the actual
message path, although keyboard creation was observed. A process-owned
WH_GETMESSAGE fence now rewrites only panel key messages addressed to the exact
game HWND on its validated owning thread before engine queue inspection.
The hidden-window test also replaces the window procedure and reads queued
messages before dispatch: open Down becomes WM_NULL, closed Down and unrelated
W remain intact. Physical DirectInput neutralization is not established by
private posted keys. The c65fb9a checkpoint did not mask Win32 GetKeyState.

Native source c65fb9a9310d5e84fc9577541f3a9699acc99856 / Base2c15479 was
built from a clean archive (no unrelated dirty source) by the canonical private
builder. Runtime SHA256
`d11c884eaf72dafc632f6b37b8430f2b2fe344e2f1a0bebad6a1fa4c65a79583`.
Title720 run20261009-172413-e8f03a: six reviewed captures show all24 rows and
footer fitting at1280x720; open Down/Right/Shift/F8 retain the stock Continue
selection while Overall50 changes51 then56 and resets50. F6 collapse retains
Continue; closed Down moves stock selection to Replay Mission. The exact
owned thread queue fence logs consuming messages. Room1440
run20261009-172642-005425: six reviewed captures show all22 rows and footer,
Length50 to51 to56 to50, selection and collapse at1920x1440 with Jeans/Naked.
These prove private posted-key title isolation and native menu appearance;
physical keyboard/DirectInput gameplay input and native game device recreation
remain unobserved. The actual hidden D3D9 renderer separately passed reset and
atlas recreation three times.

Startup run20261009-172117-3a80f5 is excluded from menu proof: frame600 reached
a Bink intro before the title surface. Its child closed cleanly. The final
title test began only after inspecting the actual Continue menu capture.
The two final children closed with exit0; no private audio sessions needed
restoring. No host keyboard/mouse/focus operation, retail installation or
garment/attachment acceptance is claimed. Ignored self-contained review and
hash evidence: MaleModBase/build/menu-refresh-20261009/review.html and
evidence.json. Witcher panel remains source/offline renderer tested; native
readability and keyboard isolation are unobserved.

The remaining Win32 polling gap is now covered by patching only the licensed
executable's GetKeyState import. An open panel with the exact owned HWND in
foreground returns neutral state for its navigation/reset/Shift/F6 keys.
Closed-panel, background and unrelated queries preserve the original SHORT,
including pressed and toggle bits. The proxy's raw GetAsyncKeyState panel
polling and other processes/modules remain untouched. The hidden-window test
uses a fake import/state returning0x8001, verifies all three cases and restores
the import on detach without reading hardware state.

Native source9b9e2f5782dcce4a42305b3fcc9dcf4fb1eb1d70 / Base2c15479 clean
archive built runtime SHA256
`4747dbb4d7a82006a26c2ab600bfea07de6d245b9247f66fa30066ae57e6e370`.
Run20261009-173408-d75f4f reviewed six title720 captures: panel navigation,
Overall50/51/56/reset50 and collapse retain the stock Continue selection;
closed Down selects Replay Mission. All24 rows/footer retain the reviewed
styling and fit. The actual executable logs its GetKeyState import hook, a
closed key1/open0 original query, and open-panel neutralization of key37
(VK_LEFT). This establishes native invocation of the added path; physical
host input remains prohibited and unobserved. The owned child exited0 with no
muted private audio sessions; retail hash7095f630 remains unchanged. The
mandatory combined attachment gate remains unaccepted.

Witcher coordination: parity's source/compiler/cache checkpoint d4f5943 does
not yet provide a safe isolated combined candidate; its baseline39 driver has
no exact-panel key/command interface. No installed Witcher runtime is modified
solely for this menu check. Native appearance/input confirmation waits for an
isolated candidate and scoped panel probe; existing offline panel evidence
remains qualified as renderer-only.
