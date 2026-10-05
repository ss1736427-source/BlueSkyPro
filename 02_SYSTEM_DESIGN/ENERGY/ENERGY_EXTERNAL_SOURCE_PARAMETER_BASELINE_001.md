---
id: ENERGY-EXTERNAL-SOURCE-PARAMETER-BASELINE-001
type: external_source_parameter_baseline
status: reference_only_for_engineering_review
parent: ENERGY-MODEL-PARAMETER-SOURCE-REGISTER-001
---

# BlueSky PRO — External Source Parameter Baseline 001

## 1. Purpose

Зафиксировать найденные внешние источники для первичной квалификации параметров `EN-P-001...EN-P-005` и не смешивать внешние reference-data с утверждённым BlueSky PRO baseline.

**Правило:** значения ниже являются source-derived reference data. Они не являются нормативными значениями BlueSky PRO и не должны напрямую использоваться как universal parameters для всех БВС.

## 2. Primary manufacturer reference — DJI TB65

Source: DJI Matrice 350 RTK official specifications.

Reference battery:

- Model: TB65
- Type: Li-ion
- Nominal capacity: 5880 mAh
- Nominal voltage: 44.76 V
- Declared energy: 263.2 Wh
- Mass: approximately 1.35 kg
- Operating temperature: -20 °C to +50 °C
- Charging temperature: -20 °C to +40 °C

Official source:
https://enterprise.dji.com/matrice-350-rtk/specs

Additional official DJI source for service life and low-temperature behaviour:
https://enterprise.dji.com/matrice-350-rtk/faq

### Parameter mapping

| Parameter | External evidence | Qualification status | BlueSky PRO use |
|---|---|---|---|
| EN-P-001 Nominal battery capacity | TB65: 5880 mAh | SOURCE IDENTIFIED | Reference profile only |
| EN-P-002 Available battery capacity | Declared energy 263.2 Wh; usable capacity not separately specified | PARTIAL | Requires model/measurement |
| EN-P-003 Current energy state | Manufacturer exposes intelligent-battery information, but no universal BlueSky estimator basis is defined | SOURCE CONCEPT IDENTIFIED | Requires runtime interface definition |
| EN-P-004 Battery degradation state | DJI states service-life criteria including 12 months, 400 cycles, or 120 days of high-power storage; no capacity-vs-cycle curve is provided in the cited page | PARTIAL | Requires approved degradation model/data |
| EN-P-005 Estimated usable energy | Not directly supplied as a universal value | GAP | Derived by BlueSky Energy Model |

## 3. Secondary manufacturer reference — Tattu 22000mAh 6S1P 22.2V 30C

Source: Tattu official product page.

Reference battery:

- Nominal capacity: 22000 mAh
- Energy: 488.4 Wh
- Nominal voltage: 22.2 V
- Configuration: 6S1P
- Maximum continuous discharge current: 110 A
- Internal resistance: <= 6 mOhm
- Cycle life: >500 cycles
- Discharge temperature: -20 °C to +60 °C
- Charge temperature: +10 °C to +50 °C

Official source:
https://www.tattuworld.com/products/tattu-22000mah-6s1p-22-2v-30c-uav-lipo-battery-with-xt90s-f-plug.html

This source demonstrates that UAV battery parameter sets vary materially by pack design and cannot be collapsed into one universal capacity, voltage, resistance, or degradation coefficient.

## 4. Engineering/research basis for degradation and SOC

### 4.1 UAV battery health / EOD

Sierra et al., *Battery health management for small-size rotary-wing electric unmanned aerial vehicles: An efficient approach for constrained computing platforms*, Reliability Engineering & System Safety, 2019.

The work describes a model-based BMS approach for UAV Li-Po batteries, including SOC estimation, End-of-Discharge prediction, a simplified equivalent-circuit battery model, and an aerodynamic-based power-consumption model.

Source:
https://www.sciencedirect.com/science/article/pii/S0951832018301406

### 4.2 UAV SOC estimation

Zhang, Zhou, Lan, *State of Charge Estimation Algorithm for Unmanned Aerial Vehicle Power-Type Lithium Battery Packs Based on the Extended Kalman Filter*, Energies, 2019.

The study uses an equivalent-circuit model with an Extended Kalman Filter and evaluates SOC estimation at different temperatures.

Source:
https://www.mdpi.com/1996-1073/12/20/3960

### 4.3 UAV battery discharge model

*Battery Testing and Discharge Model Validation for Electric Unmanned Aerial Vehicles (UAV)*.

The work develops an empirical discharge model using a small number of tunable parameters and reports validation for variable-power loads. The reported normalized prediction error is below 0.7% for the studied test setup.

Source:
https://pmc.ncbi.nlm.nih.gov/articles/PMC10422487/

## 5. Qualification conclusions

### EN-P-001 — Nominal battery capacity

**State: SOURCE IDENTIFIED.**

Manufacturer data can provide the nominal capacity for each controlled battery type. The value shall be bound to an exact battery part/model and revision.

### EN-P-002 — Available battery capacity

**State: PARTIAL.**

Nameplate energy is not equivalent to model-usable energy. Available capacity requires an approved definition covering operating conditions, discharge limits and battery state.

### EN-P-003 — Current energy state

**State: SOURCE CONCEPT IDENTIFIED.**

Runtime energy state may be obtained from BMS/telemetry or an approved estimator. The project still requires the authoritative interface, update rate, validity rules and estimator qualification.

### EN-P-004 — Battery degradation state

**State: PARTIAL.**

Manufacturer service-life statements establish that ageing is relevant, but they do not by themselves provide a quantitative degradation function suitable for `NAV-TV-009`. A controlled degradation dataset/model is required.

### EN-P-005 — Estimated usable energy

**State: GAP.**

This is a derived Energy Model quantity. It shall not be copied from a manufacturer nameplate. It depends on the approved battery state, degradation, operating conditions and usable-energy policy.

## 6. Important engineering restriction

The following shall **not** be introduced into the BlueSky PRO model merely because they appear in external material:

- universal degradation coefficient;
- universal usable-capacity percentage;
- universal reserve percentage;
- universal SOC-to-energy conversion;
- universal temperature derating;
- universal internal-resistance value;
- universal consumption coefficient.

Such values require applicability to the controlled UAV/battery/configuration or a justified model-validation basis.

## 7. Current status of NAV-TV-009 dependency

External research has moved the source status forward, but it does **not** by itself close the verification gate:

```text
EN-P-001  SOURCE IDENTIFIED
EN-P-002  PARTIAL
EN-P-003  SOURCE CONCEPT IDENTIFIED
EN-P-004  PARTIAL
EN-P-005  GAP

        ↓
Controlled UAV + battery configuration
        ↓
Engineering parameter acquisition
        ↓
Battery/degradation calibration
        ↓
Consumption calibration
        ↓
Return-energy calculation
        ↓
NAV-TV-009 execution readiness
```

## 8. Decision

Use the external sources above as **reference evidence for engineering model construction**, not as final BlueSky PRO acceptance criteria.

The next controlled engineering action is to select one concrete UAV + battery configuration and build its parameter dataset from manufacturer documentation and/or controlled measurements. This dataset will be the candidate input to the Energy Model baseline.
