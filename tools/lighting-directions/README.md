# Current full-material replay and lighting correction

Requires local R14Capture_20260920_023631 and _023611. Build via build.cmd. Run replay.exe original and replay.exe view_skin. The latter writes fixed_vs.bin/fixed_ps.bin; generate_header.py rebuilds runtime header. Optional second argument is capture directory with trailing slash. Output .dds retains actual float16 HDR; PNG is ungraded and should not be compared to the final game's tonemapped screenshot.

Run build_hook.cmd then hook_test.exe hook_angle2, and hook_test.exe hook_angle1 "C:/Games/X-Men Origins Wolverine/Binaries/R14Capture_20260920_023611/". Exit0/pass messages verify actual runtime override, off/unknown bypass, state restoration, constants/textures and release/recreation. Both hook .dds files must hash-match their corresponding view_skin experiments. No local proxy d3d9.dll should sit beside these executables; no keyboard injection/game launch.

Other replay modes are investigative controls, not proposed runtime changes. The installed correction is view_skin only; previous world_skin alone was rejected for gameplay appearance.
