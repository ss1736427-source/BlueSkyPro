# ARCH-OPS-031 Status

Status: IMPLEMENTED — CI pending

## Scope
Bind the real OS UDP transport driver to the existing MAVLink transport channel runtime without changing the session or raw-decoder contracts.

## Implemented
- UDP channel configuration with local/remote endpoints.
- UDP driver lifecycle on connect, disconnect, fail, and reconnect.
- Dynamic local UDP port capture after bind.
- Real UDP receive through `pollReceive()`.
- Real UDP transmit through `send()`.
- Existing deterministic-memory `injectReceive()` / queue seam preserved.
- Existing `MavlinkSessionRuntime` remains the semantic session boundary.
- Existing raw MAVLink2 decoder remains the wire-format boundary.
- Loopback integration test covers real OS UDP ingress and egress.

## Evidence
Target evidence class: `SIL_OS_UDP_CHANNEL_INTEGRATION`.

This is deterministic local OS loopback evidence only. It does not establish field-network, cellular, radio, serial, SITL/HIL, physical-flight, or certification evidence.

## Acceptance
`UDP socket -> Transport Channel Runtime -> MAVLink Session Runtime -> Raw MAVLink2 Decoder -> DecodedMavlinkMessage`.

Deterministic memory transport remains available for unit/integration tests that do not require an OS socket.
