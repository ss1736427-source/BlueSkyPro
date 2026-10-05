---
id: C2-REQUIREMENTS-RECONCILIATION-001
type: requirements_reconciliation
status: controlled_working_draft
system: BlueSky PRO
authority: MASTER-REQUIREMENTS-REGISTER-001
basis: C2-CLAUSE-LEVEL-MAPPING-001; C2_VERIFICATION_CASES_001; C2-ICD-BASELINE-001
---

# BLUE SKY PRO — C2 REQUIREMENTS RECONCILIATION 001

## 1. Назначение

Сопоставление C2-кандидатов, выведенных из приказа Минтранса России №142, с существующей requirements / safety / architecture / verification базой без создания второй независимой системы требований.

## 2. Authority

```text
MASTER_REQUIREMENTS_REGISTER.md
        ↓
C2 reconciliation
        ↓
SRS / Safety / ICD / Verification
```

Existing requirement IDs имеют приоритет. Candidate IDs не являются authoritative и не становятся baseline автоматически.

## 3. Контролируемые C2-кандидаты

| Candidate | Смысл | Решение текущего прохода |
|---|---|---|
| C2-REQ-001 | C2 mode/configuration | DERIVED; отдельный SYS-REQ не создаём |
| C2-REQ-002 | C2 degradation | MERGE candidate |
| C2-REQ-003 | C2 loss/recovery | MERGE candidate |
| C2-REQ-004..010 | Дополнительные функции C2 из системного baseline | DERIVED / ALLOCATION REVIEW; не переводить в master register без доказанного gap |

## 4. Сопоставление с существующей базой

| C2-кандидат | Existing coverage | Архитектурная основа | Решение |
|---|---|---|---|
| C2-REQ-001 | SYS-REQ-083 + existing C2 architecture records | Platform Independence / adapter boundary | DERIVED |
| C2-REQ-002 | SYS-REQ-086, SYS-REQ-093 | Graceful Degradation / Controlled Resource Recovery | MERGE candidate |
| C2-REQ-003 | SYS-REQ-081, SYS-REQ-086, SYS-REQ-093 | Failure Tolerance / Graceful Degradation / Controlled Recovery | MERGE candidate |
| C2-REQ-004 | existing C2 provider/interface allocation | IF-C2-001 | DERIVED / interface allocation |
| C2-REQ-005 | SYS-REQ-086, SYS-REQ-093 + diagnostics/safety allocation | C2 / HUB / SAFETY | DERIVED / covered |
| C2-REQ-006 | SYS-REQ-081, SYS-REQ-082, SYS-REQ-086, SYS-REQ-093 | FLIGHT / SAFETY | DERIVED / covered |
| C2-REQ-007 | existing event/logging records | HUB / DATA | DERIVED / covered |
| C2-REQ-008 | existing common time/reference allocation | SYSTEM / HUB | DERIVED / covered |
| C2-REQ-009 | existing safety authority chain | SAFETY / AI / FLIGHT | DERIVED / covered |
| C2-REQ-010 | existing HMI/C2 allocation | HMI / C2 | DERIVED / covered |

## 5. Проверенный результат по degradation/recovery

`SYS-REQ-086` уже определяет контролируемую деградацию и архитектурный поток:

```text
DETECT
→ CLASSIFY AS DEGRADED
→ ASSESS MISSION FEASIBILITY
→ CONTINUE / ADAPT / REALLOCATE / REDUCE SCOPE / REPLAN / ABORT
→ REVALIDATE
→ SAFETY GATE
```

Также `SYS-REQ-086` связан с `ARCH-DEC-017`, `ARCH-DEC-039`, `ARCH-DEC-023`.

`SYS-REQ-093` уже имеет архитектурную трассировку через `ARCH-DEC-017` и `ARCH-DEC-036`, с поддерживающими `ARCH-DEC-040` и `ARCH-DEC-041`.

Следствие: общий C2 degradation / recovery не является доказанным новым системным GAP.

## 6. Потеря C2

C2 loss/recovery не оформляется отдельным SYS-REQ только на основании нормативного кандидата. Проверяемая логика должна распределяться по существующим failure / degradation / recovery требованиям и safety response.

Authority chain сохраняется:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

Ни C2-кандидат, ни AI, HMI, planning или optimization не получают самостоятельную execution authority.

## 7. Quantitative criteria

Не baseline:

- transaction time;
- continuity;
- availability/readiness;
- integrity;
- loss-detection timing;
- restoration timing;
- coverage margin.

Все значения остаются `TBD` до подтверждения применимого основания, system boundary и allocation responsibility.

## 8. Verification reconciliation

Регуляторные C2-клаузы имеют прямую allocation в verification cases `C2-V01..C2-V08` через `C2_VERIFICATION_CASES_001.md`.

Working requirement allocation теперь дополнительно отражена в `C2_SYSTEM_REQUIREMENTS_BASELINE_001.md` с привязкой C2-REQ-001..010 к clause-level records `C2-142-*` и существующим authoritative coverage.

Правило покрытия:

```text
Regulatory clause
→ existing requirement / derived allocation
→ interface allocation
→ verification case
→ evidence
```

Equivalent existing TEST/VER records должны быть linked, а не дублированы. Если exact coverage не найдено, новый verification case создаётся только после подтверждения gap.

На текущем проходе case identities `C2-V01..C2-V08` уже определены; реальное выполнение и evidence отложены до test stage.

## 9. Текущее решение

```text
KEEP:
  существующие SYS-REQ и ARCH-DEC

DERIVED:
  C2 functions/interfaces, являющиеся производными от уже установленных требований

MERGE:
  C2 degradation/loss/recovery, где существующее покрытие эквивалентно

GAP:
  пока не доказан

CONFLICT:
  не установлен

NEW SYS-REQ:
  0
```

## 10. Ограничение System Boundary

Нормативные требования к внешним объектам или организациям не превращаются в требования BlueSky. Для каждого положения сначала устанавливается applicability и роль BlueSky.

```text
Regulation
→ Clause
→ Object of regulation
→ BlueSky role
→ System Boundary
→ DIRECT / INTERFACE / CONDITIONAL / EXTERNAL
→ Requirement / Interface / External Constraint
→ Verification / Evidence
```

## 11. Следующий controlled action

До появления реальных испытательных данных продолжать только те действия, которые не требуют нового нормативного или архитектурного решения:

1. сверять clause → requirement → interface → verification linkage;
2. устранять orphan links и дубли;
3. синхронизировать controlled indexes/registers;
4. поддерживать execution/configuration/evidence stubs;
5. не присваивать `BASELINED`, `VERIFIED` или `PASSED` на основании подготовительных записей.

После завершения доступного pre-execution reconciliation основной C2 work package ожидает только test-stage inputs для перехода к execution/evidence.

## 12. Status

**CONTROLLED WORKING DRAFT — C2 REQUIREMENT/INTERFACE/VERIFICATION RECONCILIATION INTEGRATED; NO NEW SYSTEM REQUIREMENT; REAL EXECUTION DEFERRED.**
