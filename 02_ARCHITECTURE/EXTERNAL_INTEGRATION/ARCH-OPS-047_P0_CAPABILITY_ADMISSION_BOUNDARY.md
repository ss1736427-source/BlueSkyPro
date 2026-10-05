# ARCH-OPS-047 — P0 Capability Admission Boundary

**Status:** IMPLEMENTATION_BASELINE

## Objective

Create the deterministic admission boundary that consumes capability compatibility results before a command or mission action is allowed to proceed toward execution.

## Contract

Input:
- device identifier;
- action identifier;
- required capability identifiers.

Flow:

`action request → capability compatibility gate → admission decision → execution path`

Rules:

1. Invalid requests are rejected.
2. Unknown devices are rejected.
3. Capability state must be `Validated`.
4. Missing capabilities reject admission.
5. Only `Compatible` may produce an admitted decision.
6. The boundary is read-only with respect to device capability state.
7. No command or mission execution occurs inside this boundary.

## Result

- `Admitted`
- `Rejected`

Every rejection carries the compatibility result and deterministic reason.

## Boundary exclusions

Authentication, transport, MAVLink parsing, command dispatch, mission execution, persistence, and policy authorization remain outside this layer.

## Evidence

Target: `SIL_CAPABILITY_ADMISSION_BOUNDARY`

The SIL test covers admitted compatible actions and rejection of invalid, unknown, unvalidated, and capability-missing actions.

## Closure

The admission boundary is connected to mission/command execution by ARCH-OPS-048. The historical next-gap statement is closed.

Connect this admission boundary to the existing mission transfer and command dispatch entry points so incompatible actions cannot reach execution.
