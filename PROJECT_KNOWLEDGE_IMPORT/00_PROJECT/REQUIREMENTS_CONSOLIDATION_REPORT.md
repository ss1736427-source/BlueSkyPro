---
id: REQUIREMENTS-CONSOLIDATION-REPORT-001
type: requirements_consolidation_report
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Requirements Consolidation Report

## 1. Назначение

Свести существующую проектную requirements/architecture/verification базу с недавно созданными certification-document records без создания второй независимой системы требований.

Этот документ фиксирует результаты текущей сверки и правила дальнейшей консолидации.

## 2. Главный результат

**Существующие `SYS-REQ-*` сохраняются как основная идентичность системных требований.**

Новые требования из `SYSTEM_REQUIREMENTS_SPECIFICATION.md` являются предварительными кандидатами до проверки на:

```text
duplicate
overlap
parent / child relationship
source
safety relevance
certification relevance
architecture allocation
verification
```

Новые ID нельзя автоматически принимать в baseline.

## 3. Existing requirements baseline

По доступным проектным материалам существует значительная последовательность `SYS-REQ-*`, начиная как минимум с:

```text
SYS-REQ-001
...
SYS-REQ-112
```

Фактический набор не следует считать непрерывным только по номеру: наличие пропусков, backup-файлов и отдельных записей требует проверки repository inventory.

Подтверждены, в частности:

```text
SYS-REQ-001
SYS-REQ-002
SYS-REQ-003
SYS-REQ-009
SYS-REQ-010
SYS-REQ-011
SYS-REQ-012
SYS-REQ-013
SYS-REQ-014
SYS-REQ-015
SYS-REQ-016
SYS-REQ-017
SYS-REQ-018
SYS-REQ-019
SYS-REQ-020
SYS-REQ-021
SYS-REQ-022
SYS-REQ-023
SYS-REQ-025
SYS-REQ-026
SYS-REQ-027
SYS-REQ-028
SYS-REQ-029
SYS-REQ-030
SYS-REQ-032
SYS-REQ-033
SYS-REQ-035
...
SYS-REQ-067..078
SYS-REQ-080..112
```

Это не утверждение о полном составе. Полный inventory должен быть получен непосредственно из repository.

## 4. Existing high-value requirement cluster

Подтверждённый кластер:

| Requirement | Title | Architecture coverage |
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
| SYS-REQ-110 | Existing project requirement | Capability traceability exists |
| SYS-REQ-111 | Existing project requirement | Capability traceability exists |
| SYS-REQ-112 | Existing project requirement | Capability traceability exists |

## 5. Architecture authority

The existing architecture contains an explicit authority model.

`ARCH-DEC-016` establishes:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

and states that planning, optimization, simulation, AI and UI must not bypass the Safety Gate.

This existing decision is therefore authoritative for reconciliation of any new safety/execution requirement unless formally superseded through change control.

## 6. Safety priority

Existing architecture establishes:

```text
SAFETY
>
MANDATORY SYSTEM CONSTRAINTS
>
MISSION REQUIREMENTS
>
OPERATOR / USER PREFERENCES
>
OPTIMIZATION PREFERENCES
```

`SYS-REQ-085` is already linked to `ARCH-DEC-007`, `ARCH-DEC-016` and `ARCH-DEC-036`.

Therefore new generic requirements such as:

```text
SAF-REQ-019 Safety Gate authority
SAF-REQ-020 No bypass
AUTH-REQ-001 Authorization boundary
AUTH-REQ-002 Unauthorized action
AI-REQ-002 AI authority boundary
```

must first be compared with the existing `SYS-REQ-*` and `ARCH-DEC-*` records.

They are currently **candidate derived requirements**, not approved new independent requirements.

## 7. Existing traceability mechanism

The project already contains requirement-to-architecture coverage.

Coverage states include:

```text
COVERED
PARTIAL
NOT_COVERED
DEFERRED
NOT_APPLICABLE
```

The existing architecture traceability model explicitly requires:

```text
REQUIREMENT
→ DECISION
→ RATIONALE
→ ARCHITECTURAL MECHANISM
→ VERIFICATION
→ EVIDENCE
```

and requires conflicts, duplicates, stale relationships and broken references to be detectable.

Therefore `MASTER_REQUIREMENTS_REGISTER.md` becomes the index/master register, while existing detailed traceability files remain evidence of the individual relationships.

## 8. Important existing coverage state

For `SYS-REQ-083..089`, existing architecture coverage is recorded as `PARTIAL`, not fully covered.

Therefore the new certification documents must not upgrade these requirements to `VERIFIED` or `FULLY COVERED` merely because an architecture reference exists.

## 9. Existing verification

The project already contains:

```text
TEST-*
```

verification records and requirement-specific capability/architecture traceability files.

Therefore the new `VERIFICATION_PLAN.md` is a planning layer.

It must not replace existing test records or create duplicate verification identities.

## 10. Existing configuration/change architecture

Existing architecture already requires:

```text
configuration binding
change impact analysis
historical traceability
supersession
```

Architecture decisions explicitly require requirement changes to trigger impact analysis against linked architecture decisions, and architecture changes to trigger impact analysis against linked requirements.

This agrees with `DOCUMENT_CONTROL_AND_BASELINE_PROCESS.md`.

## 11. New SRS consolidation status

The following new families from `SYSTEM_REQUIREMENTS_SPECIFICATION.md` require consolidation:

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

They are not automatically separate requirement databases.

For each candidate:

```text
Existing SYS-REQ?
Existing SAF-REQ?
Existing ARCH-DEC constraint?
Existing interface requirement?
Existing operational requirement?
Existing test?
External regulatory requirement?
```

must be checked.

## 12. Preliminary overlap groups

### Group A — Safety Gate / authorization

Candidate new:

```text
SAF-REQ-019
SAF-REQ-020
AUTH-REQ-001
AUTH-REQ-002
AI-REQ-002
```

Existing authority:

```text
SYS-REQ-085
SYS-REQ-082
ARCH-DEC-007
ARCH-DEC-016
ARCH-DEC-017
```

**Disposition:** consolidate, do not create parallel authority requirements.

### Group B — AI authority / safe learning

Candidate new:

```text
AI-REQ-001
AI-REQ-002
AI-REQ-003
```

Existing:

```text
SYS-REQ-087
SYS-REQ-088
SYS-REQ-094..104
ARCH-DEC-011
ARCH-DEC-025
ARCH-DEC-026
ARCH-DEC-029
ARCH-DEC-041
ARCH-DEC-046
```

**Disposition:** substantial overlap. Existing IDs must be examined before any new AI requirement is accepted.

### Group C — Resource / performance

Candidate new:

```text
CFG-REQ-*
SYS-REQ-like resource requirements
```

Existing:

```text
SYS-REQ-084
SYS-REQ-089
SYS-REQ-090
SYS-REQ-091
SYS-REQ-092
SYS-REQ-093
ARCH-DEC-022
ARCH-DEC-024
ARCH-DEC-036
ARCH-DEC-044
```

**Disposition:** consolidate with existing resource/performance requirements.

### Group D — Degradation / recovery

Candidate new:

```text
C2-REQ-002
C2-REQ-003
RET-REQ-004
```

Existing:

```text
SYS-REQ-081
SYS-REQ-086
SYS-REQ-093
ARCH-DEC-017
ARCH-DEC-023
ARCH-DEC-027
ARCH-DEC-040
```

**Disposition:** likely overlap; exact requirement-level comparison required.

### Group E — Navigation

Candidate new:

```text
NAV-REQ-001..009
```

No automatic merge is authorized.

Navigation must be compared against:

```text
existing navigation requirements
Navigation Knowledge Map
Navigation State Model
Navigation Rules
Navigation Algorithm
Navigation Verification Model
official regulatory sources
original technical source
```

**Disposition:** domain-specific consolidation required.

## 13. Requirement status problem

Existing project requirements are predominantly marked `draft`.

Existing architecture decisions include accepted decisions.

Therefore:

```text
ARCH-DEC accepted
≠
SYS-REQ automatically approved
```

Architecture coverage does not change requirement status.

## 14. Certification separation

Project engineering requirements must be separated from regulatory claims.

Correct chain:

```text
Official Source
→ Applicability
→ Certification Requirement
→ System Requirement
→ Architecture
→ Design
→ Verification
→ Evidence
```

A project requirement derived from technical knowledge is not automatically a regulatory requirement.

## 15. Navigation source separation

For navigation:

```text
Original technical source
→ Specialized Knowledge Base
→ Navigation Knowledge Map
→ Engineering Rule
→ Requirement
```

The 1973 navigation source is a technical knowledge source and must not be represented as a Russian regulatory source without an independent regulatory basis.

## 16. Current contradictions / risks

### CONS-001 — Dual requirement identity

Risk:

```text
Existing SYS-REQ-*
+
new NAV/SAF/AUTH/etc requirements
```

could become two independent systems.

**Decision:** Master Requirements Register controls identity; new SRS requirements are candidate/derived records until consolidated.

### CONS-002 — Safety Gate terminology

Risk of using:

```text
AUTHORIZATION
```

as if it replaces the existing:

```text
OPERATOR APPROVAL
```

**Decision:** preserve existing architecture authority chain and explicitly distinguish Safety Gate, operator approval and execution authority.

### CONS-003 — Verification duplication

Risk:

```text
new Verification Case
+
existing TEST-*
```

for the same behaviour.

**Decision:** link to existing test where equivalent; create a new case only where coverage is missing.

### CONS-004 — Architecture duplication

Existing architecture already has extensive `ARCH-DEC-*` coverage.

**Decision:** update or supersede existing decisions through change control instead of creating contradictory decisions.

### CONS-005 — Certification wording

Some new documents use certification terminology while their regulatory clause mapping is still pending.

**Decision:** keep `DRAFT_FOR_AGREEMENT` and do not claim compliance/certification until source and evidence are established.

## 17. Consolidation decisions

Effective immediately:

```text
1. Existing SYS-REQ IDs are preserved.
2. Existing ARCH-DEC IDs are preserved.
3. Existing TEST IDs are preserved.
4. New candidate requirements do not automatically receive baseline status.
5. Duplicate requirements are linked/merged only after comparison.
6. Superseded records remain traceable.
7. Regulatory claims require authoritative source evidence.
8. Verification records are not duplicated without a coverage gap.
9. Architecture authority follows accepted ARCH-DEC records until formally changed.
10. Master Requirements Register is the master index for requirement identity.
```

## 18. Required next operation

The next operation is repository-level extraction:

```text
ALL SYS-REQ-*.md
→ IDs
→ titles
→ requirement text
→ status
→ criticality
→ source
→ verification method

ALL ARCH-DEC-*.md
→ IDs
→ decisions
→ linked SYS-REQ

ALL TEST-*.md
→ IDs
→ requirement links
→ result/status

ALL SAFETY records
→ HAZ
→ SAF-REQ
→ mitigations
```

Then generate:

```text
Requirement Inventory
→ Duplicate Matrix
→ Gap Matrix
→ Traceability Matrix
```

## 19. Baseline rule

Do not baseline the consolidated requirements until:

```text
Complete inventory
+
Duplicate analysis
+
Conflict analysis
+
Source review
+
Safety review
+
Certification review
+
Verification mapping
```

are complete.

## 20. Current status

```text
ARCHITECTURE CONSOLIDATION: PARTIAL
REQUIREMENTS CONSOLIDATION: IN PROGRESS
VERIFICATION CONSOLIDATION: IN PROGRESS
CERTIFICATION MAPPING: IN PROGRESS
BASELINE: NOT YET ESTABLISHED
```

## 21. Status

**DRAFT_FOR_AGREEMENT**



