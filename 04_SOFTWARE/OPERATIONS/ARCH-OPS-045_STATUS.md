# ARCH-OPS-045 — Capability Discovery Boundary

**Status:** CLOSED — CI VERIFIED

## Verified

- identity admission;
- revoked-device rejection;
- normalized capability registration;
- explicit capability update;
- CTest coverage.

## Evidence

SIL_CAPABILITY_DISCOVERY_BOUNDARY

## CI

Verified through ARCH-OPS-045 pull-request CI; merged to `main`.

## Boundary

No live MAVLink parsing, authentication, certificate validation, transport management or mission execution is claimed.

## Closure

Capability compatibility is subsequently provided by ARCH-OPS-046. The historical next-gap statement is closed by downstream implementation.
