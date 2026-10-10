# ARCH-OPS-029 STATUS

**Status:** IMPLEMENTATION BASELINE — MAVLink TRANSPORT CHANNEL RUNTIME

## Implemented

- concrete channel configuration and identity;
- explicit channel lifecycle: OFFLINE -> CONNECTING -> CONNECTED -> LOST/RECOVERING;
- deterministic receive/send seam;
- routing from channel to the existing MAVLink session runtime;
- per-channel transmit/receive/rejection/failure/reconnect statistics;
- heartbeat timeout propagation from session to channel state;
- explicit reconnect resets the associated MAVLink session sequence state;
- simultaneous channel/session isolation.

## Evidence

**SIL_DETERMINISTIC_TRANSPORT_CHANNEL**

Deterministic software fixture/CTest evidence only. The channel implementation is an in-process transport seam; it is not evidence of serial, UDP, TCP, radio, SITL/HIL or physical-flight operation.

## Architectural boundary

`Channel -> MavlinkSessionRuntime -> MavlinkRawDecoderBoundary -> DecodedMavlinkMessage`

No transport-specific types enter the normalized telemetry model.

## Next mandatory slice

Bind a real supported OS/network transport driver (initial candidate: UDP) to this channel interface while retaining the same session/decoder boundary and test seam.
