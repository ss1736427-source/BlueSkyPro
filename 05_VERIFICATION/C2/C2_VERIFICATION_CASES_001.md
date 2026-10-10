---
id: C2-VERIFICATION-CASES-001
type: c2_verification_case_definitions
status: controlled_working_draft
system: BlueSky PRO
basis: C2-ICD-BASELINE-001; C2_SAFETY_ALLOCATION_001; VERIFICATION-PLAN-001; C2-RECONCILIATION-PASS-003; C2-QUANTITATIVE-PARAMETERS-BASIS-001; C2-CLAUSE-LEVEL-MAPPING-001; C2-REQUIREMENTS-RECONCILIATION-001
---

# BLUE SKY PRO — C2 VERIFICATION CASE DEFINITIONS 001

## 1. Purpose

Define the controlled verification cases allocated to the C2 interface and safety records. This document defines case identity, scope and expected verification outcomes; it does not record execution, pass/fail results or evidence.

The cases address the existing `IV-C2-001..IV-C2-008` allocation and do not create new system or safety requirements.

## 2. Control boundary

The cases verify BlueSky responsibilities within `IF-C2-001`. Provider-side infrastructure and service obligations remain external unless the approved certification scope assigns them to BlueSky.

Quantitative acceptance values remain `TBD` until the applicable operational/certification basis establishes them. No case in this record declares compliance.

## 3. Case definitions

| Case ID | Existing allocation | Objective | Method | Status |
|---|---|---|---|---|
| C2-V01 | IV-C2-001 | Verify the controlled C2 message/state schema and required fields are identified and processed consistently. | Inspection + integration test | DEFINED |
| C2-V02 | IV-C2-002 | Verify units and semantic meaning of C2 parameters are explicit and interpreted consistently. | Inspection + analysis | DEFINED |
| C2-V03 | IV-C2-003 | Verify timestamps, common time reference and freshness handling support ordering and validity decisions. | Analysis + test | DEFINED |
| C2-V04 | IV-C2-004 | Verify validity and quality information is evaluated before safety-significant C2 state is accepted. | Test + failure injection | DEFINED |
| C2-V05 | IV-C2-005 | Verify NORMAL, DEGRADED, LOST and RESTORED transitions follow controlled criteria and are not silently conflated. | Scenario test | DEFINED |
| C2-V06 | IV-C2-006 | Verify missing, invalid, stale, delayed, duplicated, out-of-order, conflicting, corrupted and unauthorized inputs produce the allocated safe response. | Failure injection + scenario test | DEFINED |
| C2-V07 | IV-C2-007 | Verify DIRECT-C2 and PROVIDER-C2 responsibilities are distinguished at the controlled interface boundary. | Inspection + integration test | DEFINED |
| C2-V08 | IV-C2-008 | Verify incompatible or unexpected interface versions are detected and handled under configuration control. | Inspection + integration test | DEFINED |

## 4. Regulatory clause allocation

The case set is directly linked to the clause-level C2 mapping. This linkage identifies the regulatory basis for verification; it does not constitute regulatory compliance evidence.

| Case | Regulatory clause mapping | Verification relevance |
|---|---|---|
| C2-V01 | `C2-142-001`, `C2-142-003`, `C2-142-005`, `C2-142-007..009` | C2 organization, interface/data exchange and operating-mode representation |
| C2-V02 | `C2-142-005`, `C2-142-009` | Explicit data composition, units and semantics |
| C2-V03 | `C2-142-005`, `C2-142-011`, `C2-142-013..014`, `C2-142-021` | Time, freshness, service quality and event ordering |
| C2-V04 | `C2-142-004`, `C2-142-011`, `C2-142-013..014` | Quality/validity gating and service-state acceptance |
| C2-V05 | `C2-142-018`, `C2-142-024`, `C2-142-025` | C2 state monitoring, loss/degradation classification and timing |
| C2-V06 | `C2-142-018`, `C2-142-023..025` | Fault detection, classification and allocated safe response |
| C2-V07 | `C2-142-001`, `C2-142-003`, `C2-142-008`, `C2-142-016..017` | DIRECT-C2 / PROVIDER-C2 responsibility boundary |
| C2-V08 | `C2-142-005`, `C2-142-009`, `C2-142-021` | Controlled interface/data version and record integrity handling |

The authoritative regulatory interpretation remains `C2_CLAUSE_LEVEL_MAPPING_001.md`; this table is a verification allocation, not an independent regulatory interpretation.

## 5. Requirement/design allocation

The following table makes the current pre-execution trace explicit. `C2-REQ-*` are subordinate working allocations; the authoritative requirement identity remains in the Master Requirements Register.

| Case | Working requirement allocation | Existing authoritative/design basis | Allocation status |
|---|---|---|---|
| C2-V01 | `C2-REQ-001` | `IF-C2-001`, `DEC-012`, `SYS-REQ-083` | DERIVED |
| C2-V02 | `C2-REQ-003` | `SAF-REQ-001..003`, `IF-C2-001` | DERIVED |
| C2-V03 | `C2-REQ-003`, `C2-REQ-008` | `SAF-REQ-003`, `SAF-REQ-013`, `SYS-REQ-091`, `IF-C2-001` | DERIVED |
| C2-V04 | `C2-REQ-002`, `C2-REQ-003`, `C2-REQ-005` | `SAF-REQ-001..004`, `SAF-REQ-013..014`, `IF-C2-001` | MERGE / DERIVED |
| C2-V05 | `C2-REQ-002`, `C2-REQ-005`, `C2-REQ-006` | `SAF-REQ-013`, `SYS-REQ-081`, `SYS-REQ-086`, `SYS-REQ-093`, `C2-F-001..003` | MERGE |
| C2-V06 | `C2-REQ-005`, `C2-REQ-006`, `C2-REQ-009` | `SAF-REQ-001..004`, `SAF-REQ-013..015`, `SYS-REQ-086`, `SYS-REQ-093` | MERGE / DERIVED |
| C2-V07 | `C2-REQ-001`, `C2-REQ-004` | `SYS-REQ-083`, `DEC-012`, `IF-C2-001` | DERIVED |
| C2-V08 | `C2-REQ-008` | `SAF-REQ-017`, `IF-C2-001`, `CONFIGURATION-BASELINE.md` | DERIVED |

No candidate requirement is promoted to `BASELINED` by this document.

## 6. Current basis and qualitative acceptance criteria

| Case | Current requirement/design basis | Qualitative acceptance criterion |
|---|---|---|
| C2-V01 | `IF-C2-001`; C2 mode/interface allocation; `DEC-012` | The controlled message/state fields, direction and operating-mode distinction are identified, processed consistently, and rejected or reported when structurally invalid. |
| C2-V02 | `SAF-REQ-001`, `SAF-REQ-002`, `SAF-REQ-003`; `IF-C2-001` | Each exercised parameter has an explicit unit and semantic meaning; incompatible or ambiguous interpretation is detected rather than accepted as valid input. |
| C2-V03 | `SAF-REQ-003`, `SAF-REQ-013`; `SYS-REQ-091`; `IF-C2-001` | Events and state data can be ordered using the controlled time reference, and stale or temporally invalid data is not accepted as current. |
| C2-V04 | `SAF-REQ-001..004`, `SAF-REQ-013`, `SAF-REQ-014`; `IF-C2-001` | Validity and quality are evaluated before safety-significant state use; invalid or stale status produces the allocated degraded, rejected, or quarantined handling. |
| C2-V05 | `SAF-REQ-013`; `SYS-REQ-081`, `SYS-REQ-086`, `SYS-REQ-093`; `C2-F-001..003` | NORMAL, DEGRADED, LOST and RESTORED transitions are distinguishable, observable, time-ordered, and follow the allocated transition and recovery logic. |
| C2-V06 | `SAF-REQ-001..004`, `SAF-REQ-013..015`; `SYS-REQ-086`, `SYS-REQ-093`; `IF-C2-001` | Each listed abnormal input produces a deterministic response covered by the allocated safety/interface behaviour, with no unsafe silent fallback to normal operation. |
| C2-V07 | `SYS-REQ-083`; `DEC-012`; `IF-C2-001` | The two operating profiles and their responsibility boundaries are distinguishable; provider-side obligations are not represented as BlueSky implementation evidence. |
| C2-V08 | `SAF-REQ-017`; `IF-C2-001`; `CONFIGURATION-BASELINE.md` | An incompatible or unexpected interface version is detected, prevented from unvalidated use, and handled through the controlled compatibility or safe-response path. |

The criteria above are qualitative. No latency, continuity, availability, integrity, loss-detection, recovery, or coverage value is inferred by this record; such values remain `TBD` pending the applicable approved operational/certification basis.

## 7. Common case controls

Each case shall identify before execution:

```text
Requirement/design basis
Regulatory clause mapping
Configuration ID
System/software version
Interface version
Test environment
Inputs/dataset
Acceptance criteria
Reviewer
```

Execution records shall separately capture:

```text
Actual result
Pass/fail disposition
Anomalies
Evidence ID
Review record
```

Unknown values remain `TBD`; they are not treated as passing defaults.

## 8. Traceability

```text
C2 regulatory clause
→ requirement / responsibility allocation
→ C2-V01..C2-V08
→ IV-C2-001..IV-C2-008
→ C2-VER-CFG-001
→ C2-VER-EXEC-001
→ C2-EVIDENCE-INDEX-001
→ real execution
→ evidence
```

Case-level requirement/design linkage remains subordinate to the authoritative master register and the applicable approved baseline.

## 9. Evidence gate

```text
CASE DEFINITION: controlled by this record
EXECUTION RESULT: prepared as controlled stub
EVIDENCE: prepared as empty controlled stub
VERIFICATION STATUS: ALLOCATED / NOT VERIFIED
```

No `PASSED`, `VERIFIED` or certification claim is made by this document.

## 10. Controlled actions before real execution

1. Maintain `C2-VER-CFG-001` as the controlled execution-configuration stub.
2. Maintain `C2-VER-EXEC-001` as the controlled execution-result stub.
3. Maintain `C2-EVIDENCE-INDEX-001` as the empty evidence stub.
4. Maintain `C2-QUANTITATIVE-PARAMETERS-BASIS-001` as the explicit quantitative-basis dependency until an authoritative/approved basis is available.
5. At real test stage, replace the applicable stubs with actual configuration, results and evidence; do not infer `PASS` / `VERIFIED` from preparation records.

**Status: CONTROLLED WORKING DRAFT — CASE DEFINITIONS COMPLETE; REQUIREMENT/DESIGN AND REGULATORY CLAUSE ALLOCATION INTEGRATED; PRE-EXECUTION PACKAGE STRUCTURED; REAL EXECUTION OPEN.**
