---
id: ENERGY-CONSUMPTION-MODEL-001
type: energy_consumption_model
status: draft_for_agreement
parent: ENERGY-MODEL-BASELINE-001
---

# BlueSky PRO — Energy Consumption Model 001

## 1. Purpose

Определить контролируемую структуру модели расхода энергии, необходимой для прогноза остаточной энергии и `NAV-TV-009 Return feasibility`.

Модель является инженерным расчётным слоем и не является самостоятельной Execution Authority.

## 2. Traceability

```text
ENERGY-MODEL-BASELINE-001
        ↓
ENERGY-MODEL-REQUIREMENTS-001
        ↓
ENERGY-CONSUMPTION-MODEL-001
        ↓
Predicted Return Energy
        ↓
NAV-TV-009
        ↓
NAV-V14 / EVD-025
```

`SAF-REQ-009` requires return feasibility evaluation from the current UAV state and applicable constraints, while `SAF-REQ-010` requires reassessment after material degradation of energy, wind, groundspeed, communication, navigation or other relevant conditions. fileciteturn346file0L2-L2

## 3. Model boundary

The model shall provide a controlled estimate of energy demand over an applicable time/route interval.

Conceptual chain:

```text
UAV / battery configuration
        +
Flight condition
        +
Environment
        +
Payload / equipment
        +
Mission phase
        ↓
Energy consumption estimate
```

The model shall distinguish measured inputs, estimated inputs, derived quantities and assumptions.

## 4. Required input classes

| Input | Role | Current state |
|---|---|---|
| Airspeed / flight condition | propulsion demand | controlled Navigation input, numerical basis TBD |
| Groundspeed | elapsed return time / route traversal | controlled Navigation input, numerical basis TBD |
| Wind | route/flight-condition effect | controlled Navigation input, numerical basis TBD |
| Altitude / operating regime | operating-condition correction | source/basis TBD |
| UAV configuration | model applicability | controlled configuration ID required |
| Propulsion configuration | propulsion demand | engineering basis TBD |
| Payload / equipment | auxiliary and propulsion demand | configuration basis TBD |
| Mission phase / mode | phase-dependent consumption | mission-state basis TBD |
| Battery condition | available-energy boundary / derating | Battery Model |

## 5. Consumption components

The final approved contributor set shall be derived from the applicable UAV architecture and engineering data. Candidate components are:

```text
Propulsion energy
Avionics energy
Payload/equipment energy
Flight-control / navigation support energy
Other system loads
```

No universal fixed consumption value is assumed.

## 6. Time-domain representation

The model shall be capable of representing consumption over the relevant mission interval.

Conceptually:

```text
E_required = integral(P(t) dt)
```

where `P(t)` represents the approved total energy-demand model for the applicable configuration and operating condition.

This is a structural formulation only. The project shall define the approved power/consumption equations, sampling/aggregation method and numerical parameters before verification.

## 7. Route and environmental dependency

For return feasibility, predicted consumption shall remain linked to the actual applicable return route and environmental/navigation state.

Relevant factors may include:

- return-route distance;
- route profile/altitude;
- groundspeed;
- wind vector;
- flight condition;
- configuration;
- payload;
- mission phase.

The final dependency set and calculation method are `TBD` pending engineering baseline.

## 8. Degraded-condition handling

When a relevant input is `DEGRADED`, `STALE`, `INVALID` or `UNAVAILABLE`, the model shall not silently treat it as nominal.

Possible controlled outcomes include:

```text
recalculate with approved degraded model
use approved fallback
mark result INDETERMINATE
block feasibility decision pending valid input
```

The applicable policy for each input class shall be baselined before verification.

## 9. Configuration applicability

Every consumption-model instance shall identify the configuration to which the model applies.

Minimum linkage:

```text
UAV configuration ID
Propulsion configuration/revision
Battery ID/model
Payload/equipment configuration
Energy Model version
```

A material configuration change shall trigger applicable revalidation or recalculation.

## 10. Uncertainty

The model shall expose uncertainty/confidence where the approved engineering basis supports it.

Uncertainty shall not be converted implicitly into a fixed reserve percentage. Any guard band or conservative treatment shall be explicitly defined in the approved Energy/Safety basis.

## 11. Outputs

The model shall provide, as applicable:

```text
Consumption estimate
Consumption rate / power estimate where defined
Calculation interval
Input quality state
Configuration reference
Model version
Calculation timestamp
Uncertainty / confidence where defined
```

These outputs feed the return-energy calculation; they do not directly authorize execution.

## 12. Verification requirements

Verification shall eventually cover at minimum:

- nominal operating condition;
- crosswind/headwind/tailwind where applicable;
- changed wind;
- changed groundspeed;
- changed configuration/payload where model-significant;
- degraded/stale/invalid input;
- reproducibility for identical controlled inputs;
- boundary conditions and model applicability limits.

Exact tolerances and acceptance criteria remain `TBD` until approved engineering/test basis exists.

## 13. Open engineering items

```text
TBD-CONS-001 propulsion model form
TBD-CONS-002 propulsion parameter source
TBD-CONS-003 auxiliary load model
TBD-CONS-004 payload/equipment load mapping
TBD-CONS-005 flight-condition dependency
TBD-CONS-006 wind dependency
TBD-CONS-007 altitude/regime dependency
TBD-CONS-008 time-domain/aggregation method
TBD-CONS-009 uncertainty treatment
TBD-CONS-010 model applicability limits
TBD-CONS-011 validation dataset
TBD-CONS-012 acceptance criteria
```

## 14. Readiness impact on NAV-TV-009

`NAV-TV-009` remains blocked until this consumption model is baselined sufficiently to calculate a controlled return-energy requirement together with the Battery/Degradation Model, return route and reserve/margin policy.

## 15. Status

**DRAFT_FOR_AGREEMENT — ENGINEERING CONSUMPTION DATA AND MODEL FORM REQUIRED**
