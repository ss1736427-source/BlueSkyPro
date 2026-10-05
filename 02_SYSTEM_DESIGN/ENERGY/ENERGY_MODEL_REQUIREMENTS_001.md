---
id: ENERGY-MODEL-REQUIREMENTS-001
type: energy_model_requirements
status: draft_for_agreement
parent: ENERGY-MODEL-BASELINE-001
---

# BlueSky PRO — Energy Model Requirements 001

## 1. Purpose

Определить контролируемые требования к Energy Model, необходимому для `NAV-TV-009` (Return Feasibility) и связанного `NAV-V14`.

Этот документ не устанавливает численные значения, которых нет в утверждённой инженерной или сертификационной базе.

## 2. Model boundary

Energy Model должен обеспечивать расчёт/оценку:

```text
Initial / Available Energy
        ↓
Energy Consumption Model
        ↓
Remaining Energy
        ↓
Return Energy Requirement
        ↓
Energy Margin
        ↓
Return Feasibility
```

Модель является расчётной основой для оценки состояния и feasibility. Она не является самостоятельным Execution Authority и не должна обходить цепочку Validation → Readiness → Safety Gate → Authorization.

## 3. Controlled input classes

| Input class | Required semantics | Status |
|---|---|---|
| Battery / energy source | идентификатор, состояние, доступная энергия/заряд, применимые ограничения | TBD |
| Battery degradation | degradation state / applicable correction | TBD |
| UAV configuration | платформа, propulsion/configuration, установленная нагрузка | TBD |
| Mission state | текущая фаза/режим и релевантные ограничения | TBD |
| Navigation state | actual position, altitude, groundspeed/airspeed, wind, quality/freshness | controlled by Navigation model |
| Return route | определённый route/version and applicable constraints | TBD |
| Environmental conditions | wind and other applicable conditions | TBD |
| Energy reserve policy | required reserve / margin basis | TBD |

## 4. Required model properties

Energy Model shall define, where applicable:

- units and reference conditions;
- input validity and freshness requirements;
- source and provenance of measured/estimated values;
- treatment of degraded or unavailable inputs;
- relationship between energy state and predicted consumption;
- relationship between route/environment and predicted energy requirement;
- reserve/margin calculation;
- applicability limits and known failure modes;
- version/configuration identification;
- verification method and acceptance basis.

## 5. Battery and degradation

The model must distinguish at least:

```text
Nominal capacity
Available capacity
Current energy state
Degraded capacity / performance
Estimated usable energy
```

Exact degradation coefficient(s), capacity limits, ageing model and threshold values remain `TBD` until supported by controlled engineering data, manufacturer data or approved test basis.

## 6. Consumption model

The model shall provide a controlled representation of energy consumption over the applicable mission interval.

Potential contributors may include:

```text
Propulsion
Avionics / payload
Flight condition
Airspeed / groundspeed
Wind
Altitude / operating regime
Installed configuration
Mission phase
```

The definitive contributor set is `TBD` and shall be baselined from the approved system configuration and engineering model.

## 7. Return-energy calculation

Return feasibility shall use the current applicable UAV state and the defined return route.

Conceptual form:

```text
E_return_required = f(
    remaining_distance,
    route profile,
    flight condition,
    wind,
    configuration,
    payload,
    energy consumption model,
    applicable constraints
)
```

No numerical formulation beyond the approved model is asserted by this document.

## 8. Energy margin and decision boundary

The model shall provide a controlled comparison:

```text
Available usable energy
        vs.
Predicted return energy requirement + applicable reserve/margin
```

The exact reserve/margin policy is `TBD` until established by the controlled safety/engineering basis.

The output shall be capable of representing at least:

```text
FEASIBLE
NOT FEASIBLE
INDETERMINATE / INPUT INVALID
```

where the final decision semantics are subject to Safety/Return baseline approval.

## 9. Degraded-input behaviour

The model shall not silently treat invalid, stale, unavailable or materially degraded energy/navigation inputs as nominal.

Required handling classes:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Exact transition thresholds remain `TBD`.

## 10. Runtime reassessment

Material change in relevant conditions shall permit/requiсre reassessment according to the approved Dynamic Return logic.

Potential triggers include:

- material energy degradation;
- significant wind change;
- groundspeed degradation;
- navigation degradation;
- communication degradation where it affects return logic;
- route/constraint change;
- configuration/load change.

The authoritative trigger set remains `TBD` pending Dynamic Return baseline.

## 11. Outputs

Energy Model outputs shall include, as applicable:

```text
Energy State
Estimated Remaining Energy
Predicted Return Energy Requirement
Energy Margin
Input Quality / Validity
Model Version
Calculation Timestamp
Confidence / Uncertainty indicator where defined
Return Feasibility state
```

## 12. Verification linkage

Primary verification linkage:

```text
ENERGY-MODEL-REQUIREMENTS-001
        ↓
ENERGY-MODEL-BASELINE-001
        ↓
NAV-TV-009
        ↓
NAV-V14
        ↓
EVD-025
```

Required future verification should cover nominal energy state, degraded energy, changed wind, reduced groundspeed, invalid/stale inputs and boundary cases for return feasibility.

## 13. Prerequisites for NAV-TV-009 readiness

Before `NAV-TV-009` may be promoted from `REQUIRES ENERGY MODEL` toward `READY`, the project must control at minimum:

```text
[ ] Energy Model baseline
[ ] Input definitions and units
[ ] Battery/degradation basis
[ ] Consumption model
[ ] Return-route energy calculation
[ ] Reserve/margin policy
[ ] Input-quality handling
[ ] Applicability limits
[ ] Verification acceptance criteria
[ ] Configuration linkage
```

## 14. Status

**DRAFT_FOR_AGREEMENT — NUMERICAL PARAMETERS AND ACCEPTANCE THRESHOLDS NOT YET BASELINED**
