---
id: NAVIGATION-ALGORITHM-001
type: navigation_algorithm
status: draft_for_agreement
version: 0.2
parent: NAVIGATION-STATE-MODEL-001
authority: NAVIGATION-RULES-001; NAVIGATION-STATE-MODEL-001; NAVIGATION-MATH-SPEC-001
---

# Navigation Algorithm

## 1. Назначение

Документ описывает логический алгоритм Navigation на основе действующей модели состояния и правил. Он не является программной реализацией и не создаёт новую независимую базу требований.

## 2. Document relationships

```text
Knowledge
  ↓
Navigation State Model
  ↓
Navigation Rules
  ↓
Navigation Algorithm
  ↓
Navigation Mathematical Specification
  ↓
Module Specifications
  ↓
Verification Model / Test Vectors
```

Системная граница и authority берутся из `02_ARCHITECTURE/SYSTEM/AVIATION_SYSTEM_DEFINITION.md`. Идентичность требований берётся из `01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md`.

## 3. Algorithm boundary

```text
INPUT SOURCES
    ↓
SOURCE VALIDATION
    ↓
NAVIGATION STATE
    ↓
REFERENCE FRAME
    ↓
PLANNED ROUTE / ACTIVE WP
    ↓
WIND / ENVIRONMENT
    ↓
COURSE / TRACK / HEADING RELATION
    ↓
DERIVED NAVIGATION VALUES
    ↓
DEVIATION / FEASIBILITY
    ↓
QUALITY ASSESSMENT
    ↓
REVALIDATION
    ↓
NAVIGATION OUTPUT
```

Navigation output является входом validation/planning и не является execution authority.

## 4. Input acquisition

Собираются доступные:

```text
Position
Altitude
Heading
Track
Airspeed
Groundspeed
Wind
Time
Route
Active WP
Mission state
Communication state
Resource / energy state
Navigation source metadata
```

Наличие поля не означает его validity.

## 5. Source validation

Для критического значения определяются:

```text
Source
Timestamp
Freshness
Validity
Confidence
```

Классы качества:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

## 6. Reference frame

Перед сравнением направлений определяется reference frame:

```text
TRUE
MAGNETIC
COMPASS
```

Преобразование между frame должно быть явным.

## 7. Navigation state construction

Состояние строится в существующей модели:

```text
PLANNED
ACTUAL
DERIVED
QUALITY
```

При необходимости допускается отдельное `SIMULATED` состояние; оно не становится `ACTUAL` без фактического подтверждения.

## 8. Intended path

Для активного сегмента:

```text
Current Position
+
Active WP
+
Next WP / Route Geometry
→ Intended Course
```

Точная геометрия определяется соответствующей Route/Geometry спецификацией, а не этим документом.

## 9. Wind relationship

При валидном Wind:

```text
Desired Course
+
Wind Vector
+
Airspeed
→ Wind Correction
→ Heading
→ Resulting Track / Groundspeed
```

Базовое векторное соотношение и математические определения принадлежат `NAVIGATION-MATH-SPEC-001`.

## 10. Actual navigation state

Определяются фактические:

```text
Position
Heading
Track
Airspeed
Groundspeed
```

Измеренные, оценочные и производные значения сохраняют соответствующую provenance.

## 11. Deviation

```text
ACTUAL
↔
PLANNED
```

Могут рассчитываться:

```text
Cross Track Error
Along Track Position
Course / Track Difference
Heading / Intended Direction Difference
Altitude Deviation
Speed Deviation
Time / ETA Deviation
```

Точные формулы и знаковые соглашения определяются `NAVIGATION-MATH-SPEC-001`.

## 12. Navigation quality

Качество оценивается отдельно от геометрии.

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Критические изменения качества передаются в соответствующий impact/revalidation контур.

## 13. Route feasibility

Оценка выполняется с учётом доступных:

```text
Position
Altitude
Track
Groundspeed
Wind
Energy
Communication
Environment
Constraints
Mission state
```

Результат:

```text
FEASIBLE
DEGRADED
NOT FEASIBLE
UNKNOWN
```

## 14. Material change and revalidation

```text
STATE CHANGE
→ IMPACT ASSESSMENT
→ REVALIDATION when required
```

При необходимости адаптации:

```text
GENERATE OPTIONS
→ SIMULATE / ANALYZE
→ VALIDATE
→ SELECT / APPROVE
→ EXECUTE
```

Authority chain определяется системной архитектурой и не переопределяется Navigation.

## 15. Navigation outputs

```text
Current Navigation State
Route Deviation
Navigation Quality
Wind / Drift Information
WCA
Track / Course Relationship
Route Feasibility
Return Feasibility
Revalidation Trigger
```

Выходы сохраняют provenance и quality.

## 16. Dynamic Return

```text
CURRENT NAVIGATION STATE
+
ENERGY / RESOURCE STATE
+
COMMUNICATION
+
ENVIRONMENT
+
MISSION CONSTRAINTS
→ RETURN FEASIBILITY
```

При существенном ухудшении:

```text
DETECT
→ CLASSIFY
→ REVALIDATE
→ UPDATE RETURN PLAN
```

## 17. Multi-UAV

Для каждого БВС сохраняется:

```text
UAV[i].NavigationState
```

Fleet coordination потребляет индивидуальные состояния и не уничтожает их provenance.

## 18. Failure / degraded handling

Различаются:

```text
Missing
Stale
Invalid
Conflicting
Degraded
```

Критическое отсутствие/недостоверность приводит к impact assessment, а не к молчаливой подстановке.

## 19. Algorithm invariants

1. `PLANNED ≠ ACTUAL`.
2. `SIMULATED ≠ ACTUAL`.
3. `Course ≠ Heading ≠ Track`.
4. `Airspeed ≠ Groundspeed`.
5. Reference frame explicit.
6. Derived values retain derived provenance.
7. Invalid/stale data is not silently treated as valid.
8. Navigation State is not execution authority.
9. Safety constraints cannot be overridden by optimization.
10. AI proposal cannot independently authorize material execution.
11. Individual UAV states remain distinct.
12. Material state changes can require revalidation.

## 20. Mathematical authority

Не дублировать здесь окончательные формулы. Использовать:

`03_SYSTEM/NAVIGATION/NAVIGATION_MATHEMATICAL_SPECIFICATION_001.md`

Открытые параметры остаются там в статусе `TBD` до отдельного решения.

## 21. Verification authority

Проверка алгоритма выполняется через:

`05_VERIFICATION/NAVIGATION/NAVIGATION_VERIFICATION_MODEL.md`

и существующие test vectors:

`05_VERIFICATION/NAVIGATION/TEST_VECTORS/NAVIGATION_TEST_VECTORS_001.md`

Новая проверка создаётся только при отсутствии существующего покрытия.

## 22. Requirements / architecture linkage

Requirements identity:

`01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md`

System boundary / functional architecture:

`02_ARCHITECTURE/SYSTEM/AVIATION_SYSTEM_DEFINITION.md`

Navigation requirement candidates `NAV-REQ-*` не становятся baseline автоматически; allocation выполняется через reconciliation.

## 23. Safety boundary

```text
NAVIGATION
    ↓
MISSION VALIDATION
    ↓
READINESS
    ↓
SAFETY GATE
    ↓
AUTHORIZATION
    ↓
C++ CORE
    ↓
EXECUTION
```

Navigation не обходит Mission Validation, Readiness, Safety Gate или требуемое approval.

## 24. External aviation reference

Для терминологической сверки используется FAA PHAK Chapter 16. Этот внешний материал является сравнительным техническим источником и не заменяет российскую certification basis проекта.

## 25. Status

**DRAFT_FOR_AGREEMENT**

Следующий этап — закрытие математических TBD, точное requirement allocation и formal verification allocation; затем переход к Software Design.
