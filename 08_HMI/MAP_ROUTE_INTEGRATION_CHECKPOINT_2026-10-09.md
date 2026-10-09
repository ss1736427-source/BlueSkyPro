# Map Route Integration — Checkpoint 2026-10-09

## Status

**IN PROGRESS — contract path exists; canonical route producer is not wired.**

## Branch

`fix/hmi-map-tile-loading-2026-10-09`

## Confirmed current state

- Canonical planning-domain route model exists at `04_SOFTWARE/PLANNING/model/route_model.hpp`. It contains ordered waypoints, `GeoPoint { latitude_deg, longitude_deg }`, altitude, mandatory status, route identity/version and lineage.
- The JSONL Planning Core adapter at `schemas/validator/planning_core_stdio.py` currently reconstructs execution-validation inputs from local `x/y` points. It does not derive WGS84 route geometry from those local coordinates.
- The request schema `schemas/planning-core-request.schema.json` now permits optional `inputs.routeGeometry` in WGS84.
- The adapter passes that field to `build_result_message`; the bridge result schema supports `result.routeGeometry`.
- `PlanningBridge` can start a process, send JSONL and publish received results.
- `MainContent.qml` exposes `planningBridge.result` to the mission profile/map route chain.
- The HMI does not currently contain a discovered call site that builds a planning request from the canonical C++ `Route` and sends it via `PlanningBridge.sendRequest()`.

## Safety/coordinate finding

Do not convert the adapter's local `x/y` values into latitude/longitude by renaming fields. The coordinate reference and transform are not established in the inspected request path. Canonical `Route` already represents geographic points, but there is no identified producer that serializes this model into the Planning Core request.

## Changes already present on this branch

- `schemas/planning-core-request.schema.json`: optional `routeGeometry` request shape.
- `schemas/validator/planning_core_stdio.py`: forwards request geometry into the result contract.
- `schemas/validator/planning_bridge_contract.py`: validates and serializes route ID/version, WGS84 coordinates, altitude and mandatory flags.
- `qt/BlueSkyPRO-HMI/src/PlanningBridge.cpp`: validates result route geometry before publishing it.
- QML: route geometry can flow from `planningBridge.result` through `MissionProfileWindow` and `FlightChart` to `GoogleMapView`.

## Verification status

- Request and result JSON schemas parse: confirmed by repository content check.
- Adapter integration test source includes route-geometry assertions: confirmed.
- Python test execution: not run in the current environment.
- Qt/MSVC build and runtime: not run in the current environment.
- CI for latest checkpoint: not confirmed.
- End-to-end canonical Route -> request -> Planning Core -> PlanningBridge -> map: **NOT IMPLEMENTED / NOT VERIFIED**.

## Next deterministic work item

Implement a single explicit serialization boundary from the canonical `Route` model to the JSONL request. It must:
1. preserve route ID/version, ordered waypoint IDs, WGS84 latitude/longitude, altitude and mandatory flag;
2. refuse to serialize if route coordinates are missing or invalid;
3. keep local `x/y` planning geometry separate unless its coordinate reference and transform are explicitly supplied;
4. include the serialized geometry in the actual request sent through `PlanningBridge.sendRequest()`;
5. add a focused round-trip/integration test and then build/test in the project environment.

Do not claim map integration complete until this path is exercised by a real selected route.
