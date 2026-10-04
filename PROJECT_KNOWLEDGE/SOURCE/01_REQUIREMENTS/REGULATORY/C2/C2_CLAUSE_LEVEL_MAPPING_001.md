---
id: C2-CLAUSE-LEVEL-MAPPING-001
type: regulatory_clause_level_mapping
status: controlled_working_draft
system: BlueSky PRO
source: Приказ Минтранса России от 24.04.2025 №142
source_register: REGULATORY_SOURCE_REGISTER.md
---

# BLUE SKY PRO — C2 CLAUSE-LEVEL MAPPING 001

## 1. Назначение

Документ переводит конкретные положения ФАП, утверждённых приказом Минтранса России от 24.04.2025 №142, в проверяемые системные, интерфейсные и эксплуатационные требования BlueSky PRO в пределах установленной границы ответственности.

Приказ зарегистрирован в Минюсте 20.06.2025 №82667, вступил в силу 01.03.2026 и действует до 01.03.2032. Источник: официальный портал опубликования правовых актов. citeturn1search0turn1search1

## 2. Принцип распределения

```text
REGULATORY CLAUSE
→ REGULATORY OBJECT
→ BLUE SKY BOUNDARY
→ DIRECT / INTERFACE / EXTERNAL
→ REQUIREMENT
→ VERIFICATION
→ EVIDENCE
```

BlueSky не принимает на себя функции владельца БВС, C2-поставщика, оператора связи или владельца внешнего оборудования, если такие функции не включены в утверждённый scope.

## 3. Clause mapping

| ID | Clause | Regulatory obligation | BlueSky allocation | Target | Verification | Evidence |
|---|---:|---|---|---|---|---|
| C2-142-001 | 1 | C2 организуется прямым каналом СВП–БВС либо через инфраструктуру поставщика | DIRECT/INTERFACE | C2 architecture, ICD | design review + integration test | architecture record, interface test |
| C2-142-002 | 2 | Для прямого канала средства СВП и БВС должны находиться в зоне действия общей радиолинии | INTERFACE/CONSTRAINT | mission/C2 applicability model | analysis + operational test | coverage assessment |
| C2-142-003 | 3 | При использовании поставщика обмен идёт через его сеть передачи данных | INTERFACE | Provider-C2 ICD | interface test | provider interface evidence |
| C2-142-004 | 4 | Линия C2 должна соответствовать требуемым характеристикам связи с учётом класса воздушного пространства и ожидаемых условий полёта | DIRECT | C2 requirements + planning constraints | requirements verification | approved requirement + test/analysis evidence |
| C2-142-005 | 5 | Эксплуатационная документация БАС должна определять состав, объём и темп обновления информации между СВП и БВС | INTERFACE | ICD / data model | document review | controlled interface specification |
| C2-142-006 | 6 | Способ организации C2 выбирается до полёта с учётом района, условий и наличия оборудования поставщика | DIRECT | mission preparation / C2 mode selection | functional test | test record |
| C2-142-007 | 7 | C2 может использовать наземные, бортовые и/или спутниковые системы | DIRECT/INTERFACE | C2 architecture | architecture review | configuration baseline |
| C2-142-008 | 9 | При Provider-C2 подключение СВП к оборудованию поставщика осуществляется по радио или наземному каналу | INTERFACE | Provider-C2 ICD | integration test | interface evidence |
| C2-142-009 | 10.1 | Для Provider-C2 заявка должна содержать местоположение и технические характеристики оборудования СВП и БВС средствами C2 | INTERFACE | C2 service request data set | data validation test | request record |
| C2-142-010 | 10.2 | Район/маршрут, включая уход на запасную площадку, должен соответствовать зоне обслуживания поставщика | DIRECT/INTERFACE | route validation / provider coverage | analysis + test | coverage validation |
| C2-142-011 | 10.3 | Оборудование поставщика должно соответствовать требованиям по времени транзакции, непрерывности, готовности и целостности | INTERFACE | C2 QoS interface | integration/acceptance test | provider QoS evidence |
| C2-142-012 | 60 | Поставщик уведомляет об изменении географических координат зоны предоставления услуг | INTERFACE | provider status input | interface test | notification log |
| C2-142-013 | 61 | Качество услуг должно соответствовать требуемым характеристикам C2 в зависимости от типа БВС и класса ВП | INTERFACE | QoS monitoring model | verification by analysis/test | QoS records |
| C2-142-014 | 62 | Поставщик обеспечивает качество обслуживания во всё время полёта | INTERFACE | runtime monitoring | integration test | continuous-service evidence |
| C2-142-015 | 66 | Внешнему пилоту предоставляется информация о качестве обслуживания C2 в реальном времени | DIRECT/INTERFACE | pilot UI + telemetry | functional test | UI/test evidence |
| C2-142-016 | 68 | При прямом C2 владелец БВС обеспечивает информационную безопасность с учётом СКЗИ | INTERFACE/SECURITY | security architecture | security verification | security evidence |
| C2-142-017 | 69 | При Provider-C2 информационную безопасность каналов обеспечивает поставщик | EXTERNAL/INTERFACE | responsibility boundary | interface/document review | responsibility allocation |
| C2-142-018 | 70 | Внешний пилот постоянно контролирует состояние C2; отслеживаются прекращение излучения, прекращение приёма и снижение объёма информации ниже установленного уровня | DIRECT | C2 state monitoring | functional + fault-injection test | event/test logs |
| C2-142-019 | 71 | Поставщик обеспечивает периодические контрольные сообщения и подтверждение/неподтверждение приёма в обоих направлениях | INTERFACE | provider interface | integration test | transaction logs |
| C2-142-020 | 72 | Внешнему пилоту предоставляется результат оценки соответствия C2 требованиям качества | DIRECT/INTERFACE | pilot display / alerting | functional test | UI + system test |
| C2-142-021 | 74 | Документируется в том числе внешний пилот, время установления/окончания связи, качество, переключения, потери C2, местоположение/время/причина и радиопомехи | DIRECT/INTERFACE | C2 event journal | record/replay test | immutable event records |
| C2-142-022 | 75 | При определённом отказе наземного оборудования поставщик оформляет акт отказа в установленный срок | EXTERNAL/INTERFACE | failure notification/evidence interface | document/interface review | provider failure act |
| C2-142-023 | 77 | При потере C2 внешний пилот незамедлительно информирует соответствующий орган ОВД о потере и мерах восстановления | OPERATIONAL INTERFACE | alert/workflow support | scenario test | operator action record |
| C2-142-024 | 78 | Причины потери/нарушения C2 определяются по установленному перечню факторов, включая радиосреду, помехи, выход за зону, отказы оборудования, сетевой отказ, ошибки настройки и затенение антенны | DIRECT | fault classification / diagnostics | fault-injection + analysis | diagnostic/event evidence |
| C2-142-025 | 79 | Время принятия поставщиком решения о потере C2 должно соответствовать эксплуатационным требованиям БАС и классу ВП | INTERFACE | loss-decision timing input | timing analysis/test | provider timing evidence |

## 4. Системные требования, которые должны быть выделены

### SYS-C2-001 — C2 Mode

BlueSky должен поддерживать конфигурацию режима организации C2 с явным различением `DIRECT-C2` и `PROVIDER-C2`.

**Тип:** System Requirement.

### SYS-C2-002 — C2 State Monitoring

BlueSky должен получать и отображать состояние линии C2 и обнаруживать предусмотренные системой признаки деградации/потери, включая отсутствие передачи/приёма и снижение объёма информации ниже установленного эксплуатационной документацией уровня.

**Тип:** System + Safety.

### SYS-C2-003 — C2 QoS

BlueSky должен использовать параметры требуемых характеристик связи как входные ограничения для подготовки и выполнения миссии, если соответствующие параметры входят в установленный scope системы.

**Тип:** System Requirement.

### SYS-C2-004 — Provider Coverage

При Provider-C2 BlueSky должен проверять соответствие планируемого маршрута и необходимых участков маршрута зоне обслуживания поставщика на основании доступных актуальных данных.

**Тип:** Interface/System Requirement.

### SYS-C2-005 — C2 Event Logging

BlueSky должен регистрировать события установления, завершения, деградации, переключения и потери C2, а также связанные временные и позиционные данные в пределах установленной модели журналирования.

**Тип:** System + Safety + Evidence.

### SYS-C2-006 — C2 Loss Workflow

При установленном состоянии потери C2 BlueSky должен инициировать предусмотренный Safety Concept сценарий и обеспечить оператору необходимые предупреждения и данные для выполнения обязательных эксплуатационных действий.

**Тип:** Safety Requirement.

### SYS-C2-007 — C2 Cause Classification

BlueSky должен классифицировать доступные признаки причины нарушения C2 по установленной модели диагностики, не выдавая неподтверждённую причину как достоверный факт.

**Тип:** System/Safety Requirement.

### SYS-C2-008 — Time Base

Для C2 telemetry, commands, state transitions, alerts и event records должна применяться единая контролируемая временная база.

**Тип:** System/Safety/Verification Requirement.

## 5. Ограничения по ответственности

```text
BlueSky
  ├─ planning
  ├─ C2 mode/configuration
  ├─ monitoring
  ├─ alerts
  ├─ route/coverage validation
  ├─ event logging
  └─ safety workflow support

External C2 Provider
  ├─ provider infrastructure
  ├─ provider-side QoS
  ├─ provider-side security
  ├─ coverage establishment
  └─ provider maintenance/failure documentation

Owner / External Pilot
  ├─ operational decisions
  ├─ required notifications
  └─ compliance with operating procedures
```

## 6. Важное правило для требований

Не переносить текст приказа №142 в SRS как необработанную нормативную цитату.

Трансформация выполняется только после определения:

```text
Regulatory obligation
→ applicability
→ responsibility
→ system function
→ measurable requirement
→ verification method
→ evidence
```

## 7. Незакрытые вопросы

1. Конкретные количественные значения `transaction time / continuity / availability / integrity` должны быть получены из применимых норм лётной годности, эксплуатационной документации и утверждённого certification basis; произвольные значения не вводятся.
2. Необходимо окончательно определить certification object BlueSky и границу между BlueSky, БАС и C2 provider.
3. Необходимо связать SYS-C2-001…008 с существующими ID в `MASTER_REQUIREMENTS_REGISTER.md` и `SYSTEM_REQUIREMENTS_SPECIFICATION.md` после проверки их текущей структуры.
4. Для loss-of-C2 необходимо выполнить отдельную hazard/safety allocation.

## 8. Следующий шаг

```text
C2 clause mapping
→ существующий MASTER_REQUIREMENTS_REGISTER
→ SRS allocation
→ SAFETY_REQUIREMENTS_BASELINE
→ ICD
→ VERIFICATION_PLAN
→ VERIFICATION_EVIDENCE_INDEX
```

## 9. Source status

Основой для данного mapping является текст приказа №142, включая пункты 1–10, 60–62, 66–72, 74–75 и 77–79. citeturn2search0turn2search1

Статус документа: `CONTROLLED_WORKING_DRAFT`.
