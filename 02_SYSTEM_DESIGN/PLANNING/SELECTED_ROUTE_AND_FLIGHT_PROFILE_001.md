# Selected Route Set and Flight Profile

**ID:** PLAN-PROFILE-001
**Status:** BASELINED

## Purpose

Freeze the selected planning result as a versioned planning artifact and derive the flight profile from the already selected route without repeating route, environment, wind, performance, or conflict calculations.

## Boundary

`SelectedRouteSet` records the mission/version, selected candidate, route elements, calculation-input version and source dependency identity.

`FlightProfile` is a deterministic projection of the selected canonical `Route` into ordered position/altitude/time-profile points. It preserves route and selection lineage.

The profile is planning data; it is not an executable mission package and does not authorize execution.

## Recalculation

Changing a route, selected candidate, or relevant upstream dependency creates a new version. Unaffected calculations are reused according to the calculation-reuse baseline.

## Next dependency

Vehicle-specific compilation must add UAV configuration/performance constraints and produce the executable-mission-package input. It must not reinterpret regulatory authorization or bypass safety/readiness gates.

Canonical pipeline:

`SELECTED ROUTE SET → FLIGHT PROFILE → VEHICLE-SPECIFIC COMPILATION → FINAL INTEGRITY CHECK`
