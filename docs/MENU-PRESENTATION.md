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
reading hardware keys or changing host focus. The repaired input candidate
still requires native open/closed title proof. Neither this source nor menu
observations establish garment or attachment acceptance.

The first input candidate fba369a failed native title isolation: panel arrows
also moved Continue to Options. Its HWND subclass was bypassed in the actual
message path, although keyboard creation was observed. A process-owned
WH_GETMESSAGE fence now rewrites only panel key messages addressed to the exact
game HWND on its validated owning thread before engine queue inspection.
The hidden-window test also replaces the window procedure and reads queued
messages before dispatch: open Down becomes WM_NULL, closed Down and unrelated
W remain intact. Native confirmation of this additional fence is pending;
physical DirectInput neutralization is not established by private posted keys.
