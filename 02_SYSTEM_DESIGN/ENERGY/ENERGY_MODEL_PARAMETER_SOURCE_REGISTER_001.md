---
id: ENERGY-MODEL-PARAMETER-SOURCE-REGISTER-001
type: energy_model_parameter_source_register
status: draft_for_agreement
parent: ENERGY-MODEL-REQUIREMENTS-001
---

# BlueSky PRO — Energy Model Parameter / Source Register 001

## 1. Purpose

Определить контролируемый реестр параметров Energy Model и порядок назначения их источника, единицы, диапазона, метода получения и verification basis.

Этот документ является реестром параметров и источников. Он не вводит численные значения или acceptance thresholds, отсутствующие в утверждённой инженерной/сертификационной базе.

## 2. Parameter register

| ID | Parameter | Required role | Unit | Source / basis | Range / limit | Acquisition / derivation | Verification | Status |
|---|---|---|---|---|---|---|---|---|
| EN-P-001 | Nominal battery capacity | baseline energy capacity | TBD | Manufacturer / approved engineering basis | TBD | controlled configuration data | Analysis / Test | TBD |
| EN-P-002 | Available battery capacity | usable capacity | TBD | approved battery model / engineering data | TBD | model calculation | Analysis / Test | TBD |
| EN-P-003 | Current energy state | initial/current energy | TBD | onboard measurement / approved estimator | TBD | runtime telemetry/estimation | Test | TBD |
| EN-P-004 | Battery degradation state | degradation correction | TBD | approved ageing/degradation model; manufacturer/test basis | TBD | controlled calculation | Analysis / Test | TBD |
| EN-P-005 | Estimated usable energy | energy available to model | TBD | derived from controlled energy state/model | TBD | model calculation | Analysis / Test | TBD |
| EN-P-006 | Propulsion consumption | propulsion energy demand | TBD | approved propulsion/airframe engineering data | TBD | model calculation / test | Analysis / Test | TBD |
| EN-P-007 | Avionics/payload consumption | non-propulsion energy demand | TBD | approved equipment/configuration data | TBD | configuration/model/test | Analysis / Test | TBD |
| EN-P-008 | Airspeed / flight condition | consumption input | TBD | Navigation/flight-state source | TBD | runtime measurement/estimation | Test | TBD |
| EN-P-009 | Groundspeed | return-time / consumption input | TBD | Navigation model | TBD | runtime calculation | Analysis / Test | TBD |
| EN-P-010 | Wind | environmental consumption/return input | TBD | approved meteorological/navigation source | TBD | runtime measurement/estimation | Analysis / Test | TBD |
| EN-P-011 | Altitude / operating regime | consumption correction | TBD | Navigation/flight-state source | TBD | runtime state | Analysis / Test | TBD |
| EN-P-012 | UAV configuration | configuration-dependent model selection | N/A | controlled configuration baseline | N/A | configuration identification | Analysis | TBD |
| EN-P-013 | Payload / installed equipment | configuration-dependent consumption | TBD | controlled equipment configuration | TBD | configuration identification | Analysis / Test | TBD |
| EN-P-014 | Mission phase / mode | phase-dependent consumption | N/A | controlled mission state | N/A | runtime state | Analysis / Test | TBD |
| EN-P-015 | Return-route distance | return energy input | TBD | active return route/version | TBD | route calculation | Analysis / Test | TBD |
| EN-P-016 | Return-route profile | route energy input | TBD | active route/profile | TBD | route calculation | Analysis | TBD |
| EN-P-017 | Predicted return energy | required return energy | TBD | Energy Model | TBD | model calculation | Analysis / Simulation / Test | TBD |
| EN-P-018 | Required reserve / margin | decision boundary | TBD | approved Safety/Engineering basis | TBD | controlled policy/model | Analysis / Test | TBD |
| EN-P-019 | Energy margin | feasibility discriminator | TBD | derived quantity | TBD | `available usable energy - return requirement - reserve/margin` subject to approved formulation | Analysis / Test | TBD |
| EN-P-020 | Return feasibility state | decision output | N/A | approved Dynamic Return/Safety baseline | N/A | model decision | Test | TBD |
| EN-P-021 | Input validity state | input quality | N/A | approved data-quality rules | N/A | runtime validation | Test | TBD |
| EN-P-022 | Model version | reproducibility/configuration | N/A | controlled model baseline | N/A | configuration metadata | Inspection / Analysis | TBD |
| EN-P-023 | Calculation timestamp | temporal traceability | TBD | system time source | TBD | runtime metadata | Inspection / Test | TBD |
| EN-P-024 | Uncertainty / confidence | decision qualification | TBD | approved model/measurement basis | TBD | model/estimator output | Analysis / Test | TBD |

## 3. Source hierarchy

Where a parameter is not yet baselined, the source shall be selected in the following controlled order, subject to project approval:

1. Approved system/engineering model;
2. Manufacturer technical data;
3. Controlled measurement/test data;
4. Approved estimation/model derivation;
5. Other source explicitly accepted by the applicable engineering/certification basis.

A source shall be recorded with sufficient identity/version information to reproduce the value or derivation.

## 4. Units and reference conditions

Each parameter shall have:

- unambiguous unit;
- reference condition where applicable;
- sign convention where applicable;
- validity range;
- timestamp or temporal validity where applicable;
- source/configuration identity.

`TBD` is retained until the authoritative basis is established.

## 5. Parameter qualification states

Each parameter should progress through:

```text
TBD
  ↓
SOURCE IDENTIFIED
  ↓
VALUE / RANGE DERIVED
  ↓
ENGINEERING REVIEWED
  ↓
BASELINED
  ↓
VERIFIED
```

`VERIFIED` is not assigned by this register. It requires actual verification evidence.

## 6. NAV-TV-009 dependency

The minimum parameter set for `NAV-TV-009` is:

```text
EN-P-005 Estimated usable energy
EN-P-008 Airspeed / flight condition
EN-P-009 Groundspeed
EN-P-010 Wind
EN-P-012 UAV configuration
EN-P-013 Payload / installed equipment
EN-P-015 Return-route distance
EN-P-016 Return-route profile
EN-P-017 Predicted return energy
EN-P-018 Required reserve / margin
EN-P-019 Energy margin
EN-P-020 Return feasibility state
EN-P-021 Input validity state
EN-P-022 Model version
EN-P-023 Calculation timestamp
```

The parameter set is a readiness dependency; it does not itself constitute execution evidence.

## 7. Explicit non-assumptions

This register does not assume:

- a universal battery degradation coefficient;
- a universal propulsion consumption value;
- a fixed reserve percentage;
- a fixed wind threshold;
- a fixed groundspeed threshold;
- a universal uncertainty value;
- a certification acceptance tolerance.

Those values require controlled engineering, manufacturer, test or certification basis.

## 8. Traceability

```text
ENERGY-MODEL-REQUIREMENTS-001
        ↓
ENERGY-MODEL-PARAMETER-SOURCE-REGISTER-001
        ↓
ENERGY-MODEL-BASELINE-001
        ↓
NAV-TV-009
        ↓
NAV-V14
        ↓
EVD-025
```

## 9. Current status

**DRAFT_FOR_AGREEMENT — SOURCE IDENTIFICATION AND NUMERICAL BASELINING REQUIRED**
