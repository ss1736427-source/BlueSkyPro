# ARCH-OPS-032 Status

Status: IMPLEMENTED — CI pending

## Scope
Harden the real UDP channel/session boundary after ARCH-OPS-031.

## Implemented
- Preserve the real UDP source endpoint in channel statistics.
- If a UDP remote endpoint is configured, ingress is accepted only from that endpoint.
- Rejected UDP source frames do not enter the MAVLink session.
- Rejected/duplicate/invalid session frames are observable as unsuccessful channel receive operations.
- Real UDP loopback test covers:
  - source endpoint capture;
  - in-order heartbeat;
  - sequence gap and packet-loss accounting;
  - duplicate rejection;
  - unexpected-source rejection;
  - heartbeat timeout -> LOST;
  - reconnect -> fresh RECOVERING session state;
  - post-reconnect heartbeat recovery.

## Evidence
Target evidence class: `SIL_OS_UDP_SESSION_HARDENING`.

This remains local OS loopback evidence. It does not establish radio, cellular, serial, field-network, SITL/HIL, physical-flight, or certification evidence.

## Design rationale
MAVLink system/component IDs identify network components and are used for routing; sequence numbers support packet-loss detection. BlueSky therefore keeps transport endpoint identity and MAVLink session identity separate rather than conflating them. citeturn0search0turn0search6
