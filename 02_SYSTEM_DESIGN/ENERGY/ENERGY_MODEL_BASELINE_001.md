---
id: ENERGY-MODEL-BASELINE-001
type: energy_model_baseline
status: DRAFT_FOR_AGREEMENT
---

# BlueSky PRO — Energy Model Baseline 001

## 1. Purpose

Определить контролируемую структуру Energy Model, необходимую для `NAV-TV-009 Return feasibility` и связанного `NAV-V14 Energy degradation`.

This document defines the model structure and interfaces only. It does not establish numerical certification thresholds or claim verification execution.

## 2. Traceability

```text
CRB-NAV-001 / Navigation State
        ↓
NAV-V14 / Energy degradation
        ↓
EVD-025 / Simulation / Test
        ↓
NAV-TV-009 / Return feasibility
        ↓
ENERGY-MODEL-BASELINE-001
```

`NAV-TV-009` defines the expected relation: return is feasible only when the predicted return-energy requirement is within the available energy margin. fileciteturn341file0L2-L2

## 3. Model boundary

The Energy Model shall provide, at minimum, a controlled basis for:

1. available energy;
2. energy already consumed;
3. predicted energy required for the remaining/return route;
4. degradation or derating effects that are explicitly included in the approved model;
5. energy margin used by the return-feasibility decision.

The model shall distinguish measured/known inputs from derived quantities and assumptions.

## 4. Input classes

| Input class | Required information | Baseline value |
|---|---|---|
| Battery | battery identity, nominal/available capacity, current state | TBD |
| Battery condition | degradation / health factor where applicable | TBD |
| UAV configuration | aircraft configuration affecting energy consumption | TBD |
| Payload / equipment | installed payload/equipment affecting consumption | TBD |
| Flight condition | airspeed / operating condition relevant to the model | TBD |
| Wind | wind vector and validity state | TBD |
| Route | remaining and return route geometry/distance | TBD |
| Mission state | current navigation/energy state and elapsed flight | TBD |
| Safety reserve | approved reserve/margin basis | TBD |

No numerical values are introduced by this baseline.

## 5. Derived quantities

The model shall support derivation of, at minimum:

- available energy for decision-making;
- predicted energy required for the return route;
- energy margin;
- return-feasibility state.

Where a calculation depends on wind, route, configuration or battery condition, those dependencies shall remain explicit in the model.

## 6. Return-feasibility relation

The controlled decision shall implement the relation stated by `NAV-TV-009`:

```text
RETURN_FEASIBLE ⇔ predicted_return_energy_requirement
                  is within available_energy_margin
```

The exact mathematical definition of `available_energy_margin`, including reserve policy and uncertainty treatment, remains `TBD` pending controlled engineering/certification basis.

## 7. Degradation handling

`NAV-V14` shall not assume nominal battery capability when an approved degradation factor/model indicates reduced available energy or changed consumption characteristics.

The following model elements require controlled definition:

- battery state-of-health representation;
- degradation coefficient or equivalent model;
- effect on available energy;
- effect on predicted consumption, if applicable;
- uncertainty/guard-band treatment;
- validity limits.

All numerical coefficients remain `TBD` until supported by an approved source or controlled engineering analysis.

## 8. Wind treatment

Wind shall be represented as a controlled input to the return-energy prediction where the approved propulsion/energy model establishes such dependency.

The model shall preserve wind validity/state information and shall not silently substitute stale or invalid wind data for valid input.

Exact update rate, validity interval and uncertainty bounds are `TBD` unless already established by the controlled system basis.

## 9. Configuration and applicability

An Energy Model instance shall be associated with the applicable UAV/configuration baseline. Changes to aircraft configuration, payload/equipment, battery characteristics or other model-significant parameters shall trigger the applicable revalidation/recalculation process.

The model shall not be treated as universally valid across different UAV configurations without an explicit applicability basis.

## 10. Verification interface

The model shall expose sufficient inputs and outputs to execute `NAV-TV-009` and record evidence for `EVD-025`.

At execution, the evidence record shall identify:

- model/configuration revision;
- input dataset;
- route definition;
- battery state/condition;
- wind state;
- calculated return requirement;
- available energy/margin;
- resulting feasibility decision;
- actual result and anomalies.

## 11. Acceptance basis

The numerical tolerance, reserve margin, uncertainty allowance and pass/fail thresholds are **TBD**.

They shall be established before `EVD-025` can move from `PLANNED` to an executable/accepted verification state.

## 12. Status and gate

**DRAFT_FOR_AGREEMENT**

This baseline does not close `NAV-TV-009`, `NAV-V14` or `EVD-025`.

Execution remains blocked until the Energy Model is approved/baselined together with its applicable parameters and acceptance basis.
