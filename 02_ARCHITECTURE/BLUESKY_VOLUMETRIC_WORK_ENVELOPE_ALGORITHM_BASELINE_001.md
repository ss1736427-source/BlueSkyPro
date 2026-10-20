# BlueSky PRO — Volumetric Work Envelope Algorithm

**Document ID:** BS-ALG-VOLUME-001  
**Status:** Architecture baseline — requirements captured; implementation and verification pending  
**Priority:** P0 for volumetric mission templates  
**Applies to:** 3D mapping, photogrammetry, building/structure inspection, construction monitoring and other missions around volumetric assets

## 1. Purpose

Define a manual-first method for creating a three-dimensional work envelope around an object or site. The operator draws the work area's ground footprint by selecting points on the map, then specifies a vertical height. BlueSky creates a vertically extruded copy of the footprint, displays the resulting 3D volume as a box-like envelope, and constrains route generation to that volume.

The envelope defines **where the UAV is permitted to perform the task**. It is not itself the flight route.

## 2. Operator workflow

1. Select a volumetric mission template, such as 3D mapping, structure inspection or construction monitoring.
2. On the map, select ground/footprint points around the building, structure or site.
3. Close the polygon to define the horizontal footprint.
4. Enter the required vertical extent/height and its vertical reference.
5. BlueSky creates a corresponding upper polygon by copying the footprint's horizontal coordinates and applying the selected vertical offset/reference.
6. Connect corresponding lower and upper polygon edges to form a closed, visible 3D work envelope.
7. Show the footprint, upper boundary, vertical side faces, height and envelope status in the map/3D view.
8. Generate one or more task-appropriate routes inside the envelope.
9. Validate that route segments, vehicle dimensions/margins, payload/sensor constraints, obstacle clearance and mandatory airspace/safety constraints are satisfied before the route can be released.

The operator should be able to edit the footprint or height and immediately see the envelope update. Updating height must not silently change the horizontal footprint.

## 3. Geometry model

The canonical work-envelope geometry contains:

- **Footprint polygon:** ordered ground points selected by the operator;
- **Lower boundary:** footprint projected to terrain/ground elevations where terrain data is available;
- **Upper boundary:** a corresponding polygon at the specified vertical extent;
- **Side faces:** surfaces joining each lower edge to its matching upper edge;
- **Vertical reference:** explicit reference such as AGL or AMSL, never an implicit or unknown datum;
- **Envelope version:** revision identifier and the inputs used to calculate the geometry.

For a simple flat-ground case, the upper polygon is an exact horizontal-coordinate copy of the footprint translated upward by the configured height. For varying terrain and AGL, the upper boundary must be calculated against terrain elevation at each corresponding footprint point; it must not assume all terrain points have the same elevation.

The displayed envelope must reflect the actual canonical geometry and vertical reference. A height edit must update the upper boundary and side faces, not only a numeric label.

## 4. Algorithm sequence

```text
MISSION TEMPLATE / OBJECTIVE
        ↓
MANUAL FOOTPRINT POINTS
        ↓
POLYGON VALIDATION
        ↓
TERRAIN + VERTICAL REFERENCE RESOLUTION
        ↓
UPPER POLYGON = FOOTPRINT + HEIGHT RULE
        ↓
CLOSED 3D WORK ENVELOPE
        ↓
VISIBLE GEOMETRY / ENVELOPE STATUS
        ↓
TASK-SPECIFIC ROUTE GENERATION INSIDE ENVELOPE
        ↓
OBSTACLE + AIRSPACE + VEHICLE + SENSOR + ENERGY VALIDATION
        ↓
ADMISSIBLE ROUTE(S) OR EXPLICIT VALIDATION ERRORS
```

## 5. Validation requirements

Before the envelope is accepted, BlueSky shall check:

- enough distinct points exist to form a polygon;
- the polygon is closed consistently and has non-zero area;
- polygon edges do not self-intersect;
- coordinates and units are valid;
- height is positive and within configured system/mission limits;
- the vertical reference is explicit and compatible with terrain and route data;
- terrain data needed for AGL conversion is available and has known quality/freshness, or the system explicitly marks the envelope as unresolved;
- the upper and lower boundaries form valid geometry;
- route candidates remain within the defined work envelope with configured safety/vehicle margins;
- mandatory airspace, NOTAM, obstacle, vehicle, payload, C2 and energy constraints are checked separately and cannot be overridden by envelope membership.

If a check fails, show the specific reason and do not present the envelope or route as validated.

## 6. Route-generation contract

The work envelope is a spatial constraint passed to the mission planner. It does not prescribe a single route pattern.

The selected mission objective determines route generation inside the envelope. Examples include:

- **3D mapping / photogrammetry:** generate coverage legs and camera viewpoints for required GSD, overlap, image geometry and reconstruction completeness;
- **building / structure inspection:** generate surface-following or viewpoint-based segments while preserving obstacle clearance and sensor geometry;
- **construction monitoring:** generate repeatable coverage routes and viewpoints suitable for comparison across mission runs.

The planner may generate multiple route candidates or coordinated routes for multiple UAVs. Each candidate must be checked against the same versioned envelope and the applicable hard safety constraints.

A footprint polygon alone is not a complete route. The system must explicitly generate the route segments and expose route/profile data from the same canonical mission model used by the map and mission profile view.

## 7. Data consistency and UI behavior

The map/3D view, mission route table, flight profile and validator must reference the same canonical envelope and route versions.

Required behavior:

- moving a footprint point updates the footprint, upper polygon and side faces;
- changing the configured height updates the upper polygon and side faces without altering footprint coordinates;
- changing terrain or vertical-reference inputs triggers a controlled geometry recalculation and records the input version;
- a work envelope with no generated route is shown as **Envelope defined — route not generated**, not as a route-ready mission;
- a route outside the envelope is visibly identified and rejected by validation;
- switching to ADMIN or another workspace must not destroy the in-progress mission geometry.

## 8. Safety and operational interpretation

The envelope is a planning boundary, not permission to fly. Airspace authorization, NOTAM restrictions, terrain/obstacle clearance, vehicle limits, C2 availability, energy reserve and all other mandatory gates remain independently applicable.

Where the real asset protrudes into the intended volume, the operator's footprint and height do not replace obstacle data or clearance checks. The planner must preserve the required separation from the asset unless a mission-specific, explicitly supported inspection geometry permits a closer path.

## 9. Acceptance criteria

1. An operator can create a footprint by selecting ground points manually.
2. The system creates and displays a corresponding upper polygon after a height is specified.
3. The lower/upper polygons and connecting faces visibly form a 3D work envelope.
4. Changing height changes the volume while preserving the horizontal footprint.
5. AGL is resolved relative to terrain; missing or incompatible vertical references are not silently guessed.
6. Invalid polygons and invalid heights are rejected with actionable messages.
7. The route planner generates task-appropriate route candidates inside the envelope.
8. The route/profile/table/map use one canonical route model and do not display contradictory states.
9. Envelope membership never bypasses obstacle, airspace, NOTAM, vehicle, payload, C2 or energy validation.
10. Envelope edits and route recalculations are versioned and traceable.
11. Virtual Flight tests cover flat terrain, varying terrain, height edits, invalid polygons, route containment and rejected safety constraints.

## 10. Implementation boundary

This document records the required algorithm and acceptance contract. It does **not** claim that the algorithm is already implemented. Implementation should begin by tracing the current manual geometry model, the target-height property, the 3D/map renderer and the mission route/profile data flow. Preserve existing mission state and avoid creating a parallel geometry model.
