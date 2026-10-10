---
id: NAVIGATION-TRACK-COURSE-MODULE-001
type: navigation_module_specification
status: draft_for_agreement
version: 0.2
parent: NAVIGATION
authority: derived_from_master_requirements_system_architecture_navigation_state_model_rules_algorithm_mathematics
---

# BlueSky PRO — Navigation / Track & Course Module Specification

## 1. Назначение

Определить архитектурную и функциональную ответственность модуля `Track / Course` блока Navigation без изменения установленной границы системы и без введения неподтверждённых нормативных требований.

Модуль отвечает за семантику и представление направлений движения, необходимых для планирования, навигационного состояния, оценки фактического движения и связанных расчётов.

## 2. Архитектурная ответственность

Модуль предоставляет потребителям Navigation State значения, относящиеся к:

- `Course` — заданному/расчётному направлению движения в контексте маршрута;
- `Track` — фактическому направлению перемещения относительно поверхности Земли;
- planned/simulated/actual контекстам;
- quality, timestamp и provenance соответствующих значений;
- производным оценкам, если они явно обозначены как derived/estimated/predicted.

Модуль не является:

- исполнительным контуром управления БВС;
- Safety Gate;
- Authorization layer;
- самостоятельным источником разрешения на изменение маршрута;
- исполнительным регулятором курса.

Базовое разделение сохраняется:

```text
Navigation State ≠ Execution Authority
```

## 3. Семантика Track и Course

### Track

Фактическое направление вектора движения БВС относительно поверхности Земли. Track определяется из валидного навигационного состояния или соответствующего навигационного источника.

### Course

Направление, заданное или рассчитанное для движения по маршруту/сегменту в соответствующем reference frame.

`Track` и `Course` не должны использоваться как синонимы.

```text
Course ≠ Track
```

Разность между ними может быть существенной при наличии ветра, сноса, коррекции курса или иных условий движения.

Точная математическая семантика и reference-frame conventions должны быть согласованы с общей математической спецификацией Navigation.

## 4. Контекст состояния

Каждое значение должно иметь однозначный контекст:

```text
PLANNED
SIMULATED
ACTUAL
```

Например:

```text
planned.course
simulated.track
actual.track
```

Значения разных контекстов не являются взаимозаменяемыми.

## 5. Логическая модель данных

```text
DirectionValue
├── value
├── type
├── reference_frame
├── units
├── timestamp
├── source
├── freshness
├── validity
├── confidence
└── state/context
```

Где `type` однозначно определяет семантику, например:

```text
COURSE
TRACK
```

Единица угла, диапазон и правила нормализации фиксируются в математической и software specifications после согласования.

## 6. Provenance

Для существенных фактических значений сохраняются:

```text
source
timestamp
freshness
validity
confidence
```

Для derived/estimated/predicted значений должен быть указан факт производности и, при необходимости, исходные параметры расчёта.

Измеренное и вычисленное направление не должны смешиваться без явной политики fusion.

## 7. Quality states

Применяется общая модель качества Navigation:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Значение с состоянием `STALE`, `INVALID` или `UNAVAILABLE` не должно молча использоваться как текущее `VALID`.

Численные пороги качества пока не фиксируются.

## 8. Входные данные

Потенциальные входы:

```text
Position
Velocity / Groundspeed
Heading
Wind
Route
Active Waypoint
Mission State
Reference Frame
Navigation Source Metadata
Time
```

Фактический набор зависит от способа получения Track/Course.

Наличие входного поля не означает его валидность.

## 9. Выходные данные

Модуль предоставляет:

```text
Actual Track
Planned Course
Simulated Track/Course
Direction Quality
Direction Provenance
Course/Track Deviation
```

`Course/Track Deviation` является производным параметром.

## 10. Связь с Position

При отсутствии независимого Track source направление фактического движения может определяться по последовательности валидных позиций:

```text
Position(t1)
+
Position(t2)
→ displacement vector
→ Track
```

При этом необходимо учитывать timestamp, spatial resolution и качество Position.

Derived Track не должен представляться как независимое измерение.

## 11. Связь с Velocity / Speed

Track тесно связан с вектором наземной скорости:

```text
Ground Velocity
├── magnitude → Groundspeed
└── direction → Track
```

Скорость и направление должны быть согласованы по времени и reference frame.

Использование Airspeed вместо Groundspeed для определения фактического Track требует соответствующей wind model и не должно происходить неявно.

## 12. Связь с Heading

`Heading` описывает ориентацию продольной оси/направление носа БВС в принятой reference-frame semantics.

`Track` описывает направление фактического движения.

При наличии ветра и/или сноса:

```text
Heading ≠ Track
```

Разность между ними должна сохраняться как отдельная навигационная характеристика, если она используется системой.

Модуль Track/Course не изменяет Heading напрямую.

## 13. Связь с Wind

Ветер может приводить к разнице между направлением Heading/Course и Track.

Логическая модель:

```text
Airspeed vector
+
Wind vector
→ Ground velocity vector
→ Track + Groundspeed
```

Точные формулы, знаки и системы координат должны быть единообразны для всего Navigation block.

## 14. Course в Route Planning

Для активного сегмента маршрута `Course` может быть производным от геометрии маршрута:

```text
Waypoint A
+
Waypoint B
→ segment vector
→ planned Course
```

Для маршрутов с криволинейной траекторией или специальной моделью движения Course должен определяться согласно соответствующей route/trajectory specification.

Course не является сам по себе командой исполнительному контуру.

## 15. Track/Course deviation

Производный параметр может использоваться для оценки отклонения фактического движения от планового направления:

```text
planned Course
      ↕
actual Track
      ↓
angular deviation
```

Нормативные/операционные пороги не назначаются этим модулем автоматически.

Если deviation становится значимым, downstream logic определяет дальнейшую реакцию в соответствии с установленными правилами.

## 16. Dynamic Return

При Dynamic Return система может пересчитывать направление возврата на основании актуального состояния:

```text
Current Position
+
Return Target
+
Wind
+
Velocity
+
Constraints
→ Return Course
→ expected Track
```

Модуль предоставляет навигационные величины, но не принимает самостоятельно решение о начале возврата.

## 17. Multi-UAV

Для каждого БВС сохраняется индивидуальное состояние:

```text
UAV[i].Track
UAV[i].Course
UAV[i].DirectionQuality
```

Fleet-level представление не должно уничтожать индивидуальные quality/provenance данные.

Разделение задания между БВС относится к соответствующему Multi-UAV/Planning уровню.

## 18. Reference Frames

Track и Course должны иметь явно определённую систему отсчёта.

Минимально необходимо различать:

```text
horizontal direction
reference frame
north reference
angle convention
```

Конкретный выбор `true/magnetic/grid` и правила преобразования не утверждаются данным документом без основания в общей Navigation/Requirements baseline.

## 19. Time consistency

Сравнение Track, Course, Position, Heading, Velocity и Wind требует временной согласованности.

Нельзя без проверки сопоставлять значения, полученные в существенно разные моменты времени.

Параметры допустимой latency и синхронизации являются TBD до определения системных требований.

## 20. Failure behaviour

### Track unavailable

```text
Track unavailable
→ quality = UNAVAILABLE
→ assess dependent calculations
```

### Course unavailable

```text
Course unavailable
→ quality = UNAVAILABLE
→ assess active-route calculations
```

### Stale

```text
STALE
→ not treated as current VALID
→ downstream impact assessment
```

### Invalid

```text
INVALID
→ reject where valid navigation input is required
→ defined downstream response
```

### Conflicting sources

```text
multiple sources
→ preserve provenance
→ detect conflict
→ source/fusion policy
→ impact assessment
```

Конкретная fusion policy остаётся TBD.

## 21. Safety boundary

```text
Track / Course
      ↓
Navigation State
      ↓
Deviation / derived calculations
      ↓
Validation
      ↓
Safety assessment / Safety Gate
```

Модуль не имеет права самостоятельно:

- разрешать полёт;
- менять mission authorization;
- изменять маршрут без соответствующего управляющего механизма;
- выдавать команду исполнительному контуру.

## 22. HMI allocation

На эксплуатационном уровне оператору должны показываться только необходимые значения.

Потенциально:

```text
TRK
CRS
Track/Course deviation
Navigation quality
```

В зависимости от режима часть параметров может быть скрыта или отображаться как производная информация.

Внутренние source/fusion/quality детали не должны перегружать основной экран, но должны быть доступны диагностике и журналу в предусмотренном контуре.

Конкретный HMI layout фиксируется в общей HMI-проработке.

## 23. Verification

Необходимы как минимум проверки:

```text
Straight track
Course from route segment
Track from Position sequence
Crosswind
Headwind / tailwind
Heading ≠ Track
Course ≠ Track
Course/Track deviation
Reference-frame conversion
Timestamp consistency
Stale input
Invalid input
Unavailable input
Conflicting sources
Wind degradation
Position degradation
Velocity degradation
Dynamic Return direction
Multi-UAV individual states
Planned vs Actual
Simulated vs Actual
```

Конкретные test IDs назначаются после сопоставления с Verification Register.

## 24. Regulatory applicability

Нормативные требования не переносятся в модуль автоматически только из-за наличия слов «курс», «навигация» или «БВС».

Для каждого применимого источника используется:

```text
Official source / clause
→ regulated object
→ BlueSky role
→ system boundary
→ applicability
→ requirement
→ module allocation
→ verification
```

Требование, относящееся к внешнему объекту или процессу, не становится функцией Track/Course без доказанной ответственности BlueSky PRO.

## 25. Open items

```text
NAV-TC-OPEN-001  точная семантика Course
NAV-TC-OPEN-002  точная семантика Track
NAV-TC-OPEN-003  true/magnetic/grid reference policy
NAV-TC-OPEN-004  angle convention и normalization
NAV-TC-OPEN-005  допустимая latency
NAV-TC-OPEN-006  timestamp synchronization
NAV-TC-OPEN-007  source priority
NAV-TC-OPEN-008  source fusion
NAV-TC-OPEN-009  Position-derived Track rules
NAV-TC-OPEN-010  Course calculation rules for non-linear trajectories
NAV-TC-OPEN-011  deviation thresholds
NAV-TC-OPEN-012  Dynamic Return integration parameters
NAV-TC-OPEN-013  HMI presentation
NAV-TC-OPEN-014  verification test IDs
```

Открытый вопрос не считается автоматически GAP.

## 26. Completion gate

Модуль может перейти в `DESIGN-READY` после закрытия:

```text
[ ] responsibility
[ ] Track semantics
[ ] Course semantics
[ ] Planned / Simulated / Actual separation
[ ] reference-frame policy
[ ] data model
[ ] provenance
[ ] quality model
[ ] time consistency
[ ] source/fusion policy
[ ] Position-derived Track rules
[ ] route Course rules
[ ] deviation rules
[ ] failure behaviour
[ ] Safety boundary
[ ] HMI allocation
[ ] verification mapping
[ ] applicable regulatory requirements
[ ] open items resolved or formally accepted
```

## 27. Relationship to adjacent Navigation modules

```text
Position ───────┐
                ├→ Ground Velocity → Track
Velocity/Speed ─┤
                ├→ Airspeed + Wind → Ground Velocity → Track
Heading ────────┘

Route / Waypoints → Course

Track + Course → Deviation

Navigation Quality → validity of all above

Reference Frames → interpretation of directions

Navigation State → unified representation
```

## 28. Controlled references

- `01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md`
- `01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md`
- `01_REQUIREMENTS/SAFETY/SAFETY_REQUIREMENTS_BASELINE.md`
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
