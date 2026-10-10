---
id: NAVIGATION-VERIFICATION-COVERAGE-RECONCILIATION-001
type: navigation_verification_coverage_reconciliation
status: working_baseline
parent: NAVIGATION-TRACEABILITY-MATRIX-001
---

# BlueSky PRO — Navigation Verification Coverage Reconciliation 001

## 1. Purpose

Reconcile `NAV-V01…NAV-V20` against the existing verification identity register, test vectors and evidence index without creating duplicate verification IDs or treating planned evidence as executed evidence.

## 2. Controlled verification identities

The project verification register establishes `NAV-V01…NAV-V20` as the stable Navigation verification cases. `NAV-TV-001…NAV-TV-010` are separate mathematical test-vector identities. fileciteturn322file0L17-L23

## 3. Coverage result

| Verification case | Existing case | Test-vector support | Evidence allocation | Status |
|---|---|---|---|---|
| NAV-V01 | YES | NAV-TV-001…005 | EVD-006 | PLANNED |
| NAV-V02 | YES | NAV-TV-003…005 | EVD-006 | PLANNED |
| NAV-V03 | YES | NAV-TV-001…005 | EVD-006 | PLANNED |
| NAV-V04 | YES | NAV-TV-001…005 | EVD-006 | PLANNED |
| NAV-V05 | YES | NAV-TV-001…005 | EVD-006 | PLANNED |
| NAV-V06 | YES | NAV-TV-006,007 | EVD-010 | PLANNED |
| NAV-V07 | YES | NAV-TV-006,007 | EVD-010 | PLANNED |
| NAV-V08 | YES | quality/failure scenarios in Verification Model | EVD-007 | PLANNED |
| NAV-V09 | YES | quality/failure scenarios in Verification Model | EVD-007 | PLANNED |
| NAV-V10 | YES | quality/failure scenarios in Verification Model | EVD-007 | PLANNED |
| NAV-V11 | YES | quality/failure scenarios in Verification Model | No dedicated EVD identified | GAP — evidence allocation |
| NAV-V12 | YES | quality/failure scenarios in Verification Model | No dedicated EVD identified | GAP — evidence allocation |
| NAV-V13 | YES | runtime/system scenario in Verification Model | No dedicated EVD identified | GAP — evidence allocation |
| NAV-V14 | YES | NAV-TV-009 requires energy model | No dedicated EVD identified | GAP — prerequisite/evidence allocation |
| NAV-V15 | YES | NAV-TV-009 requires energy model | No dedicated EVD identified | GAP — prerequisite/evidence allocation |
| NAV-V16 | YES | runtime adaptation scenario | No dedicated EVD identified | GAP — evidence allocation |
| NAV-V17 | YES | runtime adaptation scenario | No dedicated EVD identified | GAP — evidence allocation |
| NAV-V18 | YES | safety negative scenario | No dedicated EVD identified | GAP — evidence allocation |
| NAV-V19 | YES | multi-UAV scenario | No dedicated EVD identified | GAP — evidence allocation |
| NAV-V20 | YES | NAV-TV-010 / state semantics | No dedicated EVD identified | GAP — evidence allocation |

## 4. Evidence interpretation

The evidence index currently allocates `EVD-006…EVD-010` to `V-NAV-001…V-NAV-005` and marks them `PLANNED`. The index explicitly requires execution result, configuration, review and acceptance before evidence can constitute compliance evidence. fileciteturn316file0L15-L18 fileciteturn316file0L31-L36

Therefore no `NAV-V11…NAV-V20` case is declared executed or accepted by this reconciliation.

## 5. Existing structure

The Navigation Verification Model defines all twenty scenarios, including conflicting sources, degraded source, communication degradation, energy degradation, Dynamic Return degradation, runtime change, revalidation, safety rejection, Multi-UAV and Planned/Simulated/Actual separation. fileciteturn320file0L45-L65

The Navigation Traceability Matrix already allocates these cases to Navigation modules and records the applicable engineering rules. fileciteturn321file0L38-L53

## 6. Findings

- `NAV-V01…NAV-V20`: existing stable verification identities; no new duplicate cases required.
- `NAV-TV-001…NAV-TV-010`: existing separate test-vector identities; do not merge with `NAV-V*`.
- `EVD-006…EVD-010`: existing planned evidence allocations for the five CRB-NAV requirements.
- `NAV-V11…NAV-V20`: no dedicated evidence allocation identified in the current Evidence Index.
- `NAV-TV-009`: blocked by the energy model prerequisite.
- `NAV-TV-010`: blocked by the CRS baseline prerequisite.

## 7. Decision

**DO NOT CREATE NEW VERIFICATION IDs.**

The confirmed work item is an **evidence-allocation gap for NAV-V11…NAV-V20**, subject to reconciliation with existing system/safety verification evidence before any new EVD IDs are created.

The next action is to inspect the existing non-Navigation evidence allocations and determine whether each `NAV-V11…NAV-V20` can be linked to an existing evidence item. Only confirmed residual gaps may receive new evidence IDs.

## 8. Status

**WORKING BASELINE — RECONCILIATION OPEN**
