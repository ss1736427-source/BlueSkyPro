# ARCH-OPS-024 — P0 MAVLink Decoder Boundary

**Status:** IMPLEMENTATION BASELINE — SIL/DECODED-FIXTURE SCOPE

## Objective

Close the next G0-06 slice by defining one concrete decoder boundary for ArduPilot and PX4 without introducing MAVLink wire types into the BlueSky operational core.

## Boundary

    ArduPilot/PX4 MAVLink decoder
                ↓
    DecodedMavlinkMessage
                ↓
    MavlinkTelemetryDecoderBoundary
                ↓
    MavlinkTelemetrySample
                ↓
    MavlinkTelemetryMapping
                ↓
    NormalizedTelemetryState
                ↓
    Operational Runtime

The decoder boundary is common. The dialect is an explicit input attribute. ArduPilot and PX4 therefore populate the same typed telemetry sample and cannot create parallel canonical telemetry models.

## Covered message families

The fixture boundary represents the first common telemetry families:

- HEARTBEAT;
- GLOBAL_POSITION_INT;
- ATTITUDE;
- SYS_STATUS / battery telemetry;
- mission-state telemetry.

The implementation intentionally models decoded messages rather than raw MAVLink wire bytes.

## Invariants

1. MAVLink version is fixed to the current P0 reference baseline (MAVLink2).
2. Vehicle identity and MAVLink system/component identity are preserved.
3. Source and receive timestamps are mandatory for a usable sample.
4. Future-dated source timestamps are rejected at the decoder boundary.
5. Invalid decoded messages are rejected rather than fabricated.
6. ArduPilot and PX4 map into the same MavlinkTelemetrySample.
7. Mission-state fields remain typed input data and are not added to NormalizedTelemetryState unless the canonical contract is explicitly extended later.
8. Raw MAVLink parser/serialization details remain outside BlueSky operational core.

## Verification scope

Automated SIL/decoded-fixture tests cover:

- ArduPilot HEARTBEAT;
- PX4 GLOBAL_POSITION_INT;
- common ATTITUDE and battery mapping;
- identity preservation;
- invalid identity;
- invalid timestamp ordering;
- invalid decoded message rejection;
- mission-state typed boundary.

Evidence class: SIL_DECODED_FIXTURE_ONLY.

## Explicit limitation

This slice does not claim raw MAVLink wire parsing, ArduPilot SITL, PX4 SITL, HIL, real vehicle connectivity, or flight verification.

The next integration step is to bind real/recorded MAVLink decoder output from ArduPilot and PX4 to DecodedMavlinkMessage, then execute deterministic replay and integration evidence.

## Architectural rule

Do not add vendor-specific MAVLink message IDs, dialect structs, parser objects, or raw wire buffers to NormalizedTelemetryState or the operational runtime. Protocol-specific decoding belongs above this boundary.
