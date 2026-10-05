# ARCH-OPS-026 — P0 Versioned Recorded MAVLink Stream

**Lifecycle:** IMPLEMENTED → INTEGRATION EVIDENCE PENDING

## Objective

Replace purely in-memory decoded-message construction with a versioned recorded-stream seam that can later accept real decoder output without changing the replay or canonical normalization layers.

## Contract

Versioned recorded stream → MavlinkRecordedStreamReader → DecodedMavlinkMessage → MavlinkTelemetryDecoderBoundary → MavlinkTelemetryReplayAssembler → MavlinkTelemetrySample → NormalizedTelemetryState

## Invariants

1. The stream declares an explicit format version.
2. Every accepted record carries vehicle, source, system and component identity.
3. Source and receive timestamps are retained.
4. Only supported decoded message families enter the replay path.
5. A record with a mismatched version is not silently accepted.
6. ArduPilot and PX4 share the same downstream replay and normalization path.
7. The canonical NormalizedTelemetryState remains the only runtime telemetry state.
8. Raw MAVLink wire representation remains outside the operational core.

## Verification scope

The automated fixture proves versioned stream parsing, deterministic filtering of unsupported versions/message kinds, ArduPilot recorded replay, and end-to-end mapping to the existing canonical state.

Evidence class: SIL_RECORDED_DECODED_REPLAY

## Remaining G0-06 integration

Actual MAVLink transport/decoder binding remains necessary before G0-06 can be considered fully integrated and verified.
