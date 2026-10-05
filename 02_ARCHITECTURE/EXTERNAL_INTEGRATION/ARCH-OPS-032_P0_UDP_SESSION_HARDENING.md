# ARCH-OPS-032 — P0 UDP Session Hardening

## Purpose
Strengthen the first real OS transport integration without changing the existing MAVLink session or decoder contracts.

## Boundary
```
UDP source endpoint
    -> Transport Channel admission
    -> MAVLink Session identity/sequence
    -> Raw MAVLink2 decoder
```

## Rules
1. The UDP driver reports the peer endpoint; it does not interpret MAVLink semantics.
2. The channel runtime may bind ingress to the configured remote endpoint.
3. Frames from an unexpected configured UDP peer are rejected before session ingestion.
4. MAVLink sequence handling remains in `MavlinkSessionRuntime`.
5. Duplicate and sequence-gap behavior is tested through a real UDP socket.
6. Heartbeat timeout and reconnect remain session lifecycle responsibilities.
7. Deterministic-memory transport remains unchanged.
8. Endpoint identity and MAVLink system/component identity remain separate.

MAVLink defines system/component IDs as routing/addressing identity and uses the 8-bit packet sequence for loss detection. citeturn0search0turn0search6

## Acceptance
- Real UDP source endpoint captured.
- Unexpected source rejected.
- Duplicate not admitted as a new session packet.
- Sequence gap marks session degraded and increments packet loss.
- Heartbeat timeout marks session lost.
- Reconnect resets sequence state and permits a new heartbeat.
- No raw UDP details enter the decoder or normalized telemetry model.

## Evidence limitation
`SIL_OS_UDP_SESSION_HARDENING` only. No field-network, radio, serial, SITL/HIL, physical-flight, or certification evidence.
