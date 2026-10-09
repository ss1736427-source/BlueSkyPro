---
id: MT01-MT02-CASE-BINDING-001
type: verification_case_binding
status: draft_for_agreement
authority:
  - MASTER-REQUIREMENTS-REGISTER-001
  - MT01-MT02-VERIFICATION-CASES-001
  - MT01-MT02-TEST-DATASETS-001
---

# MT-01 / MT-02 Case-Level Binding

## 1. Controlled case binding

| Verification case | Dataset | Current state |
|---|---|---|
| V-M01-01 | MT01-T01 | DEFINED / EXECUTION CONFIGURATION OPEN |
| V-M02-01 | MT02-T21 | DEFINED / EXECUTION CONFIGURATION OPEN |

The case and dataset identities are controlled. No execution or evidence is implied.

## 2. Exact authoritative requirement wording

### SYS-REQ-008 — Mission Readiness

Authoritative source:

`02_SYSTEM/Requirements/SYS-REQ-008.md`

Controlled requirement text:

> Система должна формировать сводную оценку готовности миссии перед её утверждением и выполнением и отображать проблемные области.
>
> Mission Readiness должна представлять сводное состояние миссии на основании результатов валидации и доступных данных о текущих условиях выполнения.

Relevant controlled readiness areas include Airspace, Terrain, Geofence, Weather, C2 Coverage, Fleet Coordination, Energy and Contingency. Mission Readiness uses Mission Validation results and does not replace Mission Validation Engine or Safety Engine.

**MT-01 allocation:** DIRECT DEPENDENCY — planner output supplies inputs/results consumed by readiness assessment.

**MT-02 allocation:** DIRECT DEPENDENCY — same boundary.

### SYS-REQ-035 — Task to Capability Mapping

Authoritative source:

`02_SYSTEM/Requirements/SYS-REQ-035.md`

Controlled requirement text:

> Система должна преобразовывать поставленную задачу в набор необходимых capabilities.

For each task the requirement identifies, at minimum:
- task type;
- required capabilities;
- suitable UAV types;
- required payload;
- primary algorithms;
- required autonomy;
- communication requirements;
- success criteria.

The controlled record explicitly includes cartography and 3D reconstruction among example tasks.

**MT-01 allocation:** DIRECT DEPENDENCY — task/capability resolution determines the required acquisition and vehicle/equipment capability set.

**MT-02 allocation:** DIRECT DEPENDENCY — same boundary, with reconstruction-specific sensing/observation capability.

### SYS-REQ-076 — UAV Capability Profile

Authoritative source:

`02_SYSTEM/Requirements/SYS-REQ-076.md`

Controlled requirement text:

> Каждый подключённый UAV должен иметь машиночитаемый профиль возможностей и ограничений.

The profile includes, at minimum:
- platform_type;
- payload_capabilities;
- endurance;
- range;
- speed;
- altitude_limits;
- navigation_capabilities;
- communication_capabilities;
- sensing_capabilities;
- landing_capabilities;
- current_energy;
- current_health;
- current_mission_load.

**MT-01 allocation:** DIRECT DEPENDENCY — candidate generation and feasibility consume the machine-readable UAV capability/constraint profile.

**MT-02 allocation:** DIRECT DEPENDENCY — viewpoint/trajectory feasibility and reconstruction acquisition consume the same profile plus equipment-specific capability data.

## 3. Supporting controlled sources for execution configuration

### Configuration control

`00_PROJECT/CONFIGURATION/CONFIGURATION_BASELINE.md`

This establishes that verification-relevant configuration must identify the actual software, data, vehicle parameters, performance parameters, environment/weather inputs, test dataset and test environment. A result executed against a different configuration is invalid for the claimed configuration.

### Vehicle / equipment capability

`03_FLEET/BLUESKY_CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001.md` is represented in the repository as:

`03_FLEET/BLUESKY_CANONICAL_VEHICLE_EQUIPMENT_SCHEMA_001.md`

The canonical model distinguishes:
- Vehicle;
- Equipment;
- capability state;
- compatibility state;
- verification state;
- configuration version.

It explicitly requires unknown information to remain unknown/TBD rather than fabricated.

### Equipment integration

`06_EQUIPMENT/BLUESKY_EQUIPMENT_INTEGRATION_SPEC.md`

The equipment profile includes physical/operational limits, configuration, calibration, power, data outputs, aircraft compatibility and verification status. The pre-flight compatibility gate is:

`Vehicle → Equipment → Interface → Capability → Configuration/Calibration → Mission Action Compatibility → Data Storage → VERIFIED/BLOCKED`.

### Reference UAV configurations

`02_SYSTEM_DESIGN/ADMINISTRATOR/UAV_CONFIGURATION_REFERENCE_CATALOG_001.md`

This is a controlled reference population, not certification approval. It contains manufacturer-declared reference configurations including DJI Matrice 350 RTK, M300 RTK, M30/M30T and Matrice 4E/4T.

The catalog explicitly distinguishes integrated equipment from interchangeable payloads and requires explicit compatibility evidence.

### Photogrammetry planning basis

`docs/algorithm/PHOTOGRAMMETRY_SOLUTION_METHOD.md`

This controlled design baseline defines the acquisition data that the planner must consume, including camera geometry, calibration, shutter characteristics, GSD, surface-relative distance, terrain following, overlap, coverage margin, edge/corner coverage, positioning and equipment constraints.

It contains planning baselines such as ordinary nadir mapping front overlap ≥75% and side overlap ≥60%, while explicitly treating these as planning baselines rather than universal constants. These values are **not promoted here as verification acceptance criteria** without the applicable mission/profile configuration.

### Energy parameter source control

`02_SYSTEM_DESIGN/ENERGY/ENERGY_MODEL_PARAMETER_SOURCE_REGISTER_001.md`

The energy source register defines controlled inputs including usable energy, degradation state, propulsion consumption, payload/equipment consumption, airspeed, groundspeed, wind, UAV configuration, return route, required reserve/margin and input validity. Numerical values remain TBD until their engineering/manufacturer/test basis is established.

## 4. Existing verification overlap

Existing `TEST-001`, `TEST-002` and `TEST-003` cover upstream task/template/capability formation and are retained as existing verification identities. They are not repurposed as replacements for V-M01-01 or V-M02-01 because the MT cases verify the complete planning algorithms and their controlled execution configuration.

## 5. Actual execution-data status

### MT01-T01

Scenario identity exists, but the repository currently does not provide a complete controlled execution package containing all required:
- AOI geometry and CRS;
- terrain/elevation source and version;
- obstacle/restriction state;
- authorization state;
- weather/wind snapshot;
- selected UAV configuration revision;
- equipment configuration/calibration revision;
- battery/SOC/SOH/degradation state;
- C2 state;
- mission quality profile;
- planner/algorithm configuration;
- execution environment;
- expected result bound to those exact inputs.

**Disposition: CONFIGURATION GAP — DO NOT EXECUTE.**

### MT02-T21

Scenario identity exists, but the repository currently does not provide a complete controlled execution package containing all required:
- target surface/mesh/point-cloud representation and revision;
- CRS/reference frame;
- terrain/obstacle/restriction state;
- authorization state;
- weather/wind snapshot;
- selected UAV configuration revision;
- camera/gimbal/sensor/calibration revision;
- battery/SOC/SOH/degradation state;
- C2 state;
- reconstruction quality profile;
- planner/algorithm configuration;
- execution environment;
- expected result bound to those exact inputs.

**Disposition: CONFIGURATION GAP — DO NOT EXECUTE.**

## 6. No-invention rule

The following are deliberately **not** fabricated by this binding record:

- AOI coordinates;
- target geometry;
- terrain values;
- obstacle values;
- weather/wind values;
- battery state;
- degradation coefficient;
- energy reserve;
- sensor calibration;
- GSD acceptance threshold;
- overlap acceptance threshold;
- navigation accuracy threshold;
- execution-environment values.

Where a controlled source has not supplied the value for the actual test configuration, the value remains OPEN/TBD.

## 7. Current gate

```
Requirement identity: RESOLVED
Requirement wording: RESOLVED
Requirement → MT-01/MT-02 allocation: RESOLVED
Case identity: RESOLVED
Dataset identity: RESOLVED
Controlled source mapping: RESOLVED
Actual execution configuration: OPEN
Execution: NOT DONE
Evidence: OPEN
Deterministic replay execution: NOT DONE
Certification closure: OPEN
MT-03: BLOCKED
```

## 8. Next deterministic operation

Construct the actual controlled execution package for **MT01-T01 first**, using only existing versioned repository data or explicitly qualified external source data. Do not execute V-M01-01 until that package is complete.

After V-M01-01 configuration is closed, construct MT02-T21 configuration and proceed to V-M02-01.

