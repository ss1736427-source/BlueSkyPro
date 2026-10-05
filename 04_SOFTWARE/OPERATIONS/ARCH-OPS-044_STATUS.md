# ARCH-OPS-044 — Device Capability Runtime

**Status:** CLOSED — CI VERIFIED

## Verified

- deterministic capability registration and update;
- capability normalization and deduplication;
- lifecycle transition `Discovered → Validated`;
- immutable resolve-by-device snapshot;
- missing-device handling;
- CMake/CTest registration.

## Evidence

SIL_DEVICE_CAPABILITY_RUNTIME

## CI

Verified through ARCH-OPS-044 pull-request CI; merged to `main`.

## Boundary

No live transport, authentication, vendor discovery, certificate handling, persistence or mission execution is claimed.

## Closure

Capability discovery and compatibility were subsequently integrated by ARCH-OPS-045…047. This status no longer carries those historical next-gap statements.
