# MT-01-T01 — Controlled Execution Package

**Package ID:** `MT01-T01`  
**Verification case:** `V-M01-01`  
**Allocated requirements:** `SYS-REQ-008`, `SYS-REQ-035`, `SYS-REQ-076`  
**Package status:** `BLOCKED_MISSING_CONTROLLED_INPUTS`  
**Execution status:** `NOT_EXECUTED`  
**Evidence status:** `NO_EXECUTION_EVIDENCE`  
**Review baseline:** BlueSkyPro branch `docs/mt01-mapping-algorithm-001`, review head `a84c26a30cf591ffc68d8c44d58ba3491679130a` (2026-10-09)

## 1. Purpose and status semantics

This record defines the controlled input/evidence chain required to execute `V-M01-01` against `MT01-T01`. It is a package readiness record, not a substitute for the missing dataset and not a test result.

The execution is deliberately **blocked**. The repository contains algorithm specifications and a manufacturer-reference catalogue, but the reviewed project data does not provide a complete, versioned operational Area of Interest (AOI), environment snapshot, aircraft instance/readiness state, camera calibration instance, mission quality profile and execution configuration for this case.

No synthetic AOI, assumed CRS, invented wind, placeholder battery state, arbitrary target GSD or fabricated restriction set may be substituted. Manufacturer maximums are not current vehicle state or mission-specific limits. A test fixture is not evidence that a real mission is ready to fly.

## 2. Controlled requirement and design basis

| Chain element | Controlled reference | What it establishes | What it does not establish |
|---|---|---|---|
| Requirement | `02_SYSTEM/Requirements/SYS-REQ-008.md` — `SYS-REQ-008` | Mission readiness must summarize readiness areas including airspace, terrain, geofence, weather, C2, fleet coordination, energy and contingency | No actual mission-specific PASS/WARNING state or readiness data |
| Requirement | `02_SYSTEM/Requirements/SYS-REQ-035.md` — `SYS-REQ-035` | Task-to-capability mapping must resolve task type, capabilities, payload, algorithms, autonomy, communications and success criteria | No selected mission task instance or approved acceptance criteria |
| Requirement | `02_SYSTEM/Requirements/SYS-REQ-076.md` — `SYS-REQ-076` | Each UAV must expose a machine-readable capability/constraint profile, including endurance, speed, navigation, communications, sensing, current energy, health and mission load | No identified aircraft instance, current state or mission-specific capability snapshot |
| Algorithm | `docs/algorithm/MT-01_MAPPING_MISSION_ALGORITHM.md` | Intended MT-01 input, acquisition, coverage, route, performance, hard-gate and QA behavior | Does not prove implementation or provide the missing controlled inputs |
| Acquisition design | `docs/algorithm/PHOTOGRAMMETRY_SOLUTION_METHOD.md` | Camera/payload fields and planning rules; overlap baselines and need to calculate footprint from the selected camera/surface geometry | Not a mission-specific camera calibration or quality profile |
| Equipment reference | `02_SYSTEM_DESIGN/ADMINISTRATOR/UAV_OFFICIAL_REFERENCE_CATALOG_EXPANSION_002.md` | Manufacturer-reference facts for named UAV variants; the catalogue classifies source values and explicitly distinguishes UNKNOWN/DERIVED | Not an aircraft instance, installed payload confirmation, battery health record or flight authorization |
| External manufacturer reference | [DJI Mavic 3 Enterprise official specifications](https://enterprise.dji.com/mavic-3-enterprise/specs) | Mavic 3E published aircraft and camera specifications | Does not establish that a Mavic 3E is selected/available, its actual calibration, current condition, local operating conditions or mission approval |

### 2.1 Candidate equipment reference — not selected equipment

The official [DJI Mavic 3 Enterprise specifications](https://enterprise.dji.com/mavic-3-enterprise/specs) identify the Mavic 3E wide camera as a 4/3 CMOS, effective 20 MP sensor, 5280 × 3956 maximum image size, 24 mm **equivalent** focal length, mechanical shutter with a published 8–1/2000 s range, and timed JPEG intervals including 0.7 s. The [DJI Mavic 3E/3T user manual](https://dl.djicdn.com/downloads/DJI_Mavic_3_Enterprise/20240814/DJI_Mavic_3E_3T_User_Manual_EN.pdf) states a 3.3 μm pixel pitch and 0.7-second interval shooting for the Mavic 3E wide camera. DJI's official [camera sensor parameter article](https://repair.dji.com/help/content?customId=01700007368&documentType=&lang=zh-CN&paperDocType=ARTICLE&re=CN&spaceId=17) publishes the Mavic 3E wide camera's sensor dimensions as 17.4 × 13 mm and actual focal length as 12.29 mm. The project's manufacturer-reference expansion also lists the Mavic 3E as a reference platform.

These manufacturer values are sufficient to define a **model-level reference camera record**, subject to versioned source capture and source-conflict review. They are not a substitute for the installed camera's calibration/intrinsic record or actual payload/firmware identity for a specific aircraft instance. The 24 mm value is an **equivalent** focal length and must not be inserted as physical focal length in a GSD formula. The manufacturer's maximum flight time and speed are not substitutes for current battery/health, payload-specific performance, reserve and environmental calculations.

## 3. Required package contents and current disposition

| ID | Required controlled artifact | Minimum required content | Current disposition |
|---|---|---|---|
| MT01-T01-IN-01 | Mission/task definition | Mission ID/version, task intent, product type, acceptance criteria and responsible operator | **MISSING** |
| MT01-T01-IN-02 | AOI geometry | Authoritative polygon/coverage boundary, source file/hash, CRS, units, geometry validity and version | **MISSING** |
| MT01-T01-IN-03 | Terrain/surface data | Source/version, horizontal and vertical reference, resolution/quality, validity and coverage of the AOI | **MISSING** |
| MT01-T01-IN-04 | Restrictions/environment | Airspace/geofence/NOTAM records and effective times, obstacles, authorization scope and provenance | **MISSING** |
| MT01-T01-IN-05 | Weather/wind | Time- and location-matched forecast/observation, source, validity interval, uncertainty and applicable operating limits | **MISSING** |
| MT01-T01-IN-06 | UAV instance/profile | Exact aircraft identity/configuration, firmware, capability profile, payload compatibility, speed/altitude limits and terrain-follow capability | **MISSING** |
| MT01-T01-IN-07 | Current energy/readiness | Battery identity, state of charge, state of health/degradation, current mission load, reserve policy and recovery feasibility inputs | **MISSING** |
| MT01-T01-IN-08 | Camera/payload instance | Exact installed camera, lens/focal configuration, image dimensions, pixel pitch or qualified intrinsic model, shutter/trigger limits, calibration and firmware identity | **PARTIAL REFERENCE ONLY** — manufacturer model data exist; installed instance and calibration are absent |
| MT01-T01-IN-09 | Mapping quality profile | Target GSD, required coverage, forward/side overlap, positional accuracy, georeferencing method, illumination/exposure limits and product QA thresholds | **MISSING** |
| MT01-T01-IN-10 | Launch/recovery and C2 | Launch/recovery locations, operational time window, C2 coverage/limits, contingency and return/recovery assumptions | **MISSING** |
| MT01-T01-IN-11 | Algorithm/execution configuration | Planner and geometry-engine version, configuration hash, parameter provenance, deterministic run settings and dependency versions | **MISSING** |
| MT01-T01-IN-12 | Acceptance/evidence plan | Expected artifacts, pass/fail thresholds, diagnostics, output schema, evidence destination and reviewer | **PARTIAL** — scenario intent exists in the MT-01 algorithm/traceability register; case-specific thresholds and evidence configuration are absent |

No artifact marked MISSING may be populated with a plausible-looking default. If a required input is not available from a controlled source, the package remains blocked and the owner/source needed to obtain it must be recorded.

## 4. Execution procedure once the inputs are controlled

1. Freeze the complete input set and record each source URI/path, version/hash, retrieval timestamp, validity interval and source classification.
2. Verify that the AOI, CRS, terrain/surface model, camera model and quality profile are mutually compatible. Stop on missing or ambiguous mandatory data.
3. Build the environment snapshot and constrained-open-space model from current restriction, terrain, obstacle and authorization evidence.
4. Validate the selected aircraft/payload instance and current readiness/energy inputs; do not use catalogue maxima as actual flight parameters.
5. Calculate GSD and camera-to-surface distance from the qualified physical camera model. Derive footprint, overlap spacing, trigger schedule and edge coverage from those controlled inputs.
6. Generate the coverage geometry and pass route/transition/return segments through the canonical constrained-open-space route pipeline.
7. Evaluate wind-adjusted performance, trajectory, storage/data volume, per-aircraft reserve and recovery feasibility.
8. Run the required hard gates. A failed or unavailable gate blocks release; soft objectives cannot override it.
9. Persist input hashes, algorithm/configuration versions, generated artifacts, diagnostics and test logs.
10. Compare the actual result with the case-specific acceptance criteria. Record PASS only when every required assertion has direct execution evidence; otherwise record FAIL, BLOCKED or UNVERIFIED with reasons.

## 5. Acceptance criteria for V-M01-01

The case is eligible to execute only when all mandatory inputs in Section 3 are controlled and mutually consistent. On execution, at minimum:

- the AOI is valid and its CRS/units are explicit;
- the system generates coverage geometry from the controlled camera/surface/quality inputs, with stable coverage-cell and track identifiers;
- edge/corner coverage is checked against the actual image footprint, not an arbitrary buffer;
- every generated route segment is checked by the canonical constrained-open-space pipeline against the frozen environment;
- camera trigger/overlap limits are checked against the selected payload's documented capability;
- wind/performance, energy reserve and recovery gates have evidence-backed results;
- all generated artifacts reference the same mission/input/configuration versions;
- any missing evidence or failed gate prevents a PASS/release claim.

These criteria are not evidence of a pass. They define what must be demonstrated when the package is executable.

## 6. Current result

**V-M01-01: BLOCKED — NOT EXECUTED.**

Reason: the repository review did not locate a complete controlled mission input package for `MT01-T01`. The manufacturer source supplies reference camera/aircraft facts, but not the AOI, terrain, restrictions, time-matched wind, selected aircraft instance, calibration, current battery/readiness, C2 state or mission-specific quality/acceptance profile.

The next valid action is to obtain and version these source artifacts, then freeze the package and execute the test. Do not mark `V-M01-01` PASS based on specification review, CI success, a synthetic fixture or manufacturer reference data alone.
