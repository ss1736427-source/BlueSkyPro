---
id: REQUIREMENTS-INVENTORY-001
type: requirements_inventory
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Requirements Inventory

## 1. Назначение

Зафиксировать фактический inventory требований проекта перед их окончательной консолидацией.

Этот документ является рабочим реестром обнаруженных requirement records и их связей.

Он не заменяет `MASTER_REQUIREMENTS_REGISTER.md`.

## 2. Правило inventory

Inventory отвечает:

```text
Что уже существует?
Какой у него ID?
Где оно определено?
С чем оно связано?
Каков текущий статус?
```

Inventory **не изменяет** исходные записи.

## 3. Existing requirement families

На текущем этапе подтверждены следующие семейства:

```text
SYS-REQ-*
SAF-REQ-*
NAV-REQ-*
RTE-REQ-*
WP-REQ-*
MIS-REQ-*
RET-REQ-*
COL-REQ-*
C2-REQ-*
MUL-REQ-*
RDY-REQ-*
AUTH-REQ-*
HMI-REQ-*
AI-REQ-*
DATA-REQ-*
CFG-REQ-*
SW-REQ-*
HW-REQ-*
```

Не каждое семейство уже имеет baseline-статус.

## 4. Existing SYS-REQ records

В проектной базе подтверждено наличие большого набора `SYS-REQ-*`.

Подтверждённые диапазоны/records:

```text
SYS-REQ-001..003
SYS-REQ-009..023
SYS-REQ-025..030
SYS-REQ-032..033
SYS-REQ-035
SYS-REQ-067..078
SYS-REQ-080..089
SYS-REQ-090..104
SYS-REQ-107..108
SYS-REQ-110..112
```

Данный перечень является предварительным inventory, а не утверждением, что все номера внутри диапазонов существуют.

Полный inventory должен быть подтверждён поиском по repository.

## 5. Existing high-value SYS-REQ cluster

| ID | Existing role / title | Existing architecture linkage |
|---|---|---|
| SYS-REQ-080 | Dynamic Task Reallocation | ARCH-DEC-008, 012, 039 |
| SYS-REQ-081 | UAV Failure Tolerance | ARCH-DEC-017, 027, 040 |
| SYS-REQ-082 | Safe Mission Completion | ARCH-DEC-016, 017, 038 |
| SYS-REQ-083 | Platform Independence | ARCH-DEC-003, 012 |
| SYS-REQ-084 | Resource Reservation | ARCH-DEC-036, 037 |
| SYS-REQ-085 | Safety-Critical Priority | ARCH-DEC-007, 016, 036 |
| SYS-REQ-086 | Graceful Degradation | ARCH-DEC-017, 023, 039 |
| SYS-REQ-087 | AI Resource Isolation | ARCH-DEC-022, 023, 035 |
| SYS-REQ-088 | Safe Self-Learning | ARCH-DEC-011, 025, 026 |
| SYS-REQ-089 | HUB Resource Watchdog | ARCH-DEC-022, 024, 032 |
| SYS-REQ-090 | Overload Protection | ARCH-DEC-022, 036 |
| SYS-REQ-091 | Critical Latency | ARCH-DEC-022 |
| SYS-REQ-092 | Redundant HUB Resource Recovery | ARCH-DEC-027, 040 |
| SYS-REQ-093 | Controlled Resource Recovery | ARCH-DEC-017, 036 |
| SYS-REQ-094 | Data Quality for Learning | ARCH-DEC-029, 041 |
| SYS-REQ-095 | Problem and Opportunity Detection | ARCH-DEC-029, 032 |
| SYS-REQ-096 | Solution Generation | ARCH-DEC-046, 039 |
| SYS-REQ-097 | Solution Justification | ARCH-DEC-046, 029 |
| SYS-REQ-098 | Improvement Validation | ARCH-DEC-007, 025 |
| SYS-REQ-099 | Controlled Improvement Deployment | ARCH-DEC-026, 031 |
| SYS-REQ-100 | Improvement Result Evaluation | ARCH-DEC-041, 024 |
| SYS-REQ-101 | Failed Improvement Analysis | ARCH-DEC-017, 029 |
| SYS-REQ-102 | Learning from Failed Improvements | ARCH-DEC-029, 041 |
| SYS-REQ-104 | Learning Lifecycle Traceability | ARCH-DEC-029, 041 |
| SYS-REQ-107 | Rollback Applicability and Acceptance | ARCH-DEC-026, 027 |
| SYS-REQ-108 | Learning Subsystem Failure Containment | ARCH-DEC-017, 029 |
| SYS-REQ-110 | Existing project requirement | Existing capability traceability |
| SYS-REQ-111 | Existing project requirement | Existing capability traceability |
| SYS-REQ-112 | Existing project requirement | Existing capability traceability |

## 6. Existing architecture records relevant to requirements

The current project architecture contains accepted/recorded decisions in areas including:

```text
Safety Gate
Safety priority
Validation
Readiness
Operator approval
Execution authority
AI isolation
Learning control
Resource management
Failure tolerance
Graceful degradation
Recovery
Observability
Configuration
Verification
```

Relevant known decisions include:

```text
ARCH-DEC-007
ARCH-DEC-008
ARCH-DEC-011
ARCH-DEC-012
ARCH-DEC-016
ARCH-DEC-017
ARCH-DEC-022
ARCH-DEC-023
ARCH-DEC-024
ARCH-DEC-025
ARCH-DEC-026
ARCH-DEC-027
ARCH-DEC-029
ARCH-DEC-031
ARCH-DEC-032
ARCH-DEC-035
ARCH-DEC-036
ARCH-DEC-037
ARCH-DEC-038
ARCH-DEC-039
ARCH-DEC-040
ARCH-DEC-041
ARCH-DEC-044
ARCH-DEC-046
```

This is not claimed to be the complete architecture inventory.

## 7. Safety Gate / authorization cluster

Existing:

```text
SYS-REQ-082
SYS-REQ-085
ARCH-DEC-007
ARCH-DEC-016
ARCH-DEC-017
```

New candidate records:

```text
SAF-REQ-019
SAF-REQ-020
AUTH-REQ-001
AUTH-REQ-002
AI-REQ-002
```

Inventory disposition:

```text
EXISTING COVERAGE PRESENT
NEW RECORDS REQUIRE OVERLAP REVIEW
```

Do not baseline the candidate records as independent requirements until comparison is complete.

## 8. AI / learning cluster

Existing:

```text
SYS-REQ-087
SYS-REQ-088
SYS-REQ-094
SYS-REQ-095
SYS-REQ-096
SYS-REQ-097
SYS-REQ-098
SYS-REQ-099
SYS-REQ-100
SYS-REQ-101
SYS-REQ-102
SYS-REQ-104
SYS-REQ-107
SYS-REQ-108
```

Related architecture:

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

New candidates:

```text
AI-REQ-001
AI-REQ-002
AI-REQ-003
```

Disposition:

```text
HIGH OVERLAP
RETAIN EXISTING IDs
ANALYZE CANDIDATES AS DERIVED / GAP RECORDS
```

## 9. Resource / performance cluster

Existing:

```text
SYS-REQ-084
SYS-REQ-089
SYS-REQ-090
SYS-REQ-091
SYS-REQ-092
SYS-REQ-093
```

Related architecture:

```text
ARCH-DEC-022
ARCH-DEC-024
ARCH-DEC-036
ARCH-DEC-037
ARCH-DEC-040
ARCH-DEC-044
```

Disposition:

```text
EXISTING COVERAGE
NEW GENERIC RESOURCE REQUIREMENTS MUST NOT DUPLICATE
```

## 10. Degradation / recovery cluster

Existing:

```text
SYS-REQ-081
SYS-REQ-086
SYS-REQ-093
```

Related architecture:

```text
ARCH-DEC-017
ARCH-DEC-023
ARCH-DEC-027
ARCH-DEC-039
ARCH-DEC-040
```

New candidates:

```text
C2-REQ-002
C2-REQ-003
RET-REQ-004
```

Disposition:

```text
POTENTIAL OVERLAP
DETAILED WORDING COMPARISON REQUIRED
```

## 11. Navigation inventory

Navigation candidates currently include:

```text
NAV-REQ-001..009
```

They must be checked against:

```text
Navigation Knowledge Map
Navigation State Model
Navigation Rules
Navigation Algorithm
Navigation Verification Model
existing navigation requirements
original navigation source
official Russian regulatory sources
```

Current disposition:

```text
NO AUTOMATIC MERGE
DOMAIN INVENTORY REQUIRED
```

## 12. Route / waypoint / mission inventory

Candidate families:

```text
RTE-REQ-001..004
WP-REQ-001..003
MIS-REQ-001..003
```

Required comparison:

```text
Existing mission requirements
Existing route logic
Existing waypoint state
Architecture decisions
Existing TEST-* records
```

Disposition:

```text
CANDIDATE
REQUIRES EXISTING-RECORD SEARCH
```

## 13. C2 / Multi-UAV inventory

Candidate families:

```text
C2-REQ-001..003
MUL-REQ-001..003
```

Required comparison:

```text
Existing SYS-REQ-*
Existing ARCH-DEC-*
Existing TEST-*
Interface records
```

Disposition:

```text
CANDIDATE
REQUIRES EXISTING-RECORD SEARCH
```

## 14. HMI / data / configuration inventory

Candidates:

```text
HMI-REQ-001..002
DATA-REQ-001..002
CFG-REQ-001..003
```

Required comparison:

```text
Existing HMI requirements
Data integrity requirements
Configuration management requirements
Interface controls
Existing verification
```

Disposition:

```text
CANDIDATE
REQUIRES EXISTING-RECORD SEARCH
```

## 15. Software / hardware inventory

Candidates:

```text
SW-REQ-001..003
HW-REQ-001..002
```

These are not yet certification assurance requirements.

Their final form depends on:

```text
Certification Basis
System allocation
Software/hardware partition
Applicable assurance standards / regulations
```

Disposition:

```text
PRELIMINARY
CERTIFICATION MAPPING PENDING
```

## 16. Existing verification inventory

Existing project records include:

```text
TEST-*
```

and requirement/architecture traceability.

The complete TEST inventory must be extracted and mapped before new verification cases are created.

Disposition:

```text
EXISTING VERIFICATION DATABASE
NEW VERIFICATION PLAN IS A PLANNING LAYER
```

## 17. Traceability inventory

Existing traceability relationships include:

```text
SYS-REQ
→ ARCH-DEC
→ verification / capability records
```

Target master chain:

```text
SOURCE
→ REQUIREMENT
→ SAFETY / HAZARD
→ ARCHITECTURE
→ DESIGN
→ INTERFACE
→ VERIFICATION
→ EVIDENCE
→ CONFIGURATION
```

## 18. Inventory classifications

Every discovered record will receive one of:

```text
EXISTING
DUPLICATE
OVERLAP
DERIVED
GAP
CONFLICT
SUPERSEDED
NOT_APPLICABLE
```

## 19. No-silent-change rule

Inventory does not modify source records.

Any consolidation decision is recorded separately:

```text
KEEP
MERGE
LINK
SUPERSEDE
WITHDRAW
CREATE GAP
```

## 20. Current gaps

```text
INV-GAP-001 Complete filesystem extraction not yet completed
INV-GAP-002 Complete SYS-REQ wording inventory pending
INV-GAP-003 Complete SAF-REQ inventory pending
INV-GAP-004 Complete ARCH-DEC linkage inventory pending
INV-GAP-005 Complete TEST linkage inventory pending
INV-GAP-006 Complete navigation requirement inventory pending
INV-GAP-007 Duplicate matrix pending
INV-GAP-008 Gap matrix pending
INV-GAP-009 Clause-level certification mapping pending
```

## 21. Next controlled operation

После размещения этого документа выполнить:

```text
REQUIREMENTS INVENTORY
→ DUPLICATE MATRIX
→ GAP MATRIX
→ TRACEABILITY RECONCILIATION
→ MASTER REQUIREMENTS REGISTER UPDATE
```

Только после этого обновлять:

```text
SYSTEM_REQUIREMENTS_SPECIFICATION
REQUIREMENTS_TRACEABILITY_MATRIX
SAFETY_REQUIREMENTS_BASELINE
VERIFICATION_PLAN
```

## 22. Status

**DRAFT_FOR_AGREEMENT**



