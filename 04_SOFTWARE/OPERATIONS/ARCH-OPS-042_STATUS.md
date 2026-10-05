# ARCH-OPS-042 — Time synchronization runtime

**Status:** IMPLEMENTED — awaiting exact-HEAD CI verification

## Scope

Provides a deterministic runtime boundary for remote-clock offset estimation from a local send/receive pair and a remote timestamp.

## Evidence target

SIL_TIME_SYNCHRONIZATION_RUNTIME

## Semantics

- Uses the midpoint of local send/receive timestamps to estimate remote clock offset.
- Records the observed round-trip time for the same sample.
- Explicitly represents LocalOnly, Synchronizing, Synchronized and Stale states.
- Rejects invalid timestamps and timestamp regressions.
- Does not claim UTC synchronization unless the remote time source is independently established as UTC.
- Does not modify MAVLink session validation, telemetry normalization, C2 selection or failover policy.

## Verification target

A deterministic contract test demonstrates offset estimation, RTT capture, stale transition, recovery and reset.
