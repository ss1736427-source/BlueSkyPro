---
id: VERIFICATION-REGISTER-001
type: verification_register
status: draft_for_agreement
version: 0.3
parent: VERIFICATION
authority: NAVIGATION_VERIFICATION_MODEL-001; NAVIGATION_TEST_VECTORS-001; C2-VERIFICATION-CASES-001
---

# BlueSky PRO — Verification Register

## 1. Назначение

Единый реестр идентичности verification cases проекта. Реестр не является реестром результатов испытаний и не заменяет Evidence.

Правило:

```text
ONE VERIFICATION CASE
→ ONE STABLE ID
→ CONTROLLED DEFINITION
→ MANY REQUIREMENT / DESIGN RELATIONSHIPS
```

## 2. Граница

Реестр отделяет:

```text
Verification Case Definition
≠
Test Execution Result
≠
Evidence
```

Наличие записи `READY` не означает, что проверка выполнена.

## 3. Источник идентичности

Первичные verification IDs, уже определённые в Navigation Verification Model, сохраняются:

```text
NAV-V01 … NAV-V20
```

Математические тестовые векторы имеют отдельную идентичность:

```text
NAV-TV-001 … NAV-TV-010
```

C2 cases имеют отдельную устойчивую идентичность:

```text
C2-V01 … C2-V08
```

`NAV-V*`, `NAV-TV-*` и `C2-V*` не объединяются в один набор ID.

## 4. Verification hierarchy

```text
Requirement / Design Basis
        ↓
Verification Method
        ↓
Verification Case
        ↓
Test Vector / Dataset
        ↓
Execution
        ↓
Result
        ↓
Evidence
```

## 5. Current controlled Navigation cases

| ID | Case | Basis | Status |
|---|---|---|---|
| NAV-V01 | No wind | Navigation Verification Model | DEFINED |
| NAV-V02 | Crosswind | Navigation Verification Model | DEFINED |
| NAV-V03 | Headwind | Navigation Verification Model | DEFINED |
| NAV-V04 | Tailwind | Navigation Verification Model | DEFINED |
| NAV-V05 | Changing wind | Navigation Verification Model | DEFINED |
| NAV-V06 | Route deviation | Navigation Verification Model | DEFINED |
| NAV-V07 | WP transition | Navigation Verification Model | DEFINED |
| NAV-V08 | Stale navigation | Navigation Verification Model | DEFINED |
| NAV-V09 | Invalid navigation | Navigation Verification Model | DEFINED |
| NAV-V10 | Missing critical input | Navigation Verification Model | DEFINED |
| NAV-V11 | Conflicting sources | Navigation Verification Model | DEFINED |
| NAV-V12 | Degraded source | Navigation Verification Model | DEFINED |
| NAV-V13 | Communication degradation | Navigation Verification Model | DEFINED |
| NAV-V14 | Energy degradation | Navigation Verification Model | DEFINED |
| NAV-V15 | Dynamic Return degradation | Navigation Verification Model | DEFINED |
| NAV-V16 | Material runtime change | Navigation Verification Model | DEFINED |
| NAV-V17 | Revalidation required | Navigation Verification Model | DEFINED |
| NAV-V18 | Safety rejection | Navigation Verification Model | DEFINED |
| NAV-V19 | Multi-UAV | Navigation Verification Model | DEFINED |
| NAV-V20 | Planned vs Simulated vs Actual | Navigation Verification Model | DEFINED |

## 6. Current Navigation test vectors

| ID | Function | Source document | Status |
|---|---|---|---|
| NAV-TV-001 | Ground velocity | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-002 | Groundspeed | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-003 | Track | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-004 | WCA | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-005 | Drift | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-006 | Cross-track error | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-007 | Along-track position | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-008 | ETA | NAVIGATION_TEST_VECTORS_001 | READY |
| NAV-TV-009 | Return feasibility | NAVIGATION_TEST_VECTORS_001 | REQUIRES ENERGY MODEL |
| NAV-TV-010 | Coordinate reference | NAVIGATION_TEST_VECTORS_001 | REQUIRES CRS BASELINE |

## 7. Reconciliation rule

Before adding a new verification ID:

```text
SEARCH THIS REGISTER
        ↓
SEARCH EXISTING VERIFICATION DOCUMENTS
        ↓
CHECK WHETHER COVERAGE ALREADY EXISTS
        ↓
LINK EXISTING CASE
OR
IDENTIFY REAL COVERAGE GAP
        ↓
CREATE NEW CASE ONLY IF GAP IS CONFIRMED
```

No duplicate case is created merely because a new module specification appears.

## 8. Requirement traceability

Each case must identify its actual requirement/design basis as soon as reconciliation is complete.

The current Navigation requirement allocation is maintained in:

`05_VERIFICATION/NAVIGATION/NAVIGATION_REQUIREMENT_ALLOCATION_001.md`

That allocation is a working reconciliation record and does not promote candidate requirements to authoritative status.

Required fields:

```text
Verification ID
Requirement ID / Design ID
Basis document
Verification method
Input / dataset
Expected result
Acceptance criterion
Execution configuration
Result
Evidence reference
Reviewer
Status
Change ID
```

Unknown values are `TBD`.

## 9. Verification methods

Allowed project methods include:

```text
TEST
ANALYSIS
INSPECTION
SIMULATION
DEMONSTRATION
```

The selected method must be appropriate to the verification objective.

## 10. Status model

```text
IDENTIFIED
DEFINED
READY
EXECUTED
PASSED
FAILED
BLOCKED
SUPERSEDED
WITHDRAWN
```

`PASSED` requires recorded execution result and evidence reference.

## 11. Evidence rule

```text
Verification Case
→ Execution
→ Result
→ Evidence
```

A verification definition, mathematical relation, or documented expected result is not evidence of execution.

## 12. Navigation allocation

The current Navigation Verification Model is the source for the twenty `NAV-V*` case definitions. The Navigation Test Vectors document is the source for the ten `NAV-TV-*` deterministic mathematical vectors.

The controlled relationship is:

```text
MASTER-REQUIREMENTS-REGISTER-001
        ↓
NAVIGATION-REQUIREMENT-ALLOCATION-001
        ↓
NAVIGATION-TRACEABILITY-MATRIX-001
        ↓
NAV-V* / NAV-TV-*
        ↓
Execution
        ↓
Evidence
```

These documents are linked through the register; their contents are not duplicated here.

## 13. C2 verification allocation

The C2 verification case definitions are controlled in:

`05_VERIFICATION/C2/C2_VERIFICATION_CASES_001.md`

The stable case identities are:

```text
C2-V01 … C2-V08
```

The current case-to-design/interface allocation is:

| Case | Requirement / design basis | Interface | Execution configuration | Execution result | Evidence |
|---|---|---|---|---|---|
| C2-V01 | `C2-REQ-001`; `IF-C2-001`; `DEC-012`; `SYS-REQ-083` | `IV-C2-001` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |
| C2-V02 | `C2-REQ-003`; `SAF-REQ-001..003`; `IF-C2-001` | `IV-C2-002` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |
| C2-V03 | `C2-REQ-003`, `C2-REQ-008`; `SAF-REQ-003`, `SAF-REQ-013`; `SYS-REQ-091`; `IF-C2-001` | `IV-C2-003` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |
| C2-V04 | `C2-REQ-002`, `C2-REQ-003`, `C2-REQ-005`; `SAF-REQ-001..004`, `SAF-REQ-013..014`; `IF-C2-001` | `IV-C2-004` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |
| C2-V05 | `C2-REQ-002`, `C2-REQ-005`, `C2-REQ-006`; `SAF-REQ-013`; `SYS-REQ-081`, `SYS-REQ-086`, `SYS-REQ-093`; `C2-F-001..003` | `IV-C2-005` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |
| C2-V06 | `C2-REQ-005`, `C2-REQ-006`, `C2-REQ-009`; `SAF-REQ-001..004`, `SAF-REQ-013..015`; `SYS-REQ-086`, `SYS-REQ-093`; `IF-C2-001` | `IV-C2-006` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |
| C2-V07 | `C2-REQ-001`, `C2-REQ-004`; `SYS-REQ-083`; `DEC-012`; `IF-C2-001` | `IV-C2-007` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |
| C2-V08 | `C2-REQ-008`; `SAF-REQ-017`; `IF-C2-001`; `CONFIGURATION-BASELINE.md` | `IV-C2-008` | `C2-VER-CFG-001` | `C2-VER-EXEC-001` | `C2-EVIDENCE-INDEX-001` |

The case definitions are `DEFINED`; the configuration is `STUB / PLANNED`, the execution result is `NOT EXECUTED — STUB`, and the evidence index is empty. These states are controlled documentation states and do not constitute verification evidence.

The C2 definition/configuration control closure is recorded in:

`05_VERIFICATION/C2/C2_VERIFICATION_CONTROL_CLOSURE_001.md`

The exact pre-execution content/scope reconciliation is recorded in:

`05_VERIFICATION/C2/C2_RECONCILIATION_PASS_003.md`

The quantitative C2 basis dependency is controlled in:

`05_VERIFICATION/C2/C2_QUANTITATIVE_PARAMETERS_BASIS_001.md`

It remains an explicit **STUB / OPEN** dependency. No numerical C2 quality/QoS values are baselined until an authoritative or approved basis is available.

The controlled execution-result record is:

`05_VERIFICATION/C2/C2_VERIFICATION_EXECUTION_RESULT_001.md`

The evidence index is:

`05_VERIFICATION/C2/C2_EVIDENCE_INDEX_001.md`

The pre-execution gate is:

`05_VERIFICATION/C2/C2_EXECUTION_PREREQUISITE_CHECK_001.md`

## 14. Audit findings — current pass

```text
V-REG-001  Verification Register is the single identity register for controlled verification cases.
V-REG-002  NAV-V01..NAV-V20 exist as case definitions in NAVIGATION_VERIFICATION_MODEL.
V-REG-003  NAV-TV-001..NAV-TV-010 exist as mathematical vectors in NAVIGATION_TEST_VECTORS_001.
V-REG-004  Execution evidence is not implied by DEFINED / READY status.
V-REG-005  Navigation requirement allocation remains a controlled working reconciliation record.
V-REG-006  C2-VER-GAP-001 is closed for definition/configuration control; execution and evidence remain open.
V-REG-007  C2 pre-execution content/scope reconciliation is complete; no proven requirement gap or conflict was identified.
V-REG-008  C2 configuration, execution-result and evidence records are controlled stubs and shall be replaced with actual records at test stage.
V-REG-009  C2 quantitative basis dependency remains explicitly open; no numerical acceptance value is inferred.
V-REG-010  The C2 case-to-requirement/design-to-interface-to-execution-to-evidence chain is now explicitly registered.
```

## 15. Completion gate

```text
[ ] all existing verification IDs inventoried
[ ] duplicates checked
[ ] requirement/design basis assigned
[ ] verification method assigned
[ ] test vectors/datasets linked
[ ] acceptance criteria controlled
[x] C2 case identities controlled
[x] C2 requirement/design allocation controlled
[x] C2 interface allocation controlled
[x] C2 execution configuration linked
[x] C2 execution-result record linked
[x] C2 evidence index linked
[ ] execution evidence linked after real testing
[ ] regression impact linked to change control
[ ] certification-relevant verification identified
[ ] master index updated
```

## 16. Status

**DRAFT_FOR_AGREEMENT — C2 PRE-EXECUTION TRACEABILITY INTEGRATED; REAL EXECUTION AND EVIDENCE REMAIN DEFERRED.**

This register establishes verification identity and traceability control. It does not declare any requirement or verification as completed merely by registration.
