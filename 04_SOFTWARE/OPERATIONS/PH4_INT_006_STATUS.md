# PH4-INT-006 — Operational Failure Evidence Binding

id: PH4-INT-006
status: IMPLEMENTATION_BASELINE

## Scope

Bind structured failure results from existing service boundaries into the common operational evidence/audit representation without duplicating execution, authority, safety or verification semantics.

## Delivered

- preserves the existing decision identity and selected action identity;
- preserves the resulting operational state transition;
- carries structured failure result and original reason;
- records source interface and target object references;
- assigns the existing common evidence domain `EC-20 — Faults / Anomalies / Recovery`;
- executable CMake/CTest contract fixture.

## Boundary

This component only translates already-produced facts into the common operational evidence shape. It does not execute actions, change failure results, retry operations, authorize execution or determine acceptance.

## Evidence target

SIL_OPERATIONAL_FAILURE_EVIDENCE_BINDING

## Next deterministic gap

Audit the remaining PH4 service-boundary chain for any unbound structured result that would bypass the common evidence representation before adding another implementation.
