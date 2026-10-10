# BlueSky PRO — Route Constraint Validation Boundary

**ID:** PLAN-VAL-001
**Status:** BASELINED
**Scope:** Phase B — Flight Planning Core

The deterministic validator checks the canonical Route before optimization or vehicle-specific compilation.

Hard constraints are feasibility conditions, not optimization weights. Current checks cover route structure, segment waypoint references, waypoint connectivity and altitude limits.

Each result contains Allowed/Rejected status, stable finding codes, affected object identity, validator version and calculation-input version.

Boundary: Route -> Constraint Validator -> Validation Result -> Optimizer / Release Validation.

The validator does not modify Route, readiness, safety, authorization or execution state. AI proposals remain subject to the same deterministic validation boundary.