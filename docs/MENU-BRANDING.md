# Main menu branding

The game adapter replaces the native title and old blood art on the main-menu
backbuffer with two loose transparent PNGs. Both must load before the native
draws are suppressed. Missing assets leave the original menu artwork visible.
Neither PNG belongs in Git.

Install beside `Binaries/d3d9.dll` as:

- `Binaries/MenuBranding/title-and-blood.png`: SHA-256
  `a8fa02bcbc269bc527b6e4147c94d3b0bab5bab35ab55ee5ebdce4993b7d0360`.
  Derived from the game's original title and blood splat.
- `Binaries/MenuBranding/edition-stamp.png`: SHA-256
  `bed0c75dbfcbb41ca4e6c7dbfc52538adfecb7e59b6f177520f9c676f1a149af`.
  The user's replacement transparent red stamp supplied October 9, 2026.

Coordinates on a 1280 × 720 canvas:

| Layer | X | Y | Width | Height | Rotation |
| --- | ---: | ---: | ---: | ---: | ---: |
| title and blood | 284.906942 | 78.109306 | 1280 | 370 | 0° |
| stamp | 742.523903 | 299.109968 | 438.413064 | 146.14 | -4° |

The stamp pulses over 2.2 seconds about its center: size 100–102% and opacity
88–100%. It appears on the title and main menu only. Its asset is RGBA and
the black shown in some image viewers is transparent.

Observed in the isolated native game copy at 1280 × 720 on October 9, 2026:
the replacement title, blood, and requested stamp display on the title screen
and main menu, while the menu choices remain usable and gameplay does not show
the overlay. An attachment regression matrix and sampled side/oblique grey-room
captures were made for this runtime source. The full attachment gate remains
incomplete because every state, LOD, view, motion case, and campaign transition
has not been visually accepted. Keep the accepted runtime and exact rollback.
