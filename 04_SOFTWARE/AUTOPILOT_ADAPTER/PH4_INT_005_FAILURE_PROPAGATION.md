# PH4-INT-005 — Failure Propagation Fixture

id: PH4-INT-005
status: IMPLEMENTATION_BASELINE

## Scope

Exercise rejection and mismatch paths across the existing protocol-independent autopilot and C2 service boundaries.

## Delivered

- disconnected command rejection with structured error/reason;
- read-back semantic mismatch detection;
- invalid mission/configuration write rejection;
- C2 send rejection before connection;
- C2 authentication failure with explicit reason;
- deterministic executable fixture.

## Release rule

A read-back `MISSION_CONTENT_MISMATCH` is never treated as a successful verification result.

## Not claimed

No real C2, UAV, SIL/HIL, real-UAV, or certification verification is claimed.

## Next deterministic gap

Connect the executable integration chain to the common evidence event/evidence session boundary so every accepted or rejected execution step can be traced without changing the source event.
