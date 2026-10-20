# BlueSky PRO — Mission Origin Implementation Handoff

**Status:** Engineering handoff based on source inspection of `main`  
**Priority:** P0  
**Scope:** canonical mission model, HMI adapter, map UI, offline package and tests

## 1. Existing implementation seams confirmed

The following existing files should be extended rather than bypassed:

- `04_SOFTWARE/PLANNING/model/mission_model.hpp`: canonical `Mission` currently includes identity, mission mode, selected templates, geometry objects, intent, constraints, environment snapshots, vehicle/payload assignments, route references, authorization/package/execution references. It has no explicit mission-origin, local-frame or field-anchor structure.
- `qt/BlueSkyPRO-HMI/src/ManualGeometryBridge.h/.cpp`: WebChannel bridge accepts a JSON object whose `objects` member is an array and publishes the JSON string unchanged. It currently exposes `geometryJson` and `compositionComplete`.
- `qt/BlueSkyPRO-HMI/src/ManualMissionModelAdapter.cpp`: parses the geometry objects into the canonical `Mission`, validates structure and serializes a canonical JSON containing `missionId`, `missionVersion`, `missionMode`, `selectedTemplateIds` and `geometryObjects`. It currently does not parse or serialize a mission origin.
- `qt/BlueSkyPRO-HMI/qml/MapTilerWebMap.qml`: creates geographic manual geometry from map clicks and sends `{objects: objects}` through the bridge. It has a hard-coded default map viewport center, which is not a mission origin.
- `qt/BlueSkyPRO-HMI/qml/MainContent.qml`: tracks mission/editor state and calls `ManualMissionModelAdapter.buildMission(...)`; this is the integration point for the workflow state, not the place to duplicate canonical mission data.
- `qt/BlueSkyPRO-HMI/tests/ManualMissionModelAdapterTest.cpp` and `qt/BlueSkyPRO-HMI/CMakeLists.txt`: existing contract test and CTest target are available for regression coverage.
- `02_ARCHITECTURE/MISSION_PACKAGE/BLUESKY_MISSION_PACKAGE_BASELINE.md`: already requires explicit coordinate and altitude/reference conventions and a validated package lifecycle. The origin/anchor contract should extend it rather than create a competing package specification.

## 2. Important integration constraint

The current HMI map captures geometry, but that is not equivalent to a complete mission-origin workflow. Adding an origin marker only in the browser map would be insufficient: the origin must survive map JSON → typed canonical Mission → canonical serialization → offline package → vehicle adapter, with its coordinate semantics intact.

Likewise, adding an origin field only to QML would create a second, non-authoritative state. The implementation must use the canonical C++ model as authority and invalidate route/validation evidence whenever origin or transform changes.

## 3. Recommended minimal canonical types

Add explicit typed structures to `mission_model.hpp`:

- `MissionOrigin`: planning mode, coordinate reference system, geographic coordinates when known, and explicit local-frame origin definition when not known.
- `FieldAnchor`: actual field position/reference, source, timestamp, quality/uncertainty and confirmation status.
- `SiteTransform`: translation/orientation parameters, reference method, control references and residual/error estimate.
- Separate references for planned take-off/origin, first route waypoint, recovery/home point and work-area reference.

Use optional values to represent unresolved information. Never populate unknown geographic coordinates with the map's viewport center. A local mission with unresolved orientation must remain unbound and must not be represented as flight-release eligible.

## 4. Implementation order

### Step 1 — Canonical model and adapter contract

1. Add the typed structures to `mission_model.hpp`.
2. Extend `ManualMissionModelAdapter` to parse and validate an explicit origin payload, then serialize the same values into canonical mission JSON.
3. Keep legacy map geometry input parseable during transition, but return a clear blocking validation error when a mission is submitted for completion without the required origin/planning mode.
4. Add round-trip and negative tests for missing origin, invalid longitude/latitude, unknown CRS, local-frame origin, missing orientation and a valid georeferenced origin.

### Step 2 — Map-first operator flow

1. Start new mission in **Origin required** state.
2. Provide an explicit map action to select the planned origin; show a distinct marker and coordinates.
3. Offer the deliberate choice between a known georeferenced site and a local/site-to-be-confirmed plan. Do not treat a map click as an actual GNSS fix.
4. Disable template geometry capture until the origin mode is defined.
5. Include origin in the bridge payload and publish origin changes through the same canonical adapter.
6. Keep origin marker, work geometry and route candidate visually distinct.

### Step 3 — Versioning and invalidation

Changes to origin, coordinate frame, orientation, field anchor or transform must increment the mission version and invalidate the route candidate and any dependent validation evidence. The map, route table, profile and export must reference the same version.

### Step 4 — Offline package and field anchoring

Extend the existing Mission Package contract with origin/frame/anchor/transform data, integrity metadata and required cached inputs. Import must work offline. At the field site, georeferenced missions compare actual and planned origin; local missions require sufficient references to resolve translation and orientation. Never silently move or rotate a route.

### Step 5 — Virtual Flight acceptance

Add deterministic cases for:
- mission cannot complete without origin mode;
- georeferenced origin round-trips unchanged;
- a local origin with only one point but unresolved orientation remains blocked;
- origin/transform change invalidates prior route validation;
- mismatched actual location does not auto-rebase the plan;
- offline import works while stale/missing cached environment data remains visibly flagged;
- corrupted or incompatible packages are rejected.

## 5. Safety boundary

A planning origin identifies the intended reference; it does not itself establish actual field position, GNSS quality, airspace permission, obstacle clearance or flight authorization. Those checks remain separate and authoritative. Offline mode means no live update, not that cached operational data are current.

## 6. Status

This is an implementation handoff, not a claim that origin selection or offline anchoring is already implemented. The next code patch should include the canonical model, adapter, map payload, HMI state and tests together, then be validated by the existing Qt HMI and Windows map CI workflows before the feature is described as implemented.
