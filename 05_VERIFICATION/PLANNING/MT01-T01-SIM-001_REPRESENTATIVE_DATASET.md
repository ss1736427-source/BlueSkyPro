---
id: MT01-T01-SIM-001
type: representative_simulation_dataset
status: TEST_FIXTURE_ONLY
parent_dataset: MT01-T01
verification_case: V-M01-01-SIM-001
parent_verification_case: V-M01-01
operational_use: PROHIBITED
---

# MT01-T01-SIM-001 — Representative MT-01 Simulation Dataset

## Purpose and status

This is a reproducible representative simulation fixture to exercise the existing MT-01 C++ planning components together. It is not operational data and does not unblock the real V-M01-01 / MT01-T01 case.

The executable is 04_SOFTWARE/PLANNING/mt01_t01_representative_simulation_test.cpp and is registered in CTest. Its code constants are the source of truth for values consumed by the test.

## Aircraft/camera model reference

Candidate model: DJI Mavic 3 Enterprise (Mavic 3E). This is a model-level reference, not a selected aircraft instance.

| Parameter | Value | Provenance |
|---|---:|---|
| Sensor | 4/3 CMOS, 20 MP | Official DJI specifications |
| Image dimensions | 5280 x 3956 px | Official DJI specifications |
| Sensor dimensions | 17.4 x 13.0 mm | Official DJI camera sensor parameter article |
| Physical focal length | 12.29 mm | Official DJI camera sensor parameter article |
| Mechanical shutter | Supported | Official DJI specifications |
| Timed capture interval | 0.7 s listed mode | Official DJI specifications |
| Intrinsic calibration, principal point, lens distortion | Not provided | Not modelled; ideal pinhole geometry only |

Sources: [DJI Mavic 3 Enterprise specifications](https://enterprise.dji.com/mavic-3-enterprise/specs), [DJI user manual](https://dl.djicdn.com/downloads/DJI_Mavic_3_Enterprise/20240814/DJI_Mavic_3E_3T_User_Manual_EN.pdf), [DJI camera sensor parameter article](https://repair.dji.com/help/content?customId=01700007368&documentType=&lang=zh-CN&paperDocType=ARTICLE&re=CN&spaceId=17).

Physical focal length is used in GSD calculations. The 24 mm equivalent focal length must not be substituted for the physical focal length. Published model data do not constitute calibration of a specific installed camera.

## Synthetic AOI and environment

| Parameter | Fixture value | Provenance / limitation |
|---|---|---|
| AOI polygon | (59.0000,30.0000), (59.0000,30.0014), (59.0009,30.0014), (59.0009,30.0000) | Synthetic geometry anchor only; not a surveyed or authorized site |
| CRS | WGS 84 latitude/longitude | Test-model convention; no external AOI source file |
| Terrain | Flat synthetic mean elevation 25 m | Assumption; no DEM raster is loaded |
| Camera-to-surface distance | 100 m | Assumption; terrain-following is not tested |
| Nominal altitude | 125 m synthetic datum | Assumption; not a regulatory altitude |
| Obstacles | None in fixture | Does not imply real obstacles are absent |
| Restriction list | Empty synthetic snapshot | Does not imply actual airspace is clear |
| Wind | 0.0 m/s north, 1.5 m/s east, 0.0 m/s vertical | Synthetic constant mean, not measured or time-matched weather |
| Temperature / precipitation | 15 C / 0 mm per hour | Metadata-only assumptions, not consumed by this test |
| C2 | Available in simulation | No real telemetry or radio-link budget |
| Battery SOC / SOH | 80% / 95% metadata only | Assumptions, not measurements |
| Usable energy / protected reserve | 60 Wh / 15 Wh | Test-only energy inputs, not operational battery limits |
| Cruise airspeed / wind tolerance | 10 m/s / 8 m/s | Test assumptions, not aircraft limits |
| Energy coefficients | 0.005 Wh/m horizontal, 0.05 Wh/m climb, 0.02 Wh/m descent | Simplified simulation coefficients |

External datasets that could support a future real historical snapshot are documented at [Open-Meteo Historical Weather API](https://open-meteo.com/en/docs/historical-weather-api) and [Copernicus DEM](https://dataspace.copernicus.eu/explore-data/data-collections/copernicus-contributing-missions/collections-description/COP-DEM). The fixture values above are not claimed to have been retrieved from either source.

## Mapping parameters

| Parameter | Fixture value | Provenance |
|---|---:|---|
| Target GSD | 0.030 m/px | Simulation acceptance threshold |
| Minimum planned coverage ratio | 0.95 | Simulation acceptance threshold |
| Forward overlap | 75% | Project baseline assumption |
| Side overlap | 60% | Project baseline assumption |
| Ground speed | 10 m/s | Simulation assumption |
| Orientation search | 0 degrees only | Simulation assumption; not orientation optimization |
| Positional accuracy / GCP / RTK / PPK | Not modelled | No georeferencing evidence |
| Image matching / orthomosaic / DSM QA | Not modelled | No imagery or reconstruction engine in this test |

## Connected software chain

The executable invokes these components in sequence:
1. Acquisition geometry / GSD / footprint / trigger timing.
2. Coverage orientation.
3. AOI decomposition.
4. Coverage track generation.
5. Edge-gap evaluation.
6. Acquisition-event generation.
7. Mapping-quality evaluation.
8. Transition graph.
9. Route-candidate generation.
10. Wind/energy trajectory evaluation.
11. Route selection by energy, then time.

It asserts that the component results are valid, the calculated GSD and coverage satisfy this fixture's thresholds, the trigger interval is not faster than the published 0.7 s model-level mode, a feasible route is selected, and the remaining energy preserves the test-only 15 Wh reserve. It prints the observed metrics to the CI log.

This is a connected C++ simulation of the listed components, not a full application execution. It does not exercise final mission-integrity/release authorization, a real terrain raster, NOTAM services, aircraft telemetry, real image capture, image matching or final mapping-product accuracy.

## Evidence classification

- V-M01-01-SIM-001: **SIMULATION PASS** on [GitHub Actions run #367](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37979485939), exact commit 3cf2b3a757cd6d623d3dffc8540a1c332d0dcccb.
- Build and CTest: passed; 41/41 tests, zero failures.
- Observed metrics and limitations are recorded in [MT01-T01-SIM-001_EXECUTION_REPORT.md](MT01-T01-SIM-001_EXECUTION_REPORT.md).
- Parent V-M01-01 / MT01-T01: remains BLOCKED_MISSING_CONTROLLED_INPUTS / NOT EXECUTED until the real controlled inputs and configuration are provided.
- A passing simulation proves only that this code path ran and met assertions for this fixture. It cannot prove real-aircraft readiness, legal clearance, camera calibration, real weather suitability or operational flight performance.
