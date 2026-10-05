# ARCH-OPS-027 STATUS

**Status:** IMPLEMENTATION BASELINE — RAW MAVLink2 DECODER BOUNDARY

Implemented a narrow raw MAVLink2 wire-frame decoder boundary for the common telemetry families already represented downstream.

## Implemented

- MAVLink2 frame envelope validation;
- system/component identity preservation;
- MAVLink v2 CRC-X25 validation with message-specific CRC extra;
- HEARTBEAT, GLOBAL_POSITION_INT, ATTITUDE and SYS_STATUS decoding;
- rejection of malformed, unsupported and CRC-invalid frames;
- output directly into the existing `DecodedMavlinkMessage` boundary.

## Architectural boundary

Raw MAVLink2 frame → MavlinkRawDecoderBoundary → DecodedMavlinkMessage → existing replay/normalization path

No raw MAVLink types enter the canonical operational state.

## Evidence

`SIL_RAW_MAVLINK_DECODED_FIXTURE`

The current slice proves deterministic raw-frame parsing and CRC rejection in software tests. It does not claim live transport, SITL/HIL, physical-UAV operation or certification evidence.

## Important limitation

Flight-mode decoding is intentionally represented as a non-semantic `BASE_MODE:<value>` fact at this boundary. Autopilot-specific mode semantics remain adapter responsibility.

## Next mandatory slice

Bind this decoder boundary to the existing MAVLink transport/session runtime, preserving session identity, heartbeat/liveness, sequence handling, reconnect and link-state semantics.
