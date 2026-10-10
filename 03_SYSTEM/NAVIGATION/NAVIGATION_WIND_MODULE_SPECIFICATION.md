---
id: NAVIGATION-WIND-MODULE-001
type: navigation_module_specification
status: draft_for_agreement
version: 0.2
parent: NAVIGATION
authority: NAVIGATION_STATE_MODEL; NAVIGATION_RULES; NAVIGATION_ALGORITHM; NAVIGATION_MATHEMATICAL_SPECIFICATION_001; MASTER_REQUIREMENTS_REGISTER
---

# BlueSky PRO — Navigation / Wind Module Specification

## 1. Назначение

Определить модуль `Wind` блока `Navigation` как источник и обработчик информации о ветре, необходимой для формирования навигационного состояния и связанных расчётов.

Модуль не является исполнительным контуром управления БВС и не принимает самостоятельно решений о разрешении, изменении или прекращении полёта.

## 2. Архитектурная граница

```text
External / onboard wind sources
            ↓
       WIND MODULE
            ↓
   Navigation State
            ↓
Planning / ETA / Energy / Return
            ↓
       Safety assessment
```

Модуль должен сохранять происхождение и качество ветровых данных. Расчётное значение не должно представляться как непосредственно измеренное.

## 3. Семантика

Основная сущность — `Wind Vector`:

```text
WindVector
├── speed
├── direction
├── reference
├── timestamp
├── source
├── freshness
├── validity
├── confidence
└── state/context
```

Направление должно иметь однозначную семантику: откуда дует ветер либо направление вектора движения воздушной массы. В конкретной реализации это правило должно быть единообразным во всех алгоритмах и HMI.

## 4. Контексты состояния

Сохраняется различие:

```text
PLANNED
SIMULATED
ACTUAL
```

`PLANNED` — ветровая модель, использованная при планировании.

`SIMULATED` — значение/поле, используемое в моделировании.

`ACTUAL` — актуальная оценка, полученная из фактических источников или определённая системой по валидным фактическим данным.

Эти состояния не должны смешиваться без явного правила.

## 5. Источники

Потенциальные источники:

- внешние метеоданные;
- onboard / БВС источники;
- вычислительная оценка по навигационным данным;
- данные, заданные пользователем или конфигурацией;
- синхронизированные ранее полученные данные при offline-работе.

Конкретный перечень допустимых источников и их приоритеты определяются requirements/design.

## 6. Provenance и качество

Для существенного значения сохраняются:

```text
source
source_timestamp
received_timestamp
freshness
validity
confidence
state
```

Применяется общая модель качества Navigation:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Старое или недостоверное значение не должно молча использоваться как актуальное.

## 7. Пространственная привязка

Ветер является векторной величиной и должен быть связан с установленной системой координат и временным контекстом.

Для расчётов должна быть известна применимая привязка:

```text
horizontal reference
vertical reference, if applicable
location
altitude / flight level, if applicable
time
```

Reference Frames являются общей ответственностью соответствующего модуля Navigation, поэтому Wind не должен самостоятельно вводить альтернативную систему координат.

## 8. Входные данные

Потенциальные входы:

```text
Weather data
Onboard wind estimate
Position
Ground velocity
Airspeed
Heading
Altitude
Time
Route / active WP
Navigation quality
Mission state
```

Каждый вход перед использованием проходит проверку validity/quality и временной согласованности.

## 9. Выходные данные

Модуль предоставляет потребителям:

```text
WindVector
WindQuality
WindProvenance
WindAge / Freshness
WindConfidence
WindProfile / Forecast reference, if applicable
```

Дополнительные производные параметры могут быть сформированы только при наличии определённой математической модели.

## 10. Связь с Navigation

Основные связи:

```text
Position + Airspeed + Heading + Ground Velocity
                     ↓
              Wind estimation
```

и для навигационных расчётов:

```text
Airspeed + Heading + Wind
          ↓
Ground Velocity
          ↓
Groundspeed + Track
```

Система должна сохранять различие между измерением, оценкой и прогнозом.

## 11. Связь с Route Planning

Wind используется для:

- оценки времени прохождения участков;
- расчёта Groundspeed;
- ETA;
- энергетической оценки;
- оценки выполнимости маршрута;
- оптимизации маршрута с учётом ветра;
- выбора экономичного режима полёта в пределах заданных ограничений.

Планировщик получает ветровые данные через определённый интерфейс, а не обращается к внутреннему состоянию Wind Module напрямую.

## 12. Ветровой профиль

При наличии соответствующих данных допускается представление ветра как функции координаты, высоты и времени:

```text
Wind = f(position, altitude, time)
```

Для различных участков маршрута могут использоваться различные ветровые оценки.

Переход к полноценной 4D wind field модели является архитектурно допустимым расширением, но не считается реализованной функцией без соответствующих требований и спецификации.

## 13. Offline / synchronization

При отсутствии внешней связи система может использовать ранее синхронизированные ветровые данные в пределах их установленной свежести.

```text
ONLINE
  ↓
sync / update
  ↓
local wind data
  ↓
OFFLINE
  ↓
validate freshness
  ↓
use / degrade / reject
```

Локальное использование устаревшего прогноза не должно скрываться от потребителей.

## 14. Dynamic Return

Dynamic Return может использовать актуальную оценку ветра для определения:

- направления и ожидаемой скорости возврата;
- времени возврата;
- энергетической выполнимости;
- изменения условий относительно исходного планирования.

Wind Module только предоставляет данные и их качество. Решение о возврате относится к вышестоящей логике.

## 15. Safety boundary

Wind не имеет самостоятельной authority:

- на изменение маршрута;
- на разрешение полёта;
- на команду исполнительным механизмам;
- на изменение safety policy.

Изменение качества ветровых данных передаётся в потребляющие контуры для установленной оценки последствий.

## 16. Failure behaviour

### 16.1 Нет данных

```text
NO DATA
→ UNAVAILABLE
→ dependent calculations evaluate impact
```

### 16.2 Устаревшие данные

```text
STALE
→ data freshness violation
→ no silent use as current data
→ degrade / reject according to consumer rules
```

### 16.3 Конфликт источников

```text
multiple sources
→ preserve provenance
→ detect conflict
→ apply defined source/fusion policy
```

Политика fusion не фиксируется до отдельного согласования.

### 16.4 Нереалистичное значение

Значения вне установленного диапазона или с нарушением внутренних consistency checks должны маркироваться `INVALID` либо соответствующим degraded state.

Численные пороги здесь не устанавливаются.

## 17. Связь с Velocity / Speed

```text
Airspeed + Wind Vector
→ Ground Velocity
→ Groundspeed / Track
```

Wind Module не дублирует семантику `Velocity / Speed`, а предоставляет ему необходимый векторный параметр.

## 18. Связь с Heading и Track/Course

```text
Heading
+
Airspeed
+
Wind
→ Ground Velocity
→ Track
```

Для движения по заданному Course возможна обратная задача определения требуемого Heading с учётом ветра. Конкретный алгоритм и ограничения относятся к соответствующей математической/планировочной спецификации.

## 19. Multi-UAV

Ветреное состояние может быть общим для района и одновременно иметь индивидуальные оценки для каждого БВС.

```text
Fleet Wind Model
       ↓
UAV[i] navigation state
```

Индивидуальные оценки не должны терять provenance и quality при объединении в fleet-level model.

## 20. HMI

На эксплуатационном экране оператору показывается только информация, необходимая в соответствующем режиме.

Потенциальные элементы:

```text
Wind speed
Wind direction
Wind quality / freshness
Wind effect on route
```

Внутренние source/fusion/quality details могут оставаться на диагностическом или расширенном уровне.

Конкретный HMI layout, thresholds и visual encoding определяются в общей HMI-проработке.

## 21. Verification

Минимальный набор проверок:

```text
Nominal wind input
Zero / calm wind
Headwind
Tailwind
Crosswind
Changing wind
Altitude-dependent wind
Spatially varying wind, if implemented
Stale data
Invalid data
Missing data
Conflicting sources
Estimated wind
Forecast wind
Actual wind
Offline cached wind
Synchronization update
Wind impact on Groundspeed
Wind impact on Track
Wind impact on ETA
Wind impact on energy model
Wind impact on Dynamic Return
Multi-UAV consistency
```

Конкретные test IDs назначаются после сопоставления с Verification Register.

## 22. Regulatory applicability

Нормативная применимость определяется не названием `Wind`, а конкретной функцией и границей ответственности BlueSky PRO.

Применяется цепочка:

```text
official source / clause
→ regulated object
→ BlueSky role
→ system boundary
→ applicability
→ requirement
→ module allocation
→ verification
```

Требования к внешним метеорологическим службам, источникам данных или иным внешним объектам не переносятся автоматически в функции Wind Module.

## 23. Open items

```text
NAV-WIND-OPEN-001  authoritative wind source(s)
NAV-WIND-OPEN-002  source priority
NAV-WIND-OPEN-003  fusion policy
NAV-WIND-OPEN-004  direction convention
NAV-WIND-OPEN-005  units
NAV-WIND-OPEN-006  validity limits
NAV-WIND-OPEN-007  freshness thresholds
NAV-WIND-OPEN-008  confidence model
NAV-WIND-OPEN-009  spatial interpolation
NAV-WIND-OPEN-010  altitude interpolation
NAV-WIND-OPEN-011  temporal interpolation
NAV-WIND-OPEN-012  forecast-to-actual transition
NAV-WIND-OPEN-013  offline cache lifetime
NAV-WIND-OPEN-014  exact verification test IDs
NAV-WIND-OPEN-015  HMI presentation and thresholds
```

Открытый вопрос не считается GAP автоматически.

## 24. Completion gate

Модуль может перейти в `DESIGN-READY` после закрытия:

```text
[ ] responsibility
[ ] source model
[ ] wind vector semantics
[ ] reference frame
[ ] time semantics
[ ] provenance
[ ] quality model
[ ] source conflict policy
[ ] derived/estimated semantics
[ ] Route Planning interface
[ ] Velocity/Speed interface
[ ] Heading / Track interface
[ ] Dynamic Return interface
[ ] offline behaviour
[ ] failure behaviour
[ ] HMI allocation
[ ] verification mapping
[ ] applicable regulatory requirements
[ ] open items resolved or formally accepted
```

До закрытия completion gate статус остаётся `DRAFT_FOR_AGREEMENT`.

## 25. Архитектурный принцип

Wind является **данными и моделью навигационного состояния**, а не самостоятельным центром принятия решений.

Это сохраняет общую архитектурную границу BlueSky PRO:

```text
DATA / STATE
      ↓
CALCULATION
      ↓
VALIDATION
      ↓
DECISION / SAFETY
      ↓
EXECUTION AUTHORITY
```

Модуль Wind не пересекает границу `Execution Authority`.

## 26. Controlled references

- `01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md`
- `01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md`
- `01_REQUIREMENTS/SAFETY/SAFETY_REQUIREMENTS_BASELINE.md`
- `01_REQUIREMENTS/REGULATORY/REGULATORY_SOURCE_REGISTER.md`
- `01_REQUIREMENTS/REGULATORY/REGULATORY_CLAUSE_MAPPING_001.md`
- `06_CERTIFICATION/BASIS/CERTIFICATION_BASIS.md`
- `02_ARCHITECTURE/SYSTEM/AVIATION_SYSTEM_DEFINITION.md`
- `03_SYSTEM/NAVIGATION/NAVIGATION_STATE_MODEL.md`
- `03_SYSTEM/NAVIGATION/NAVIGATION_RULES.md`
- `03_SYSTEM/NAVIGATION/NAVIGATION_ALGORITHM.md`
- `03_SYSTEM/NAVIGATION/NAVIGATION_MATHEMATICAL_SPECIFICATION_001.md`
- `05_VERIFICATION/NAVIGATION/NAVIGATION_TRACEABILITY_MATRIX_001.md`
- `05_VERIFICATION/NAVIGATION/NAVIGATION_VERIFICATION_MODEL.md`
- `05_VERIFICATION/NAVIGATION/TEST_VECTORS/NAVIGATION_TEST_VECTORS_001.md`
- `05_VERIFICATION/EVIDENCE/VERIFICATION_EVIDENCE_INDEX.md`
- `08_HMI/`

**Status: DRAFT_FOR_AGREEMENT**
