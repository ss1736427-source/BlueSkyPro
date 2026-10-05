---
id: NAVIGATION-VELOCITY-SPEED-MODULE-001
type: navigation_module_specification
status: draft_for_agreement
version: 0.2
parent: NAVIGATION
authority: derived_from_master_requirements_system_architecture_navigation_state_model_rules_algorithm_mathematics
---

# BlueSky PRO — Navigation / Velocity & Speed Module Specification

## 1. Назначение

Определить модуль `Velocity / Speed` блока Navigation без изменения установленной архитектурной границы и без введения неподтверждённых нормативных или системных требований.

Модуль отвечает за представление и контроль семантики скоростных параметров, необходимых для навигационного состояния и связанных расчётов.

## 2. Архитектурная ответственность

Модуль предоставляет навигационным потребителям:

- фактическую скорость относительно воздушной массы (`Airspeed`), если доступна;
- фактическую скорость относительно поверхности Земли (`Groundspeed`), если доступна;
- плановую скорость в составе `PLANNED Navigation State`;
- качество, временную актуальность и происхождение существенных значений;
- производные оценки, только если они явно обозначены как derived/estimated/predicted.

Модуль не является:

- исполнительным контуром управления БВС;
- Safety Gate;
- Authorization layer;
- источником разрешения на изменение маршрута;
- самостоятельным регулятором двигателя или исполнительных механизмов.

Установленная граница сохраняется:

```text
Navigation State ≠ Execution Authority
```

## 3. Семантика скоростей

В соответствии с Navigation State Model нельзя использовать одно универсальное поле `speed`, если это приводит к потере смысла.

### Airspeed

Скорость движения БВС относительно воздушной массы.

### Groundspeed

Скорость перемещения БВС относительно поверхности Земли.

### Planned Speed

Намеренное/расчётное значение скорости в плановом состоянии миссии или маршрута.

### Simulated Speed

Расчётное значение, полученное в режиме моделирования. Не является фактической скоростью.

```text
PLANNED ≠ SIMULATED ≠ ACTUAL
```

Точное деление `Airspeed` на IAS/TAS/EAS и соответствующие модели не вводится до появления основания в requirements/design.

## 4. Минимальная логическая модель данных

```text
SpeedValue
├── value
├── type
├── units
├── timestamp
├── source
├── freshness
├── validity
├── confidence
└── state/context
```

Где `type` однозначно определяет семантику значения, например:

```text
PLANNED
SIMULATED
AIRSPEED
GROUNDSPEED
```

Точный API schema и storage schema являются предметом Software Design.

## 5. Provenance

Для каждого существенного фактического значения сохраняется:

```text
source
timestamp
freshness
validity
confidence
```

Если значение вычислено системой, источник и статус должны отражать факт расчёта.

Запрещается выдавать estimated/derived значение как measured без явной маркировки.

## 6. Quality states

Для существенных скоростных данных применяется общая модель качества Navigation State:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Значения `STALE`, `INVALID` и `UNAVAILABLE` не должны молча трактоваться как актуальные `VALID` данные.

Численные пороги качества не устанавливаются этим документом.

## 7. Входные данные

Потенциальные входы:

```text
Airspeed source
Groundspeed source
Position
Time
Wind
Navigation source metadata
Planned speed
Route / active WP
Mission state
```

Использование каждого входа определяется конкретным расчётом.

Наличие поля во входном сообщении не означает его валидность.

## 8. Выходные данные

Модуль может передавать:

```text
Actual Airspeed
Actual Groundspeed
Planned Speed
Simulated Speed
Speed Quality
Speed Provenance
Speed Deviation
Speed-related Navigation State
```

`Speed Deviation` является производным параметром и не должен маскироваться под измеренное значение.

## 9. Связь с Position

`Groundspeed` и изменение `Position` должны быть согласованы по времени и контексту.

Логически:

```text
Position(t1)
+
Position(t2)
+
Δt
→ derived Groundspeed
```

если такое вычисление применяется.

Если одновременно существует независимый источник Groundspeed, оба значения не должны автоматически считаться эквивалентными: необходимо сохранить provenance и применить установленную source/fusion policy.

Точная политика fusion остаётся TBD.

## 10. Связь с Wind

Навигационный алгоритм использует различие между движением относительно воздушной массы и движением относительно Земли.

Логическая связь:

```text
Airspeed
+
Wind Vector
→ Ground Velocity
→ Groundspeed / Track
```

Обратные оценки также возможны при наличии необходимых валидных входов, но конкретный алгоритм и условия применимости определяются математической спецификацией.

## 11. Связь с Course / Heading / Track

Модуль Velocity / Speed не определяет самостоятельно значения `Course`, `Heading` или `Track`.

Он предоставляет скоростные параметры для общего Navigation State.

Основная связь:

```text
Course
+
Wind
+
Airspeed
→ Heading / Ground Velocity
```

и:

```text
Heading
+
Airspeed
+
Wind
→ Track + Groundspeed
```

Точные формулы, знаки и reference frames фиксируются в математической спецификации.

## 12. Planned / Actual / Simulated

Модуль обязан сохранять различие контекстов:

```text
PLANNED
SIMULATED
ACTUAL
```

Например:

```text
planned.speed
simulated.airspeed
actual.groundspeed
```

не являются взаимозаменяемыми сущностями.

Переход от расчётного значения к фактическому происходит только при наличии соответствующего фактического источника/подтверждения.

## 13. Time consistency

Для сравнения скорости с другими навигационными величинами необходимо учитывать timestamp.

Нельзя без проверки сравнивать:

```text
Position(t0)
с
Groundspeed(t1)
```

если временная разница способна существенно повлиять на расчёт.

Требования к допустимой latency и синхронизации остаются TBD.

## 14. Failure behaviour

### 14.1 Airspeed unavailable

```text
Airspeed unavailable
→ quality = UNAVAILABLE
→ determine impact on dependent calculations
```

Нельзя молча заменить отсутствующий Airspeed произвольным Groundspeed.

### 14.2 Groundspeed unavailable

```text
Groundspeed unavailable
→ quality = UNAVAILABLE
→ determine impact
```

При наличии достаточных валидных данных система может иметь возможность вычислить derived value, но такое значение должно сохранять derived/estimated semantics.

### 14.3 Stale

```text
STALE
→ value not treated as current VALID
→ impact assessment
```

### 14.4 Invalid

```text
INVALID
→ value rejected from calculations requiring valid input
→ defined downstream response
```

### 14.5 Conflicting sources

Если разные источники дают несовместимые значения:

```text
sources preserved
→ conflict detected
→ source/fusion policy
→ impact assessment
```

Конкретная policy не фиксируется без отдельного согласования.

## 15. Navigation Quality impact

Качество скорости оценивается отдельно от самой величины скорости.

Пример логики:

```text
VALID
→ normal use

DEGRADED
→ use only where permitted by defined rules

STALE / INVALID / UNAVAILABLE
→ determine impact
→ revalidation when required
```

Окончательное решение о безопасности находится выше данного модуля.

## 16. Связь с Route Planning

Route Planning использует скоростные данные для расчётов, связанных с:

- временем прохождения сегментов;
- ETA;
- влиянием ветра;
- энергетической моделью;
- оценкой выполнимости маршрута;
- оптимизацией маршрута.

Плановая скорость должна быть доступна как часть планового состояния, а фактическая скорость — как часть actual state.

## 17. Связь с Dynamic Return

Dynamic Return может использовать:

```text
Current Groundspeed
Current Airspeed
Wind
Position
Energy / Resource State
Communication State
Mission Constraints
```

для оценки возврата.

Скорость сама по себе не принимает решение о возврате.

## 18. Multi-UAV

Для каждого БВС сохраняется индивидуальное состояние:

```text
UAV[i].Airspeed
UAV[i].Groundspeed
UAV[i].SpeedQuality
```

Fleet-level calculations не должны уничтожать индивидуальную provenance и quality information.

## 19. Safety boundary

```text
Velocity / Speed
        ↓
Navigation State
        ↓
Derived calculations
        ↓
Validation
        ↓
Safety assessment / Safety Gate
```

Изменение или деградация скорости не даёт модулю права самостоятельно:

- менять маршрут;
- разрешать выполнение миссии;
- выдавать authorization;
- управлять исполнительными механизмами.

## 20. HMI allocation

Для оператора не требуется отображать весь внутренний поток скоростных данных.

На эксплуатационном уровне должны быть представлены только те значения и состояния, которые необходимы для принятия решения в соответствующем режиме.

Потенциальные элементы:

```text
GS
AS
Target / Planned Speed
Speed deviation
Speed quality / degradation
```

Конкретный состав и представление HMI не утверждаются до общей HMI-проработки блока Navigation.

Принцип BlueSky PRO сохраняется: техническая сложность остаётся внутри системы, а оператор получает ясное эксплуатационное состояние.

## 21. Verification mapping

Проверки должны охватывать как минимум:

```text
Nominal Airspeed
Nominal Groundspeed
Wind influence
Crosswind
Headwind / tailwind
Position-derived Groundspeed
Source timestamp consistency
Stale speed
Invalid speed
Missing speed
Conflicting speed sources
Degraded navigation source
Planned vs Actual speed
Simulated vs Actual speed
Speed impact on ETA
Speed impact on Return Feasibility
Multi-UAV individual speed state
Material runtime change
```

Конкретные test IDs назначаются после сопоставления с Verification Register.

## 22. Regulatory applicability

Нормативное требование не переносится в данный модуль автоматически только потому, что оно относится к БАС или навигации.

Для каждого применимого требования используется цепочка:

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

Если требование относится к внешнему объекту, процессу или оборудованию, оно не становится функцией Velocity / Speed без доказанной связи с ответственностью системы.

## 23. Open items

```text
NAV-VEL-OPEN-001  точная семантика Airspeed для конкретных источников
NAV-VEL-OPEN-002  IAS/TAS/EAS applicability, если требуется
NAV-VEL-OPEN-003  единицы измерения
NAV-VEL-OPEN-004  допустимые диапазоны
NAV-VEL-OPEN-005  precision
NAV-VEL-OPEN-006  freshness threshold
NAV-VEL-OPEN-007  latency / time synchronization
NAV-VEL-OPEN-008  source priority
NAV-VEL-OPEN-009  source fusion
NAV-VEL-OPEN-010  правила derived Groundspeed
NAV-VEL-OPEN-011  правила определения Speed Deviation
NAV-VEL-OPEN-012  связь с энергетической моделью
NAV-VEL-OPEN-013  точные verification test IDs
NAV-VEL-OPEN-014  HMI thresholds / presentation
```

Открытый вопрос не считается GAP автоматически.

## 24. Completion gate

Модуль `Velocity / Speed` может перейти в `DESIGN-READY` после закрытия:

```text
[ ] responsibility
[ ] Airspeed semantics
[ ] Groundspeed semantics
[ ] Planned / Simulated / Actual separation
[ ] data model
[ ] provenance
[ ] quality model
[ ] time consistency
[ ] Position relationship
[ ] Wind relationship
[ ] Course/Heading/Track relationship
[ ] failure behaviour
[ ] safety allocation
[ ] HMI allocation
[ ] verification mapping
[ ] regulatory applicability
[ ] requirement traceability
```

## 25. Controlled references

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

## 26. Status

**DRAFT_FOR_AGREEMENT**

Документ является детальной рабочей спецификацией модуля. Новых SYS-REQ автоматически не создаёт.

Следующий модуль Navigation:

```text
HEADING
```
