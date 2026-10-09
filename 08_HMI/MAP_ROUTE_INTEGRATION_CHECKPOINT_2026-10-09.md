# Map Route Integration — Checkpoint 2026-10-09

## Status

**IN PROGRESS — geometry serialization and request attachment are implemented and CI passes; runtime ownership and end-to-end selected-route wiring remain missing.**

## Branch

`fix/hmi-map-tile-loading-2026-10-09`

## Confirmed architecture

- Canonical geographic route type is `bluesky::planning::Route` in `04_SOFTWARE/PLANNING/model/route_model.hpp`. It contains ordered waypoints with WGS84 latitude/longitude, altitude, mandatory flags, route identity/version and lineage.
- `SelectedRouteSet` in `04_SOFTWARE/PLANNING/selected_route_set.hpp` records mission/version, selected candidate and `route_elements` as strings. It does **not** itself contain waypoint coordinates.
- `Mission.selected_solution` in `04_SOFTWARE/PLANNING/model/mission_model.hpp` stores a selected candidate reference, not route geometry.
- `FlightProfileBuilder::build(selected, route, ...)` in `04_SOFTWARE/PLANNING/flight_profile.hpp` accepts both the selected-route metadata and canonical `Route`, then projects the route into profile points. This is a domain-level API, not evidence that the HMI currently owns or receives the selected `Route`.
- The JSONL Planning Core adapter at `schemas/validator/planning_core_stdio.py` reconstructs validation routes from local `x/y` coordinates. It does not derive WGS84 geometry from those local coordinates.
- `schemas/planning-core-request.schema.json` permits optional `inputs.routeGeometry` in WGS84. The adapter forwards it to the result contract.
- `PlanningBridge` can start a process, send JSONL and publish received results.
- `MainContent.qml` exposes `planningBridge.result` to the mission profile/map route chain.
- No HMI call site was found that obtains the actual selected canonical C++ `Route`, assembles the complete Planning Core request, attaches route geometry and sends it through `PlanningBridge.sendRequest()`.

## Coordinate and ownership boundary

Do not convert the adapter's local `x/y` values into latitude/longitude by renaming fields. Their coordinate reference and transform are not established in the inspected request path.

Do not synthesize a geographic `Route` from `SelectedRouteSet.route_elements` alone: those are string references, not coordinates. The integration point must obtain the canonical route object from the authoritative route-generation/selection lifecycle, or introduce an explicit, tested resolver from selected route elements to that object.

## Map preview-route correction

- Found a misleading visual fallback: `GoogleMapView.qml` contained five hardcoded Moscow-area coordinates, and `MissionProfileWindow.qml` fell back to the design-time table coordinates whenever Planning Core geometry was absent.
- Removed both paths from map rendering. The map now receives an empty route until `planningResult.result.routeGeometry` contains at least two valid WGS84 points.
- The mission profile's design-time table fixture remains a UI preview only; it is no longer promoted to the geographic map as if it were the selected mission route.
- This is a correctness fix, not proof of live end-to-end route integration. The map may now correctly show no route until the request owner is wired to send canonical geometry.

## Latest planning-domain hand-off work

- Added `04_SOFTWARE/PLANNING/canonical_route_builder.hpp/.cpp`: resolves `SelectedRouteSet.route_elements` against the authoritative `PlanningGraph` node IDs and constructs the canonical geographic `Route` from each node's `GeoPoint` and altitude.
- The resolver preserves mission/candidate lineage and selected route order, creates adjacent segments with great-circle distances, marks graph start/goal points mandatory, and rejects missing/duplicate nodes, invalid WGS84 coordinates, non-finite altitude, missing lineage and routes shorter than two points.
- Follow-up hardening also requires selected endpoints to match the graph start/goal and every adjacent waypoint pair to have a directed, finite, non-negative graph edge. Tests cover disconnected edges and mismatched endpoints.
- Added `canonical_route_builder_test.cpp` for successful mapping and negative cases; registered source and test executable in `04_SOFTWARE/PLANNING/CMakeLists.txt`.
- This closes the domain-level conversion from a selected candidate's graph-node IDs into canonical route geometry. It does **not** yet connect a live selected route into the HMI or PlanningBridge request path.

## Changes on this branch

- `schemas/planning-core-request.schema.json`: optional `routeGeometry` request shape.
- `schemas/validator/planning_core_stdio.py`: forwards request geometry into the result contract.
- `schemas/validator/planning_bridge_contract.py`: validates and serializes route ID/version, WGS84 coordinates, altitude and mandatory flags.
- `qt/BlueSkyPRO-HMI/src/PlanningBridge.cpp`: validates result route geometry before publishing it.
- QML: route geometry can flow from `planningBridge.result` through `MissionProfileWindow` and `FlightChart` to `GoogleMapView`.
- `qt/BlueSkyPRO-HMI/src/RouteGeometrySerializer.h/.cpp`: converts canonical `Route` into `routeGeometry` and attaches it to an already assembled `planning.request`, preserving existing request inputs and rejecting invalid envelopes/routes without modifying the request.
- `qt/BlueSkyPRO-HMI/tests/RouteGeometrySerializerTest.cpp`: contract test covers serialization, preservation of existing inputs, and rejection cases.
- `.github/workflows/bluesky-pro-hmi.yml`: runs CTest after the HMI build.
- `qt/BlueSkyPRO-HMI/CMakeLists.txt`: includes serializer in HMI target and registers its contract test.

## Verification

- HMI build and HMI contract tests passed: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37913107681.
- Planning benchmark build/tests passed: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37913107679.
- Multi-UAV validator regression checks passed: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37913107676.
- The initial canonical-route-builder commits passed Planning Benchmark and Multi-UAV Validator at SHA `517e19f1fa35df7d4aacd6cc3d1bbcedcecffea2`; the HMI Qt Build was still running at last check. Subsequent endpoint/connectivity hardening at SHA `6992729c73ff31f0fdf20873c1b961d44bf89881` has new CI runs pending/in progress (for example 37915287102, 37915277546, 37915277378). Do not treat the latest tests as passed until these runs finish.
- These CI results verify builds and automated tests, not a live mission route flowing to the map.
- Local Windows runtime has not been verified.
- End-to-end canonical Route -> request -> Planning Core -> PlanningBridge -> map remains **NOT IMPLEMENTED / NOT VERIFIED**.

## Next deterministic work item

1. Run CI for `CanonicalRouteBuilder`; fix only failures demonstrated by the build/tests.
2. Identify the owner that assembles the complete Planning Core request and give it access to the selected route's authoritative `PlanningGraph` and `SelectedRouteSet`; resolve the canonical `Route` there using the new builder.
3. At that request owner, call `attachRouteGeometryToRequest()` and send the JSONL through `PlanningBridge.sendRequest()`.
4. Add an integration test that exercises the request through the JSONL adapter and verifies `result.routeGeometry` reaches the QML map binding.
5. Verify on the Windows project build. Do not claim map integration complete until a real selected route is exercised end to end.
