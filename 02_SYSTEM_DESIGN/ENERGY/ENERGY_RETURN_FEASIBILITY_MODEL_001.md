---
id: ENERGY-RETURN-FEASIBILITY-MODEL-001
type: energy_return_feasibility_model
status: draft_for_agreement
parent: ENERGY-MODEL-BASELINE-001
---

# BlueSky PRO — Return Energy / Feasibility Model 001

## 1. Purpose

Определить контролируемую структуру расчёта требуемой энергии для возвратного маршрута и критерий оценки Return Feasibility. Документ является инженерным слоем между Battery/Degradation Model, Energy Consumption Model и `NAV-TV-009`.

Документ не устанавливает неподтверждённые численные коэффициенты, reserve percentage или acceptance thresholds.

## 2. Traceability

```text
ENERGY-MODEL-BASELINE-001
        ↓
ENERGY-MODEL-REQUIREMENTS-001
        ↓
ENERGY-BATTERY-DEGRADATION-MODEL-001
        ↓
ENERGY-CONSUMPTION-MODEL-001
        ↓
ENERGY-RETURN-FEASIBILITY-MODEL-001
        ↓
NAV-TV-009
        ↓
NAV-V14 / EVD-025
```

## 3. Model boundary

Модель должна определить, может ли текущего энергетического ресурса быть достаточно для выполнения контролируемого возвратного маршрута с учётом применимой модели потребления, состояния батареи, маршрута и заданной политики запаса.

Модель не является Execution Authority и не заменяет отдельные safety requirements или operational decision logic.

## 4. Required inputs

Минимальный набор входов:

| Input | Role | Status |
|---|---|---|
| Current usable energy | available resource | CONTROLLED INPUT REQUIRED |
| Battery/degradation state | capacity correction | CONTROLLED INPUT REQUIRED |
| Return route geometry | distance/segments | CONTROLLED INPUT REQUIRED |
| Altitude / flight regime | consumption selection | CONTROLLED INPUT REQUIRED |
| Airspeed / groundspeed | segment timing and consumption | CONTROLLED INPUT REQUIRED |
| Wind vector | groundspeed / energy impact | CONTROLLED INPUT REQUIRED |
| UAV configuration | vehicle-specific model selection | CONTROLLED INPUT REQUIRED |
| Payload/equipment | energy-significant load | CONTROLLED INPUT REQUIRED |
| Reserve / margin policy | feasibility boundary | TBD / APPROVAL REQUIRED |
| Model uncertainty/confidence | conservative treatment | TBD / APPROVAL REQUIRED |

## 5. Return route decomposition

The return route shall be represented as a sequence of controllable segments or equivalent route elements:

```text
Current position
      ↓
Return route
      ↓
Segment 1 → Segment 2 → ... → Segment N
      ↓
Per-segment distance / direction / altitude / regime
      ↓
Per-segment energy requirement
```

The model shall use the actual controlled return route available to the navigation function rather than an unspecified nominal distance.

## 6. Wind treatment

Wind shall be an explicit input to return-energy calculation where the approved consumption model requires it.

For each applicable route segment, the model shall distinguish at minimum:

```text
air-relative motion
wind vector
resulting groundspeed
segment duration
energy consumption
```

The implementation shall not silently substitute a fixed wind assumption when valid wind data are required by the approved model.

## 7. Segment energy calculation

Conceptual structure:

```text
Segment Energy
    = f(distance,
        flight regime,
        airspeed,
        groundspeed,
        wind,
        altitude,
        UAV configuration,
        payload/equipment,
        battery/energy state)
```

The definitive equation, coefficients, calibration basis and applicability limits remain subject to engineering baseline approval.

## 8. Total return energy requirement

Conceptually:

```text
Predicted Return Energy
    = Σ Segment Energy
      + applicable reserve / margin treatment
      + applicable uncertainty treatment
```

The reserve and uncertainty terms shall not be double-counted. Their definitions and interaction must be controlled in the approved model.

## 9. Feasibility criterion

The core decision shall compare the predicted return requirement with the current usable energy:

```text
RETURN FEASIBLE
    when available usable energy
    is sufficient for the controlled return requirement
    including the approved margin/reserve treatment.
```

```text
RETURN NOT FEASIBLE
    when available usable energy
    is insufficient for the controlled return requirement.
```

Exact inequality, threshold, reserve policy and uncertainty treatment: `TBD`.

## 10. Recalculation triggers

Return feasibility shall be recalculated when a material change occurs in an input affecting the result, including as applicable:

- current energy state;
- battery degradation/condition;
- return route;
- wind;
- groundspeed/airspeed;
- altitude or flight regime;
- UAV configuration;
- payload/equipment;
- model validity or data quality.

This supports the existing Dynamic Return requirement that material degradation of energy, wind, groundspeed, communication, navigation or other relevant conditions triggers reassessment. `SAF-REQ-009` and `SAF-REQ-010` remain the applicable higher-level safety requirements.

## 11. Data quality and validity

The calculation shall expose an explicit result validity state. At minimum:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

An invalid or unavailable mandatory input shall not be silently treated as nominal.

Exact freshness limits and transition rules remain `TBD`.

## 12. Model version and configuration control

Each feasibility result shall be traceable to:

```text
Energy Model version
Battery Model version
UAV configuration ID
Battery ID / revision
Payload/equipment configuration
Return route revision
Input data timestamp
Applicable wind/navigation data revision
Calculation result/state
```

A change affecting model applicability shall trigger the defined recalculation or revalidation process.

## 13. Uncertainty and conservative treatment

The model shall provide a controlled method for handling uncertainty in energy estimation and return-feasibility assessment.

Until the uncertainty method is approved, no universal fixed percentage shall be assumed.

Required controlled items:

```text
uncertainty source
uncertainty representation
combination rule
conservative treatment
acceptance basis
```

## 14. Verification mapping

Primary verification consumer:

```text
NAV-TV-009 — Return feasibility
EVD-025 — NAV-V14 Energy degradation
```

The verification shall eventually demonstrate at minimum:

1. feasible return with sufficient energy margin;
2. not-feasible return when the predicted requirement exceeds available energy;
3. response to degraded battery condition;
4. response to material wind change;
5. response to return-route change;
6. invalid/stale mandatory input handling;
7. reproducibility of the calculated result;
8. traceability of the result to model/configuration versions.

Actual test vectors, numerical acceptance criteria and approved datasets remain controlled prerequisites.

## 15. Open engineering items

```text
TBD-RETURN-001 approved segment energy equation
TBD-RETURN-002 propulsion/consumption parameter set
TBD-RETURN-003 wind-to-groundspeed treatment
TBD-RETURN-004 reserve/margin policy
TBD-RETURN-005 uncertainty combination rule
TBD-RETURN-006 feasibility inequality and thresholds
TBD-RETURN-007 minimum data-quality/freshness limits
TBD-RETURN-008 approved validation dataset
TBD-RETURN-009 acceptance criteria for NAV-TV-009
```

## 16. Readiness status

```text
Model structure: DEFINED
Numerical basis: NOT ESTABLISHED
Acceptance basis: NOT ESTABLISHED
NAV-TV-009 readiness: BLOCKED
EVD-025 readiness: PLANNED
```

**Status: DRAFT_FOR_AGREEMENT — ENGINEERING AND ACCEPTANCE BASIS REQUIRED**
