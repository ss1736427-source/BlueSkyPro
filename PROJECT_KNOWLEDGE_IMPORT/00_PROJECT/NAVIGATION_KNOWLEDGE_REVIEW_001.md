---
id: NAV-KNOWLEDGE-REVIEW-001
type: knowledge_review
title: Navigation Knowledge Review — Navigation State
status: draft_for_agreement
---

# Navigation Knowledge Review — Navigation State

## 1. Purpose

Проверить существующую Navigation Knowledge Map по установленному процессу:

Общая база → специализированные записи → оригинальный источник → внешний источник → сравнение → противоречия → согласованное решение → накопление.

Этот документ **не является ещё принятой спецификацией**.

## 2. Project knowledge already established

В существующих проектных материалах уже закреплено:

- для каждого БВС формируется собственный маршрут;
- для каждой WP рассчитываются положение, высота, курс, скорость и временные параметры;
- после изменения исходных данных или WP маршрут повторно проверяется;
- при конфликте или нарушении ограничения выполняется перерасчёт, если это возможно;
- после изменения одного маршрута повторно проверяется взаимодействие с маршрутами остальных БВС;
- карта и WP Table должны представлять одно и то же текущее состояние рабочего маршрута;
- фактическое состояние БВС используется для повторной проверки;
- для каждого БВС ведётся персональная временная шкала;
- возможность безопасного возврата оценивается постоянно и актуализируется при изменении условий;
- Dynamic Return оценивается по реально выполнимой траектории, а не только по геометрической дистанции.

Это подтверждается существующими проектными материалами Route Planning, WP Table, Multi-UAV Coordination, UAV State and Telemetry и Dynamic Return Calculation.

## 3. Primary source knowledge

В специализированной записи по Черному М. А. и Кораблину В. И. (1973) навигация рассматривается через взаимосвязанные навигационные элементы и контроль фактического полёта.

Для дальнейшего моделирования особенно важны:

- курс;
- путевое направление;
- пеленг;
- скорость;
- угол сноса;
- ветер;
- фактическое положение;
- контроль пути;
- расчёт и корректировка маршрута.

Исторический источник используется как предметная база. Он не заменяет современные нормативные и эксплуатационные источники.

## 4. External source check

В качестве современного независимого внешнего источника использован FAA Pilot’s Handbook of Aeronautical Knowledge, Chapter 16 — Navigation.

FAA различает:

- **Course** — intended path over the ground / direction of intended path on chart;
- **Heading** — direction in which the aircraft nose points;
- **Track** — actual path over the ground;
- **Drift angle** — angle between heading and track;
- **WCA** — correction applied to course to establish heading so track coincides with course;
- **Airspeed** — rate of progress through the air;
- **Groundspeed (GS)** — rate of progress over the ground.

Источник также показывает связь между heading, desired course, wind, drift angle и track.

## 5. Comparison

### 5.1 Agreement

Существующая проектная модель и внешний источник согласуются в фундаментальном разделении:

```text
INTENDED PATH / COURSE
        ≠
AIRCRAFT HEADING
        ≠
ACTUAL GROUND TRACK
```

Следовательно, проект не должен использовать одно поле `course` для всех трёх смыслов.

Также согласуется необходимость учитывать влияние ветра и различать airspeed и groundspeed.

### 5.2 Important clarification

В проектных документах сейчас для WP явно используется термин «курс».

Этого недостаточно для Navigation State.

Для каждой навигационной сущности необходимо хранить семантический тип направления, а не только числовой угол.

### 5.3 No contradiction established

На этом проходе прямого противоречия между проектной моделью и FAA по указанным базовым терминам не выявлено.

Есть **терминологический пробел**, а не противоречие.

## 6. Current gaps

### GAP-NAV-001 — Navigation State composition

Не утверждён окончательный состав Navigation State.

### GAP-NAV-002 — Direction semantics

Необходимо различать как минимум:

- course;
- heading;
- track;
- bearing;
- drift angle;
- wind direction.

### GAP-NAV-003 — Speed semantics

Необходимо различать:

- airspeed;
- groundspeed.

### GAP-NAV-004 — Reference frame

Для каждого направления требуется явно хранить reference:

- true;
- magnetic;
- compass, если применимо.

FAA AIM указывает, что направления, course, heading и bearing должны явно различать магнитную и истинную ссылку там, где это применимо.

### GAP-NAV-005 — Data quality

Нужно определить состояние навигационного значения:

- valid;
- degraded;
- stale;
- invalid;
- unavailable.

Также нужны timestamp/freshness и, где применимо, confidence.

### GAP-NAV-006 — Planned / Actual / Simulated

Проект уже различает Planned, Simulated и Actual.

Navigation State должен поддерживать эту семантику без смешивания планового и фактического состояния.

## 7. Proposed Navigation State structure

Это **рабочее предложение для согласования**, а не принятое решение:

```text
NavigationState
├── Position
├── Altitude
├── Heading
├── Course
├── Track
├── Bearing
├── Airspeed
├── Groundspeed
├── DriftAngle
├── Wind
├── ActiveRoute
├── ActiveWaypoint
├── CrossTrackError
├── AlongTrackPosition
├── ReferenceFrame
├── Validity
├── Freshness
├── Confidence
└── Timestamp
```

## 8. Important design principle

`Position` и `Navigation State` не должны быть единственным представлением навигации.

Минимальная логическая модель:

```text
PLANNED
  └─ intended route / course / WP / constraints

ACTUAL
  └─ position / heading / track / speed / wind-related state

DERIVED
  └─ deviation / cross-track error / along-track position /
     ETA / return feasibility

QUALITY
  └─ validity / freshness / confidence / source
```

## 9. Safety boundary

Навигационные расчёты не получают права непосредственного исполнения.

Контур остаётся:

```text
NAVIGATION DATA
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ AUTHORIZATION
→ C++ CORE
→ EXECUTION
```

## 10. Decision candidates

Пока предлагается принять только следующие положения:

1. `Course`, `Heading` и `Track` — разные сущности.
2. `Airspeed` и `Groundspeed` — разные сущности.
3. `DriftAngle` является производным навигационным параметром.
4. Направления должны иметь явно указанную reference frame.
5. Planned и Actual navigation state не смешиваются.
6. Каждое критическое навигационное значение должно иметь качество/актуальность.
7. Navigation State является источником данных для Route Planning, Dynamic Return, Collision Avoidance и Mission Execution, но не authority для непосредственного исполнения.

**Статус всех семи пунктов: PROPOSED — требуется согласование.**

## 11. Next step

После согласования этих положений создать:

`NAVIGATION_STATE_MODEL.md`

Затем вывести из него:

`NAVIGATION_RULES.md`

И только после этого формировать:

`NAVIGATION_ALGORITHM.md`

## 12. Sources

Primary project sources:
- Route Planning / Route Generation
- Route Planning / WP Table
- Route Planning / Dynamic Return Calculation
- Route Planning / Collision Avoidance
- Route Planning / 2D-3D Route Visualization
- UAV / UAV State and Telemetry
- UAV / Multi-UAV Coordination
- Mission / Mission Calculation
- Mission / Mission Execution
- Interface / Map as Mission Tool

Primary historical source:
- Черный М. А., Кораблин В. И. — Самолетовождение (1973)

External source:
- FAA, Pilot’s Handbook of Aeronautical Knowledge, Chapter 16 — Navigation
- FAA Aeronautical Information Manual, current edition



