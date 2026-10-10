---
id: PRACTICAL-FLIGHT-PLANNING-RECONCILIATION-001
type: requirements_reconciliation
status: UNDER_REVIEW
system: BlueSky PRO
source_basis: External practical source — GeoPlus article on selecting drone flight-planning software
---

# Practical Flight Planning Requirements — Reconciliation

## 1. Назначение

Документ фиксирует результат проверки кандидатных требований `PFP-REQ-001..012` из `01_REQUIREMENTS/SYSTEM/PRACTICAL_FLIGHT_PLANNING_INTEROPERABILITY_REQUIREMENTS_001.md` на дублирование с существующей архитектурой и требованиями BlueSky PRO.

Внешний источник используется как практический benchmark, а не как нормативный источник.

## 2. Правило принятия

Не создавать отдельную систему функций, если возможность уже покрыта существующей архитектурой.

Не создавать второй requirement для той же контролируемой функции.

Если практическая функция является реальным новым системным capability gap, она сохраняется как отдельный candidate requirement и далее проходит обычный жизненный цикл требований.

## 3. Результат

| Candidate | Решение | Основание |
|---|---|---|
| PFP-REQ-001 | KEEP AS CANDIDATE | Offline Mission Package не представлен как отдельное формализованное системное требование в проверенном Master Register |
| PFP-REQ-002 | KEEP AS CANDIDATE | Offline Continuity требует явного требования на непрерывность работы planning workspace |
| PFP-REQ-003 | KEEP AS CANDIDATE | Local Result Retention требует явного требования на локальное удержание результатов |
| PFP-REQ-004 | KEEP AS CANDIDATE | Synchronization State требует отдельного пользовательского/системного состояния |
| PFP-REQ-005 | KEEP AS CANDIDATE | Controlled Synchronization требует явных правил version/object identity/conflict detection |
| PFP-REQ-006 | KEEP AS CANDIDATE | KML interoperability не обнаружена как отдельное формализованное требование в проверенном Master Register |
| PFP-REQ-007 | KEEP AS CANDIDATE | KML preview/validation является полезной защитой перед применением внешней геометрии |
| PFP-REQ-008 | KEEP AS CANDIDATE | Разделение geometry import и flight authorization должно быть явно закреплено |
| PFP-REQ-009 | DERIVED / NO NEW REQUIREMENT YET | Использование сохранённой миссии как основы должно быть сопоставлено с существующими mission requirements до выделения нового ID |
| PFP-REQ-010 | DERIVED / NO NEW REQUIREMENT YET | Изменение миссии и перерасчёт маршрута относятся к существующим planning/navigation capabilities; требуется allocation, а не автоматическое создание нового требования |
| PFP-REQ-011 | DERIVED / NO NEW REQUIREMENT YET | Назначение UAV/pilot/equipment уже связано с Administrator/role/configuration architecture |
| PFP-REQ-012 | DERIVED / KEEP LINK | Уже закреплён системный принцип минимализма UX: постоянно доступно только необходимое, вторичное — контекстно |

## 4. Что считаем реальным полезным дополнением

После overlap review первоочередными кандидатами остаются:

1. Offline Mission Package;
2. Offline Continuity;
3. Local Result Retention;
4. Synchronization State;
5. Controlled Synchronization;
6. KML Import;
7. KML Import Preview/Validation;
8. Geometry-to-Mission Separation.

Это не означает автоматического baseline. Все восемь записей проходят `AGREED`/`BASELINED` только после проверки источника, safety impact, architecture allocation и verification method.

## 5. Практическая проверка внешним benchmark

Mission Planner показывает, что в реальных GCS планирование включает waypoint/grid tools, загрузку и сохранение mission files, KML/SHP, предварительное получение карт для offline use, а также работу с mission logs. citeturn0search0turn0search3

Следовательно, для BlueSky PRO практически значимы не сами названия функций, а следующие свойства рабочего контура:

```text
PLAN
→ VALIDATE
→ SAVE LOCALLY
→ EXECUTE
→ RETAIN RESULT
→ SYNCHRONIZE
```

Импорт внешней геометрии должен оставаться отдельным шагом перед формированием разрешённой к выполнению миссии.

## 6. Архитектурное ограничение

Новые interoperability capabilities не должны превращаться в самостоятельные крупные подсистемы.

```text
External source
      ↓
Integration / Import
      ↓
Validation
      ↓
BlueSky internal model
      ↓
Mission Planning
      ↓
Safety / Readiness
      ↓
Execution
```

Offline и synchronization являются свойствами рабочего контура данных, а не отдельным пользовательским режимом с дублирующими экранами.

## 7. UX consequence

Новые возможности должны подчиняться:

- часто используемая информация — постоянно доступна;
- вторичная и объёмная — в подменю;
- состояние — через компактные индикаторы;
- подробности — по запросу;
- импорт/синхронизация не должны перегружать Flight Chart;
- ошибки и конфликты показываются только когда требуют действия пользователя.

## 8. Safety boundary

Ни offline, ни KML, ни synchronization, ни repeat/revision не получают самостоятельной authority на выполнение полёта.

Сохраняется цепочка:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

## 9. Следующее действие

До изменения authoritative `MASTER_REQUIREMENTS_REGISTER.md` необходимо:

1. присвоить каждому из восьми retained candidates владельца;
2. определить allocated subsystem/function;
3. определить safety/certification relevance;
4. определить verification method;
5. проверить exact overlap с `SYS-REQ-*`, `MIS-REQ-*`, `HMI-REQ-*`, `DATA-REQ-*`, `CFG-REQ-*`;
6. после согласования перенести только подтверждённые новые требования в Master Register;
7. связать их с architecture/design/verification.

## 10. Status

**UNDER_REVIEW — RECONCILIATION COMPLETE FOR CURRENT PRACTICAL BENCHMARK.**

No existing requirement IDs are renumbered or silently replaced.
