# Map Route Integration — Checkpoint 2026-10-09

## Status

**IN PROGRESS — canonical route resolution, geometry serialization, request attachment, and a narrow bridge-submission service exist. Live runtime ownership, authoritative request supply, end-to-end integration, and Windows verification remain incomplete.**

## Branch

`fix/hmi-map-tile-loading-2026-10-09`

## Confirmed architecture

- Canonical geographic route type is `bluesky::planning::Route` in `04_SOFTWARE/PLANNING/model/route_model.hpp`. It contains ordered waypoints with WGS84 latitude/longitude, altitude, mandatory flags, route identity/version and lineage.
- `SelectedRouteSet` in `04_SOFTWARE/PLANNING/selected_route_set.hpp` records mission/version, selected candidate and `route_elements` as strings. It does **not** itself contain waypoint coordinates.
- `Mission.selected_solution` in `04_SOFTWARE/PLANNING/model/mission_model.hpp` stores a selected candidate reference, not route geometry.
- `FlightProfileBuilder::build(selected, route, ...)` in `04_SOFTWARE/PLANNING/flight_profile.hpp` accepts both selected-route metadata and canonical `Route`; this is a domain API, not proof the HMI currently owns the selected route.
- `AlgorithmOrchestrator::solve()` now returns the selected `CandidateSolution` value as well as its ID. `MissionProblem` carries the authoritative graph pointer; candidate route elements are graph-node IDs.
- The JSONL adapter at `schemas/validator/planning_core_stdio.py` reconstructs validation routes from local `x/y`; it does not derive WGS84 from those coordinates.
- `schemas/planning-core-request.schema.json` permits optional `inputs.routeGeometry` in WGS84. The request still requires authoritative `zoneStatus`, `assignmentStatus`, `routes`, `performance`, `trajectories`, and `minimums`.
- `PlanningBridge` can start a process, send JSONL and publish received results. `MainContent.qml` exposes `planningBridge.result` to the mission profile/map route chain.
- `main.cpp` currently constructs and exposes `PlanningBridge` and `TileCacheManager`; it does not own an `AlgorithmOrchestrator`, selected candidate, graph, or complete planning-request provider.

## Coordinate and ownership boundary

Do not convert adapter-local `x/y` into latitude/longitude by renaming fields. Their coordinate reference and transform are not established in this request path.

Do not synthesize a geographic route from `SelectedRouteSet.route_elements` alone. The integration must use the authoritative graph that generated the candidate and preserve the selected route order.

Do not create placeholder fleet, performance, trajectory, safety, or minimum-separation inputs to make the request pass. These are required by the current request schema and must come from the authoritative request-input provider.

## Implemented selected-route submission path

- `qt/BlueSkyPRO-HMI/src/SelectedRouteMapHandoff.h/.cpp` exposes `attachSelectedRouteGeometryToRequest(...)`. It checks graph identity and candidate feasibility, resolves graph-node IDs into canonical `Route`, validates the route and attaches its WGS84 geometry to an already assembled request without inventing other fields.
- `qt/BlueSkyPRO-HMI/src/SelectedRoutePlanningService.h/.cpp` now provides `SelectedRoutePlanningService::submitSelectedRoute(...)`: it calls `attachSelectedRouteGeometryToRequest(...)`, serializes the preserved request as compact JSON, and passes it to `PlanningBridge.sendRequest()`. It reports attachment/send failure to its caller.
- The service is compiled into the HMI target. It deliberately accepts a complete request and selected planning objects as explicit inputs; it does not fabricate missing values or start a process with guessed paths.
- The service is a submission seam, **not yet an active runtime caller**. There is still no application owner that supplies the real selected decision/graph and the full validated request to it.
- To protect performance, the eventual owner must call the service on a changed selected candidate or changed calculation inputs, not on every QML refresh. No cache or deduplication policy has been added without a defined request identity/replan contract.
- `PlanningBridge::sendRequest()` previously called `waitForBytesWritten(1000)`, which could synchronously block its caller (typically the GUI thread). It now queues one JSONL frame into `QProcess` and returns based on whether the complete frame was accepted into Qt's write buffer. This removes the explicit one-second synchronous wait; a `true` result means queued, not that Planning Core has processed the request. Process/result errors remain asynchronous and must be handled via bridge signals and result correlation.

## Map preview-route correction

- Removed the five hardcoded Moscow-area route coordinates from `GoogleMapView.qml`.
- Removed the design-time mission-table fallback from `MissionProfileWindow.qml` map rendering. The map remains route-empty until `planningResult.result.routeGeometry` contains at least two valid WGS84 points.
- The design-time table fixture remains a UI preview only and is not represented as the selected geographic route.
- This is a correctness fix, not proof of live route integration.

## Planning-domain resolver and tests

- `04_SOFTWARE/PLANNING/canonical_route_builder.hpp/.cpp` resolves `SelectedRouteSet.route_elements` against authoritative `PlanningGraph` node IDs and constructs canonical `Route` geometry.
- It preserves lineage/order, builds adjacent segments with great-circle distances, marks graph start/goal mandatory, and rejects missing/duplicate nodes, invalid WGS84, non-finite altitude, missing lineage, fewer than two points, wrong endpoints, and missing/invalid directed edges.
- Tests cover successful mapping and invalid/disconnected route cases.
- `qt/BlueSkyPRO-HMI/tests/SelectedRouteMapHandoffTest.cpp` covers ordered WGS84 output, preservation of existing request inputs, graph mismatch, and infeasible-candidate rejection without request mutation.
- `qt/BlueSkyPRO-HMI/tests/RouteGeometrySerializerTest.cpp` covers geometry serialization and invalid-request/route rejection. Commit `02bd4114` adds fail-closed checks for the Planning Core request's required `zoneStatus`, `assignmentStatus`, non-empty `routes`, `performance`, `trajectories`, and `minimums.horizontalM/verticalM` before attaching geometry. Commit `a27c50a7` updates the contract fixture and adds tests that incomplete requests are rejected without mutation. This prevents the HMI handoff from knowingly submitting a request missing schema-required planning inputs; it does not supply those inputs.
- HMI CMake registers both contract tests and compiles the new `SelectedRoutePlanningService`.

## Verification state

- Earlier CI passed for route serialization, canonical route validation, graph-connectivity checks, and removal of preview geometry.
- CI passed on commit `dc2e68a7179e5396daebcc1c3ed29395d30e7dfd` for HMI Qt Build, Planning Benchmark, Multi-UAV Validator, and journal-service CI.
- CI passed on commit `85d9942719085b3188016fa97b8e8e99a61c2a1a` for HMI Qt Build, Planning Benchmark, Multi-UAV Validator, and journal-service CI.
- The `SelectedRoutePlanningService` is included in HMI commit `7f52818ccc4038bbe399835c42d992a3424566cf`. HMI Qt Build run [37918167417](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37918167417) completed successfully, including Configure, Build, and HMI contract tests. Planning Benchmark run [37918167525](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37918167525) also completed successfully, including Build and Test. Multi-UAV Validator and journal-service CI runs for the same commit completed successfully.
- Commit `62adf13d3d2480e50500b275cacea62442899d64` removes the synchronous `waitForBytesWritten(1000)` from `PlanningBridge::sendRequest()` so sending a planning request does not explicitly wait up to one second on the caller thread. HMI Qt Build run [37918697314](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37918697314) completed successfully, including Configure, Build, and HMI contract tests. Planning Benchmark run [37918697253](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37918697253) completed successfully, including Build and Test. Multi-UAV Validator run [37918697235](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37918697235) and journal-service CI run [37918697281](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37918697281) also completed successfully.
- A recursive inventory of the current branch's HMI sources confirms the HMI has `main.cpp`, `PlanningBridge`, route serializer/handoff/service, and QML files, but no HMI-side `AlgorithmOrchestrator`, `MissionProblem`/`PlanningGraph` owner, or full request-input provider file. The orchestration/domain code exists outside the HMI target; there is no current caller joining it to the HMI service.
- The current HMI `main.cpp` only instantiates `PlanningBridge`, `TileCacheManager`, and the QML engine. The existing `AlgorithmOrchestrator` API returns `OrchestratorDecision::selected_candidate`, but its `solve(SolverContext&)` requires an authoritative context and graph; no HMI runtime construction path for that context/graph or complete JSONL request is present in the HMI target. The mission-transfer runtime is for transferring a compiled mission to an autopilot, not for constructing the Planning Core JSONL request, so it is not a substitute.
- CI for the request completeness guard and new negative tests is green: HMI Qt Build [37919455910](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37919455910), Planning Benchmark [37919455898](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37919455898), Multi-UAV Validator [37919455968](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37919455968), and journal-service-ci [37919455904](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37919455904) all completed successfully for checkpoint commit `938de07ac5a43cebc1210aaf6f6df6a3e72cd4cb`.
- Local Windows build/runtime has not been verified.
- End-to-end selected route -> complete request -> JSONL Planning Core -> `PlanningBridge.result` -> QML map remains **NOT IMPLEMENTED / NOT VERIFIED**.

## Next deterministic work item

1. Confirm CI for commits `02bd4114` and `a27c50a7`; fix only demonstrated failures.
2. Design the missing application-level integration owner only after identifying authoritative APIs for candidate selection, graph lifetime, and complete request inputs. The current HMI source inventory contains no such owner/provider; adding a guessed `main.cpp` construction would fabricate mission state.
3. Define the interface that supplies the already validated full request alongside the exact selected candidate and graph, including route/input version identity for change-triggered submission.
4. Connect `SelectedRoutePlanningService::submitSelectedRoute(...)` at that owner, triggered by a changed selected route or relevant calculation inputs—not by every QML refresh.
5. Extend integration coverage to assert route geometry survives the JSONL adapter and is accepted by `PlanningBridge`, then reaches the QML map binding.
6. Verify the existing Windows project build and a real selected-route scenario. Do not claim integration complete before this passes.
