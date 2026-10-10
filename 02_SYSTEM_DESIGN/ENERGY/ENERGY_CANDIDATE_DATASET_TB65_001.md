---
id: ENERGY-CANDIDATE-DATASET-TB65-001
type: candidate_energy_parameter_dataset
status: engineering_reference_only
parent: ENERGY-MODEL-PARAMETER-SOURCE-REGISTER-001
---

# BlueSky PRO — Candidate Energy Dataset 001 — DJI TB65 Reference

## 1. Purpose

Сформировать первый конкретный candidate dataset для проверки цепочки Energy Model на связке **DJI Matrice 350 RTK + TB65**, используя только внешние manufacturer/research references, уже зафиксированные в `ENERGY_EXTERNAL_SOURCE_PARAMETER_BASELINE_001`.

Этот документ **не является утверждённым BlueSky PRO baseline** и не устанавливает универсальные коэффициенты для других БВС.

## 2. Controlled configuration

| Item | Value | Status | Source |
|---|---|---|---|
| UAV | DJI Matrice 350 RTK | Reference configuration | DJI official specification |
| Battery | TB65 | Reference battery | DJI official specification |
| Battery chemistry | Li-ion | Source identified | DJI official specification |
| Nominal capacity | 5880 mAh | Source identified | DJI official specification |
| Nominal voltage | 44.76 V | Source identified | DJI official specification |
| Declared energy | 263.2 Wh | Source identified | DJI official specification |
| Mass | approx. 1.35 kg | Source identified | DJI official specification |
| Operating temperature | -20 °C to +50 °C | Source identified | DJI official specification |
| Charging temperature | -20 °C to +40 °C | Source identified | DJI official specification |

## 3. Parameter mapping to EN-P-001...005

### EN-P-001 — Nominal battery capacity

Candidate reference value:

`5880 mAh`

Status: **SOURCE IDENTIFIED / CANDIDATE VALUE**.

The value is bound to TB65 and shall not be generalized to another battery model.

### EN-P-002 — Available battery capacity

Candidate source provides declared energy of:

`263.2 Wh`

Status: **PARTIAL**.

The declared energy shall not be treated as usable mission energy. A usable-energy definition still requires discharge limits, operating conditions and battery-state policy.

### EN-P-003 — Current energy state

No universal numerical value can be extracted from the manufacturer specification for a generic mission state.

Status: **REQUIRES RUNTIME/BMS INTERFACE OR QUALIFIED ESTIMATOR**.

Required future evidence:

- authoritative battery telemetry field(s);
- SOC/SOE definition;
- update rate;
- validity/staleness rules;
- behavior on invalid or missing battery data.

### EN-P-004 — Battery degradation state

The external source establishes service-life/ageing relevance but does not provide a complete capacity-versus-cycle/age function suitable for direct use in BlueSky PRO.

Status: **PARTIAL / MODEL DATA REQUIRED**.

Required future evidence:

- cycle count;
- battery health/SoH source;
- temperature history if applicable;
- degradation curve/model;
- uncertainty and confidence treatment.

### EN-P-005 — Estimated usable energy

No direct manufacturer universal value.

Status: **DERIVED / GAP**.

Candidate relation for engineering model definition only:

`Estimated usable energy = f(current energy state, battery degradation, operating conditions, discharge limits, uncertainty policy)`

No numerical coefficient is introduced here.

## 4. Data that remains unfilled

The following are intentionally left open because the cited external sources do not establish a BlueSky PRO-approved value:

- usable-energy fraction;
- reserve/margin;
- SOC-to-energy conversion;
- temperature derating function;
- internal-resistance model for mission prediction;
- propulsion/avionics/payload consumption;
- wind-dependent consumption;
- model uncertainty;
- acceptance tolerances.

## 5. Engineering use

This candidate dataset can be used to exercise the **data structure and traceability path** of the Energy Model:

```text
M350 RTK + TB65
      ↓
EN-P-001 nominal capacity
      ↓
EN-P-002 declared energy / available-energy source
      ↓
EN-P-003 runtime energy state
      ↓
EN-P-004 degradation state
      ↓
EN-P-005 estimated usable energy
      ↓
Consumption Model
      ↓
Return Energy Calculation
      ↓
NAV-TV-009
```

Only the first two source-derived values are presently concrete. The remaining parameters require controlled runtime data, engineering documentation or validation testing.

## 6. Qualification decision

**Decision: ACCEPT AS CANDIDATE REFERENCE DATASET; DO NOT BASELINE FOR BLUE SKY PRO.**

The dataset demonstrates that a concrete manufacturer configuration can be mapped to the Energy Model without inventing missing parameters. It is suitable as a template for subsequent controlled UAV/battery datasets.

## 7. Sources

Primary manufacturer source already registered in the project:

- DJI Matrice 350 RTK official specifications: https://enterprise.dji.com/matrice-350-rtk/specs
- DJI Matrice 350 RTK FAQ/service-life information: https://enterprise.dji.com/matrice-350-rtk/faq

Research sources already registered in the project:

- Sierra et al., Battery health management for small-size rotary-wing electric unmanned aerial vehicles: https://www.sciencedirect.com/science/article/pii/S0951832018301406
- Zhang, Zhou, Lan, UAV lithium-battery SOC estimation using EKF: https://www.mdpi.com/1996-1073/12/20/3960
- Battery Testing and Discharge Model Validation for Electric UAVs: https://pmc.ncbi.nlm.nih.gov/articles/PMC10422487/
