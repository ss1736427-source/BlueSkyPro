# ARCH-OPS-031 — P0 UDP Transport Channel Integration

## Purpose
Close the integration gap between the real OS UDP transport driver delivered by ARCH-OPS-030 and the stable transport channel/session/decoder chain.

## Boundary
```
UDP OS socket
    -> MavlinkUdpTransportDriver
    -> MavlinkTransportChannelRuntime
    -> MavlinkSessionRuntime
    -> MavlinkRawDecoderBoundary
    -> DecodedMavlinkMessage
```

The channel runtime owns transport lifecycle and statistics. The session runtime owns MAVLink session identity, sequence handling, heartbeat/liveness, loss detection and reconnect state. The raw decoder owns MAVLink2 frame validation and supported message decoding.

## Design Rules
1. No MAVLink semantic parsing is added to the UDP driver.
2. No raw socket details enter the session runtime.
3. The deterministic-memory seam remains intact.
4. UDP dynamic local port allocation is supported.
5. Remote UDP endpoint must be explicit before transmit.
6. Received frames enter the same session/decoder path as deterministic injected frames.
7. Evidence is limited to local OS UDP loopback.

## Verification
The integration test uses two real OS UDP sockets:
- external sender -> BlueSky UDP channel -> session/decoder;
- BlueSky UDP channel -> external receiver.

The test also retains the existing deterministic-memory lifecycle coverage.

## Evidence Limitation
This does not demonstrate radio/cellular/serial/field-network behavior, SITL/HIL, physical flight, or certification compliance.
