# BlueSky PRO — NOTAM Prohibited-Zone Validation

**ID:** PLAN-VAL-002  
**Status:** DRAFT  
**Scope:** Phase B — Flight Planning Core

## Purpose

Deterministically verify that route waypoints and route segments are outside active NOTAM-defined prohibited zones before a route can be accepted as a valid planning candidate.

## Boundary

NOTAM data is an external, versioned planning input. The validator does not fetch NOTAMs, interpret free-form NOTAM text, authorize flight, change readiness, bypass safety, or command an aircraft.

The upstream NOTAM adapter is responsible for converting an authoritative NOTAM source into a normalized snapshot containing geometry, validity interval, altitude limits, restriction type, source identity and snapshot version.

## Required checks

1. Route references a NOTAM snapshot when NOTAM avoidance is required.
2. The snapshot is valid for the route evaluation time.
3. Every route waypoint is tested against every active prohibited zone whose altitude band overlaps the waypoint altitude.
4. Every adjacent route segment is tested against active prohibited zones when the segment altitude interval overlaps the zone altitude band.
5. A waypoint inside a zone or a segment intersecting a zone produces a deterministic rejection finding identifying the NOTAM and waypoint/segment.
6. Missing, invalid or stale NOTAM input is never silently treated as an empty airspace.

## Geometry

The initial normalized geometry supports circles and polygons. Geodesic conversion/projection belongs to the adapter; the validator receives latitude/longitude in the canonical route model.

## Geometry implementation note

Circle segment checks use a local tangent-plane distance approximation; polygon checks use local planar segment-edge intersection. This is appropriate for the current bounded planning test cases but is not a substitute for validation against authoritative geospatial test vectors, especially for long segments, polar routes, or geometry crossing the antimeridian. The validator remains dependent on upstream normalization and does not interpret raw NOTAM text.

## Mission-time applicability, wind-aware timing, and optimizer feedback — mandatory rule

A prohibited NOTAM zone is a hard planning constraint only when the planned aircraft occupancy of the zone overlaps the NOTAM validity interval. Mission-start time alone is not sufficient to decide whether a route conflicts with a NOTAM.

The optimizer/validator workflow shall execute in this order:

1. Generate a candidate route using the existing optimization objective and constraints.
2. Apply the selected wind model to that candidate and calculate a time profile for the resulting route, including ground/track speed and estimated passage time at each waypoint and the time interval for every segment. The profile must be derived from the wind-adjusted candidate, not copied from the pre-wind route.
3. For every prohibited zone, check horizontal geometry, vertical overlap, and temporal overlap together. Check the full interval during which each route segment occupies/intersects the zone, not only waypoint timestamps, the segment endpoints, or mission-start time.
4. If any waypoint or any portion of a segment occupies the zone during the inclusive NOTAM validity interval, reject that candidate and return a deterministic conflict finding identifying the NOTAM and affected waypoint/segment to the optimizer.
5. Rebuild the candidate to avoid the conflicting zone, then reapply wind and recalculate the complete time profile. Revalidate the entire changed route against all zones and constraints. Never reuse the previous candidate's timing profile after route geometry changes.
6. Accept the route only when all required checks pass. Missing, invalid, stale, or unreliable timing/validity inputs must not be silently treated as clear airspace.

All timestamps shall be normalized to a common UTC epoch before comparison. The NOTAM validity endpoints are inclusive. If the NOTAM is inactive throughout the aircraft's occupancy interval, that NOTAM alone must not reject or penalize the route. If the NOTAM becomes active during any part of the occupancy interval, the candidate conflicts.

## Shortest feasible detour — mandatory optimization rule

When a prohibited zone conflicts with the candidate route, the optimizer shall seek the best feasible detour around the zone, rather than inserting an arbitrary or unnecessarily long diversion. “Best” is determined by the optimizer's existing objective function among routes that satisfy all applicable hard constraints. If the objective is route distance, choose the shortest feasible path; if the objective is flight time or another defined mission cost, optimize that objective using the wind-adjusted time/cost model. For equal objective values, prefer the shorter route where this is consistent with the existing optimizer's tie-breaking rules.

A detour must respect the zone's horizontal boundary, applicable altitude limits, NOTAM validity during actual occupancy, aircraft performance limits, and all other route constraints. Where multiple zones conflict, evaluate the candidate against all of them; a detour around one zone must not be accepted if it introduces a conflict with another. After every route change, rerun wind application, time-profile calculation, and full-route validation until a feasible candidate is found or the optimizer reports that no feasible candidate could be established within its defined search limits.

Wind-aware time feasibility and route optimality are coupled: the geometrically shortest path is not necessarily the shortest-time path in wind. The objective function already defined for the optimizer remains authoritative; this requirement does not silently replace it with a different objective.

## NOTAM boundary visualization — mandatory HMI behavior

The existence of a NOTAM and restriction information in its source text is sufficient to require an HMI representation of the stated spatial limits. Visualization must not be suppressed merely because a boundary cannot yet be converted into the canonical route altitude datum or evaluated for route applicability.

1. **Show source-defined limits.** Display the NOTAM's horizontal geometry and its stated lower/upper vertical limits on the map and in the mission altitude-profile workspace whenever those values are available in the source record. Preserve the original value, unit, and reference wording (for example, SFC, AMSL, AGL, or FL) alongside any normalized representation.
2. **Do not invent geometry or altitude.** If horizontal geometry is absent or cannot be parsed, show the NOTAM as a spatial restriction record with an explicit geometry-unavailable state; do not fabricate a footprint. If one vertical limit is absent or ambiguous, show the known limit and mark the other as unknown. Never substitute zero or infer AGL/AMSL/Flight Level.
3. **Represent uncertainty visibly.** When source limits cannot be converted to the profile's vertical datum, render them as a separately identified source-datum boundary/annotation, not as a falsely aligned altitude band. Clearly label datum/units and conversion status. An uncomputed route intersection must be marked as such; do not invent affected route distance.
4. **Separate display from validation.** Rendering a NOTAM boundary does not prove that the route intersects or avoids it, does not imply that the NOTAM is active for the mission interval, and does not authorize flight. The authoritative validator continues to evaluate a versioned normalized snapshot and must fail safely when required geometry, validity, or altitude interpretation is missing.
5. **Retain records when overlays are filtered.** Map/profile display filters may change which boundaries are emphasized for a selected date or mission interval, but must not delete source records from the snapshot supplied to validation. The pre-route overview should make source-defined restrictions visible; mission-time applicability must be separately identified once a trustworthy mission interval exists.

This is a mandatory presentation requirement for the map and mission profile. It does not relax any validation or release gate.

