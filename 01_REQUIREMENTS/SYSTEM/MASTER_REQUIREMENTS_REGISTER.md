---
id: MASTER-REQUIREMENTS-REGISTER-001
type: master_requirements_register
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Master Requirements Register

## 1. Назначение

Единый authoritative register идентичности требований проекта BlueSky.

Главное правило:

```text
ONE REQUIREMENT
→ ONE STABLE ID
→ ONE CONTROLLED WORDING
→ MANY RELATIONSHIPS
```

Другие документы (`SRS`, safety, architecture, design, verification) не являются второй независимой базой требований.

## 2. Источник истины

Для requirement identity используется этот register.

При конфликте документов:

```text
existing controlled requirement
→ comparison
→ impact assessment
→ controlled change
→ updated requirement record
```

Исходная запись не удаляется молча.

## 3. Requirement hierarchy

```text
EXTERNAL / REGULATORY SOURCE
        ↓
CERTIFICATION REQUIREMENT
        ↓
SYSTEM REQUIREMENT
        ↓
SUBSYSTEM / FUNCTION REQUIREMENT
        ↓
INTERFACE / DESIGN REQUIREMENT
        ↓
VERIFICATION
        ↓
EVIDENCE
```

Safety:

```text
HAZARD
→ SAFETY OBJECTIVE
→ SAFETY REQUIREMENT
→ DESIGN MITIGATION
→ VERIFICATION
→ EVIDENCE
```

## 4. Existing IDs have priority

Существующие проектные IDs сохраняются.

В частности:

```text
SYS-REQ-*
SAF-REQ-*
```

не заменяются новыми IDs только потому, что появился новый certification/system document.

Изменение существующего ID допускается только через controlled change.

## 5. Requirement record

Каждая запись должна иметь:

```text
Requirement ID
Title
Requirement Text
Type
Status
Source / Basis
Source Clause, if applicable
Parent Requirement, if applicable
Owner
Safety Relevance
Certification Relevance
Allocated Function
Allocated Subsystem
Interface
Architecture Decision
Design Element
Verification Method
Verification Case
Evidence
Configuration
Change ID
Rationale
```

Неизвестное значение обозначается `TBD`, а не выдумывается.

## 6. Status model

```text
IDENTIFIED
DRAFT
UNDER_REVIEW
AGREED
BASELINED
VERIFIED
SUPERSEDED
WITHDRAWN
```

Регуляторное approval/acceptance хранится отдельно и не выводится из слова `BASELINED`.

## 7. Existing system requirements

Подтверждённые существующие records:

```text
SYS-REQ-001..003
SYS-REQ-009..023
SYS-REQ-025..030
SYS-REQ-032..033
SYS-REQ-035
SYS-REQ-067..078
SYS-REQ-080..104
SYS-REQ-107..108
SYS-REQ-110..112
```

Это inventory подтверждённых/известных records, а не утверждение, что каждый номер внутри диапазона существует.

Полный filesystem inventory остаётся контрольной задачей.

## 8. Existing high-value system requirements

| ID | Title | Existing architecture linkage | Consolidation status |
|---|---|---|---|
| SYS-REQ-080 | Dynamic Task Reallocation | ARCH-DEC-008, 012, 039 | KEEP |
| SYS-REQ-081 | UAV Failure Tolerance | ARCH-DEC-017, 027, 040 | KEEP |
| SYS-REQ-082 | Safe Mission Completion | ARCH-DEC-016, 017, 038 | KEEP |
| SYS-REQ-083 | Platform Independence | ARCH-DEC-003, 012 | KEEP |
| SYS-REQ-084 | Resource Reservation | ARCH-DEC-036, 037 | KEEP |
| SYS-REQ-085 | Safety-Critical Priority | ARCH-DEC-007, 016, 036 | KEEP |
| SYS-REQ-086 | Graceful Degradation | ARCH-DEC-017, 023, 039 | KEEP |
| SYS-REQ-087 | AI Resource Isolation | ARCH-DEC-022, 023, 035 | KEEP |
| SYS-REQ-088 | Safe Self-Learning | ARCH-DEC-011, 025, 026 | KEEP |
| SYS-REQ-089 | HUB Resource Watchdog | ARCH-DEC-022, 024, 032 | KEEP |
| SYS-REQ-090 | Overload Protection | ARCH-DEC-022, 036 | KEEP |
| SYS-REQ-091 | Critical Latency | ARCH-DEC-022 | KEEP |
| SYS-REQ-092 | Redundant HUB Resource Recovery | ARCH-DEC-027, 040 | KEEP |
| SYS-REQ-093 | Controlled Resource Recovery | ARCH-DEC-017, 036 | KEEP |
| SYS-REQ-094 | Data Quality for Learning | ARCH-DEC-029, 041 | KEEP |
| SYS-REQ-095 | Problem and Opportunity Detection | ARCH-DEC-029, 032 | KEEP |
| SYS-REQ-096 | Solution Generation | ARCH-DEC-046, 039 | KEEP |
| SYS-REQ-097 | Solution Justification | ARCH-DEC-046, 029 | KEEP |
| SYS-REQ-098 | Improvement Validation | ARCH-DEC-007, 025 | KEEP |
| SYS-REQ-099 | Controlled Improvement Deployment | ARCH-DEC-026, 031 | KEEP |
| SYS-REQ-100 | Improvement Result Evaluation | ARCH-DEC-041, 024 | KEEP |
| SYS-REQ-101 | Failed Improvement Analysis | ARCH-DEC-017, 029 | KEEP |
| SYS-REQ-102 | Learning from Failed Improvements | ARCH-DEC-029, 041 | KEEP |
| SYS-REQ-104 | Learning Lifecycle Traceability | ARCH-DEC-029, 041 | KEEP |
| SYS-REQ-107 | Rollback Applicability and Acceptance | ARCH-DEC-026, 027 | KEEP |
| SYS-REQ-108 | Learning Subsystem Failure Containment | ARCH-DEC-017, 029 | KEEP |
| SYS-REQ-110 | Existing project requirement | Existing capability traceability | KEEP |
| SYS-REQ-111 | Existing project requirement | Existing capability traceability | KEEP |
| SYS-REQ-112 | Existing project requirement | Existing capability traceability | KEEP |

## 9. Architecture authority already established

Existing accepted architecture decisions remain authoritative until changed.

Critical chain:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

The following are not allowed to bypass this chain:

```text
AI
UI / HMI
Simulation
Optimization
Planning
```

Therefore the following new candidate requirements are not automatically independent baseline requirements:

```text
SAF-REQ-019
SAF-REQ-020
AUTH-REQ-001
AUTH-REQ-002
AI-REQ-002
```

They are mapped as candidate derived/clarifying requirements pending exact wording and source comparison.

## 10. Candidate requirement families

The following families were introduced by the new SRS:

```text
NAV-REQ-*
RTE-REQ-*
WP-REQ-*
MIS-REQ-*
RET-REQ-*
COL-REQ-*
C2-REQ-*
MUL-REQ-*
RDY-REQ-*
SAF-REQ-*
AUTH-REQ-*
HMI-REQ-*
AI-REQ-*
DATA-REQ-*
CFG-REQ-*
SW-REQ-*
HW-REQ-*
```

Their IDs are **candidate IDs** until overlap analysis is complete.

### Candidate status rule

```text
CANDIDATE
≠
AGREED
≠
BASELINED
```

## 11. Consolidation decisions already made

### 11.1 Safety Gate / authorization

Existing authoritative records:

```text
SYS-REQ-082
SYS-REQ-085
ARCH-DEC-007
ARCH-DEC-016
ARCH-DEC-017
```

Candidate records:

```text
SAF-REQ-019
SAF-REQ-020
AUTH-REQ-001
AUTH-REQ-002
AI-REQ-002
```

Decision:

```text
KEEP EXISTING RECORDS
ANALYZE CANDIDATES AS DERIVED / GAP
DO NOT DUPLICATE AUTHORITY
```

### 11.2 AI / learning

Existing:

```text
SYS-REQ-087
SYS-REQ-088
SYS-REQ-094..104
SYS-REQ-107
SYS-REQ-108
```

Related:

```text
ARCH-DEC-011
ARCH-DEC-022
ARCH-DEC-025
ARCH-DEC-026
ARCH-DEC-029
ARCH-DEC-032
ARCH-DEC-035
ARCH-DEC-041
ARCH-DEC-046
```

Candidates:

```text
AI-REQ-001
AI-REQ-002
AI-REQ-003
```

Decision:

```text
KEEP EXISTING IDs
MAP NEW CANDIDATES TO EXISTING RECORDS
CREATE A NEW REQUIREMENT ONLY IF A REAL GAP IS PROVEN
```

### 11.3 Resource / performance

Existing:

```text
SYS-REQ-084
SYS-REQ-089
SYS-REQ-090
SYS-REQ-091
SYS-REQ-092
SYS-REQ-093
```

Decision:

```text
KEEP EXISTING RECORDS
NO GENERIC DUPLICATE RESOURCE REQUIREMENTS
```

### 11.4 Degradation / recovery

Existing:

```text
SYS-REQ-081
SYS-REQ-086
SYS-REQ-093
```

Candidates:

```text
C2-REQ-002
C2-REQ-003
RET-REQ-004
```

Decision:

```text
POTENTIAL OVERLAP
EXACT WORDING / SCOPE CHECK REQUIRED
```

### 11.5 Navigation

Candidates:

```text
NAV-REQ-001..009
```

Decision:

```text
DO NOT AUTO-MERGE
COMPARE AGAINST EXISTING NAVIGATION RECORDS
COMPARE AGAINST NAVIGATION KNOWLEDGE MAP
COMPARE AGAINST ORIGINAL SOURCE
COMPARE AGAINST OFFICIAL REGULATORY SOURCES
```

## 12. Existing verification linkage

Existing verification records include:

```text
TEST-*
```

and existing traceability relationships.

Rule:

```text
Equivalent existing test
→ LINK

No coverage
→ CREATE NEW TEST CASE

Changed requirement
→ IMPACT + REGRESSION REVIEW
```

Do not create a second verification record for identical coverage.

## 13. Requirement-to-architecture relationship

Use:

```text
REQUIREMENT
→ ARCH-DEC
→ ARCHITECTURAL MECHANISM
→ DESIGN ELEMENT
```

An architecture decision does not automatically mean the requirement is verified.

## 14. Requirement-to-verification relationship

Use:

```text
REQUIREMENT
→ VERIFICATION METHOD
→ TEST / ANALYSIS / INSPECTION / SIMULATION / DEMONSTRATION
→ RESULT
→ EVIDENCE
```

`VERIFIED` requires controlled evidence.

## 15. Certification relationship

Certification-relevant requirement:

```text
OFFICIAL SOURCE
→ SOURCE CLAUSE
→ APPLICABILITY
→ CERTIFICATION REQUIREMENT
→ SYSTEM REQUIREMENT
→ COMPLIANCE METHOD
→ VERIFICATION
→ EVIDENCE
```

If official source/clause is not yet confirmed:

```text
SOURCE = TBD
```

Do not claim regulatory compliance.

## 16. Navigation knowledge relationship

Navigation engineering knowledge:

```text
Original technical source
→ Specialized Knowledge Base
→ Navigation Knowledge Map
→ Engineering Rule
→ Requirement
→ Design
→ Verification
```

The 1973 navigation source is a technical knowledge source and is not itself a Russian regulatory requirement.

## 17. Requirement quality gate

Before `AGREED`:

```text
[ ] Unique
[ ] Unambiguous
[ ] Testable / analyzable / inspectable
[ ] Scope defined
[ ] Source/basis identified
[ ] Safety impact checked
[ ] Certification impact checked
[ ] Architecture allocation defined
[ ] Verification method defined
[ ] No duplicate
```

## 18. Change impact

Any material requirement change triggers:

```text
Requirement
→ Safety
→ Hazard
→ Architecture
→ Design
→ Interface
→ Verification
→ Evidence
→ Configuration
→ Certification Matrix
```

Only affected records are updated.

## 19. Conflict resolution

When two requirements conflict:

```text
1. Preserve both records.
2. Identify source and authority.
3. Compare scope.
4. Determine whether conflict is real.
5. Record decision.
6. Change authoritative record through CHG.
7. Update derived documents.
8. Re-check verification.
```

No silent overwrite.

## 20. Orphan rules

Flag:

```text
Requirement without basis
Requirement without owner
Requirement without allocation
Requirement without verification method
Certification requirement without source clause
Safety requirement without hazard/objective
Verified requirement without evidence
```

## 21. Consolidation state

```text
EXISTING SYS-REQ: PRESERVED
EXISTING ARCH-DEC: PRESERVED
EXISTING TEST-* : PRESERVED

NEW CANDIDATE REQUIREMENTS: NOT YET BASELINED

DUPLICATE REVIEW: PARTIAL
GAP REVIEW: PENDING
FULL TRACEABILITY RECONCILIATION: PENDING
CERTIFICATION CLAUSE MAPPING: PENDING
```

## 22. Next controlled operation

The next operation is now **content reconciliation**, not creation of another requirements database:

```text
1. Extract exact existing SYS-REQ text.
2. Extract exact existing SAF-REQ text.
3. Extract linked ARCH-DEC text.
4. Extract linked TEST-* references.
5. Compare candidate SRS requirements.
6. Mark KEEP / DERIVED / MERGE / GAP / CONFLICT.
7. Update this register.
8. Synchronize SRS.
9. Synchronize traceability.
10. Baseline only after review.
```

## 23. Audit result — Requirements Register

Дата: 2026-09-04.

### Проверено

- `MASTER-REQUIREMENTS-REGISTER-001` является единственным authoritative register идентичности системных требований.
- Существующие `SYS-REQ-*` и `SAF-REQ-*` должны сохранять идентичность при консолидации.
- Диапазоны `SYS-REQ-*` в разделе 7 являются inventory, а не доказательством существования каждого номера внутри диапазона.
- Семейства `NAV-REQ-*` и другие семейства из нового SRS имеют статус candidate до overlap/source analysis.
- Требования не должны получать статус `VERIFIED` без результата и evidence.

### Выявлено

```text
REQ-AUD-001  полный текст всех SYS-REQ/SAF-REQ ещё требует построчного inventory;
REQ-AUD-002  candidate families нельзя считать baseline requirements;
REQ-AUD-003  verification linkage должен использовать отдельный Verification Register;
REQ-AUD-004  regulatory source/clause должен быть подтверждён до compliance claim;
REQ-AUD-005  требования без фактической allocation/verification/evidence остаются незакрытыми.
```

### Решение

```text
KEEP existing requirement identities
DO NOT renumber
DO NOT duplicate
DO NOT promote candidate IDs automatically
USE VERIFICATION-REGISTER-001 for verification identity
CONTINUE exact content reconciliation
```

## 24. Status

**DRAFT_FOR_AGREEMENT**


## C2 №142 — CONTROLLED INTEGRATION
Source: Приказ Минтранса России от 24.04.2025 №142.
Integration record: C2-WP142-CLAUSE-RECON-001.
Status: WORKING / NOT BASELINED.

Existing SYS-REQ identities are preserved. C2 allocation:
SYS-REQ-080, SYS-REQ-081, SYS-REQ-082, SYS-REQ-085, SYS-REQ-086, SYS-REQ-091, SYS-REQ-092, SYS-REQ-093.

Derived SRS records: SRS-C2-001..007.

Baseline gate:
authority → applicability → safety → architecture → verification → evidence → configuration.

No certification claim is made by this integration.

## 25. C2 verification traceability synchronization

C2-specific verification is additionally controlled through:

```text
C2_VERIFICATION_CASES_001.md
C2_VERIFICATION_EXECUTION_RESULT_001.md
C2_EVIDENCE_INDEX_001.md
```

Current C2 traceability follows:

```text
REGULATORY CLAUSE
→ C2-REQ / EXISTING SYS-REQ
→ IF-C2-001
→ C2 SAFETY ALLOCATION
→ C2-V01..C2-V08
→ CONFIGURATION
→ EXECUTION
→ EVIDENCE
```

The C2 candidate IDs remain derived working allocations. Equivalent existing requirements/tests must be linked rather than duplicated. A new authoritative requirement or test case is created only when a real coverage gap is demonstrated.

No real test result is implied by this linkage. Execution and evidence remain pending until the real test stage.

**Status: CONTROLLED WORKING DRAFT — C2 TRACEABILITY SYNCHRONIZED.**
