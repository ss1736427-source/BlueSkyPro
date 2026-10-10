# BlueSky PRO — Multi-UAV Contract Verification Matrix

**Status:** ARCHITECTURE BASELINE — validator/test contract  
**Schema:** `schemas/multi-uav-planning.schema.json`

## 1. Purpose

This document defines the minimum automated checks required before the Multi-UAV planning contracts can be considered executable-ready.

The fixtures in `schemas/examples/multi-uav-regression-fixtures.json` are test inputs/expected outcomes. They are not a substitute for the future planning implementation.

## 2. Verification layers

### Layer A — Schema validation

Every artifact must satisfy the JSON Schema:

```text
JSON INSTANCE
    ↓
STRUCTURAL VALIDATION
    ↓
SCHEMA PASS / FAIL
```

Checks include:

- required fields;
- data types;
- enumerations;
- numeric bounds;
- timestamp format;
- object structure;
- no undeclared properties.

### Layer B — Cross-object semantic validation

Schema validity alone is insufficient.

The validator shall verify:

- every artifact belongs to the same `missionId`;
- every `sourceRef` resolves to an existing artifact/version;
- `traceability.inputVersions` agrees with source references;
- downstream versions correspond to the exact inputs used;
- UAV and zone identifiers resolve;
- every assigned zone has a feasible UAV;
- every route segment references its assigned zone/UAV;
- every performance segment resolves to a route segment;
- every trajectory belongs to a known UAV;
- every conflict report references the authoritative trajectory version.

### Layer C — Geometry validation

The validator shall verify:

- operational geometry is valid;
- excluded geometry is respected;
- every zone is contained in constrained operational space;
- zone overlap is zero except explicitly modeled shared volumes;
- coverage is complete or explicitly unavailable;
- every route remains in its assigned zone except explicit transitions;
- route geometry is continuous and valid.

### Layer D — Temporal / 4D validation

The validator shall verify:

- trajectory timestamps are monotonic per UAV;
- trajectory points contain valid position and altitude;
- segment timing is consistent with performance results;
- speed and climb/descent limits are respected;
- shared launch/recovery volumes are explicitly represented;
- conflict analysis uses the supplied TrajectorySet rather than reconstructing timing.

### Layer E — Conflict validation

The validator shall distinguish:

```text
2D crossing
    ≠
automatic conflict
```

A conflict requires evaluation of:

- horizontal separation;
- temporal overlap;
- vertical separation;
- required separation;
- uncertainty;
- applicable operational volumes.

Required classifications:

- `NO_CONFLICT`
- `WARNING`
- `CONFLICT`
- `UNRESOLVED`

`UNRESOLVED` is release-blocking.

### Layer F — Resolution validation

For temporal correction:

```text
0 ≤ delay ≤ 5 s
```

The validator shall reject:

- negative delays;
- delays above 5 seconds;
- corrections without source conflict;
- corrections without before/after values;
- corrections without re-verification;
- corrections whose re-verification creates a secondary conflict.

Vertical correction is accepted only when the applicable altitude, airspace, aircraft, mission, performance, authorization and conflict constraints pass.

### Layer G — Invalidation validation

A changed source version shall invalidate all dependent PASS results.

Minimum dependency graph:

```text
ConstrainedOpenSpace
        ↓
ZoneSet
        ↓
ZoneAssignmentSet
        ↓
RouteSet
        ↓
PerformanceAdjustedRouteSet
        ↓
TrajectorySet
        ↓
ConflictReport
        ↓
FinalCheckResult
```

Additional sources such as UAV configuration, readiness, environment and authorization constraints invalidate every affected downstream stage.

The validator must never accept a stale downstream PASS.

### Layer H — Final gate validation

`RELEASE_ELIGIBLE` requires:

- all mandatory checks PASS;
- zero unresolved conflicts;
- complete traceability;
- consistent versions;
- current UAV readiness/configuration;
- current environmental inputs;
- no blocking findings.

`RELEASE_ELIGIBLE` must remain distinct from regulatory authorization.

## 3. Mandatory regression cases

| Test | Required outcome |
|---|---|
| E2E-014 | `CONFLICT` → BLOCKED |
| E2E-015 | `UNRESOLVED` → BLOCKED |
| E2E-016 | ≤5 s correction → fleet reverify → RESOLVED |
| E2E-018 | secondary conflict → correction rejected |
| E2E-019 | no feasible correction → BLOCKED |
| E2E-020 | route change → downstream invalidated |
| E2E-021 | performance change → trajectory/conflict invalidated |
| E2E-022 | UAV config/readiness change → affected chain invalidated |
| E2E-023 | identical inputs → deterministic identical result |
| E2E-026 | version mismatch → FINAL_CHECK_FAIL |
| E2E-027 | technical release ≠ authorization |

## 4. Evidence requirements

Each automated verification record should retain:

- test ID;
- mission ID/version;
- artifact IDs/versions;
- algorithm version;
- configuration version;
- environment references;
- input hash or replay key;
- expected result;
- actual result;
- pass/fail;
- evidence references;
- timestamp;
- validator version.

## 5. First implementation target

The first executable validator should support:

```text
LOAD SCHEMA
   ↓
LOAD BASELINE INSTANCE
   ↓
SCHEMA VALIDATE
   ↓
SEMANTIC VALIDATE
   ↓
DEPENDENCY VALIDATE
   ↓
FINAL GATE VALIDATE
   ↓
EXPECTED = RELEASE_ELIGIBLE
```

Then execute the negative fixtures individually.

## 6. Boundary

This matrix defines verification behavior. It does not claim that the repository already contains the validator or flight-planning executable.

The next implementation layer is therefore a small deterministic contract-validation service/test harness, not a full route planner.
