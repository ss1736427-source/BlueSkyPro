---
id: C2-HMI-ALLOCATION-001
type: c2_hmi_allocation_record
status: controlled_working_draft
system: BlueSky PRO
basis: C2-REQUIREMENTS-RECONCILIATION-001 / C2-ICD-BASELINE-001
---

# BLUE SKY PRO — C2 HMI ALLOCATION 001

## 1. Purpose

Зафиксировать allocation C2-related operator indication/warning только в пределах функций BlueSky PRO. Документ не создаёт новый SYS-REQ без доказанного gap.

## 2. Current controlled evidence

`C2-ICD-BASELINE-001` определяет C2 state vocabulary:

```text
NORMAL
DEGRADED
LOST
RESTORED
```

и требует, чтобы переходы состояний были allocated to system/safety requirements; восстановленное состояние не должно считаться валидным до выполнения recovery criteria.

`C2-REQUIREMENTS-RECONCILIATION-001` определяет `C2-REQ-010` как derived/coverage-confirmed allocation pending any later controlled wording change. Controlled wording `SYS-C2-006` и clause mapping `C2-142-020` подтверждают операторские предупреждения и данные при потере C2.

## 3. Allocation decision

На текущем evidence level:

```text
C2 state / failure information
→ BlueSky internal state model
→ operator-facing indication/warning
→ HMI allocation
```

Это является функциональной областью BlueSky, если конкретная индикация/предупреждение входит в утверждённый HMI scope.

Операторская функция уже распределена между существующим safety workflow и HMI scope; новый authoritative SYS-REQ не требуется.

Therefore:

```text
C2-REQ-010
→ DERIVED / COVERAGE CONFIRMED
→ SYS-C2-006 / C2-142-020
→ NO NEW SYS-REQ
```

## 4. Safety allocation

C2 degradation/failure processing already has safety allocation. Operator indication shall not override or weaken the Safety Gate or safety response. Any safety-significant warning must remain traceable to the allocated system/safety requirement and verification case.

## 5. HMI information classes

The controlled interface model may expose:

- current C2 state;
- degraded/lost condition;
- recovery state only after defined recovery criteria;
- safety-relevant C2 warning/status;
- acknowledgement/state visibility where already allocated.

These are **allocation targets**, not newly baselined requirements.

## 6. Verification linkage

Candidate verification linkage remains:

```text
C2-REQ-010
→ IF-C2-001
→ IV-C2-005 / IV-C2-006
→ VP-008
→ HMI-specific verification where existing plan requires it
```

`VP-008` remains `PLANNED`; therefore no HMI function is declared verified by this record.

## 7. Gate

```text
NEW SYS-REQ: 0
NEW SAF-REQ: 0
MASTER REGISTER CHANGE: 0
STATUS: DERIVED / COVERAGE CONFIRMED
```

## 8. Next controlled action

Maintain the allocation links to `SYS-C2-006`, `C2-142-020`, `IF-C2-001` and the applicable HMI/safety verification. No new SYS-REQ is created by this record.

## 9. Scope protection

External requirements remain external. This allocation does not convert requirements for landing sites, external C2 providers, or other external organizations into BlueSky system requirements.
