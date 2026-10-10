---
id: NAVIGATION-STATE-MODEL-001
type: navigation_state_model
status: draft_for_agreement
authority: derived_from_knowledge_review
---

# Navigation State Model

## 1. Purpose

Определить структуру навигационного состояния БВС на основе накопленных знаний и проведённого сравнения источников.

Это **не алгоритм исполнения** и не даёт навигационному модулю права непосредственно управлять БВС.

## 2. Model layers

Navigation State разделяется на четыре логических слоя:

```text
PLANNED
ACTUAL
DERIVED
QUALITY
```

### PLANNED

Описывает намерение системы:

- route;
- active waypoint;
- intended course;
- planned altitude;
- planned speed;
- time parameters;
- constraints.

### ACTUAL

Описывает фактическое состояние:

- position;
- altitude;
- heading;
- track;
- airspeed;
- groundspeed;
- wind, если доступен/оценён;
- navigation source state.

### DERIVED

Рассчитывается из исходных данных:

- drift angle;
- cross-track error;
- along-track position;
- deviation;
- ETA;
- return feasibility.

Производные значения не должны маскироваться под измеренные значения.

### QUALITY

Для существенных навигационных значений:

- source;
- timestamp;
- freshness;
- validity;
- confidence.

Предлагаемые состояния:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

## 3. Direction semantics

В модели запрещается использовать одно поле `course` для нескольких смыслов.

### Course

Намеренное направление/путь относительно земли, связанный с планируемой траекторией.

### Heading

Направление продольной оси/носа БВС.

### Track

Фактическое направление движения относительно земли.

### Bearing

Направление на объект/точку относительно выбранной системы отсчёта.

### Drift Angle

Производная величина, характеризующая угловое различие между heading и track.

## 4. Speed semantics

### Airspeed

Скорость движения относительно воздушной массы.

### Groundspeed

Скорость перемещения относительно поверхности Земли.

Они не должны храниться как одно универсальное `speed`.

## 5. Reference frame

Каждое направление должно явно указывать reference frame.

Минимальный набор:

```text
TRUE
MAGNETIC
COMPASS
```

`COMPASS` применяется только там, где действительно используется компасная система.

Не допускается молча смешивать истинные и магнитные направления.

## 6. NavigationState structure

Рабочая структура:

```text
NavigationState
├── planned
│   ├── routeId
│   ├── activeWaypointId
│   ├── course
│   ├── altitude
│   ├── speed
│   └── time
│
├── actual
│   ├── position
│   ├── altitude
│   ├── heading
│   ├── track
│   ├── airspeed
│   ├── groundspeed
│   ├── wind
│   └── navigationSource
│
├── derived
│   ├── driftAngle
│   ├── crossTrackError
│   ├── alongTrackPosition
│   ├── deviation
│   ├── eta
│   └── returnFeasibility
│
└── quality
    ├── source
    ├── timestamp
    ├── freshness
    ├── validity
    └── confidence
```

## 7. Planned / Actual / Simulated

Проект уже различает Planned, Simulated и Actual.

Поэтому модель должна допускать:

```text
PLANNED Navigation State
SIMULATED Navigation State
ACTUAL Navigation State
```

Но эти состояния не смешиваются.

Simulation может производить ожидаемое состояние, но оно не становится Actual до получения/подтверждения фактических данных.

## 8. Data provenance

Каждое критическое значение должно позволять определить:

```text
WHERE DID THIS VALUE COME FROM?
WHEN WAS IT VALID?
HOW FRESH IS IT?
HOW RELIABLE IS IT?
```

Минимально:

```text
source
timestamp
freshness
validity
confidence
```

## 9. Navigation State consumers

Navigation State является входом для:

```text
Route Planning
Dynamic Return
Collision Avoidance
Mission Calculation
Mission Execution
Multi-UAV Coordination
Map / Dashboard
Safety Validation
```

Но:

```text
Navigation State
≠
Execution Authority
```

## 10. Safety boundary

```text
NAVIGATION STATE
        ↓
DERIVED NAVIGATION DATA
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

Навигационный расчёт сам по себе не может обойти Safety Gate.

## 11. Open points

Следующие вопросы остаются открытыми и не должны решаться молча:

1. Точный тип и reference системы координат для Position.
2. Vertical reference / altitude datum.
3. Единицы измерения каждого поля.
4. Допустимые диапазоны и precision.
5. Правила определения stale/degraded.
6. Формула и знак CrossTrackError.
7. Правило выбора navigation source при нескольких источниках.
8. Правило fusion нескольких источников.
9. Точный смысл `Course` для каждого типа маршрута.
10. Правила преобразования true/magnetic/compass.
11. Правила расчёта drift при неполных данных.
12. Связь Navigation State с конкретными версиями Route/WP.

## 12. Status

**DRAFT_FOR_AGREEMENT**

Эта модель выведена из текущей Knowledge Review. До закрытия открытых вопросов она не является окончательной технической спецификацией.

Следующий этап:

```text
Navigation State Model
→ Navigation Rules
→ Algorithm
```
