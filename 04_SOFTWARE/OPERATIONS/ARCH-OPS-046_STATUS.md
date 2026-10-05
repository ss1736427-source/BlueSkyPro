# ARCH-OPS-046 — Capability Compatibility Gate

**Status:** CLOSED — CI VERIFIED

## Verified

- deterministic capability compatibility request;
- validated capability lifecycle prerequisite;
- normalization and deduplication;
- explicit missing-capability result;
- unknown and unvalidated device handling;
- CTest coverage.

## Evidence target

SIL_CAPABILITY_COMPATIBILITY_GATE

## CI

Verified through ARCH-OPS-046 pull-request CI; merged to `main`.

## Boundary

Authentication, transport, MAVLink/vendor parsing, command dispatch, mission execution and persistence remain outside this gate.

## Closure

Compatibility results are consumed by ARCH-OPS-047 admission and ARCH-OPS-048 execution integration.
