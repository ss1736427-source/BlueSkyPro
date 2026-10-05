# ARCH-OPS-043 — Device identity runtime

**Status:** CLOSED — CI VERIFIED

## Scope

Deterministic device identity registry for UAV, autopilot and component identity with credential-fingerprint uniqueness and explicit lifecycle state.

## Verified

- registration and idempotency;
- identity collision and credential collision handling;
- resolution and lifecycle transitions;
- explicit separation from MAVLink session identity and system/component addressing;
- no private-key storage or live authentication claim.

## Evidence target

SIL_DEVICE_IDENTITY_RUNTIME

## CI

Verified through the ARCH-OPS-043 pull-request CI gate. The implementation was merged to `main`.

## Boundary

Cryptographic validation, key provisioning, certificate trust and live-link authentication remain outside this slice.

## Next

Continue from the already integrated capability/session chain; do not reopen this closed runtime slice.
