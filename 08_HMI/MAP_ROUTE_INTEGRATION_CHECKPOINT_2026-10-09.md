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

## Runtime ownership trace — follow-up

- `AlgorithmOrchestrator::solve()` validates and selects a candidate but returns only the selected solver/candidate IDs in `OrchestratorDecision`; it does not publish the concrete `Route` or `SelectedRouteSet` to the HMI.
- `MissionProblem` does carry a pointer to `PlanningGraph`, and `CandidateSolution.route_elements` contains graph-node IDs. This is sufficient for the new resolver only while both the selected candidate and its exact graph remain available in the planning runtime.
- `PlanningBridge` is exposed to QML and can send JSONL, but the inspected `MainContent.qml` has no request-assembly or `sendRequest()` call site. The application currently has no discovered runtime owner that combines authoritative graph/candidate data with the required safety and performance inputs for the complete request schema.
- Therefore, do not fabricate a request from the QML design fixtures. The remaining integration boundary is the application/planning runtime that owns both the selected candidate and graph, plus the authoritative request-input provider. It must create the canonical `Route`, attach it to the already valid request, then send it through the bridge.

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
- `qt/BlueSkyPRO-HMI/src/SelectedRouteMapHandoff.h/.cpp`: adds the missing domain-to-request handoff function. It accepts a selected candidate, the exact authoritative graph, mission metadata, and an already assembled request; rejects graph identity mismatch or non-feasible candidates; resolves `SelectedRouteSet` -> canonical `Route`; then attaches WGS84 geometry without inventing request inputs.
- `qt/BlueSkyPRO-HMI/tests/SelectedRouteMapHandoffTest.cpp`: verifies ordered graph-node IDs become ordered WGS84 points, existing authoritative inputs are preserved, and graph mismatch / infeasible candidate fail without mutating the request.
- `qt/BlueSkyPRO-HMI/CMakeLists.txt`: compiles the handoff and planning-domain resolver into HMI and registers `selected_route_map_handoff_contract`.
- `qt/BlueSkyPRO-HMI/tests/RouteGeometrySerializerTest.cpp`: contract test covers serialization, preservation of existing inputs, and rejection cases.
- `.github/workflows/bluesky-pro-hmi.yml`: runs CTest after the HMI build.
- `qt/BlueSkyPRO-HMI/CMakeLists.txt`: includes serializer in HMI target and registers its contract test.

## Verification

- HMI build and HMI contract tests passed: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37913107681.
- Planning benchmark build/tests passed: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37913107679.
- Multi-UAV validator regression checks passed: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37913107676.
- CI passed at SHA `15e4bda0d4c88613648262b80bb69dd4fd256759`: Planning Benchmark, Multi-UAV Validator, journal-service CI and HMI Qt Build. This includes the canonical route builder, graph-connectivity checks, and removal of hardcoded map preview geometry.
- These CI results verify builds and automated tests, not a live mission route flowing to the map.
- Local Windows runtime has not been verified.
- End-to-end canonical Route -> request -> Planning Core -> PlanningBridge -> map remains **NOT IMPLEMENTED / NOT VERIFIED**.

## Selected-route request handoff contract — follow-up

- Added `attachSelectedRouteGeometryToRequest(...)` as a reusable C++ integration seam. It requires the `MissionProblem` to reference the exact `PlanningGraph` supplied to the resolver and requires a feasible selected candidate. It does not build placeholder performance, safety, or fleet inputs.
- Added an HMI CTest contract test for successful route resolution/serialization and negative cases. CI is running for the change; the test result must be checked before this checkpoint is treated as verified.
- This closes the **reusable handoff function** gap but does not create a runtime caller: no discovered application owner currently supplies both the selected candidate/graph and complete authoritative Planning Core request, and no call to `PlanningBridge.sendRequest()` was added. Therefore the live map route remains NOT IMPLEMENTED / NOT VERIFIED end to end.

## Selected candidate handoff payload — follow-up

- `OrchestratorDecision` now includes the selected `CandidateSolution` value in addition to `selected_candidate_id`; the successful solve path populates both from the same selected candidate.
- The route-validation test asserts the returned candidate ID and ordered route elements, guarding against an ID-only decision that loses the route path needed for geometry resolution.
- CI is running for these changes. Confirm all relevant workflows pass before treating this checkpoint as verified.
- The HMI still has no runtime caller that owns `SolverContext`/`MissionProblem`, selected decision, and a complete authoritative request at once. Do not invent a bridge in `main.cpp` without an input provider for required performance/safety data.

## Next deterministic work item

1. Check CI for the selected-candidate payload and `selected_route_map_handoff_contract`.
2. Trace the application-level owner of `SolverContext` / `AlgorithmOrchestrator` and the source of the required request fields; if no owner exists, implement a narrow application service with explicit inputs rather than default values.
3. Connect that owner to `attachSelectedRouteGeometryToRequest(...)` and `PlanningBridge.sendRequest()`.
4. Add an integration test through the JSONL adapter and verify `result.routeGeometry` reaches the QML map binding.
5. Verify on the Windows project build. Do not claim map integration complete until a real selected route is exercised end to end.
