# ARCH-OPS-028 — P0 MAVLink Session Runtime

## Objective

Bind raw MAVLink2 frames to a deterministic session lifecycle without coupling transport lifecycle into the raw decoder or canonical telemetry state.

## Contract

Transport -> raw frame + receive timestamp + session identity -> MavlinkSessionRuntime -> MavlinkRawDecoderBoundary -> DecodedMavlinkMessage -> existing replay/normalization path.

## Responsibilities

- preserve transport/session identity;
- preserve MAVLink system/component identity;
- consume MAVLink2 sequence for loss detection;
- reject duplicates;
- account for sequence gaps;
- process HEARTBEAT for liveness;
- detect heartbeat timeout;
- transition to LOST;
- reset lifecycle on explicit reconnect;
- isolate simultaneous sessions.

MAVLink 2 carries sequence, system ID and component ID in its header; sequence is explicitly intended for packet-loss detection. Official documentation: https://mavlink.io/en/guide/serialization.html

## Invariants

1. Invalid raw frames never create a session.
2. Session identity is distinct from vehicle identity.
3. System/component identity is preserved.
4. Sequence state is scoped to a session.
5. Duplicate packets do not reach downstream normalization.
6. Sequence gaps are observable through packet-loss accounting.
7. Heartbeat timeout cannot silently remain healthy.
8. Reconnect resets sequence state.
9. Multiple sessions do not share lifecycle state.
10. No transport-specific types enter canonical normalized state.

## Evidence boundary

SIL_RAW_MAVLINK_SESSION_FIXTURE

Deterministic software test evidence only. No live serial/UDP/TCP/radio transport, SITL/HIL, physical flight or certification evidence.

## Next

Bind the runtime to a concrete transport channel abstraction and add connection establishment/termination, receive/send, link statistics, routing and channel-failure handling.
