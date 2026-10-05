# ENERGY REFERENCE PLATFORM — DJI MATRICE 300 RTK

## 1. Purpose

This document introduces a second concrete, manufacturer-documented UAV reference platform for the BlueSky PRO Energy Model.

The selected platform is DJI Matrice 300 RTK (M300 RTK), a mature enterprise multirotor platform with extensive manufacturer documentation. It is used as a **reference dataset and model-calibration candidate**, not as a normative representation of all UAVs supported by BlueSky PRO.

## 2. Selection rationale

M300 RTK is suitable as a reference platform because:

- it is an enterprise multirotor UAV;
- DJI provides detailed aircraft, battery, propulsion, payload and environmental specifications;
- the manufacturer provides user, maintenance, battery-safety and accessory documentation;
- the platform has a documented dual-battery architecture;
- its published characteristics provide a useful independent comparison against the M350 RTK/TB65 reference dataset already present in the project.

External source: DJI official M300 RTK support/specification and download documentation.

## 3. Manufacturer-documented aircraft parameters

| Parameter | M300 RTK documented value | Energy-model relevance | Status |
|---|---:|---|---|
| Aircraft weight, without batteries | approx. 3.6 kg | mass/configuration input | REFERENCE |
| Weight with two TB60 batteries | approx. 6.3 kg | operating mass | REFERENCE |
| Max takeoff weight | 9 kg | configuration boundary | REFERENCE |
| Diagonal wheelbase | 895 mm | aerodynamic/vehicle descriptor | REFERENCE |
| Max horizontal speed | 23 m/s (S mode) | flight-condition input | REFERENCE |
| Max wind resistance | 12 m/s | environmental boundary | REFERENCE |
| Max flight time | 55 min | reference endurance check | REFERENCE |
| Operating temperature | -20 to +50 °C | temperature model input | REFERENCE |
| GNSS | GPS + GLONASS + BeiDou + Galileo | navigation input | REFERENCE |
| Max service ceiling | 5000 m / 7000 m depending on propeller and weight | altitude/configuration boundary | REFERENCE |
| Payload capacity at single gimbal damper | 930 g | payload/configuration input | REFERENCE |

## 4. Battery reference parameters

The M300 RTK uses the DJI TB60 Intelligent Flight Battery.

| Parameter | Manufacturer-documented value | Energy-model relevance | Status |
|---|---:|---|---|
| Battery model | TB60 | battery configuration key | REFERENCE |
| Capacity | 5935 mAh | nominal capacity | REFERENCE |
| Voltage | 52.8 V | electrical parameter | REFERENCE |
| Energy | 274 Wh | nominal energy | REFERENCE |
| Battery type | LiPo 12S | battery chemistry/configuration | REFERENCE |
| Net battery weight | approx. 1.35 kg | mass input | REFERENCE |
| Battery operating temperature | -20 to +50 °C | derating/degradation context | REFERENCE |
| Ideal storage temperature | 22 to 30 °C | storage/degradation context | REFERENCE |
| Charging temperature | -20 to +40 °C | battery lifecycle context | REFERENCE |

## 5. Published endurance limitation

DJI publishes a maximum flight-time value of 55 minutes for M300 RTK. This value is a **manufacturer reference condition**, not a universal consumption coefficient.

The BlueSky PRO Energy Model shall not convert the published maximum flight time directly into Wh/km, W, or usable-energy reserve without controlled operating-condition data.

## 6. Parameters still required for BlueSky PRO calibration

The manufacturer specification does not by itself provide a complete operational energy-consumption model. The following remain required or TBD:

- electrical power versus flight speed;
- electrical power versus payload;
- hover power under defined mass/configuration;
- climb/descent power;
- wind-dependent consumption;
- temperature-dependent usable energy;
- battery internal state and SOC estimation behavior;
- degradation / SOH model;
- reserve policy and uncertainty margin;
- route-segment energy model;
- telemetry-derived current/voltage/power records.

These shall be obtained from controlled flight-test data, approved engineering data, or additional authoritative manufacturer documentation before being baselined as BlueSky PRO model coefficients.

## 7. Relationship to existing reference platform

Current reference platforms:

1. DJI Matrice 350 RTK + TB65 — primary modern enterprise reference dataset.
2. DJI Matrice 300 RTK + TB60 — secondary mature enterprise reference dataset.

The two platforms shall be retained as separate configurations. Their numerical parameters shall not be averaged into a single UAV model unless a later engineering analysis explicitly demonstrates the validity of such abstraction.

## 8. Traceability intent

This reference platform supports:

`EN-P-001` Nominal battery capacity

`EN-P-002` Available battery capacity

`EN-P-003` Current energy state

`EN-P-004` Battery degradation state

`EN-P-005` Estimated usable energy

and provides vehicle-level reference inputs for the Consumption Model and Return Feasibility Model.

## 9. Evidence classification

Classification: `EXTERNAL MANUFACTURER REFERENCE DATA`

It is not yet a BlueSky PRO certified parameter baseline.

Acceptance for engineering use requires source identification, document/version control, applicability review, unit verification and, where required, confirmation by controlled test.

## 10. Primary external sources

- DJI Matrice 300 RTK official specifications/support documentation.
- DJI Matrice 300 RTK official download center, including user manual, maintenance manual, battery safety guide and accessory documentation.

## 11. Current conclusion

M300 RTK is accepted as a second reference platform for the BlueSky PRO Energy Model. It materially improves the external reference dataset by providing an independently documented UAV/battery configuration with a different battery architecture from M350 RTK/TB65.

No BlueSky PRO operational coefficient is baselined from these values at this stage.
