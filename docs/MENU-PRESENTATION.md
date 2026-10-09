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
