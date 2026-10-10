# ARCH-OPS-028 STATUS

**Status:** IMPLEMENTATION BASELINE — MAVLink SESSION RUNTIME

This slice binds the existing raw MAVLink2 decoder boundary to a deterministic session runtime.

## Implemented

- transport/session identity;
- raw-frame sequence extraction;
- sequence first/in-order/gap/duplicate handling;
- packet-loss accounting;
- HEARTBEAT health and liveness;
- timeout to LOST;
- explicit reconnect to RECOVERING;
- recovery through the next valid HEARTBEAT;
- multi-session isolation;
- rejection of invalid raw frames before session creation.

## Evidence

SIL_RAW_MAVLINK_SESSION_FIXTURE

Software fixture/CTest evidence only. No live serial/UDP/TCP/radio transport, SITL/HIL, physical-flight or certification evidence is claimed.

## State model

HEALTHY -> DEGRADED -> LOST -> RECOVERING -> HEALTHY

## Next mandatory slice

Bind the session runtime to an actual transport channel abstraction and verify connect/disconnect, receive/send, timeout, reconnect, link statistics and routing across simultaneous channels.
