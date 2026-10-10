# BlueSky PRO — Interactive Flight Profile: Data Synchronization and Recalculation

**Status:** APPROVED REQUIREMENT  
**Scope:** Mission planning HMI, canonical route model, validation and recalculation  
**Applies to:** Flight profile, waypoint table, map and all calculation data flows

## 1. System requirement

The flight profile is a fully interactive mission-editing surface, equivalent in editing authority to the map and waypoint table. It is not a standalone visualization and must not maintain an independent authoritative route.

Any supported edit made in the profile must be applied to the canonical mission/route data model and propagated to every dependent view and calculation flow.

## 2. Synchronized views

The following representations must reflect the same current route state:

- flight profile;
- waypoint/flight-parameter table;
- active map and route geometry;
- mission calculation inputs and derived values;
- validation results, warnings and mission readiness state.

No view may silently retain stale values after an edit or recalculation.

## 3. Editing behavior

- Selecting a waypoint or route-profile point selects the corresponding canonical waypoint.
- Editing a waypoint altitude in the profile updates that waypoint's altitude in the table and map.
- Editing the same waypoint in the table or map updates the profile.
- Creating or moving a mandatory waypoint/constraint in the profile updates the canonical route data and all corresponding representations.
- A mandatory altitude is a route constraint, not merely a graphical marker or user-interface preference.
- Edits to one waypoint must not unintentionally modify other waypoints. Any downstream changes made by the planner must be explicit outputs of recalculation.

## 4. Validation and recalculation transaction

Every edit that can affect the route or its calculated properties must initiate this controlled sequence:

1. Apply the edit to a route candidate / working mission version.
2. Validate input values and mandatory constraints.
3. Revalidate affected route constraints, including applicable airspace/restrictions, terrain and obstacles, mission/task requirements, vehicle limits, wind/environmental inputs, and conflicts with other UAV routes.
4. Recalculate affected route and mission values, including segment course, distance, altitude, air/ground speed where applicable, segment and total time, ETO/ETA, energy/battery estimates, and other dependent values.
5. Publish the resulting current route/profile/table/map state and validation/readiness results together.
6. Preserve traceability to the mission version, route version, calculation-input version and environmental-data snapshots.

The implementation may use incremental recalculation where dependencies allow it, but the published result must be internally consistent. If a full recalculation is required, it must be performed before the edited route is presented as validated.

## 5. Invalid or incomplete edits

- An edit that fails validation must not silently become the accepted/validated route.
- The interface must identify the failed constraint or missing/stale input.
- The system must retain the last valid route state or clearly distinguish the unvalidated candidate from it.
- Readiness must be recalculated from the validation result; editing must never directly force a READY state or bypass safety/authorization gates.

## 6. Data authority and persistence

- The canonical route/mission model is authoritative; UI-local models are presentation/editing buffers only.
- User display preferences (for example, visible columns or column order) must not be used as storage for operational route constraints.
- Mandatory waypoint identity and altitude belong to the mission/route data model and must persist with the applicable mission/route version.
- Accepted changes create a traceable new mission/route version when required by the canonical route-versioning rules.
- Previously archived mission versions remain immutable.

## 7. Acceptance criteria

1. Changing a waypoint altitude in the profile updates the same waypoint in the table and map.
2. Changing the altitude in the table or map updates the profile.
3. Adding, moving or selecting a mandatory profile point updates the canonical route constraint and its corresponding table/map representation.
4. Each operational edit triggers validation and the required recalculation of dependent values.
5. Course, distance, speeds, segment time, ETO/ETA, trip time and energy values shown across views agree with the same calculation result.
6. An invalid edit produces an explicit validation result and cannot be represented as a validated/READY route.
7. Other waypoints remain unchanged unless the planner explicitly changes them as a reported result of recalculation.
8. All views update from one consistent route/calculation version; stale values are not silently displayed as current.
9. Route changes and accepted results remain traceable; archived versions are not overwritten.

## 8. Architectural boundary

The HMI initiates edits and presents results. It does not independently implement authoritative route validation, safety decisions, flight authorization or vehicle commands. These remain with the relevant planning, validation and safety components.

## Related canonical specification

See [Canonical Route Model (PLAN-DATA-001)](../../../02_SYSTEM_DESIGN/PLANNING/FLIGHT_PLANNING_CANONICAL_ROUTE_MODEL_001.md) and [Flight Profile Interaction](Flight%20Profile%20Interaction.md).


## 9. Mandatory profile points — interaction and shared display

### 9.1 Multiple points and route geometry

- The operator may create and retain multiple mandatory points simultaneously.
- Every mandatory point is a node/constraint in the common route profile, not an overlay independent of the route.
- The profile line must pass through each mandatory point in route order. With one mandatory point, the line connects the preceding route point to the mandatory point and then to the following route point, preserving one continuous profile.
- Moving a mandatory point updates the adjoining profile segments and the common route geometry. Other mandatory points remain intact.
- The route/profile must preserve the rest of the route unless the planner explicitly changes it as part of recalculation.
- Mandatory points must be represented in the canonical route/mission model. A UI-only list or saved display preference is not an operational source of truth.

### 9.2 Removal and input methods

- Desktop: remove a mandatory point with the right mouse button on that point.
- Tablet/touch: remove a mandatory point with a double tap on that point.
- Point selection and dragging must distinguish an existing mandatory point from a normal route waypoint and from empty chart space.
- Input handling must avoid accidental deletion or creation during a drag gesture.

### 9.3 Shared visual identity

- Mandatory points use the same blue color in every view: `#155BFF`.
- They must be visible in every active representation that displays the route, including the flight profile, map and route/waypoint table where applicable.
- The same canonical point identity, coordinates, altitude and mandatory status must be shown consistently across all views.
- Creating, moving or removing a point updates all affected views from the same route state.

### 9.4 Acceptance criteria for mandatory points

1. Two or more mandatory points can coexist and remain individually selectable.
2. The profile line passes through all mandatory points in route order.
3. Moving one point updates the two adjoining profile segments without removing or moving other mandatory points.
4. Right-click removes the selected point on desktop; double tap removes it on a tablet.
5. All route views show mandatory points in `#155BFF` and agree on their identity and position.
6. Each change is validated and recalculated through the controlled transaction in Section 4; an invalid candidate is not presented as accepted/READY.

### 9.5 Unified route-point numbering

- Every route point has one shared sequential route number, ordered from route start to route finish.
- The same route number must be shown in the waypoint table, map and flight profile.
- A mandatory point does not receive a separate mandatory-point ordinal. It retains the number of its corresponding route point.
- Mandatory status is shown separately by the blue marker (`#155BFF`) and the status/type label “Обязательная”.
- A mandatory point inserted between existing route points becomes a node in the ordered route sequence and receives the corresponding shared route number.
- Reordering or moving route points must update numbering consistently across all route views.
- Acceptance: the sequence of point numbers on the map and profile exactly matches the table's `#` column; no view maintains an independent numbering scheme.
