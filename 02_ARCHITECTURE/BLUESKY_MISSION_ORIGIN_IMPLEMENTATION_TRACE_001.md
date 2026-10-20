# BlueSky PRO — Mission Origin Implementation Trace

**Document ID:** BS-ENG-MISSION-ORIGIN-TRACE-001  
**Reviewed branch:** `main`  
**Reviewed scope:** `MainContent.qml`, `FlightChart.ui.qml`, `MapTilerWebMap.qml`, `BLUESKY_MISSION_MODEL.md`  
**Status:** Source trace complete for the files listed; implementation gap confirmed. No runtime behavior or CI result is asserted by this document.

## 1. Verified current code facts

### 1.1 Mission UI state

`qt/BlueSkyPRO-HMI/qml/MainContent.qml` contains mission editor state such as:
- `missionState` (`AUTO`, `HIDDEN`, `MANUAL`, `VALIDATING`);
- `missionId` and `manualMissionVersion`;
- selected template IDs;
- the planning result bridge and route-candidate synchronization;
- `manualGeometryBridge` connections for geometry changes and composition completion.

In the inspected file, there is no explicit canonical field for planned mission origin, actual field anchor, site transform, or anchoring state.

### 1.2 Map view and route rendering

`qt/BlueSkyPRO-HMI/qml/FlightChart.ui.qml` documents the Flight Chart as a schematic workspace, with the geographic map delegated to `MapTilerWebMap.qml`. Route coordinates are supplied from the route controller; QML does not calculate the authoritative route.

### 1.3 MapTiler map integration

`qt/BlueSkyPRO-HMI/qml/MapTilerWebMap.qml`:
- initializes the map at a hard-coded default center `[31.95990,49.85842]` and zoom `6.7`;
- provides style selection including `streets-v4`;
- contains a manual geometry capture UI and template-to-geometry-type mapping;
- publishes manual geometry through `manualGeometryBridge.publishGeometry(JSON.stringify({objects:objects}), complete)`;
- renders geometry features from the captured objects and draft points.

The inspected geometry payload contains an `objects` collection; no explicit `missionOrigin`, `plannedOrigin`, `fieldAnchor`, or `siteTransform` contract is present in this payload. The hard-coded initial map center is a viewport default only and must not be treated as the mission's take-off point or a geographic anchor.

### 1.4 Canonical architecture

`02_ARCHITECTURE/BLUESKY_MISSION_MODEL.md` already defines a vendor-independent canonical mission, mission versioning, environment snapshots and vehicle-specific executable packages. The new origin/anchoring baseline extends this architecture but is not yet reflected as implemented fields in the inspected QML state.

## 2. Confirmed gap

The reviewed HMI path currently supports selecting task templates and capturing work geometry. It does not yet demonstrate an end-to-end workflow that:
1. explicitly defines the planned take-off/origin point before template geometry;
2. distinguishes planned origin from first route waypoint and recovery/home point;
3. stores local-frame definition and field-anchor transformation;
4. saves and reopens a versioned offline mission package;
5. binds the prepared mission to field coordinates with a visible validation gate.

This is a source-inspection finding, not a claim that no related code exists anywhere else in the repository. Core services, C++ bridge implementations, package formats and tests must be traced before declaring repository-wide absence.

## 3. Required implementation sequence

### Phase A — Trace existing domain and bridge contracts

Before adding a competing data model, locate and document:
- the C++ `manualGeometryBridge` registration and payload schema;
- `PlanningBridge` request/result schema and mission version linkage;
- `canonicalRouteController` route data and coordinate units;
- existing mission import/export and vehicle adapter formats;
- test/Virtual Flight contracts and CI validation scripts.

Reuse canonical types where they already exist. Do not duplicate route or mission authority in QML.

### Phase B — Add canonical origin and anchoring fields

Extend the authoritative mission contract (not only UI properties) with:
- planned origin and coordinate-frame semantics;
- separate first route waypoint and recovery/home point;
- planning mode: georeferenced or local/site-to-be-confirmed;
- local-frame origin, orientation convention and units where applicable;
- field anchor with source, timestamp, quality and uncertainty;
- site transform with method, parameters, references and residual/error estimate;
- explicit anchoring state and mission version invalidation.

A map click can nominate a planned point, but it is not itself proof of actual GNSS position or field survey accuracy.

### Phase C — Operator workflow on the map

1. New mission starts in **Origin required** state.
2. Operator selects a known point on the map or explicitly chooses local/site-to-be-confirmed planning.
3. The map shows a distinct origin marker and coordinate/reference status.
4. Only after origin mode is resolved does the template create its initial geometry.
5. Field anchoring is a separate, explicit workflow with offset/orientation and positioning quality shown.
6. Any material change to origin, frame, orientation or transform invalidates dependent route and validation results.

### Phase D — Offline package

Use the canonical mission model to generate a versioned, integrity-checked package. Include coordinate and vertical reference systems, route/geometry, vehicle compatibility, validation evidence and snapshots/provenance for required cached data. Import must not silently rebase or rotate a mission. Missing or stale environment data must remain visible offline.

### Phase E — Virtual Flight and regression tests

Required scenarios:
- no origin: geometry generation is blocked;
- georeferenced origin and route round-trip unchanged through export/import;
- local-frame mission with one anchor but unresolved orientation remains blocked;
- actual field position outside configured tolerance does not auto-shift the route;
- changing origin/frame invalidates prior route validation;
- stale or missing cached NOTAM/weather/terrain inputs are visibly flagged;
- package corruption/schema mismatch/vehicle incompatibility is rejected;
- map, route table and profile use the same mission version after anchoring.

## 4. Safety boundaries

- The default map viewport center is not a mission coordinate.
- The first route waypoint is not implicitly the take-off point.
- A single point can establish translation but cannot independently resolve arbitrary horizontal rotation.
- No automatic relocation, rotation or scaling of a mission without an explicit transform workflow and recorded operator confirmation.
- Offline mode does not establish current airspace, NOTAM or weather status.
- UI state alone must never authorize flight; release remains controlled by the canonical planning/safety pipeline.

## 5. Acceptance and implementation status

This trace establishes the starting point for implementation. It does not assert that origin selection, field anchoring, offline packaging, or field release validation is implemented. The next code change should follow Phase A, then introduce the canonical domain contract and tests before wiring UI actions.
