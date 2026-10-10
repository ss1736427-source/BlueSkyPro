---
id: ENERGY-EXTERNAL-SOURCE-RESEARCH-001
type: external_source_research
status: research_baseline
parent: ENERGY-MODEL-REQUIREMENTS-001
---

# BlueSky PRO — Energy Model External Source Research 001

## 1. Purpose

Зафиксировать внешние источники, которые могут быть использованы как инженерная и нормативная основа для квалификации параметров Energy Model и последующей разработки NAV-TV-009.

Источники разделены по назначению. Внешний источник не считается автоматически утверждённым проектным параметром: применимость, версия, конкретная конфигурация БВС/батареи и метод получения данных должны быть отдельно подтверждены.

## 2. Priority source classes

1. Нормативные документы для требований и методов испытаний.
2. Документация изготовителя конкретной батареи/БВС.
3. Результаты собственных испытаний конкретной конфигурации.
4. Рецензируемые научные публикации для выбора и обоснования модели.
5. Публичные datasets — только после проверки происхождения, качества и применимости.

## 3. Identified external sources

| ID | Source | Primary use | Candidate parameters | Applicability status |
|---|---|---|---|---|
| EXT-EN-001 | ISO 24352:2023, Technical requirements for small unmanned aircraft electric energy systems | Требования и методы испытаний EES малых БВС | battery/EES characteristics, test basis | CANDIDATE NORMATIVE BASIS |
| EXT-EN-002 | IEC 62660-1:2018, Secondary lithium-ion cells — Part 1: Performance testing | Методическая основа performance/life testing | capacity, power, energy, cycle-life data | CANDIDATE TEST BASIS; scope is EV propulsion cells |
| EXT-EN-003 | Zhang, Zhou, Lan, Energies 2019, UAV Li-Po SOC estimation using EKF | SOC estimation / battery model selection | SOC, voltage, current, temperature, ageing effects | RESEARCH BASIS |
| EXT-EN-004 | Jung & Jeong, Energies 2017, EKF-based SOC/SOP estimation for UAV Li-Po packs | SOC/SOP estimation | SOC, SOP, voltage/current, temperature, resistance | RESEARCH BASIS |
| EXT-EN-005 | Tang et al., Microelectronics Reliability 2020, power-transfer model for electric rotatory-wing UAV | Coupled battery + UAV power model | SOE, EOD, power consumption, flight conditions | RESEARCH BASIS |
| EXT-EN-006 | Jastrzębska, Lerke, Kwiatkowski, Electric Power Systems Research 2026, prediction of UAV energy consumption | Hybrid physics/data-driven energy prediction | battery state, weather, trajectory, velocity | RESEARCH BASIS; recent |
| EXT-EN-007 | Muli, Park, Liu, A Comparative Study on Energy Consumption Models for Drones, 2022 | Model selection / sensitivity | speed, payload, energy per distance, range | RESEARCH COMPARISON |
| EXT-EN-008 | DJI Matrice 350 RTK official specifications | Manufacturer example for concrete battery data | capacity, voltage, energy, mass, operating temperature | PRODUCT-SPECIFIC REFERENCE |
| EXT-EN-009 | DJI TB65 official FAQ | Manufacturer operational/life information | service life, cycles, temperature effects | PRODUCT-SPECIFIC REFERENCE |
| EXT-EN-010 | Tattu/Gens Ace UAV LiPo product specification | Manufacturer battery example | capacity, configuration, discharge rate, mass, temperature | PRODUCT-SPECIFIC REFERENCE |

## 4. What the sources establish

### 4.1 Battery capacity / energy

Manufacturer documentation can provide nominal capacity, nominal voltage, stated energy, mass, configuration and temperature limits for a specific battery. These values are configuration-specific and shall not be generalized across UAV families.

### 4.2 SOC / SOE

UAV battery research supports treating SOC/SOE as an estimated state rather than assuming that a single voltage reading is an exact remaining-energy measurement. EKF/equivalent-circuit approaches and state-space approaches are documented in the literature.

### 4.3 Degradation / SOH

The literature supports capacity and internal resistance as important health indicators. A 2024 review explicitly distinguishes capacity, resistance and energy based SOH definitions and notes that ageing affects capacity and resistance. The project should therefore avoid a single unexplained generic degradation coefficient unless it is backed by an approved dataset or test programme.

### 4.4 Consumption

Published UAV models use different levels of fidelity, from empirical rates to physics-based propulsion/aerodynamic models and hybrid data-driven approaches. A 2021 comparative assessment found substantial variation between models and emphasizes the importance of drone design and operating conditions. Therefore the consumption model must be tied to the actual UAV/configuration class and validated against representative data.

### 4.5 Wind and trajectory

Recent UAV energy-consumption research identifies weather, trajectory design and velocity control as material factors. The return model should therefore retain wind and route/velocity as explicit inputs rather than replacing them with a fixed endurance number.

## 5. Parameter qualification mapping

| Project parameter | Candidate external basis | What must still be obtained/confirmed |
|---|---|---|
| EN-P-001 Nominal battery capacity | Manufacturer datasheet; test standard | Exact battery model/revision and rated conditions |
| EN-P-002 Available battery capacity | Manufacturer/BMS + controlled measurement | Definition of usable capacity and operating limits |
| EN-P-003 Current energy state | BMS/telemetry + SOC/SOE estimation literature | Sensor set, estimator, initialization and uncertainty |
| EN-P-004 Battery degradation state | Manufacturer life data + ageing/SOH literature + test data | Project-specific SOH definition and degradation evidence |
| EN-P-005 Estimated usable energy | Battery model + BMS/SOC/SOE + temperature/current effects | Approved calculation and validation dataset |

## 6. Important limitation

External literature is suitable for model architecture, parameter candidates, test methodology and sensitivity analysis. It is not sufficient by itself to establish BlueSky PRO acceptance values for an unspecified UAV fleet.

For certification-grade use, the final numerical parameter set should be derived from controlled manufacturer data and/or representative tests for each supported UAV/battery/configuration class, with configuration and revision traceability.

## 7. Immediate engineering conclusion

The external-source search confirms that the Energy Model approach is technically supported, but also confirms that there is no universal UAV energy-consumption/degradation coefficient that can safely be inserted into BlueSky PRO without qualification.

The next controlled task is therefore to collect concrete manufacturer data and/or project test data for the first supported UAV + battery configuration and populate EN-P-001…EN-P-005.
