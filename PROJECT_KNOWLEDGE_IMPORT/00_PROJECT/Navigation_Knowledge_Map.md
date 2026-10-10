---
id: NAV-KNOWLEDGE-MAP-001
type: knowledge_map
title: Unified Navigation Knowledge Map
status: draft
authority: project_knowledge_base
scope: BlueSky Navigation
---

# Unified Navigation Knowledge Map

> Рабочий слой объединения знаний. Не заменяет первичные источники, требования или принятые архитектурные решения.

## 1. Правило формирования знания

```text
SOURCE
→ EXTRACTED KNOWLEDGE
→ CONCEPT
→ RULE / CONSTRAINT
→ REQUIREMENT
→ ARCHITECTURE
→ CAPABILITY
→ ALGORITHM
→ VERIFICATION
→ EVIDENCE
```

При конфликте сведения не объединяются молча. Сначала фиксируется противоречие, затем определяется authority источников и принимается отдельное согласованное решение.

---

## 2. Navigation Domain — центральная модель

### 2.1 Planned State

Планируемое состояние должно включать как минимум:

- маршрут;
- WP / waypoint;
- пространственное положение элементов маршрута;
- высоту;
- временные параметры;
- параметры движения;
- ограничения и зоны;
- применимые ресурсы и возможности БВС.

Источники проекта уже связывают маршрут, WP и план полёта, а карта должна отображать маршрут, БПЛА и waypoint.

### 2.2 Actual State

Фактическое состояние БВС должно поступать через телеметрию и включать доступные:

- position;
- altitude;
- velocity;
- attitude;
- navigation state;
- battery / energy;
- health;
- connection / communication state;
- mission state.

Архитектура UAV Adapter требует единого представления этих состояний.

### 2.3 Navigation State

`Navigation State` пока является архитектурной сущностью, но его внутренний состав в проекте ещё не зафиксирован полностью.

Рабочая связь:

```text
Telemetry
→ Actual Position / Motion State
→ Navigation State
→ Comparison with Planned Route / WP
→ Deviation
→ Revalidation / Recalculation when required
```

Проект прямо требует постоянно сопоставлять фактическое положение БВС с рассчитанным маршрутом и WP; отклонение должно учитываться при повторной проверке и перерасчёте.

---

## 3. Core Navigation Concepts

| Concept | Current project meaning | Status |
|---|---|---|
| Position | Фактическое положение БВС; используется как исходное состояние последующих расчётов | ACCEPTED BY DESIGN |
| Route | Рассчитанный маршрут миссии | ACCEPTED BY DESIGN |
| WP / Waypoint | Точка маршрута, используемая в планировании, отображении и контроле | ACCEPTED BY DESIGN |
| Planned State | Расчётное состояние миссии/маршрута | ARCHITECTURAL |
| Actual State | Фактическое состояние по телеметрии | ARCHITECTURAL |
| Navigation State | Состояние навигации БВС | NEEDS FORMALIZATION |
| Route Deviation | Отклонение фактического движения от рассчитанного маршрута/WP | ACCEPTED BY DESIGN |
| Ground/Track Speed | Путевая скорость как навигационный элемент | SOURCE-SUPPORTED |
| Drift Angle | Угол сноса как навигационный элемент | SOURCE-SUPPORTED |
| Wind | Среда/условие, влияющее на навигационные расчёты | SOURCE-SUPPORTED |
| Time | Временная составляющая маршрута и индивидуальная временная шкала БВС | ACCEPTED BY DESIGN |
| Safe Return | Возможность безопасного возврата конкретного БВС | ACCEPTED BY DESIGN |
| Map | Представление пространственного состояния и operational layer | ACCEPTED BY DESIGN |

---

## 4. Source Knowledge — классическое самолётовождение

Источник:

`Черный-М.А.-Кораблин-В.И.-Самолетовождение.pdf`

Зафиксированные положения:

1. При полёте над безориентирной местностью требуется точное выдерживание расчётного курса с последующим уточнением по фактическому углу сноса.
2. Необходимо уделять внимание определению путевой скорости и достоверности используемых ориентиров.
3. Для трудноопознаваемого пункта маршрут может быть проложен к характерному ориентиру, от которого рассчитываются курс и время до пункта назначения.
4. Угол сноса может определяться различными способами, включая известный ветер, отметки места на карте, радиопеленги, доплеровский измеритель и другие средства.
5. Путевая скорость может определяться через изменение пройденного расстояния за известный интервал времени.
6. Навигационная аппаратура может одновременно предоставлять угол сноса, путевую скорость и координаты места самолёта.

Это означает, что для BlueSky навигация должна рассматриваться не только как `position`, но как совокупность пространственных, кинематических и временных элементов.

---

## 5. Project Design Knowledge

### UAV State and Telemetry

Зафиксировано:

```text
Telemetry
→ актуализация положения и состояния БВС

Actual Position
↔ Calculated Route / WP

Deviation
→ повторная проверка / перерасчёт при необходимости

State / Telemetry / Resource / Position / Mission Progress change
→ соответствующая актуализация
→ revalidation where safety or mission feasibility is affected

Current state
→ continuous Safe Return assessment
```

Также предусмотрена персональная временная шкала каждого БВС; общий временной статус миссии не должен подменять индивидуальную шкалу.

### Route Planning

В проекте существуют отдельные области:

- Route Generation;
- WP Table;
- 2D/3D Route Visualization;
- Collision Avoidance;
- Dynamic Return Calculation.

Они должны рассматриваться как специализированные потребители общей Navigation Model, а не как независимые определения навигации.

### Map

Operational Layer карты уже включает:

- route;
- UAV;
- waypoint;
- boundaries;
- zones;
- NOTAM;
- weather;
- corrections.

Карта также должна отображать фактическое положение.

Map states:

```text
Planning
→ Validating
→ Ready
→ Flying
→ Paused
→ Emergency
→ Completed
```

---

## 6. Architecture Integration

### 6.1 C++ Core authority

C++ Core остаётся authoritative runtime state owner.

UAV Adapter предоставляет унифицированное представление:

```text
UAV state
Position
Altitude
Velocity
Attitude
Battery
Payload state
Navigation state
Health
Connection state
Mission state
```

### 6.2 C2 / Communication

C2 является отдельным интерфейсом.

Изменение communication state, включая loss/degradation, должно быть доступно Mission Validation и Safety Engine.

Следовательно:

```text
Communication State
→ Navigation / Mission Feasibility
→ Validation
→ Safety
```

а не просто UI-индикатором.

### 6.3 Planned / Simulated / Actual

Архитектура требует различать:

```text
PLANNED
SIMULATED
ACTUAL
```

Поэтому Navigation Knowledge Map должна поддерживать минимум три представления одного навигационного объекта/состояния:

- planned;
- simulated;
- actual.

---

## 7. Safety Integration

Навигационное решение не является самостоятельной authority.

Контрольная цепочка:

```text
NAVIGATION DATA
→ MISSION VALIDATION
→ READINESS
→ SAFETY GATE
→ AUTHORIZATION
→ C++ CORE
→ EXECUTION
```

Safety Engine имеет приоритет над оптимизацией и Mission AI.

Не допускается:

```text
Navigation Algorithm
→ direct execution
```

или:

```text
Mission AI
→ direct UAV action
```

---

## 8. Dynamic Navigation Loop

Текущая совокупность проектных правил приводит к следующему циклу:

```text
PLANNED ROUTE
      ↓
CURRENT UAV STATE
      ↓
NAVIGATION STATE
      ↓
COMPARE WITH PLAN / WP
      ↓
DEVIATION / CHANGE DETECTION
      ↓
VALIDATION
      ↓
 ┌───────────────┐
 │ no material   │
 │ change        │
 └───────┬───────┘
         ↓
      CONTINUE

При material change:

CURRENT STATE
→ REVALIDATION
→ REPLAN / ADAPT if permitted
→ READINESS
→ SAFETY GATE
→ AUTHORIZATION if required
→ EXECUTION
```

Это не новый алгоритм; это интеграционная модель, выведенная из уже зафиксированных проектных требований и архитектурных решений.

---

## 9. Safe Return Integration

Safe Return должен быть связан с:

- текущим положением;
- текущей траекторией;
- доступным energy/resource state;
- communication state;
- environmental conditions;
- актуальным состоянием миссии;
- применимыми safety constraints.

Проект требует постоянной оценки возможности безопасного возврата каждого БВС и актуализации расчёта при изменении этой возможности.

---

## 10. Multi-UAV Integration

Fleet Coordination использует:

```text
MISSION
→ REQUIRED CAPABILITIES
→ AVAILABLE RESOURCES
→ FLEET / UAV ROLES
→ RESOURCE ALLOCATION
→ MISSION PLAN
→ VALIDATION
→ EXECUTION
```

Navigation therefore belongs to both:

- individual UAV state;
- fleet-level mission coordination.

Каждый UAV сохраняет индивидуальное execution/navigation state.

---

## 11. Traceability Model

Для каждого существенного навигационного решения должна быть возможна цепочка:

```text
SOURCE
→ KNOWLEDGE ITEM
→ NAVIGATION CONCEPT
→ INPUT DATA
→ CALCULATION / RULE
→ RESULT
→ VALIDATION
→ SAFETY DECISION
→ AUTHORIZATION
→ EXECUTION
→ ACTUAL RESULT
```

Для AI-assisted navigation additionally:

```text
AI TASK
→ AGENT
→ CONTEXT
→ KNOWLEDGE
→ ANALYSIS
→ PROPOSAL
→ VALIDATION
→ SAFETY DECISION
→ AUTHORIZATION
→ EXECUTION
→ RESULT
```

AI proposal сама по себе не является authoritative.

---

## 12. Contradictions / Gaps

Пока НЕ считаем следующими пунктами противоречиями:

### GAP-NAV-001 — Navigation State composition

Не зафиксирован окончательный формальный состав `Navigation State`.

Нужно определить, какие поля являются:
- authoritative;
- derived;
- estimated;
- stale/invalid;
- confidence-qualified.

**Статус: OPEN.**

### GAP-NAV-002 — Navigation element terminology

Нужно унифицировать в проекте определения:

- heading / course;
- track / ground track;
- ground speed;
- airspeed;
- drift angle;
- bearing;
- course angle;
- wind vector;
- cross-track error;
- along-track error.

**Статус: OPEN.**

Не заменять термины общей инженерной трактовкой без отдельного согласования.

### GAP-NAV-003 — Coordinate/reference systems

Не найдено окончательное единое решение по:

- horizontal reference frame;
- altitude reference;
- vertical datum;
- coordinate precision;
- time reference;
- transformation rules.

**Статус: OPEN.**

### GAP-NAV-004 — Navigation confidence / validity

Нужно формализовать:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

для каждого существенного навигационного источника/элемента.

**Статус: OPEN.**

### GAP-NAV-005 — Source hierarchy

Нужно определить authority/precedence между:

- GNSS;
- inertial;
- air-data;
- terrain/map matching;
- radio/navigation sources;
- visual/perception;
- external data;
- operator input;
- AI inference.

**Статус: OPEN.**

### GAP-NAV-006 — External authoritative source

Перед окончательным закреплением спорных навигационных определений требуется внешний источник согласно установленному проектом процессу:

```text
Общая база
→ специализированная запись источника
→ оригинальный источник
→ внешний источник
→ сравнение
→ противоречия
→ согласованное решение
→ накопление
```

**Статус: OPEN.**

---

## 13. Current Authority Order for Navigation Knowledge

До отдельного согласования используем:

```text
1. Accepted project decisions / safety architecture
2. Explicit project requirements
3. Approved project design
4. Primary source material
5. External authoritative sources
6. Model interpretation / proposal
```

Последний уровень не может молча превращаться в проектное правило.

---

## 14. Immediate Next Step

Следующий объект после этой карты:

# NAVIGATION STATE MODEL

Он должен быть создан только после закрытия GAP-NAV-001/002/003/004/005 настолько, насколько это возможно на основании источников.

Целевая форма:

```text
NavigationState
├── Position
├── Altitude
├── Velocity
├── Heading / Course
├── Track
├── Ground Speed
├── Air Speed
├── Drift
├── Wind
├── Route Reference
├── Active WP
├── Cross Track Error
├── Along Track Position
├── Navigation Source
├── Validity
├── Freshness
├── Confidence
└── Timestamp
```

**Важно:** этот список пока является структурной рабочей гипотезой, а не принятой спецификацией. Его элементы нельзя считать утверждёнными до прохождения процедуры сравнения источников и согласования.

---

## 15. Sources Used for This Map

- `Черный-М.А.-Кораблин-В.И.-Самолетовождение.pdf`
- `UAV/UAV State and Telemetry.md`
- `Route Planning/Route Generation.md`
- `Route Planning/WP Table.md`
- `Route Planning/2D-3D Route Visualization.md`
- `Route Planning/Collision Avoidance.md`
- `Route Planning/Dynamic Return Calculation.md`
- `Mission/Mission Calculation.md`
- `Mission/Mission Execution.md`
- `Mission/Mission Decision Philosophy.md`
- `Mission/Mission Formation.md`
- `Interface/Flight Profile Interaction.md`
- `Interface/Mission Dashboard.md`
- `Требования к карте. .txt`
- `ARCH-DEC-003`
- `ARCH-DEC-007`
- `ARCH-DEC-012`
- `ARCH-DEC-016`
- `ARCH-DEC-033`
- `ARCH-DEC-046`

---

## 16. Status

**NAV-KNOWLEDGE-MAP-001: DRAFT / SOURCE-GROUNDED**

Не является окончательной спецификацией Navigation.

Следующее действие — сравнение терминологии и структуры Navigation с первичным источником и внешним авторитетным источником, после чего GAP/CONFLICT records переводятся в согласованные решения.



