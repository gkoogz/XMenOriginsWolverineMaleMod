# Release 1.6.0

Each of the four main teaching pulses receives its own volume, duration, and angle-intensity sample at sequence start. The samples vary between demonstrations while remaining internally consistent through that run. Long duration samples overlap the next pulse; overlapping windows share a continuous stream and the tapered feed envelopes blend through the transition. At the default settings, at least one main pulse extends beyond the following 1.5-second onset.

`PulseVolumeVariation`, `PulseDurationVariation`, and `AngleVariation` in `Binaries/TeachingFluid.ini` set relative bounds. Set any value to zero for a fixed parameter. `Duration` is the base feed time and `Volume` the base volume per pulse. The angle variance changes the phase-two pose pulse and does not alter saved slider settings. Values are illustrative game units, not clinical measurements.

The installer preserves existing user configuration. The release bundle contains no user-provided study recordings. Validation details are recorded in `docs/TEACHING-FLUID-1.6-VALIDATION.md`; live gameplay and FPS have not been verified.
