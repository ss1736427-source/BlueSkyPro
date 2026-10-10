# BlueSky PRO — Zonal Route Graph Boundary

**ID:** PLAN-MULTI-008  
**Status:** BASELINED

The zonal route graph is the deterministic boundary between zone assignment and route search.

For an assigned UAV, the route-search graph is filtered so that:
- only nodes inside the assigned operational zone are searchable;
- only edges whose endpoints and midpoint remain inside the zone are searchable;
- the original mission/environment constraints remain applicable;
- a route cannot be selected merely because it crosses another UAV's zone.

The zone is a planning boundary, not an authorization or safety override.

The graph result carries zone identity/version and dependency identity and is reusable until its source graph or zone changes.

If the resulting graph has no valid start/goal path, zonal planning is infeasible for that assignment. The planner may then invoke the established multi-UAV ground conflict-resolution fallback rather than silently allowing cross-zone routing.

This component does not authorize, set READY, execute commands, or alter the constrained-open-space environment.
