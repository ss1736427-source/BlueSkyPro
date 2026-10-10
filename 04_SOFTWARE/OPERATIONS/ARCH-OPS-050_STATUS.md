# ARCH-OPS-050 — External Operational Request Intake

**Status:** CLOSED — CI VERIFIED

## Verified

- external operational request envelope;
- mandatory request/action/device identity validation;
- explicit required capabilities;
- mandatory entry through `run_orchestrator_pipeline()`;
- exact selected-action match;
- deterministic rejection for blocked/non-executable/non-selected actions;
- no direct execution in the intake boundary.

## Evidence target

SIL_EXTERNAL_OPERATIONAL_REQUEST_INTAKE

## CI

Verified through ARCH-OPS-050 pull-request CI; merged to `main`.

## Closure

Accepted intake results are bound to ARCH-OPS-049 by ARCH-OPS-051.
