# Constrained Planning Graph

**ID:** PLAN-GEN-001
**Status:** BASELINED

The route generator shall search only the current feasible/open spatial domain.

The graph builder consumes the authorization-qualified constrained environment. Each graph edge is tested before entering the searchable graph. A blocked edge is not emitted as a selectable route candidate.

Authorization-qualified regulatory restrictions are therefore resolved upstream. Physical constraints remain hard.

The resulting graph retains the original node set but contains only feasible edges. The environment snapshot and dependency identity are carried with the result for calculation reuse and invalidation.

This component does not authorize flight, select a route, or execute the UAV. It is the deterministic spatial-search boundary consumed by route solvers such as Dijkstra/A*.

## NOTAM avoidance and bypass policy

For active prohibited NOTAM restrictions, the route-search graph must exclude every edge whose horizontal geometry intersects the restriction while the edge's vertical envelope overlaps the restriction's vertical band. The optimizer may seek an alternate path laterally around the footprint or a horizontal leg above the upper limit, but only when the altitude reference is compatible and the applicable upper limit and aircraft operating envelope are known. A climb/descent segment crossing the restricted vertical band inside the footprint remains blocked. If the datum, geometry, validity, or limit is uncertain, the system must not assume the route is clear; it must reject or mark the candidate unverified. Every generated candidate must pass the same final route/restriction validation before selection. If no feasible candidate exists, return no route and a reason rather than relaxing the restriction.
