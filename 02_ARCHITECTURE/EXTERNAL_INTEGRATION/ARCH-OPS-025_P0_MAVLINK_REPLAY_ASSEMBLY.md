# ARCH-OPS-025 — P0 MAVLink Replay Assembly

**Status:** IMPLEMENTATION BASELINE — SIL/DECODED-REPLAY SCOPE

## Objective

Turn the individual decoded MAVLink telemetry messages from ARCH-OPS-024 into deterministic per-vehicle telemetry snapshots before canonical normalization.

## Boundary

    Recorded / decoded MAVLink messages
                ↓
    MavlinkTelemetryDecoderBoundary
                ↓
    MavlinkTelemetryReplayAssembler
                ↓
    MavlinkTelemetrySample
                ↓
    MavlinkTelemetryMapping
                ↓
    NormalizedTelemetryState

## Design

A MAVLink stream contains independent telemetry message families. A single message must therefore not be treated as a complete canonical vehicle snapshot.

The assembler:

- accepts decoded messages only;
- keeps vehicle/source/system/component identity together;
- excludes invalid decoded messages;
- aggregates telemetry fields from the same vehicle/source;
- advances the snapshot timestamp monotonically;
- emits a deterministic replay fixture;
- leaves canonical state ownership with MavlinkTelemetryMapping.

ArduPilot and PX4 use the same assembly path.

## Verification

Tests cover:

- ArduPilot multi-message replay producing a usable canonical state;
- PX4 replay remaining unusable when mandatory navigation/attitude facts are incomplete;
- invalid decoded messages being excluded.

Evidence class: SIL_DECODED_REPLAY_ONLY.

## Explicit limitation

This is not raw MAVLink parsing and not SITL/HIL/real-UAV evidence. It establishes the deterministic replay seam needed before binding an actual recorded MAVLink stream.

## Next mandatory slice

Introduce versioned recorded-stream fixtures or a real decoder adapter at the boundary, preserving message identity and source timestamps, then run end-to-end replay evidence into NormalizedTelemetryState.
