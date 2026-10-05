# PH4-INT-006 — Operational Failure Evidence Binding

**Status:** IMPLEMENTATION_BASELINE — P0

## Objective

Connect structured failure outcomes from existing PH4 service boundaries to the common operational evidence/audit representation without introducing a second execution or decision model.

## Flow

`existing decision evidence + service failure result`
→ `failure evidence binding`
→ `common operational evidence record`

## Rules

1. The binding consumes facts already produced by the service boundary.
2. Decision ID and selected action ID are preserved.
3. The resulting state transition is preserved.
4. The original failure result and reason are preserved.
5. Source interface and target object remain explicit.
6. The existing `EC-20` fault/anomaly evidence domain is reused.
7. The binding performs no execution, retry, authorization, safety evaluation or acceptance decision.
8. The binding does not replace the verification evidence framework; it provides the operational bridge into it.
9. No new failure semantics are created.

## Evidence target

`SIL_OPERATIONAL_FAILURE_EVIDENCE_BINDING`

## Gate

Complete after CI passes on the branch and the merged main baseline.
