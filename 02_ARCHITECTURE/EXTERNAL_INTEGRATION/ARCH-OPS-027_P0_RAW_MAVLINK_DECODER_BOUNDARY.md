# ARCH-OPS-027 — P0 Raw MAVLink2 Decoder Boundary

## Objective

Introduce the first real wire-level MAVLink2 decoding seam without coupling BlueSky Core to MAVLink library types or autopilot-specific semantics.

## Contract

```text
MAVLink2 wire frame
      ↓
MavlinkRawDecoderBoundary
      ↓
DecodedMavlinkMessage
      ↓
Replay Assembler
      ↓
MavlinkTelemetrySample
      ↓
NormalizedTelemetryState
```

The official MAVLink C/C++ reference implementation exposes byte-wise MAVLink parsing and preserves system/component/message identity at the decoded message level. citeturn0search0turn0search2

## Scope

Supported common message families:

- HEARTBEAT (0)
- SYS_STATUS (1)
- ATTITUDE (30)
- GLOBAL_POSITION_INT (33)

CRC-extra values are tied to the corresponding common message definitions; for example SYS_STATUS uses CRC extra 124. citeturn1search0

## Invariants

1. Only MAVLink2 frames enter this boundary.
2. CRC-invalid frames are rejected.
3. Unsupported message IDs are rejected.
4. System/component identity is retained.
5. The output type remains `DecodedMavlinkMessage`.
6. No MAVLink wire types enter `NormalizedTelemetryState`.
7. Autopilot-specific flight-mode semantics remain outside this decoder.

## Evidence boundary

`SIL_RAW_MAVLINK_DECODED_FIXTURE`.

This is software fixture evidence only. It is not live transport, SITL/HIL, physical-flight or certification evidence.

## Next stage

Integrate the decoder with the existing MAVLink session/transport boundary from ARCH-OPS-018 and verify heartbeat, sequence, timeout, reconnect and multi-session behaviour.
