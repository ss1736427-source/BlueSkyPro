# ARCH-OPS-020 — Mission Transfer Runtime

## Status
IMPLEMENTATION_BASELINE

## Objective
Close the smallest remaining G0-04 runtime slice beneath the existing Mission Interface contract.

## Delivered
- Vehicle capability compatibility gate before mission transfer.
- Reuse of the canonical Mission Model and existing vehicle-specific compiler boundary.
- Mission compilation through the Universal Autopilot Adapter.
- Connected-state gate before transfer.
- Mission upload.
- Mission read-back.
- Deterministic planned/external mission integrity comparison.
- Explicit rejection/failure reasons and deterministic CTest coverage.

## Boundary
This is a protocol-independent mission transfer lifecycle at the Universal Autopilot Adapter boundary. It does not claim:
- real MAVLink wire transport;
- autopilot-specific mission-item semantics;
- physical vehicle upload;
- HIL or flight-test evidence;
- rollback of a partially accepted physical upload.

## Remaining G0-04 integration work
- Concrete MAVLink Mission Protocol item mapping/translation.
- Vehicle-specific semantic verification beyond the deterministic adapter representation.
- Physical transport and end-to-end vehicle evidence.

## Evidence class
SIL_FIXTURE_ONLY / IMPLEMENTATION_BASELINE.
