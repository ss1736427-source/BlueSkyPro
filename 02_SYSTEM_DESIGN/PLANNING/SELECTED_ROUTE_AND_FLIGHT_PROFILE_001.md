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


## Flight Profile — NOTAM Vertical Bands and Time Applicability

The flight-profile view shall display the same applicable NOTAM restrictions as the Flight Chart, derived from the same versioned normalized environment snapshot. Each restriction with supported geometry and altitude limits shall be represented as a vertical band over the route-distance interval affected by that geometry, bounded by the NOTAM's lower and upper altitude limits.

- The horizontal axis represents distance along the selected route; the vertical axis represents altitude using an explicitly identified vertical reference.
- Band labels/inspection shall expose the NOTAM identifier, lower and upper altitude limits and datum, validity interval, source, and snapshot/version. Selecting a band shall open the same original NOTAM text as selecting the zone on the map.
- After route calculation, the displayed restrictions shall be filtered to those whose validity intervals overlap the mission interval. Where estimated time of passage is available, the profile should identify temporal applicability along the affected route segment.
- AGL, AMSL, and Flight Level values must not be compared or drawn on one common vertical scale unless a valid conversion is available. If conversion is unavailable or a bound is missing, the profile must mark the band/limit as unknown rather than imply a safe vertical separation.
- The profile is a synchronized presentation of the planning environment and does not independently calculate, remove, or authorize restrictions. Changes to the selected route or relevant planning inputs must update the displayed profile version and trigger the prescribed revalidation flow.
