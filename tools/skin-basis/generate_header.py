from pathlib import Path
import struct
capture = Path("C:/Games/X-Men Origins Wolverine/Binaries/Capture168_20260916_055428")
out = Path(__file__).resolve().parents[2] / "src/runtime/skin_basis_shaders.h"
items = [("skinOriginalVS", capture / "draw_023_vs.bin"), ("skinOriginalPS", capture / "draw_023_ps.bin"), ("skinFixedVS", Path("fixed_vs.bin")), ("skinFixedPS", Path("fixed_ps.bin"))]
lines = ["// Generated from the exact captured pair and world_skin replay.\n"]
for name, path in items:
    data = path.read_bytes()
    assert len(data) % 4 == 0
    words = struct.unpack("<" + "I" * (len(data) // 4), data)
    lines.append("static const DWORD " + name + "[] = {\n")
    for i in range(0, len(words), 8):
        lines.append("  " + ",".join("0x%08Xu" % x for x in words[i:i+8]) + ",\n")
    lines.append("};\n")
out.write_text("".join(lines))
