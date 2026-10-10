# ARCH-OPS-049 — Operational Action Routing

**Status:** CLOSED — CI VERIFIED

## Verified

- single operational action execution router;
- exact selected-action to execution-binding match;
- explicit mission-transfer and command-dispatch routing kinds;
- explicit `device_id` and required capabilities;
- mandatory handoff through ARCH-OPS-048;
- rejection before execution when selection or capability admission fails;
- CTest coverage.

## Evidence target

SIL_OPERATIONAL_ACTION_EXECUTION_ROUTER

## CI

Verified through ARCH-OPS-049 pull-request CI; merged to `main`.

## Closure

External operational request intake is provided by ARCH-OPS-050 and bound to this router by ARCH-OPS-051.
