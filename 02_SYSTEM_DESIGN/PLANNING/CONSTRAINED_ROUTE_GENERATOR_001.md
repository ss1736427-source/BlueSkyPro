# Constrained Planning Graph

**ID:** PLAN-GEN-001
**Status:** BASELINED

The route generator shall search only the current feasible/open spatial domain.

The graph builder consumes the authorization-qualified constrained environment. Each graph edge is tested before entering the searchable graph. A blocked edge is not emitted as a selectable route candidate.

Authorization-qualified regulatory restrictions are therefore resolved upstream. Physical constraints remain hard.

The resulting graph retains the original node set but contains only feasible edges. The environment snapshot and dependency identity are carried with the result for calculation reuse and invalidation.

This component does not authorize flight, select a route, or execute the UAV. It is the deterministic spatial-search boundary consumed by route solvers such as Dijkstra/A*.
