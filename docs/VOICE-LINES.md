# Numbered voice script

[voice-lines-wolverine.csv](voice-lines-wolverine.csv) is an editable 24-line
development script with global tags1–24. Corresponding cue/variant slots use the
same numbers and permanent MM-VOICE IDs as Geralt and the Core template.
Phase1 is tags1–13; Phase2 is tags14–24, without restarting the tag number.

The text is copied exactly from the existing filename transcript in
`wolverine-voice-pools.csv`. That original provenance CSV remains unchanged.
The new script starts as `needs_recording` with no recording paths, so it does
not imply any new takes were reviewed or change the currently installed voices.

Core owns the schema, numeric tag checks and character scaffolding. Its exact
voice-tool revision and file hashes are recorded in `voice-pipeline.lock.json`;
this is separate from the runtime's numerical dependency:

```powershell
python ../MaleMod/tools/voice_pipeline.py check --script docs/voice-lines-wolverine.csv
```

See `../MaleMod/docs/VOICE-PIPELINE.md` for retake revisions and approved delivery.
This change adds development text only. No runtime, installed audio, geometry,
settings, release or native gameplay qualification changed.
