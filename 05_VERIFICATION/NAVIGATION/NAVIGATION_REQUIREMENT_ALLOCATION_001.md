---
id: NAVIGATION-REQUIREMENT-ALLOCATION-001
type: navigation_requirement_allocation
status: WORKING
authority: MASTER-REQUIREMENTS-REGISTER-001; NAVIGATION-TRACEABILITY-MATRIX-001
parent: NAVIGATION
version: 0.1
---

# BlueSky PRO — Navigation Requirement Allocation

## 1. Назначение

Контролируемая рабочая запись сопоставления существующих системных требований с блоком `Navigation`.

Документ **не является самостоятельным реестром требований** и не создаёт новые requirement IDs. Идентичность требований определяется только `MASTER-REQUIREMENTS-REGISTER.md`.

## 2. Метод сопоставления

Для каждого требования проверяется:

```text
Requirement ID
→ controlled title / available wording
→ existing architecture linkage
→ Navigation responsibility
→ module allocation
→ verification linkage
→ disposition
```

Классификация:

- `DIRECT` — требование непосредственно реализуется/обеспечивается Navigation.
- `SUPPORTING` — Navigation является частью обеспечения системного требования, но не является единственным владельцем результата.
- `NOT_APPLICABLE` — подтверждённой ответственности Navigation нет.
- `PENDING_WORDING` — по имеющимся данным нельзя надёжно определить прямую применимость без полного controlled wording.
- `CONFLICT` — выявлено противоречие; требует change control.

**Важно:** отсутствие прямой allocation не является GAP автоматически.

## 3. Результат первичного сопоставления

| Requirement | Controlled title | Navigation relation | Module allocation | Existing architecture basis | Verification linkage | Disposition |
|---|---|---|---|---|---|---|
| SYS-REQ-081 | UAV Failure Tolerance | Navigation supplies current state and quality/degradation information used by fault-tolerant behaviour; system fault tolerance is broader than Navigation | Position; Velocity/Speed; Heading; Track/Course; Wind | ARCH-DEC-017, ARCH-DEC-027, ARCH-DEC-040 | NAV-V08–V12, V19, V20 | SUPPORTING |
| SYS-REQ-082 | Safe Mission Completion | Navigation actual state, route deviation and navigation quality support safe completion; completion authority remains outside Navigation | Position; Velocity/Speed; Track/Course | ARCH-DEC-016, ARCH-DEC-017, ARCH-DEC-038 | NAV-V06, V07, V15, V18, V20 | SUPPORTING |
| SYS-REQ-085 | Safety-Critical Priority | Navigation must obey the established safety priority and must not bypass Safety Gate; priority itself is system-level | All applicable Navigation modules | ARCH-DEC-007, ARCH-DEC-016, ARCH-DEC-036 | NAV-V18 | SUPPORTING |
| SYS-REQ-086 | Graceful Degradation | Navigation quality states and source degradation are directly relevant to continued operation with degraded navigation data | Position; Velocity/Speed; Heading; Track/Course; Wind | ARCH-DEC-017, ARCH-DEC-023, ARCH-DEC-039 | NAV-V08–V12, V16, V17, V20 | DIRECT |
| SYS-REQ-091 | Critical Latency | Navigation has timing/freshness dependencies; system critical-latency ownership remains broader than Navigation | All applicable Navigation modules | ARCH-DEC-022 | NAV-V08–V12, V20 | SUPPORTING |
| SYS-REQ-093 | Controlled Resource Recovery | No confirmed Navigation-specific ownership from the controlled title/architecture linkage; resource recovery is a system/resource-management function unless allocation is explicitly established | None allocated at this pass | ARCH-DEC-017, ARCH-DEC-036 | No direct Navigation verification identified | PENDING_WORDING |

## 4. Important boundary decisions

### 4.1 Navigation is not Execution Authority

The following existing architecture chain remains authoritative:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

Therefore `SYS-REQ-082` and `SYS-REQ-085` do not transfer execution or Safety Gate authority to Navigation.

### 4.2 Degradation

`SYS-REQ-086` has the strongest direct relationship with Navigation because Navigation explicitly models degraded/invalid/stale/unavailable state and source quality.

This allocation does not by itself establish quantitative thresholds. Thresholds require a controlled requirement, engineering analysis or approved design basis.

### 4.3 Failure tolerance

`SYS-REQ-081` remains a system-level requirement. Navigation contributes state quality, source loss/degradation handling and individual-UAV state, but cannot be declared sole owner without system allocation evidence.

### 4.4 Latency

`SYS-REQ-091` is treated as supporting because the existing architecture linkage points to system resource/latency management. Navigation-specific latency/freshness limits remain design parameters until their requirement basis is identified.

### 4.5 Resource recovery

`SYS-REQ-093` is deliberately not allocated to Navigation on title similarity alone. This prevents false traceability. The requirement must be reviewed at full controlled wording level before allocation is changed.

## 5. Verification distinction

The existing Navigation verification records remain verification definitions, not execution evidence:

```text
Requirement
→ allocation
→ verification case / test vector
→ execution
→ result
→ evidence
```

Existing Navigation verification IDs are reused; new tests are created only after an actual coverage gap is demonstrated.

## 6. Current status

```text
SYS-REQ-081  SUPPORTING
SYS-REQ-082  SUPPORTING
SYS-REQ-085  SUPPORTING
SYS-REQ-086  DIRECT
SYS-REQ-091  SUPPORTING
SYS-REQ-093  PENDING_WORDING
```

No Master Requirements Register record was renamed, deleted, merged or baselined by this allocation pass.

## 7. Required follow-up

The next controlled step is to obtain/inspect the complete controlled wording and source/basis fields for the six requirements and confirm or revise the provisional allocations.

After confirmation, update:

```text
MASTER REQUIREMENTS REGISTER
→ requirement allocation
→ Navigation Traceability Matrix
→ module specifications
→ Verification Register
→ Evidence index
```

## 8. Completion gate

```text
[ ] full controlled wording confirmed
[ ] source/basis confirmed
[ ] safety relevance confirmed
[ ] certification relevance confirmed
[ ] Navigation allocation confirmed
[ ] module allocation confirmed
[ ] verification coverage confirmed
[ ] interface allocation confirmed where applicable
[ ] HMI allocation confirmed where applicable
[ ] no duplicate requirement created
[ ] no false traceability remains
```
