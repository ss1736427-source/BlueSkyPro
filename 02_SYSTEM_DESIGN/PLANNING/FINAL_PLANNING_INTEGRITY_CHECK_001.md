# Final Planning Integrity Check

**ID:** PLAN-INTEGRITY-001
**Status:** BASELINED

The final integrity check verifies that the planning chain is internally complete before the result can be handed to downstream readiness/mission-package stages.

It checks identity and lineage continuity across Mission, Route, Flight Profile, UAV configuration and calculation-input version, and requires successful vehicle-specific compilation.

This is a consistency check, not a recalculation. It does not grant authorization, change safety state, establish READY, or execute a mission.

Canonical planning chain:

`MISSION → CONSTRAINED ROUTE SEARCH → CANDIDATES → SELECTION → FLIGHT PROFILE → VEHICLE COMPILATION → FINAL INTEGRITY`

Only affected downstream artifacts are invalidated when a dependency changes.
