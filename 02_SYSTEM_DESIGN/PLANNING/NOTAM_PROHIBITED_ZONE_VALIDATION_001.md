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

## Mission-time applicability — mandatory rule

A prohibited NOTAM zone is an active planning constraint only during the validity interval supplied by the normalized NOTAM record. The map's active-restriction presentation and route optimizer/validator must use the same mission-time applicability rule: a zone is shown as a current mission restriction and constrains route evaluation when the mission/flight interval overlaps the NOTAM validity interval; outside that interval it is not shown as an active prohibited zone and must not reject or penalize the route on the basis of that NOTAM. For a point-time evaluation, the evaluation timestamp must be inside the inclusive NOTAM validity interval. A route segment that has an explicit time interval must be checked against NOTAM validity during the segment's occupancy of the zone, not solely against mission start time. Source records must remain available even when inactive overlays are hidden. All timestamps must be normalized to a common UTC epoch before comparison; missing or invalid validity timestamps must not be silently interpreted as active or inactive.

## Authority

The result is observational/planning validation only. It does not mutate the Route, readiness, safety, authorization or execution state.

## NOTAM boundary visualization — mandatory HMI behavior

The existence of a NOTAM and restriction information in its source text is sufficient to require an HMI representation of the stated spatial limits. Visualization must not be suppressed merely because a boundary cannot yet be converted into the canonical route altitude datum or evaluated for route applicability.

1. **Show source-defined limits.** Display the NOTAM's horizontal geometry and its stated lower/upper vertical limits on the map and in the mission altitude-profile workspace whenever those values are available in the source record. Preserve the original value, unit, and reference wording (for example, SFC, AMSL, AGL, or FL) alongside any normalized representation.
2. **Do not invent geometry or altitude.** If horizontal geometry is absent or cannot be parsed, show the NOTAM as a spatial restriction record with an explicit geometry-unavailable state; do not fabricate a footprint. If one vertical limit is absent or ambiguous, show the known limit and mark the other as unknown. Never substitute zero or infer AGL/AMSL/Flight Level.
3. **Represent uncertainty visibly.** When source limits cannot be converted to the profile's vertical datum, render them as a separately identified source-datum boundary/annotation, not as a falsely aligned altitude band. Clearly label datum/units and conversion status. An uncomputed route intersection must be marked as such; do not invent affected route distance.
4. **Separate display from validation.** Rendering a NOTAM boundary does not prove that the route intersects or avoids it, does not imply that the NOTAM is active for the mission interval, and does not authorize flight. The authoritative validator continues to evaluate a versioned normalized snapshot and must fail safely when required geometry, validity, or altitude interpretation is missing.
5. **Retain records when overlays are filtered.** Map/profile display filters may change which boundaries are emphasized for a selected date or mission interval, but must not delete source records from the snapshot supplied to validation. The pre-route overview should make source-defined restrictions visible; mission-time applicability must be separately identified once a trustworthy mission interval exists.

This is a mandatory presentation requirement for the map and mission profile. It does not relax any validation or release gate.

