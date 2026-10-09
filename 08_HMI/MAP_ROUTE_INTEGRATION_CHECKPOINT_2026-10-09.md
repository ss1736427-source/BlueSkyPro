# Map Route Integration — Checkpoint 2026-10-09

## Status

**IN PROGRESS — canonical Route geometry serializer is implemented; real selected-route request wiring is still missing.**

## Branch

`fix/hmi-map-tile-loading-2026-10-09`

## Confirmed current state

- Canonical planning-domain route model exists at `04_SOFTWARE/PLANNING/model/route_model.hpp`. It contains ordered waypoints, `GeoPoint { latitude_deg, longitude_deg }`, altitude, mandatory status, route identity/version and lineage.
- The JSONL Planning Core adapter at `schemas/validator/planning_core_stdio.py` reconstructs execution-validation inputs from local `x/y` points. It does not derive WGS84 route geometry from those local coordinates.
- The request schema `schemas/planning-core-request.schema.json` permits optional `inputs.routeGeometry` in WGS84.
- The adapter passes that field to `build_result_message`; the bridge result schema supports `result.routeGeometry`.
- `PlanningBridge` can start a process, send JSONL and publish received results.
- `MainContent.qml` exposes `planningBridge.result` to the mission profile/map route chain.
- The HMI still has no discovered call site that obtains the actual selected canonical C++ `Route`, builds the complete Planning Core request, and sends it through `PlanningBridge.sendRequest()`.

## Safety/coordinate finding

Do not convert the adapter's local `x/y` values into latitude/longitude by renaming fields. The coordinate reference and transform are not established in the inspected request path. The canonical `Route` model already represents geographic points, but the selected route's runtime owner is not yet wired to the HMI bridge.

## Changes on this branch

- `schemas/planning-core-request.schema.json`: optional `routeGeometry` request shape.
- `schemas/validator/planning_core_stdio.py`: forwards request geometry into the result contract.
- `schemas/validator/planning_bridge_contract.py`: validates and serializes route ID/version, WGS84 coordinates, altitude and mandatory flags.
- `qt/BlueSkyPRO-HMI/src/PlanningBridge.cpp`: validates result route geometry before publishing it.
- QML: route geometry can flow from `planningBridge.result` through `MissionProfileWindow` and `FlightChart` to `GoogleMapView`.
- `qt/BlueSkyPRO-HMI/src/RouteGeometrySerializer.h/.cpp`: new serializer converts canonical `Route` into the request's `routeGeometry` object; preserves order, IDs, route version, WGS84 coordinates, altitude and mandatory flags; rejects missing route identity, fewer than two points, empty/duplicate waypoint IDs, invalid coordinates and non-finite altitude.
- `qt/BlueSkyPRO-HMI/tests/RouteGeometrySerializerTest.cpp`: contract test for serialization and rejection cases.
- `qt/BlueSkyPRO-HMI/CMakeLists.txt`: includes serializer in the HMI target and registers the contract test with CTest.

## Verification status

- Request and result schema structure was inspected.
- Serializer and test sources are committed, but **the new C++ test has not yet been run in the project environment**.
- Qt build and runtime have not been verified after these commits.
- CI for the latest commits is not yet confirmed.
- End-to-end canonical Route -> request -> Planning Core -> PlanningBridge -> map remains **NOT IMPLEMENTED / NOT VERIFIED**.

## Next deterministic work item

1. Find or define the runtime owner of the selected canonical `Route` and its lifecycle in the mission workflow.
2. At that owner, call `serializeRouteGeometry()`, merge the resulting object into a complete schema-valid `planning.request` under `inputs.routeGeometry`, and send the request via `PlanningBridge.sendRequest()`.
3. Do not invent other required Planning Core inputs; source them from the existing authoritative planning pipeline.
4. Add an integration test exercising the request through the JSONL adapter and assert that the returned `result.routeGeometry` reaches the map.
5. Run CMake build, CTest and CI. Do not claim map integration complete until a real selected route is exercised end to end.
