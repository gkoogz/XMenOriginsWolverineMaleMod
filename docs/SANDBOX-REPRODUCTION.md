# Grey room recipe reproduction, October5

The tracked source-only package contains27 files with LF-normalized SHA256 in
dev/sandbox/delta-manifest.json. A separate clean source-only copy passed its
own verifier and the final checkpoint initializer. No helper binary, saved
profile, capture, private settings/audio or cooked game resource was supplied
in that copy. Runtime source is Git16affd49 and exact Base99ff741. LZO/launcher
compile from the external clean pinned dependency and tracked launcher source.

Native stock donors were reconstructed locally from the three published
reversible WBX deltas and checked against every vanilla manifest hash, then
used to construct a new owned clone. Retail was read only. The profile
format was NOT guessed: the engine generates its own2748-byte default
profile, with magic/size/count and SHA1 verified. A small observed typed numeric
reference recipe seeds the developer checkpoint; no player's save is imported.
Field meanings not confirmed by the SDK remain explicitly opaque; no new
source units or engine capability is invented.

The earlier initializer failure on inherited read-only PCTOC was fixed only
for the owned copy. A fresh default device can pass null HWND while its
D3DPRESENT_PARAMETERS contains the native device window; private activation
now uses that measured fallback. The default profile alone reaches the title,
not gameplay; field27 alone also fails. These failures are not acceptance.
The final complete typed reference recipe then reaches native gameplay.

Fresh final proof is local C:/MaleModFreshSandbox/reproduction-proof.json;
checkpoint-20261005-223957/native-frame-1200.png was visually reviewed: complete
grounded Wolverine on the authored grey floor/backdrop, native animation and
live numerical/render output. Native gameplay receipts advance through550
frames. Exact child exit124 is the bounded timeout; owned audio restoration
HRESULT0 and process absence pass. The previous same fresh-clone run also
acknowledged walk/F6/Overall51/capture. Human storage/preflight passed using an
independent local state directory; no human window was launched over the host.

Accepted studio layer regenerates exact41B9764D bytes. Quiet carrier exact
1D783CC8,812 disabled input links and retained WorldInfo/RCheckpoint actors.
Only stock runtime and source/deltas are needed: private saves are not a
reproduction prerequisite. Game prerequisites/edition hashes and authored
scene constraints are in tools/iteration/README.md.

This still retains native jungle1_p startup/streaming dependencies and is not a
standalone smallest cooked world. StaticMesh fluid-world origin, later walking
cloth, extreme fits and whole-game performance remain separate gates. The
Beta2 release includes the approved retail4F49 runtime, not this private
graphics/control derivative or later development source cloth.
