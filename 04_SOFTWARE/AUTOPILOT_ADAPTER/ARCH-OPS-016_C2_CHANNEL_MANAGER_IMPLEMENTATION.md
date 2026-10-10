# ARCH-OPS-016 — C2 Channel Manager Implementation

**Status:** IMPLEMENTATION BASELINE — contract/test level

## Purpose

Provide a deterministic transport-independent Channel Manager behind the existing C2 abstraction. The manager selects a qualified channel using current channel state and measured communication properties and identifies when failover is required.

## Implemented boundary

```text
BlueSky C2 abstraction
        ↓
C2 Channel Manager
        ↓
channel snapshots
        ↓
qualified channel selection
        ↓
transport-specific C2 driver
```

The implementation does not own transport I/O, autopilot semantics, vehicle failsafe behaviour, runtime state transitions, UI, or numerical safety thresholds.

## Contract behaviour

A channel is qualified only when it is in an operational state (`Available`, `Active`, or `Standby`), has valid integrity/authentication state, and, for command-critical traffic, has non-zero declared capacity.

Selection is deterministic. Priority is considered first, followed by integrity/authentication state, latency, packet loss, capacity for command-critical traffic, and channel ID as a final stable tie-breaker.

`requestFailover()` reports whether the active channel is no longer qualified and an alternative qualified channel exists. It does not perform the physical switch.

## Safety and authority boundary

- The manager does not declare a command executed.
- The manager does not replace onboard failsafe logic.
- The manager does not silently modify safety or regulatory configuration.
- Exact timeout, hysteresis and acceptance thresholds remain controlled requirements/configuration and are not invented here.
- Actual channel switching and transport recovery remain integration responsibilities.

## Verification

The contract test covers:

1. primary channel selection;
2. degraded primary with qualified backup;
3. no qualified channel;
4. stable priority selection;
5. integrity failure exclusion.

The dedicated GitHub Actions workflow compiles with C++20, warnings-as-errors and runs the contract test.

## Next dependency

With the C2 abstraction and deterministic channel-selection boundary available at contract/test level, the next P0 dependency is normalized telemetry/state input. Runtime state-machine implementation must consume normalized observations rather than transport-specific frames.

SIL/HIL and real-UAV verification remain pending.
