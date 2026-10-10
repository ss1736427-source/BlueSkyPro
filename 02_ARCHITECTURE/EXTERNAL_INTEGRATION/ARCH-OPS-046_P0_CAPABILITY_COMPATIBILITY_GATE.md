# ARCH-OPS-046 — P0 Capability Compatibility Gate

**Status:** IMPLEMENTATION_BASELINE

## Objective

Provide a deterministic gate between normalized device capability state and mission/command compatibility resolution.

## Contract

`CapabilityCompatibilityGate` evaluates a device against an explicit set of required capability identifiers.

Admission rules:

1. The device capability snapshot must exist.
2. The snapshot must be in `Validated` lifecycle.
3. The request must contain a non-empty device identifier.
4. The request must contain at least one non-empty required capability.
5. Required capability identifiers are normalized and deduplicated before evaluation.
6. Every required capability must be present in the validated snapshot.
7. Evaluation is read-only; it does not execute a mission or command.
8. The gate does not interpret vendor-specific protocol messages.

## Result semantics

- `Compatible` — all requested capabilities are present and validated.
- `CapabilityNotFound` — no capability snapshot exists for the device.
- `CapabilityNotValidated` — snapshot exists but is not validated.
- `MissingCapability` — at least one required capability is absent.
- `InvalidRequest` — device or capability requirements are structurally invalid.

## Boundary

This layer does not perform authentication, transport management, MAVLink parsing, mission execution, command dispatch, or persistence.

## Evidence

Target: `SIL_CAPABILITY_COMPATIBILITY_GATE`

The SIL test must cover compatible, missing-capability, unvalidated, unknown-device, invalid-request, and duplicate-requirement cases.

## Closure

Compatibility results are consumed by ARCH-OPS-047 admission. The historical next-gap statement is closed.

Bind compatibility results into the command/mission admission path without allowing incompatible actions to enter execution.
