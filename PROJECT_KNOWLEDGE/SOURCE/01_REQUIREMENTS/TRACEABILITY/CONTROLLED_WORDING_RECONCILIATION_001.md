---
id: CONTROLLED-WORDING-RECONCILIATION-001
type: controlled_wording_reconciliation
status: working
jurisdiction: Russian Federation
system: BlueSky PRO
authority: MASTER-REQUIREMENTS-REGISTER-001
---

# BlueSky PRO — Controlled Wording Reconciliation 001

Date: 2026-09-04

## 1. Purpose

This pass performs the next controlled operation defined by the requirements governance documents: reconcile existing requirement identities and candidate records against the wording and source material actually present in the repository.

The pass does **not** silently renumber, delete, merge, or baseline requirements.

The governing rule remains:

```text
ONE REQUIREMENT
→ ONE STABLE ID
→ ONE CONTROLLED WORDING
→ MANY RELATIONSHIPS
```

## 2. Repository evidence inspected

The reconciliation was performed against the current repository structure and the controlled records available there, including:

```text
01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md
01_REQUIREMENTS/SYSTEM/REQUIREMENTS_INVENTORY.md
01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md
01_REQUIREMENTS/SAFETY/SAFETY_REQUIREMENTS_BASELINE.md
01_REQUIREMENTS/REGULATORY/REGULATORY_SOURCE_REGISTER.md
01_REQUIREMENTS/REGULATORY/C2/C2_CLAUSE_LEVEL_MAPPING_001.md
01_REQUIREMENTS/REGULATORY/C2/C2_REGULATORY_REQUIREMENTS_BASELINE_001.md
01_REQUIREMENTS/TRACEABILITY/REQUIREMENTS_RECONCILIATION.md
01_REQUIREMENTS/TRACEABILITY/REQUIREMENTS_CONSOLIDATION_REPORT.md
03_SYSTEM/NAVIGATION/NAVIGATION_BASELINE_RECONCILIATION.md
```

The repository inventory currently identifies 99 unique requirement IDs and 399 ID occurrences. The reconciliation register classifies 46 IDs as present in both inventory and Master Register and 53 as inventory-only. The latter are treated below as **Master Register population gaps**, not automatically as missing system functionality.

## 3. Critical finding — distinction between requirement GAP and register GAP

The existing `REQUIREMENTS_RECONCILIATION.csv` uses `GAP` when an ID exists in Inventory but is absent from the Master Register.

This is a repository/control gap:

```text
Inventory = True
Master = False
→ MASTER-REGISTER POPULATION GAP
```

It is **not sufficient evidence of a functional requirement gap**.

This distinction is material because the Master Register itself declares existing `SYS-REQ-*` and `SAF-REQ-*` identities to be preserved. Therefore records such as `SYS-REQ-002`, `SYS-REQ-010…023`, and `SAF-REQ-001…018` must not be treated as newly invented requirements merely because their identities have not yet been copied into the current Master Register.

## 4. Controlled wording status model used in this pass

```text
FINAL-WORDING
    Exact controlled project wording is present and can be retained.

DERIVED-WORDING
    Wording is explicitly derived from a source/architecture/safety record, but
    it is not itself a quotation of the external source.

MERGE / CLARIFY
    Candidate wording overlaps an existing authoritative requirement or authority
    chain and should not become an independent requirement.

PENDING-SOURCE
    Project wording exists, but authoritative source/clause or applicability is not
    sufficiently established for a certification claim.

MASTER-REGISTER-GAP
    Requirement identity exists in repository evidence but is not yet represented
    in the current Master Register.

SOURCE-GAP
    Exact authoritative source text/clause is still required.

NO-AUTO-PROMOTION
    Candidate family exists, but the repository evidence does not justify baseline.
```

## 5. Decisions that can be closed now

### 5.1 Safety requirements SAF-REQ-001…018

The repository contains explicit controlled project wording for `SAF-REQ-001…018` in `SAFETY_REQUIREMENTS_BASELINE.md`.

Therefore their wording is no longer merely `SUPPORTING_WORDING`.

Disposition:

```text
SAF-REQ-001…018
→ retain existing IDs
→ retain current controlled wording
→ classify as DERIVED-WORDING / OPEN
→ populate Master Register
→ retain source/clause as TBD until formal basis is established
→ do not claim regulatory compliance
```

The available wording covers, among other things, validated state, invalid/stale data, loss of critical data, Safety Gate integrity, authorization boundary, navigation safety, route safety, dynamic return, collision avoidance, multi-UAV state isolation, C2 degradation, conflicting sources, revalidation, traceability, configuration control and AI authority boundary.

The wording itself is suitable as a project safety requirement formulation. It must not be represented as verbatim regulatory text.

### 5.2 SAF-REQ-019 / SAF-REQ-020

These records are explicitly identified by the current SRS and Master Register as candidate derived/clarifying records associated with the existing Safety Gate / authorization chain.

Disposition:

```text
SAF-REQ-019 → MERGE / CLARIFY against SYS-REQ-082, SYS-REQ-085 and ARCH-DEC-016/017
SAF-REQ-020 → MERGE / CLARIFY against the same authority chain
```

They should not create a parallel safety-authority requirement set.

### 5.3 AUTH-REQ-001 / AUTH-REQ-002

The current controlled architecture already defines:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

Disposition:

```text
AUTH-REQ-001 → DERIVED / CLARIFY existing authorization boundary
AUTH-REQ-002 → DERIVED / CLARIFY unauthorized-execution handling
```

No independent execution authority is introduced.

### 5.4 AI-REQ-001…003

The current Master Register already identifies substantial AI/learning coverage in `SYS-REQ-087`, `SYS-REQ-088`, `SYS-REQ-094…104`, `SYS-REQ-107` and `SYS-REQ-108`, with related architecture decisions.

Disposition:

```text
AI-REQ-001…003
→ retain as candidate references only
→ compare exact wording against existing SYS-REQ cluster
→ do not baseline as independent requirements
→ create a new requirement only if a factual coverage gap is demonstrated
```

The architectural boundary remains:

```text
AI OUTPUT
≠
AUTHORITATIVE STATE
≠
EXECUTION AUTHORITY
```

### 5.5 Resource/performance candidates

Existing resource/performance coverage is already assigned to:

```text
SYS-REQ-084
SYS-REQ-089
SYS-REQ-090
SYS-REQ-091
SYS-REQ-092
SYS-REQ-093
```

Disposition:

```text
No generic duplicate resource/performance requirement shall be created.
```

### 5.6 Degradation/recovery candidates

Existing coverage includes:

```text
SYS-REQ-081
SYS-REQ-086
SYS-REQ-093
```

Therefore:

```text
C2-REQ-002
C2-REQ-003
RET-REQ-004
```

remain `MERGE / CLARIFY` candidates pending exact requirement-level comparison. They are not independent baseline requirements at this stage.

## 6. C2 controlled wording closure

The repository contains clause-level mapping for Order No. 142 and a separate working C2 regulatory baseline. The regulatory mapping identifies clauses 1–10, 60–62, 66–72, 74–75 and 77–79 as the current working scope.

The repository also defines the following derived C2 system wording:

### SYS-C2-001 — C2 Mode

```text
BlueSky должен поддерживать конфигурацию режима организации C2 с явным различением DIRECT-C2 и PROVIDER-C2.
```

Disposition: `DERIVED-WORDING`.

Basis: Order No. 142 clause mapping + established C2 system boundary.

### SYS-C2-002 — C2 State Monitoring

```text
BlueSky должен получать и отображать состояние линии C2 и обнаруживать предусмотренные системой признаки деградации/потери, включая отсутствие передачи/приёма и снижение объёма информации ниже установленного эксплуатационной документацией уровня.
```

Disposition: `DERIVED-WORDING`.

### SYS-C2-003 — C2 QoS

```text
BlueSky должен использовать параметры требуемых характеристик связи как входные ограничения для подготовки и выполнения миссии, если соответствующие параметры входят в установленный scope системы.
```

Disposition: `DERIVED-WORDING / CONDITIONAL`.

### SYS-C2-004 — Provider Coverage

```text
При Provider-C2 BlueSky должен проверять соответствие планируемого маршрута и необходимых участков маршрута зоне обслуживания поставщика на основании доступных актуальных данных.
```

Disposition: `DERIVED-WORDING / INTERFACE`.

### SYS-C2-005 — C2 Event Logging

```text
BlueSky должен регистрировать события установления, завершения, деградации, переключения и потери C2, а также связанные временные и позиционные данные в пределах установленной модели журналирования.
```

Disposition: `DERIVED-WORDING`.

### SYS-C2-006 — C2 Loss Workflow

```text
При установленном состоянии потери C2 BlueSky должен инициировать предусмотренный Safety Concept сценарий и обеспечить оператору необходимые предупреждения и данные для выполнения обязательных эксплуатационных действий.
```

Disposition: `DERIVED-WORDING / SAFETY INTERFACE`.

### SYS-C2-007 — C2 Cause Classification

```text
BlueSky должен классифицировать доступные признаки причины нарушения C2 по установленной модели диагностики, не выдавая неподтверждённую причину как достоверный факт.
```

Disposition: `DERIVED-WORDING / SAFETY`.

### SYS-C2-008 — Time Base

```text
Для C2 telemetry, commands, state transitions, alerts и event records должна применяться единая контролируемая временная база.
```

Disposition: `DERIVED-WORDING / SAFETY / VERIFICATION`.

Important: these are controlled **project formulations derived from the regulatory mapping**. They are not verbatim quotations of Order No. 142.

## 7. Regulatory source closure

### Confirmed source

The official publication portal confirms Order of the Ministry of Transport of Russia dated 24.04.2025 No. 142, registered 20.06.2025 under No. 82667 and published 23.06.2025. The repository records its effective period as 01.03.2026–01.03.2032.

### Confirmed standard status

The official Rosstandart portal lists `ГОСТ Р 59517-2021` as действующий. Its stated scope is classification and categorization of unmanned aircraft systems for determining flight-safety requirements and the necessary scope of certification work.

### Applicability decision

The repository rule is retained:

```text
SOURCE
→ CLAUSE
→ REGULATORY OBJECT
→ BLUE SKY ROLE
→ SYSTEM BOUNDARY
→ APPLICABILITY
→ REQUIREMENT
```

No regulatory requirement is promoted solely because its source is present in the source register.

## 8. Navigation controlled wording status

Navigation is not ready for blanket promotion to `FINAL-WORDING`.

The repository has established the semantic authority chain:

```text
NAVIGATION STATE MODEL
→ NAVIGATION RULES
→ NAVIGATION ALGORITHM
→ NAVIGATION MATHEMATICS
→ MODULE SPECIFICATIONS
```

It also explicitly preserves:

```text
Course ≠ Heading ≠ Track ≠ Bearing
Airspeed ≠ Groundspeed
MEASURED ≠ ESTIMATED ≠ DERIVED ≠ PREDICTED
PLANNED ≠ ACTUAL
SIMULATED ≠ ACTUAL
```

`NAV-REQ-001` remains a candidate because the repository has not yet established a complete exact wording/source allocation against the Navigation State Model, Rules, Algorithm, Mathematics and applicable regulatory clauses.

Disposition:

```text
NAV-REQ-001 → NO-AUTO-PROMOTION / PENDING-SOURCE
NAV-REQ-002…009 → NO-AUTO-PROMOTION
```

The existing Navigation allocation of `SYS-REQ-081`, `082`, `085`, `086`, `091`, `093` remains an allocation candidate until exact text comparison is completed.

## 9. Master Register population result

The current reconciliation table should be interpreted as follows:

```text
46 IDs present in Inventory + Master
→ existing candidate/working records requiring wording/source review

53 Inventory-only IDs
→ Master Register population gaps
→ not functional GAP by default
```

Priority for controlled population:

```text
1. SAF-REQ-001…018
2. SYS-REQ-002…003, SYS-REQ-010…023, SYS-REQ-026…030, SYS-REQ-033
3. remaining Inventory-only SYS/SAF records
4. candidate families after duplicate analysis
```

The current repository does not provide sufficient controlled wording for all Inventory-only SYS records in the current Master Register. Therefore their wording must be recovered from the authoritative historical/project records before transcription.

## 10. Certification wording rule

The following distinction is mandatory:

```text
REGULATORY TEXT
≠
PROJECT DERIVED REQUIREMENT
≠
DESIGN DECISION
≠
VERIFICATION CASE
≠
EVIDENCE
```

A requirement can be final in project wording while its certification applicability remains pending.

Conversely, a regulatory clause can be confirmed while the BlueSky system allocation remains pending.

## 11. Current controlled disposition matrix

| Area | Disposition | Controlled action |
|---|---|---|
| SAF-REQ-001…018 | DERIVED-WORDING | retain wording; populate Master Register; source/clause TBD |
| SAF-REQ-019…020 | MERGE / CLARIFY | do not create parallel authority |
| AUTH-REQ-001…002 | DERIVED / CLARIFY | subordinate to existing authority chain |
| AI-REQ-001…003 | NO-AUTO-PROMOTION | compare with existing AI SYS cluster |
| C2-REQ-001 | MASTER-REGISTER-GAP | exact C2 wording exists in clause mapping; integrate only after identity decision |
| C2-REQ-002…003 | MERGE / CLARIFY candidate | compare with SYS-REQ-081/086/093 |
| CFG-REQ-001 | MASTER-REGISTER-GAP | source/design allocation required |
| COL-REQ-001 | MASTER-REGISTER-GAP | safety/system wording and basis required |
| DATA-REQ-001 | MASTER-REGISTER-GAP | source/design allocation required |
| HMI-REQ-001 | MASTER-REGISTER-GAP | interface allocation required |
| HW-REQ-001 | MASTER-REGISTER-GAP | engineering scope required |
| MIS-REQ-001 | MASTER-REGISTER-GAP | system allocation required |
| MUL-REQ-001 | MASTER-REGISTER-GAP | compare with existing SYS/SAF coverage |
| NAV-REQ-001 | NO-AUTO-PROMOTION | exact source/allocation pending |
| RDY-REQ-001 | MASTER-REGISTER-GAP | readiness state allocation required |
| RET-REQ-001 | MASTER-REGISTER-GAP | return function/safety allocation required |
| RET-REQ-004 | MERGE / CLARIFY candidate | compare with SYS-REQ-081/086/093 |
| RTE-REQ-001 | MASTER-REGISTER-GAP | route function allocation required |
| SW-REQ-001 | MASTER-REGISTER-GAP | engineering requirement vs assurance obligation must be separated |
| WP-REQ-001 | MASTER-REGISTER-GAP | waypoint function allocation required |

## 12. What remains genuinely open

The following items cannot responsibly be converted to final wording from the currently inspected evidence alone:

1. Exact controlled text for all Inventory-only legacy `SYS-REQ-*` records.
2. Exact controlled text and hazard/objective basis for the full `SAF-REQ-001…018` population in the Master Register.
3. Final certification clause mapping for each system function.
4. Numerical C2 QoS, availability, continuity, integrity and timing parameters.
5. Aircraft-specific loss-of-C2 behaviour.
6. Exact navigation requirement wording and regulatory allocation.
7. Collision-avoidance separation criteria.
8. Final readiness state transition criteria.
9. Final operational/certification scope and responsibility allocation where external providers are involved.

## 13. Conclusion

This pass closes several classification ambiguities without fabricating missing evidence:

```text
SUPPORTING_WORDING
→ replaced where repository wording is explicit

PENDING_WORDING
→ converted to DERIVED / MERGE / NO-AUTO-PROMOTION according to evidence

INVENTORY GAP
≠
FUNCTIONAL REQUIREMENT GAP
```

The most important immediate action is to **populate the Master Register with the already existing legacy requirement identities and their exact wording**, rather than creating another candidate requirements database.

No requirement is marked `BASELINED` or `VERIFIED` by this report.

## 14. Baseline gate

Before baseline:

```text
[ ] exact wording recovered
[ ] source/clause confirmed where applicable
[ ] applicability confirmed
[ ] safety allocation confirmed
[ ] architecture allocation confirmed
[ ] verification method/case confirmed
[ ] evidence path confirmed
[ ] configuration binding confirmed
[ ] duplicate/overlap closed
[ ] controlled change recorded where wording changes
```

**Status: WORKING — CONTROLLED WORDING RECONCILIATION IN PROGRESS**
