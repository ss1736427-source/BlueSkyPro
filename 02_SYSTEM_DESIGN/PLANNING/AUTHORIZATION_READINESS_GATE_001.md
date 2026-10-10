# BlueSky PRO — Authorization Readiness Gate

**ID:** PLAN-READY-002
**Status:** BASELINED
**Scope:** Planning / Regulatory Authorization / Readiness

## Purpose

Provide a deterministic readiness contract for operations located within a regulatory restriction where an explicit authorization is required.

This is an additional gate to the constrained-open-space model. It does not replace spatial planning and does not remove physical obstacles.

## Contract

When a regulatory restriction applies and authorization is required:

`RESTRICTED -> AUTHORIZATION REQUIRED -> NOT READY`

The operation can reach authorization-qualified readiness only when:
1. authorization evidence is present;
2. authorization is current and valid;
3. authorization scope matches the operation;
4. pilot confirmation is present;
5. all other independent readiness gates pass.

The pilot confirmation is not itself an authorization source.

## States

- `NotApplicable`: no applicable regulatory authorization gate.
- `Block`: required authorization is missing, invalid or out of scope.
- `Review`: authorization is valid and in scope, but pilot confirmation is still required.
- `Ready`: authorization conditions are satisfied. This result does not override other readiness gates.

## Boundary

The gate:
- does not grant regulatory permission;
- does not modify authoritative airspace data;
- does not remove terrain/obstacle constraints;
- does not execute flight;
- does not replace safety, insurance, technical, weather or other readiness gates;
- does not create a second spatial calculation.

Authorization validity/scope is supplied by the authoritative authorization layer. The gate only evaluates the deterministic readiness contract.

## Relationship to route planning

The validated authorization state is an input to the authorization-qualified constrained environment. Route search may use the authorized regulatory state, while physical obstacles remain hard constraints.

The same state is exposed to Flight Chart and readiness so that the operator sees the restriction, authorization status and remaining constraints consistently.
