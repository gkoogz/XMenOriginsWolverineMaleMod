# Meridian garment production integration

The normal runtime now selects the measured meridian garment for clothing style
1 without an environment variable. The shared algorithms are consumed through
the exact Base pin in dependencies/base.lock.json. Each render frame deforms the
last successful wrap from the existing physics pose, checks current contact and
rebuilds when needed. The guide solids are numerical only. The pouch is 32
longitudes by 24 rows; the complete garment has 2471 vertices and 4816 triangles.
The existing waistband, hem and glute straps remain, with ribbed cloth and thin
red/blue waistband stripes responding to the native lighting passes.

src/runtime/meridian_recipe.h is the version5 measured binding contract, not a
body export. It preserves donor IDs, barycentric/frame offsets, proxy control
points, topology, UV aliases and uncovered collar indices. Its LF-normalized
hash and source input hashes are in provenance/meridian-binding.json. Raw body
and anatomy exports, settings, captures and stock-derived packages are omitted.
tools/author_meridian_runtime.py regenerates it from licensed private inputs
using --base, --inspection, --lod, --body-input, --anatomy, --columns 32 and
--output. Base tools/author_meridian_lod.py authors the 32-ray fitted chart from
the measured inspection. Exact regeneration needs the input hashes in the
binding provenance; these inputs are not distributed as a game package.

Build with build.cmd after selecting the clean pinned Base dependency through
MALEMOD_BASE_PATH. Run tools/Test-MeridianBinding.py before compiling. The
standard build contains no sealed sandbox factory, room-origin override or
automated input/capture hook. Rejected pose files are disabled by default;
explicit MALEMOD_MERIDIAN_RAW_CAPTURE=1 is developer instrumentation.

This integration is a development checkpoint, not a new published release.
manifest.json and the release installer still identify the accepted Beta2
payload. Do not install a sandbox DLL into retail or relabel the old release.
The full mandatory attachment gate is pending: the prototype sampled18 cases,
but extreme cloth faceting, uncertified fallback contact, every supported LOD
and campaign/tank visual coverage remain incomplete. Source/offline/native and
observed visual evidence must stay separate. Retain the accepted retail runtime
and exact rollback until the replacement passes.
