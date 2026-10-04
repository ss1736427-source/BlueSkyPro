---
id: C2-TRACEABILITY-BASELINE-001
type: c2_traceability_baseline
status: controlled_working_draft
system: BlueSky PRO
basis: C2-ICD-BASELINE-001; C2-RECONCILIATION-PASS-003; C2-VERIFICATION-CASES-001; C2-VERIFICATION-CONFIGURATION-001; C2-SAFETY-ALLOCATION-001
---

# BLUE SKY PRO — C2 TRACEABILITY BASELINE

## 1. Purpose

Establish the controlled traceability chain for C2-related requirements without creating duplicate system requirement identities.

## 2. Traceability chain

```text
REG-013 / applicable clause
        ↓
existing SYS-REQ / derived allocation
        ↓
C2 interface allocation
        ↓
safety/design allocation
        ↓
verification case
        ↓
execution result
        ↓
evidence
```

## 3. Controlled records

| SYS-REQ | C2 role | Interface | Safety / design relation | Verification |
|---|---|---|---|---|
| SYS-REQ-080 | mission continuity / reallocation | IF-C2-001 | ARCH-DEC-008, 012, 039 | C2-V05/C2-V06 where applicable; no separate C2 requirement created |
| SYS-REQ-081 | UAV failure tolerance | IF-C2-001 | ARCH-DEC-017, 027, 040 | C2-V05, C2-V06 |
| SYS-REQ-082 | safe mission completion | IF-C2-001 | ARCH-DEC-016, 017, 038 | C2-V06 where applicable; no separate C2 requirement created |
| SYS-REQ-085 | safety priority | IF-C2-001 | ARCH-DEC-007, 016, 036 | C2-V06 where applicable; no separate C2 requirement created |
| SYS-REQ-086 | graceful degradation | IF-C2-001 | ARCH-DEC-017, 023, 039 | C2-V05, C2-V06 |
| SYS-REQ-091 | critical latency | IF-C2-001 | ARCH-DEC-022 | C2-V03 |
| SYS-REQ-092 | redundant recovery | IF-C2-001 | ARCH-DEC-027, 040 | C2-V06 where applicable; no separate C2 requirement created |
| SYS-REQ-093 | controlled recovery | IF-C2-001 | ARCH-DEC-017, 036 | C2-V05, C2-V06 |

## 4. Clause-to-case allocation

The current regulatory mapping is connected to the verification case set without promoting candidate requirements to the authoritative register.

| Regulatory mapping | Verification case(s) | Current status |
|---|---|---|
| `C2-142-001..009` | C2-V01, C2-V02, C2-V07 | ALLOCATED / NOT VERIFIED |
| `C2-142-004`, `C2-142-011`, `C2-142-013..014` | C2-V03, C2-V04 | ALLOCATED / NOT VERIFIED |
| `C2-142-018`, `C2-142-024..025` | C2-V05, C2-V06 | ALLOCATED / NOT VERIFIED |
| `C2-142-020` | C2-V06 / HMI allocation | ALLOCATED / NOT VERIFIED |
| `C2-142-021` | C2-V03, C2-V08 | ALLOCATED / NOT VERIFIED |
| `C2-142-016..017` | C2-V07 | ALLOCATED / NOT VERIFIED |

The authoritative clause interpretation remains `C2_CLAUSE_LEVEL_MAPPING_001.md`. The case allocation above does not constitute evidence of compliance.

## 5. Verification status

No C2 requirement is marked VERIFIED merely because an interface document exists.

`C2-RECONCILIATION-PASS-003` establishes the pre-execution content and
scope reconciliation for the controlled case set `C2-V01..C2-V08`. The
case definitions are `DEFINED` and their execution configuration
`C2-VER-CFG-001` is prepared as a controlled stub. This closes the
pre-execution definition/configuration control gate only; it does not
create an execution result, evidence, quantitative acceptance value or
compliance claim.

The following evidence classes are required as applicable:

```text
requirements inspection
interface/schema test
state-transition test
fault injection
latency/timing measurement
integration test
security/integrity verification
configuration record
```

Actual acceptance thresholds are **TBD** until established by the applicable certification basis, system requirements, safety analysis and approved verification planning.

## 6. Gaps / deferred dependencies

```text
C2-TG-001  Clause-level acceptance values TBD
C2-TG-002  Final physical C2 boundary TBD
C2-TG-003  Protocol/data schema TBD
C2-TG-004  Interface owner TBD
C2-TG-006  Evidence records deferred until real execution
C2-TG-007  Execution configuration fields remain STUB; execution results deferred
```

The quantitative dependency is controlled separately in `C2_QUANTITATIVE_PARAMETERS_BASIS_001.md`. No numerical value is inferred.

## 7. Controlled pre-execution records

```text
C2-VER-CFG-001
→ controlled execution configuration stub

C2-VER-EXEC-001
→ controlled execution result stub

C2-EVIDENCE-INDEX-001
→ controlled empty evidence stub
```

These records are temporary preparation artifacts. At the real test stage they shall be replaced with actual configuration, results and evidence. A successful status may be recorded only when the actual evidence satisfies the applicable acceptance criteria.

## 8. Baseline rule

The document remains **CONTROLLED WORKING DRAFT — NOT BASELINED** until authority, applicability, safety, architecture, verification, evidence and configuration gates are closed.

## 9. Next controlled operation

With the C2 pre-execution traceability chain structurally complete, no further C2 execution status is to be inferred without test data. Continue only with independent repository consistency work or the next approved development block; return to C2 execution when the real test stage becomes available.
