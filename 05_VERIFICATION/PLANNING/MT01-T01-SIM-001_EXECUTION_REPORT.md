---
id: MT01-T01-SIM-001-EXECUTION-REPORT
type: simulation_execution_report
status: SIMULATION_PASS_OPERATIONAL_CASE_BLOCKED
dataset_id: MT01-T01-SIM-001
verification_case: V-M01-01-SIM-001
parent_verification_case: V-M01-01
execution_environment: GitHub Actions Ubuntu / CMake / CTest
execution_commit: 3cf2b3a757cd6d623d3dffc8540a1c332d0dcccb
---

# MT01-T01-SIM-001 — Execution Report

## 1. Result

**Representative simulation: PASS.**  
**Parent operational case V-M01-01 / MT01-T01: BLOCKED — NOT EXECUTED.**

- CI workflow: [BlueSky Planning Benchmark run #367](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37979485939)
- Exact tested commit: 3cf2b3a757cd6d623d3dffc8540a1c332d0dcccb
- Build: PASS
- CTest: 41/41 passed, 0 failed
- Dedicated test: mt01_t01_representative_simulation_test — PASS
- The workflow printed the metrics below after CTest completed.

## 2. Observed outputs from the executable

| Output | Observed value |
|---|---:|
| Scenario | V-M01-01-SIM-001 |
| Dataset | MT01-T01-SIM-001 |
| Execution class | REPRESENTATIVE_SIMULATION_NOT_FLIGHT_EVIDENCE |
| UAV reference | DJI-MAVIC-3E-REFERENCE-SIM |
| Synthetic AOI area | 8041.8555 m² |
| Calculated horizontal GSD | 0.0268 m/px |
| Calculated vertical GSD | 0.0267 m/px |
| Calculated footprint width | 141.5785 m |
| Calculated footprint height | 105.7771 m |
| Calculated track spacing | 56.6314 m |
| Calculated image spacing | 26.4443 m |
| Calculated trigger interval | 2.6444 s |
| Orientation candidates | 1 |
| Coverage cells | 2 |
| Coverage tracks | 2 |
| Reported edge gaps | 0 |
| Acquisition events | 6 |
| Calculated coverage ratio | 1.0000 |
| Calculated uncovered area | 0.0000 m² |
| Transition edges | 2 |
| Route candidates | 2 |
| Selected route | MT01-ROUTE-0 |
| Selected route time | 6.9719 s |
| Selected route energy | 0.4009 Wh |
| Remaining energy under test model | 59.5991 Wh |
| Simulation acceptance | PASS |
| Operational V-M01-01 | BLOCKED_NOT_EXECUTED |

These values are the program's outputs for the named synthetic fixture. They are not measured flight results. In particular, the 6.97-second time and 0.4009 Wh energy are outputs of simplified route/performance coefficients, not validated Mavic 3E performance estimates.

## 3. What this test demonstrates

For the fixed synthetic inputs, the connected component path successfully:
1. calculates camera geometry and GSD using model-level DJI camera dimensions;
2. generates an orientation candidate;
3. decomposes the AOI into two planning cells;
4. generates two coverage tracks and six acquisition-event records;
5. evaluates the edge/coverage model and reports 100% estimated coverage for this fixture;
6. builds a two-edge transition graph and two route candidates;
7. evaluates candidates using a constant synthetic wind and simplified energy coefficients;
8. selects a feasible candidate while preserving the test-only energy reserve.

The acceptance assertions passed, including GSD <= 3 cm/px, coverage ratio >= 0.95, no invalid acquisition events, trigger interval >= 0.7 s, feasible route selection and remaining energy >= the 15 Wh test reserve.

## 4. Limitations and anomalies

- This is a deterministic software simulation, not HIL, a flight test, or processing of real captured imagery.
- AOI coordinates and terrain are synthetic; the terrain is represented only by a fixed mean value. No DEM raster or terrain-following calculation is executed.
- The weather vector is a constant synthetic mean; no time-matched forecast or reanalysis data are loaded.
- The restriction list is intentionally empty in the fixture. This is not actual NOTAM or airspace clearance.
- The DJI camera parameters are model-level published specifications. No installed-unit intrinsic calibration or lens-distortion correction is supplied.
- The reported edge-gap count is from the current edge evaluator; it does not prove actual image boundary/corner coverage after acquisition.
- The mapping coverage ratio is a planning geometry estimate, not image-derived completeness. Actual overlap, georeferencing accuracy, GCP/RTK/PPK, orthomosaic/DSM quality and image-sequence completeness are not tested.
- Final mission integrity, authorization/readiness release, actual battery state and C2 readiness are outside this fixture.

## 5. Disposition

Keep the simulation result as supporting evidence for the code path only. Do not mark the parent controlled V-M01-01 / MT01-T01 as passed. That parent case remains blocked until its real controlled input set, aircraft instance, calibration, environmental snapshot and mission-specific acceptance profile are bound.

The simulation is useful precisely because it exposes the current pipeline's outputs and boundaries without pretending that synthetic inputs establish operational safety.
