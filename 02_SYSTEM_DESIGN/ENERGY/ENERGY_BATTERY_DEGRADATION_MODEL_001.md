---
id: ENERGY-BATTERY-DEGRADATION-MODEL-001
type: energy_battery_degradation_model
status: draft_for_agreement
parent: ENERGY-MODEL-BASELINE-001
---

# BlueSky PRO — Battery / Degradation Model 001

## 1. Purpose

Определить контролируемую структуру модели батареи и её деградации как подмодели Energy Model. Документ предназначен для подготовки `NAV-TV-009` и `EVD-025`; он не вводит неподтверждённые численные коэффициенты.

## 2. Traceability

```text
ENERGY-MODEL-BASELINE-001
        ↓
ENERGY-MODEL-REQUIREMENTS-001
        ↓
ENERGY-MODEL-PARAMETER-SOURCE-REGISTER-001
        ↓
ENERGY-BATTERY-DEGRADATION-MODEL-001
        ↓
NAV-TV-009
        ↓
NAV-V14 / EVD-025
```

## 3. Model boundary

Подмодель должна обеспечивать контролируемое представление:

```text
Nominal Battery Capacity
        ↓
Available Capacity
        ↓
Current Energy State
        ↓
Degradation / Derating
        ↓
Estimated Usable Energy
```

Она не определяет самостоятельно решение о возврате и не является Execution Authority.

## 4. Controlled battery states

Минимальные различаемые состояния:

- NOMINAL — в пределах утверждённой применимости модели;
- DEGRADED — применима контролируемая коррекция/снижение характеристик;
- INVALID — данные не позволяют выполнять расчёт;
- UNAVAILABLE — необходимые данные отсутствуют.

Exact transition criteria: `TBD`.

## 5. Required parameters

| Parameter | Purpose | Current basis |
|---|---|---|
| EN-P-001 Nominal battery capacity | reference capacity | Manufacturer / approved engineering basis |
| EN-P-002 Available battery capacity | usable capacity basis | Approved battery model / engineering data |
| EN-P-003 Current energy state | runtime state | Onboard measurement / approved estimator |
| EN-P-004 Battery degradation state | degradation correction | Approved ageing/degradation model; manufacturer/test basis |
| EN-P-005 Estimated usable energy | energy available to Energy Model | Derived from controlled battery state/model |
| EN-P-024 Uncertainty / confidence | qualification of energy estimate | Approved model/measurement basis |

No value, coefficient or threshold is assigned here.

## 6. Degradation model structure

The approved degradation representation shall explicitly identify, as applicable:

```text
Battery identity
Battery chemistry/type
Age / cycle state
State of Health
Available capacity
Temperature dependence
Operating-condition dependence
Measurement/estimation uncertainty
Model applicability limits
```

The definitive set is subject to engineering baseline approval.

## 7. Degradation correction

The model may use a controlled correction or a higher-fidelity degradation function. The selected representation shall be traceable to its source and validation basis.

Conceptual form only:

```text
Estimated usable energy
    = f(nominal/available capacity,
        current energy state,
        battery condition,
        approved degradation model,
        applicable operating conditions)
```

No universal degradation coefficient is assumed.

## 8. Quality and failure handling

The submodel shall prevent nominal-energy treatment when required battery inputs are invalid, stale or unavailable.

At minimum the following outcomes shall be distinguishable:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Exact freshness limits and transition rules remain `TBD`.

## 9. Uncertainty and conservative use

The model shall expose uncertainty/confidence information where established by the approved basis. The treatment of uncertainty in return-feasibility decisions shall be controlled by the approved reserve/margin policy.

The model shall not silently convert uncertainty into a fixed reserve percentage.

## 10. Configuration linkage

Battery model applicability shall be tied to the controlled battery and UAV configuration identifiers. A change that can affect energy characteristics shall trigger applicable model revalidation or recalculation.

Minimum configuration linkage:

```text
UAV configuration ID
Battery ID / model
Battery configuration/revision
Payload/equipment configuration where energy-significant
Energy Model version
```

## 11. Verification requirements

Battery/degradation verification shall eventually cover:

- nominal battery state;
- degraded battery state;
- state transition into degradation;
- invalid battery input;
- stale battery input;
- unavailable battery input;
- model/configuration mismatch;
- uncertainty handling;
- reproducibility of derived usable energy.

Actual verification IDs and acceptance criteria shall be linked through the Verification Register and evidence system.

## 12. NAV-TV-009 readiness impact

`NAV-TV-009` cannot be considered ready solely because this model structure exists.

Required controlled inputs before execution include at minimum:

```text
Nominal / available capacity basis
Current energy-state source
Approved degradation representation
Usable-energy derivation
Uncertainty / confidence treatment where applicable
Configuration linkage
Acceptance criteria
```

## 13. Open engineering items

```text
TBD-ENERGY-001 battery data source set
TBD-ENERGY-002 capacity definition and unit
TBD-ENERGY-003 state-of-charge / energy-state semantics
TBD-ENERGY-004 state-of-health definition
TBD-ENERGY-005 degradation model form
TBD-ENERGY-006 temperature dependence applicability
TBD-ENERGY-007 estimation uncertainty
TBD-ENERGY-008 quality/freshness thresholds
TBD-ENERGY-009 acceptance criteria
```

## 14. Status

**DRAFT_FOR_AGREEMENT — ENGINEERING DATA AND DEGRADATION BASIS REQUIRED**
