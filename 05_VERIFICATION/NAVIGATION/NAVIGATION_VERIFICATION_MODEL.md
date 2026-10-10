---
id: NAVIGATION-VERIFICATION-MODEL-001
type: navigation_verification_model
status: draft_for_agreement
authority: derived_from_navigation_algorithm
---

# Navigation Verification Model

## 1. Назначение

Определить, как проверять корректность Navigation Algorithm от входных данных до результата и safety decision.

Модель не изменяет алгоритм. Она проверяет его.

Базовая навигационная терминология сверяется с FAA PHAK Chapter 16, где отдельно определены Course, Heading, Track, Drift Angle, WCA, Airspeed и Groundspeed. urlFAA Chapter 16 — Navigationhttps://www.faa.gov/regulationspolicies/handbooksmanuals/aviation/phak/chapter-16-navigation

## 2. Verification chain

```text
TEST INPUT
    ↓
SOURCE / QUALITY
    ↓
NAVIGATION STATE
    ↓
CALCULATION
    ↓
EXPECTED RESULT
    ↓
ACTUAL RESULT
    ↓
COMPARISON
    ↓
VALIDATION
    ↓
SAFETY DECISION
    ↓
TRACE / EVIDENCE
```

Каждый verification case должен позволять восстановить эту цепочку.

## 3. Verification levels

### V1 — Data integrity

Проверяем:

- наличие обязательных полей;
- тип;
- единицы;
- reference frame;
- timestamp;
- source;
- freshness;
- validity;
- confidence.

### V2 — State construction

Проверяем правильность разделения:

```text
PLANNED
ACTUAL
SIMULATED
DERIVED
QUALITY
```

### V3 — Navigation semantics

Проверяем:

```text
Course
Heading
Track
Bearing
Drift Angle
WCA
Airspeed
Groundspeed
Wind
```

Нельзя принимать одинаковые значения как доказательство правильности: необходимо проверять их семантическую роль.

### V4 — Mathematical calculation

Проверяются конкретные формулы и численные результаты.

До утверждения coordinate/reference conventions математические тесты не должны фиксировать неподтверждённые соглашения.

### V5 — State transition

Проверяем реакцию на:

```text
NO CHANGE
MATERIAL CHANGE
DEGRADED
STALE
INVALID
UNAVAILABLE
CONFLICTING
```

### V6 — System integration

Проверяем взаимодействие:

```text
Navigation
→ Route Planning
→ Dynamic Return
→ Collision Avoidance
→ Mission
→ Multi-UAV
→ Safety
```

### V7 — Safety

Проверяем, что navigation result не обходит:

```text
Validation
→ Readiness
→ Safety Gate
→ Authorization
→ C++ Core
```

---

## 4. Mandatory verification scenarios

| ID | Scenario | Expected verification |
|---|---|---|
| NAV-V01 | No wind | Course/Heading/Track relationship consistent |
| NAV-V02 | Crosswind | WCA/heading/track relationship consistent |
| NAV-V03 | Headwind | Groundspeed reflects wind effect |
| NAV-V04 | Tailwind | Groundspeed reflects wind effect |
| NAV-V05 | Changing wind | State and derived values update |
| NAV-V06 | Route deviation | Deviation detected |
| NAV-V07 | WP transition | Active WP changes consistently |
| NAV-V08 | Stale navigation | Quality changes; impact assessed |
| NAV-V09 | Invalid navigation | Invalid value not silently treated as valid |
| NAV-V10 | Missing critical input | Defined degraded/failure path |
| NAV-V11 | Conflicting sources | Conflict detected and handled |
| NAV-V12 | Degraded source | Quality preserved and operational impact assessed |
| NAV-V13 | Communication degradation | Mission/safety impact evaluated |
| NAV-V14 | Energy degradation | Feasibility recalculated |
| NAV-V15 | Dynamic Return degradation | Return feasibility changes correctly |
| NAV-V16 | Material runtime change | Revalidation triggered |
| NAV-V17 | Revalidation required | Adaptation chain follows project authority |
| NAV-V18 | Safety rejection | Unsafe proposal cannot execute |
| NAV-V19 | Multi-UAV | Individual states remain separate |
| NAV-V20 | Planned vs Simulated vs Actual | States are not conflated |

---

## 5. Wind-triangle verification

The external aviation reference establishes the conceptual relationship:

```text
Desired Course
      ↓
Wind + Airspeed
      ↓
WCA / Heading
      ↓
Track + Groundspeed
```

FAA describes WCA as the correction applied to course to establish heading so that track coincides with course, and distinguishes airspeed from groundspeed. urlFAA PHAK Chapter 16 PDFhttps://www.faa.gov/sites/faa.gov/files/18_phak_ch16.pdf

Therefore the test suite must include:

- zero wind;
- pure crosswind;
- headwind;
- tailwind;
- oblique wind;
- wind direction reversal;
- wind magnitude change;
- wind unavailable;
- wind stale.

---

## 6. Reference-frame verification

Tests must explicitly identify:

```text
TRUE
MAGNETIC
COMPASS
```

A test passes only if conversions are explicit and no true/magnetic/compass value is silently mixed.

---

## 7. Quality-state verification

For each critical navigation input:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

must be testable.

Example:

```text
VALID
→ normal calculation

STALE
→ stale state retained
→ impact assessment

INVALID
→ value rejected for unsafe use

UNAVAILABLE
→ defined degraded path
```

Exact thresholds remain an open specification item.

---

## 8. Deviation verification

The verification suite must test:

```text
Cross Track Error
Along Track Position
Course / Track difference
Heading / Intended Direction difference
Altitude deviation
Speed deviation
Time / ETA deviation
```

The sign conventions and coordinate formulas are not fixed here until the project coordinate model is approved.

---

## 9. Route / WP verification

A test case must identify:

```text
Mission ID
Route ID
Route Version
Active WP
WP Version
Navigation State Timestamp
```

The test must prove that Actual State is compared against the intended version of the route.

WP transition must not create a mismatch between:

```text
Route
Map
WP Table
Navigation State
```

---

## 10. Dynamic Return verification

At minimum:

```text
Normal return
Reduced energy
Changed wind
Reduced groundspeed
Communication degradation
Route obstacle / constraint
Navigation degradation
Combined degradation
```

Expected result is not merely a distance.

The verification must confirm that return feasibility is derived from current state and applicable constraints.

---

## 11. Multi-UAV verification

For N UAVs:

```text
UAV[1].NavigationState
UAV[2].NavigationState
...
UAV[N].NavigationState
```

must remain independently traceable.

Tests must include:

- one UAV deviates;
- one UAV loses navigation quality;
- one UAV changes route;
- two UAVs interact;
- fleet-level revalidation;
- one UAV failure without corrupting other states.

---

## 12. Runtime adaptation verification

For material change:

```text
DETECT
→ CLASSIFY
→ EVALUATE
→ GENERATE OPTIONS
→ SIMULATE / ANALYZE
→ VALIDATE
→ SELECT / APPROVE
→ EXECUTE
```

The test must verify both:

1. correct detection/classification;
2. correct authority path.

A technically correct navigation result still fails verification if it bypasses required approval or safety controls.

---

## 13. Safety verification

Negative tests are mandatory.

Examples:

```text
INVALID position
→ must not silently become VALID

STALE critical state
→ must not silently continue as fresh

Unsafe route
→ must not execute

Safety rejection
→ must block execution

AI proposal without required approval
→ must not execute

Conflicting navigation sources
→ must not silently select an unsafe result
```

---

## 14. Numerical verification

For every approved mathematical formula:

```text
Input vector
Expected numerical result
Tolerance
Actual numerical result
Error
Pass / Fail
```

Tolerance must be defined per parameter.

No universal tolerance is assumed.

---

## 15. Property / invariant tests

The following invariants should be machine-testable:

```text
PLANNED ≠ ACTUAL
SIMULATED ≠ ACTUAL

Course ≠ Heading ≠ Track
Airspeed ≠ Groundspeed

Invalid ≠ Valid
Stale ≠ Fresh

Navigation State ≠ Execution Authority

UAV[i] state ≠ UAV[j] state
unless explicitly correlated by fleet logic
```

Additional mathematical properties will be added after coordinate and formula specifications are approved.

---

## 16. Regression verification

Every accepted change to:

- Navigation State;
- Navigation Rules;
- navigation formulas;
- route generation;
- WP logic;
- Dynamic Return;
- Collision Avoidance;
- telemetry interpretation;

must trigger the relevant regression suite.

Knowledge changes must be traceable to affected tests.

---

## 17. Evidence record

Each completed verification case should record:

```text
Test ID
Date
Software / model version
Input dataset
Source data version
Navigation State snapshot
Expected result
Actual result
Tolerance
Pass / Fail
Safety outcome
Evidence location
Reviewer / approval
```

---

## 18. Traceability

Required chain:

```text
SOURCE
→ KNOWLEDGE
→ RULE
→ ALGORITHM
→ TEST
→ RESULT
→ DECISION
→ EVIDENCE
```

If a test fails because a rule is ambiguous, the failure must lead to clarification of the rule rather than an undocumented test workaround.

---

## 19. Open verification items

Not fixed yet:

- coordinate reference system;
- altitude datum;
- exact units;
- numerical precision;
- WCA formula;
- Drift Angle sign convention;
- Cross Track Error formula/sign;
- source-fusion algorithm;
- source priority;
- stale thresholds;
- confidence calculation;
- turn anticipation;
- route-segment transition mathematics.

These become explicit verification prerequisites before numerical implementation tests are considered final.

## 20. Status

**DRAFT_FOR_AGREEMENT**

This document defines the verification framework for the current Navigation Algorithm.

Next step:

```text
Verification Model
→ formal test cases
→ mathematical specifications
→ Software Design
→ C++ implementation
```
