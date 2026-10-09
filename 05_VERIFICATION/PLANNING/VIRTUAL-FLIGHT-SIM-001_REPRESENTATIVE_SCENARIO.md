# Virtual Flight — Representative Simulation Scenario SIM-001

## Purpose

Provide a repeatable first end-to-end UI exercise of the MT-01 representative planning chain: run the existing C++ planning test executable, serialize the selected candidate's calculated coverage-track coordinates, deliver its JSON result through `PlanningBridge`, and display those coordinates and summary indicators in the Virtual Flight workspace.

This is a simulation/UI integration fixture. It is not flight evidence and does not authorize a real mission.

## UAV reference

- Model: DJI Mavic 3 Enterprise (reference model; not a specific physical aircraft).
- Wide camera: 4/3 CMOS, 20 MP, image size 5280 × 3956, mechanical shutter supported.
- Published reference: https://enterprise.dji.com/mavic-3-enterprise/specs
- The planning geometry also uses a physical focal length of 12.29 mm as recorded in the scenario source. No installed-camera intrinsic calibration, principal-point correction, lens-distortion correction, RTK/PPK solution, or image-derived quality evidence is supplied.

## Synthetic AOI and altitude

- Coordinate system: WGS84 latitude/longitude.
- AOI corners: (59.0000, 30.0000), (59.0000, 30.0014), (59.0009, 30.0014), (59.0009, 30.0000).
- Flat synthetic terrain reference: 25 m; nominal camera-to-surface distance: 100 m; synthetic datum altitude: 125 m.
- These are test coordinates and assumptions, not a surveyed or operational site.
- Route polyline points come from the selected candidate's calculated coverage tracks. Straight connecting segments between tracks are display connectors; this fixture does not claim obstacle-aware transit geometry between tracks.

## Synthetic weather profile

| Parameter | Fixture value | Use / qualification |
|---|---:|---|
| Temperature | 15 °C | Metadata only |
| North wind component | 0.0 m/s | Consumed by the simplified trajectory model |
| East wind component | 1.5 m/s | Consumed by the simplified trajectory model |
| Vertical wind component | 0.0 m/s | Consumed by the simplified trajectory model |
| Precipitation | 0.0 mm/h | Metadata only |
| Cruise airspeed | 10 m/s | Simulation assumption |
| Wind tolerance | 8 m/s | Simulation assumption |
| Usable energy | 60 Wh | Synthetic battery model |
| Reserve requirement | 15 Wh | Synthetic battery model |

This is a representative synthetic profile, not a historical or current weather observation. Open-Meteo historical weather can be considered for a later controlled dataset: https://open-meteo.com/en/docs/historical-weather-api. The present run does not fetch that service.

## Synthetic NOTAM-like restriction

- ID: `SIM-NOTAM-001`
- Source: `VIRTUAL-FLIGHT-SYNTHETIC`
- Geometry: polygon immediately east of the AOI, outside the test mapping boundary.
- Vertical interval: 0–500 m.
- Purpose: exercise the environment snapshot and expose a restriction indicator in the UI.
- Status: synthetic only; it does not represent a real NOTAM, prove that the area is clear, or replace a NOTAM/AIS and geozone check.

NOTAMs are operational notices, not statistical averages. This scenario therefore uses one explicitly synthetic sample restriction instead of inventing a “mean NOTAM”.

## Mapping assumptions

- Target GSD: 0.030 m/px.
- Front overlap: 75%.
- Side overlap: 60%.
- Candidate orientation search: 0° only.
- The current test reports estimated geometric coverage. It does not validate captured-image overlap, orthomosaic/DSM quality, or terrain-following.

## Run in the desktop environment

Build the planning executable from the repository root:

```powershell
cmake -S 04_SOFTWARE/PLANNING -B build/planning -DCMAKE_BUILD_TYPE=Release
cmake --build build/planning --target mt01_t01_representative_simulation_test --parallel
$env:BLUESKY_VIRTUAL_FLIGHT_RUNNER = (Resolve-Path ".\build\planning\mt01_t01_representative_simulation_test.exe").Path
$env:BLUESKY_YANDEX_MAPS_API_KEY = "<your existing Yandex Maps key>"
```

Launch `appBlueSkyPRO.exe` / Qt Design Studio from the same PowerShell session so the environment variables are inherited. Open **VIRTUAL FLT** and select **RUN SIMULATION**. If the runner executable is not beside the HMI executable, set `BLUESKY_VIRTUAL_FLIGHT_RUNNER` to its full path as shown above.

The map still needs a valid Yandex Maps API key to load its basemap. The route result is delivered separately from map tile loading; a missing key must remain visible as a map error, not be interpreted as a planning failure.

## Expected result and interpretation

- JSON is emitted as a single `planning.result` line and validated by `PlanningBridge`.
- The selected coverage-track coordinates are shown on the geographic map.
- The Virtual Flight panel displays the UAV reference, synthetic weather, synthetic restriction ID, GSD, estimated coverage, track count, and modelled route energy.
- `simulationAcceptance=PASS` means this representative planning simulation passed its programmed assertions.
- `releaseStatus=BLOCKED`, `verified=false`, and `operationalStatus=BLOCKED_NOT_EXECUTED` remain mandatory. A simulation pass must never imply operational readiness.

## Known limitations

The fixture does not provide current NOTAMs, validated operational airspace restrictions, a real DEM/obstacle raster, aircraft-specific performance curves, real battery telemetry, actual camera calibration, image capture, image-derived overlap, or post-flight product QA. It does not yet test route avoidance around an intersecting restriction or generate detour waypoints. The operational verification case `V-M01-01` remains blocked.
