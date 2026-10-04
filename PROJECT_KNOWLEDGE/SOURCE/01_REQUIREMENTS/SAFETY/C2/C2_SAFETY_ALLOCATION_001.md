---
id: C2-SAFETY-ALLOCATION-001
type: safety_requirements_allocation
status: controlled_working_draft
system: BlueSky PRO
source: C2_CLAUSE_LEVEL_MAPPING_001.md
---

# BLUE SKY PRO — C2 SAFETY ALLOCATION 001

## 1. Purpose

Allocate C2-related failure conditions to the BlueSky safety model without assigning unsupported quantitative thresholds.

## 2. Safety-relevant states

```text
C2 NORMAL
C2 DEGRADED
C2 LOST
C2 RESTORED
```

Each transition shall be observable, time-ordered and recorded where required by the approved system behaviour.

## 3. Failure conditions

| ID | Condition | Source allocation | Safety effect to analyse | Required response | Verification allocation |
|---|---|---|---|---|---|
| C2-F-001 | degradation of C2 quality | C2-142-018, C2-142-020 | loss of reliable command/monitoring | transition to DEGRADED and evaluate contingency criteria | C2-V05, C2-V06 |
| C2-F-002 | complete C2 loss | C2-142-023, C2-142-024 | inability to receive/confirm commands | invoke approved lost-C2 behaviour | C2-V05, C2-V06 |
| C2-F-003 | intermittent C2 | C2-142-018, C2-142-024 | unstable command/monitoring availability | prevent unsafe oscillation between states; apply hysteresis defined in requirements | C2-V05, C2-V06 |
| C2-F-004 | invalid/stale C2 status | C2-142-018, C2-142-024 | incorrect system-state assessment | reject or quarantine invalid status and maintain safe state | C2-V03, C2-V04, C2-V06 |
| C2-F-005 | time-reference inconsistency | C2-142-005, C2-142-021 | incorrect event ordering / stale-state decisions | use controlled common time reference | C2-V03 |

## 4. Allocation principle

BlueSky shall not assume responsibility for an external C2 provider's internal implementation. Where PROVIDER-C2 is used, the safety allocation is limited to the defined interface, detection, reaction and evidence obligations of BlueSky.

## 5. Verification

Safety allocation shall be verified by:

- requirements inspection;
- failure-mode analysis;
- interface/integration tests;
- lost-C2 scenario tests;
- event-log evidence review.

The defined verification cases are `C2-V01..C2-V08`; the safety-specific failure conditions are explicitly allocated above to the applicable cases. Quantitative acceptance criteria remain OPEN until derived from the approved requirements/certification baseline.

## 6. Traceability chain

```text
C2 regulatory clause
→ C2 system/interface allocation
→ C2-F-001..005
→ safety response
→ C2-V01..C2-V08
→ execution result
→ evidence
```

The allocation is not evidence of successful verification. No `VERIFIED` or `PASSED` status is assigned by this record.

## 7. Controlled cross-reference

The source-to-safety links are maintained consistently across:

```text
C2_CLAUSE_LEVEL_MAPPING_001.md
C2_SYSTEM_REQUIREMENTS_BASELINE_001.md
C2_REQUIREMENTS_RECONCILIATION_001.md
C2_ICD_BASELINE_001.md
C2_VERIFICATION_CASES_001.md
C2_VERIFICATION_EXECUTION_RESULT_001.md
C2_EVIDENCE_INDEX_001.md
```

Where a source allocation is not sufficient to establish a quantitative safety criterion, the criterion remains `TBD` and is deferred to the applicable certification/engineering basis and real test stage.

## 8. Next step

Maintain the allocation links in the controlled ICD, verification cases, verification register and master traceability records. At real test stage, replace the applicable execution/evidence stubs with actual results and evidence.

**Status: CONTROLLED WORKING DRAFT — SAFETY ALLOCATION INTEGRATED WITH C2 REGULATORY CLAUSES AND VERIFICATION CASES; REAL EXECUTION DEFERRED.**
