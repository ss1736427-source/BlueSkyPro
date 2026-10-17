# BlueSky PRO — Route Constraint Validation Boundary

**ID:** PLAN-VAL-001  
**Status:** BASELINED  
**Scope:** Phase B — Flight Planning Core

The deterministic validator checks the canonical Route before optimization or vehicle-specific compilation.

Hard constraints are feasibility conditions, not optimization weights. The structural validation slice checks:

- required route, mission, generator and calculation-input lineage;
- at least two ordered waypoints;
- unique, non-empty waypoint identifiers;
- finite geodetic positions within latitude [-90, 90] and longitude [-180, 180];
- finite waypoint altitudes and configured altitude bounds;
- finite, internally consistent altitude limits and non-negative minimum clearance;
- unique, non-empty segment identifiers;
- existing segment endpoint references and no self-referencing or duplicate directed segments;
- finite non-negative segment distance and finite positive nominal speed;
- connectivity of every waypoint to at least one segment;
- a directed segment between each pair of consecutive waypoints in canonical order, with no backward or waypoint-skipping segments.

Each result contains Allowed/Rejected status, stable finding codes, affected object identity, validator version and calculation-input version. The validator is deterministic and does not mutate the input Route.

**Boundary limitation:** these checks establish structural and numeric consistency only. They do not prove terrain/obstacle clearance, airspace legality, weather/wind freshness, vehicle feasibility, multi-UAV conflict freedom, authorization, or release eligibility. Those checks require their corresponding versioned inputs and validators. An Allowed result from this validator alone must not be interpreted as a flight-ready route.

Boundary: Route -> Constraint Validator -> Validation Result -> Optimizer / Release Validation.

The validator does not modify Route, readiness, safety, authorization or execution state. AI proposals remain subject to the same deterministic validation boundary.
