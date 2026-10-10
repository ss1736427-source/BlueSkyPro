---
id: NAVIGATION-MATH-SPEC-001
type: navigation_mathematical_specification
status: draft_for_agreement
authority: derived_from_navigation_state_model_rules_algorithm_verification_model
---

# BlueSky PRO — Navigation Mathematical Specification 001

## 1. Назначение

Формализовать математический слой Navigation без введения неподтверждённых проектных параметров.

Документ развивает существующий `NAVIGATION_ALGORITHM.md` только в части математических определений. Он не изменяет system boundary, execution authority или существующий набор SYS-REQ.

## 2. Статус параметров

Каждая формула и параметр имеет один из статусов:

- `DEFINED` — определено проектом или согласованным источником;
- `DERIVED` — однозначно следует из уже определённых величин;
- `TBD` — необходимо отдельное проектное решение;
- `VERIFY` — формула/реализация определена, но требует verification.

Нельзя подменять `TBD` произвольным численным значением.

## 3. Vector representation

Для расчётов движения вводится векторная модель:

```text
Airspeed vector
+
Wind vector
=
Ground velocity vector
```

Векторные величины должны иметь единые:

```text
reference frame
units
sign convention
timestamp
provenance
```

Конкретная система координат проекта — `TBD`.

## 4. Course / Heading / Track

Рабочая семантика:

```text
Course  = intended ground direction/path
Heading = aircraft longitudinal-axis direction
Track   = actual ground-motion direction
```

При отсутствии ветра и при согласованных условиях `Heading` и `Track` могут совпадать, но система не должна использовать это как общее определение.

При наличии ветра необходимо рассчитывать их взаимосвязь через векторную модель движения.

## 5. Wind triangle

Пусть:

```text
V_a = airspeed vector
V_w = wind vector
V_g = ground velocity vector
```

Базовое векторное соотношение:

```text
V_g = V_a + V_w
```

Из `V_g` выводятся:

```text
Track = direction(V_g)
Groundspeed = |V_g|
```

При известном требуемом `Course` задача wind correction заключается в определении такого `Heading`/`V_a`, при котором фактический `Track` соответствует требуемому пути при допустимых условиях.

## 6. Wind Correction Angle

Рабочее определение:

```text
WCA = angular correction between intended course and required heading
```

Знак WCA и точная формула — `TBD`.

До утверждения системы координат, единиц углов и знаковой конвенции numerical implementation не baseline.

## 7. Drift Angle

Из `NAVIGATION_RULES`:

```text
Heading
+
Track
→ Drift Angle
```

Математически величина является угловым различием между фактическим `Heading` и `Track` в одной reference frame.

Точная функция нормализации угла и знак:

```text
DriftAngle = TBD
```

Нельзя смешивать Drift Angle и WCA: первая описывает фактическое/выведенное угловое различие, вторая — требуемую коррекцию для достижения intended course.

## 8. Groundspeed

```text
Groundspeed = |V_g|
```

где `V_g` — ground velocity vector.

`Airspeed` и `Groundspeed` являются разными физическими величинами и не должны храниться в одном универсальном поле `speed`.

## 9. Cross Track Error

Для активного сегмента маршрута:

```text
planned path
+
actual position
→ Cross Track Error
```

Crosstrack error должен иметь:

```text
magnitude
sign convention
reference geometry
units
```

Математическая формула и знак — `TBD` до утверждения coordinate/route geometry model.

## 10. Along Track Position

`Along Track Position` определяется относительно активного сегмента маршрута и фактической позиции.

Минимальная математическая модель должна позволять получить:

```text
position along active segment
remaining distance to target
segment transition state
```

Точная геометрическая модель и поведение около WP — `TBD`.

## 11. ETA

ETA является производной величиной.

Концептуально:

```text
ETA = current time + predicted remaining travel time
```

Predicted remaining travel time должен использовать согласованные:

```text
route geometry
groundspeed / predicted groundspeed
wind
environmental constraints
mission constraints
```

Точная модель прогноза — `TBD`.

## 12. Return Feasibility

Return Feasibility не определяется одной геометрической дистанцией.

Базовая зависимость:

```text
Current Navigation State
+
Energy / Resource State
+
Communication State
+
Environmental Constraints
+
Mission Constraints
→ Return Feasibility
```

Результат модели:

```text
FEASIBLE
DEGRADED
NOT FEASIBLE
UNKNOWN
```

Пороговые значения, energy model и критерий перехода между состояниями — `TBD`.

## 13. Reference frame

Каждое направление обязано содержать reference frame:

```text
TRUE
MAGNETIC
COMPASS
```

Преобразование между frame должно быть явным.

Не разрешается:

```text
TRUE value
→ treated as MAGNETIC
```

или обратная операция без зарегистрированной трансформации.

Точная модель magnetic variation и правила compass correction — `TBD`.

## 14. Position / coordinate model

Для Position должны быть явно определены:

```text
horizontal CRS
vertical reference / altitude datum
units
precision
valid range
```

Текущая Navigation State Model оставляет эти пункты открытыми. Поэтому настоящий документ не назначает конкретный CRS или datum.

## 15. Quality and temporal validity

Для критических навигационных значений сохраняются:

```text
source
timestamp
freshness
validity
confidence
```

Состояния:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Переходы между состояниями должны быть формализованы после утверждения thresholds.

`stale threshold` и `degraded threshold` — `TBD`.

## 16. Source fusion

При наличии нескольких источников Navigation должен сохранять:

```text
individual source provenance
source quality
fusion result provenance
```

Алгоритм fusion и source priority не определяются данным документом.

```text
Fusion algorithm = TBD
Source priority = TBD
Confidence calculation = TBD
```

До их утверждения нельзя считать конкретный fusion method частью baseline implementation.

## 17. Numerical precision

Для каждого математического параметра впоследствии должны быть определены:

```text
storage precision
calculation precision
output precision
acceptance tolerance
```

Универсальная tolerance для всех параметров не устанавливается.

## 18. Route segment / waypoint transition

Расчёт навигационных отклонений должен использовать конкретную версию:

```text
Mission ID
Route ID
Route Version
Active WP
WP Version
Navigation State Timestamp
```

При смене активного WP должна быть однозначно определена новая геометрия активного сегмента.

Turn anticipation и transition geometry — `TBD`.

## 19. Verification mapping

Математический слой проверяется существующими сценариями:

| Mathematical area | Verification |
|---|---|
| Wind triangle | NAV-V01…NAV-V05 |
| Course / Heading / Track | NAV-V01, NAV-V02, NAV-V06 |
| Drift / WCA | NAV-V02, V4 |
| Groundspeed | NAV-V03, NAV-V04 |
| Cross Track Error | NAV-V06 |
| WP transition | NAV-V07 |
| Temporal validity | NAV-V08…NAV-V12 |
| Dynamic Return | NAV-V15 |
| Runtime revalidation | NAV-V16, NAV-V17 |
| Multi-UAV state isolation | NAV-V19 |
| Planned / Simulated / Actual | NAV-V20 |

## 20. Implementation gate

До перехода к production implementation необходимо закрыть как минимум:

```text
CRS
Altitude Datum
Units
Precision
WCA
Drift sign
Cross Track Error sign/formula
Source fusion
Source priority
Freshness thresholds
Confidence model
Route geometry
WP transition / turn anticipation
```

После закрытия:

```text
Mathematical Specification
→ Test vectors
→ Automated verification
→ Software Design
→ C++ implementation
```

## 21. Safety boundary

Математически корректный результат Navigation не является разрешением на выполнение.

Сохраняется установленная цепочка:

```text
Navigation
→ Mission Validation
→ Readiness
→ Safety Gate
→ Authorization
→ C++ Core
→ Execution
```

## 22. Baseline impact

```text
NEW SYS-REQ = 0
NEW SAF-REQ = 0
MASTER REQUIREMENTS REGISTER = NO CHANGE
```

Документ является формальной спецификацией математического слоя и не создаёт автоматически новых системных требований.

## 23. Status

**DRAFT_FOR_AGREEMENT**

Документ готовит математический слой к следующему этапу — формированию test vectors и последующей Software Design.