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

## Authority

The result is observational/planning validation only. It does not mutate the Route, readiness, safety, authorization or execution state.
