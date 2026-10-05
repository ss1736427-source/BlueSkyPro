---
id: NAVIGATION-ALGORITHM-001
type: navigation_algorithm
status: draft_for_agreement
authority: derived_from_agreed_navigation_rules
---

# Navigation Algorithm

## 1. Status

Базовые навигационные положения и различение Course / Heading / Track / Drift / Airspeed / Groundspeed согласованы как основа модели.

Этот документ описывает **логический алгоритм**, а не программную реализацию.

Современный внешний источник FAA подтверждает wind-triangle relationship: desired course, heading/airspeed, wind и actual ground track/groundspeed являются различными элементами. urlFAA PHAK Chapter 16 — Navigationhttps://www.faa.gov/sites/faa.gov/files/18_phak_ch16.pdf

---

## 2. Algorithm boundary

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

Navigation output is an input to validation and planning components. It is not direct execution authority.

---

## 3. Step 1 — Acquire inputs

Collect available:

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

No value is assumed valid merely because a field exists.

---

## 4. Step 2 — Validate source data

For every critical value determine:

```text
Source
Timestamp
Freshness
Validity
Confidence
```

Classify:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Invalid or unavailable data shall not silently be substituted with an assumed value.

---

## 5. Step 3 — Establish reference frame

Before comparing directional values, determine the applicable reference frame.

```text
TRUE
MAGNETIC
COMPASS
```

Do not compare or combine values from different reference frames without an explicit transformation.

---

## 6. Step 4 — Establish navigation state

Construct:

```text
PLANNED
ACTUAL
DERIVED
QUALITY
```

### Planned

```text
Route
Active WP
Intended Course
Planned Altitude
Planned Speed
Time
Constraints
```

### Actual

```text
Position
Altitude
Heading
Track
Airspeed
Groundspeed
Wind
Navigation Source
```

### Derived

```text
Drift Angle
Wind Correction Angle
Cross Track Error
Along Track Position
Deviation
ETA
Return Feasibility
```

---

## 7. Step 5 — Determine intended path

For the active route segment:

```text
Current Position
+
Active WP
+
Next WP / Route Geometry
→ Intended Course
```

The exact course definition depends on the route segment and remains subject to the project's route-generation rules.

---

## 8. Step 6 — Apply wind relationship

Where wind data is valid:

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

The wind triangle is the conceptual model.

FAA material explicitly distinguishes heading/airspeed, wind, and ground track/groundspeed. urlFAA Chapter 16 Navigationhttps://www.faa.gov/sites/faa.gov/files/18_phak_ch16.pdf

The algorithm must not assume that Course = Heading when wind is present.

---

## 9. Step 7 — Determine actual navigation state

From available actual sources determine:

```text
Actual Position
Actual Heading
Actual Track
Actual Airspeed
Actual Groundspeed
```

If a value is estimated or derived rather than measured, preserve that provenance.

---

## 10. Step 8 — Calculate navigation deviations

Compare:

```text
ACTUAL
↔
PLANNED
```

Potential outputs:

```text
Cross Track Error
Along Track Position
Course / Track Difference
Heading / Desired Direction Difference
Altitude Deviation
Speed Deviation
Time / ETA Deviation
```

The exact sign convention and numerical formulas are not fixed in this document until the corresponding project geometry and coordinate rules are approved.

---

## 11. Step 9 — Evaluate navigation quality

Navigation quality is evaluated separately from navigation geometry.

Example:

```text
VALID + small deviation
→ normal navigation

DEGRADED + acceptable deviation
→ continue with degraded status if permitted

STALE / INVALID / UNAVAILABLE critical data
→ determine operational impact
→ revalidation or other defined response
```

No automatic safety conclusion is made solely from one navigation field.

---

## 12. Step 10 — Evaluate route feasibility

Use the current state to evaluate whether the active route remains feasible.

Inputs may include:

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

Output:

```text
FEASIBLE
DEGRADED
NOT FEASIBLE
UNKNOWN
```

---

## 13. Step 11 — Detect material change

Detect changes in:

- actual position;
- route deviation;
- navigation-source quality;
- wind/environment;
- energy/resource state;
- communication;
- route or WP;
- mission constraints.

Classify the change before deciding what to do.

```text
NO MATERIAL CHANGE
OPERATIONAL
MISSION-IMPACTING
SAFETY-IMPACTING
APPROVAL-REQUIRED
```

This follows the existing project runtime-adaptation model rather than inventing a separate navigation authority.

---

## 14. Step 12 — Revalidation

For material changes:

```text
DETECT
→ CLASSIFY
→ EVALUATE
→ REVALIDATE
```

If a new route or adaptation is required:

```text
GENERATE OPTIONS
→ SIMULATE / ANALYZE
→ VALIDATE
→ SELECT / APPROVE
→ EXECUTE
```

The project already defines this controlled adaptation chain.

---

## 15. Step 13 — Navigation outputs

Navigation subsystem may provide:

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

Outputs must retain provenance and quality information.

---

## 16. Step 14 — Safety boundary

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

Navigation does not bypass:

- Mission Validation;
- Readiness;
- Safety Engine;
- operator approval where required.

---

## 17. Dynamic Return branch

At every relevant state update:

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

If return feasibility materially degrades:

```text
DETECT
→ CLASSIFY
→ REVALIDATE
→ UPDATE RETURN PLAN
```

The return calculation must remain tied to the actual current state of the specific UAV.

---

## 18. Multi-UAV branch

For each UAV:

```text
UAV[i].NavigationState
```

is maintained independently.

Fleet-level coordination consumes individual states:

```text
UAV-1 State
UAV-2 State
...
UAV-N State
        ↓
Fleet Coordination
```

Fleet state must not erase individual navigation state.

---

## 19. Failure / degraded data handling

The algorithm shall distinguish:

```text
Missing
Stale
Invalid
Conflicting
Degraded
```

A critical missing/invalid value triggers impact assessment rather than silent substitution.

If the resulting state affects mission feasibility or safety:

```text
→ REVALIDATION
```

---

## 20. Algorithm invariants

The following are invariants:

1. `PLANNED ≠ ACTUAL`.
2. `SIMULATED ≠ ACTUAL`.
3. `Course ≠ Heading ≠ Track`.
4. `Airspeed ≠ Groundspeed`.
5. Reference frame must be explicit.
6. Derived values retain derived provenance.
7. Invalid/stale data is not silently treated as valid.
8. Navigation State is not execution authority.
9. Safety constraints cannot be overridden by optimization.
10. AI proposal cannot independently authorize material execution.
11. Individual UAV states remain distinct in fleet coordination.
12. Material state changes can require revalidation.

---

## 21. Open mathematical specifications

Not silently fixed here:

- coordinate reference system;
- altitude datum;
- units;
- precision;
- exact WCA formula and applicability;
- exact Drift Angle convention/sign;
- Cross Track Error sign convention;
- source-fusion algorithm;
- navigation-source priority;
- stale thresholds;
- confidence calculation;
- route-segment course definition;
- turn anticipation model.

These belong to the next formal specification layer.

---

## 22. Verification model

The algorithm must eventually be verified against at least:

```text
Nominal navigation
Crosswind
Headwind / tailwind
Changing wind
Route deviation
WP transition
Stale navigation data
Invalid navigation data
Missing navigation data
Conflicting navigation sources
Communication degradation
Energy degradation
Dynamic Return degradation
Multi-UAV interaction
Material runtime adaptation
Safety-gate rejection
```

Verification shall preserve:

```text
INPUT
→ NAVIGATION STATE
→ CALCULATION
→ RESULT
→ VALIDATION
→ DECISION
```

---

## 23. Status

**DRAFT_FOR_AGREEMENT**

This is the first logical navigation algorithm derived from the agreed Navigation Knowledge / State / Rules chain.

It is not yet implementation code and does not define unresolved mathematical conventions.

Next formal layer:

```text
NAVIGATION ALGORITHM
→ NAVIGATION VERIFICATION MODEL
→ SOFTWARE DESIGN
→ C++ IMPLEMENTATION
```



