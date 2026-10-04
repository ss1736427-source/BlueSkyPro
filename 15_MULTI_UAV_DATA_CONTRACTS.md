# BlueSky PRO — Multi-UAV Planning Data Contracts

**Status:** ARCHITECTURE BASELINE — machine-readable contract  
**Schema:** `schemas/multi-uav-planning.schema.json`  
**Version:** 1.0

## 1. Purpose

This contract defines the canonical planning artifacts between the Common Mission Model, Multi-UAV planning stages, Operational Validation and the final technical release gate.

It extends the existing Common Mission Model. It does not replace or duplicate the Mission Model.

```text
COMMON MISSION MODEL
        ↓
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
ConflictResolution (when required)
        ↓
TrajectorySet → ConflictReport
        ↓
FinalCheckResult
```

## 2. Contract rules

Every artifact has:

- `artifactId`
- monotonically increasing `version`
- `missionId`
- `status`
- `sourceRefs`
- `algorithmVersion`
- `configVersion`
- `traceability`

The traceability object records source versions, evidence references, invalidation references and an optional deterministic replay key.

An artifact is valid only against the exact source versions recorded in its traceability block.

## 3. Canonical artifacts

| Artifact | Role | Authoritative output |
|---|---|---|
| `ConstrainedOpenSpace` | Removes prohibited/restricted operational geometry | Valid planning space |
| `ZoneSet` | Spatially decomposes the valid space | Verified non-overlapping zones |
| `ZoneAssignmentSet` | Assigns UAVs to zones | Feasible UAV↔zone mapping |
| `RouteSet` | Generates routes inside assigned zones | Geometrically valid routes |
| `PerformanceAdjustedRouteSet` | Applies wind/performance/energy model | Time/energy-adjusted segments |
| `TrajectorySet` | Adds authoritative time and altitude | 4D trajectories |
| `ConflictReport` | Verifies fleet-wide 4D separation | Conflict classification/evidence |
| `ConflictResolution` | Records bounded pre-execution correction | Explicit correction + re-verification references |
| `FinalCheckResult` | Performs final technical dependency gate | RELEASE_ELIGIBLE or BLOCKED |

## 4. Version and invalidation rules

The dependency graph is strict:

```text
Mission / Environment
        ↓
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

Changes invalidate downstream artifacts:

- constrained space → zones and everything downstream;
- zones → assignment and everything downstream;
- assignment → routes and everything downstream;
- routes → performance, trajectory, conflict and final check;
- performance/environment/timing → trajectory, conflict and final check;
- trajectory → conflict and final check;
- conflict resolution affecting trajectory → new trajectory, fleet-wide conflict report and final check;
- UAV configuration/readiness changes → all affected feasibility/performance/trajectory/conflict checks.

No downstream PASS may be reused after its source version changes.

## 5. Geometry semantics

`ConstrainedOpenSpace` is the authoritative operational geometry available to planning.

`ZoneSet` shall satisfy:

```text
ZONE ⊂ CONSTRAINED OPEN SPACE
ZONE OVERLAP = 0
COVERAGE = COMPLETE
        OR
COVERAGE = EXPLICITLY UNAVAILABLE
```

Any shared corridor, transition volume or recovery volume must be explicit and subsequently verified.

`RouteSet` routes remain inside the assigned zone except for explicitly represented launch/recovery/transition segments.

## 6. Assignment semantics

Assignment separates feasibility from optimization.

A candidate assignment records:

- feasibility;
- capability margin;
- energy margin;
- time margin;
- wind margin;
- launch/recovery margin;
- authorization/C2 status;
- score;
- evidence.

An infeasible candidate cannot become feasible through optimization scoring.

The normal topology is one primary UAV per zone and one primary zone per UAV. Concurrent multi-zone ownership requires explicit scheduling semantics.

## 7. Performance semantics

`PerformanceAdjustedRouteSet` carries segment-level:

- distance;
- ground speed;
- time;
- energy;
- wind reference;
- cumulative time;
- cumulative energy;
- reserve.

Forecast wind and observed wind remain distinguishable through source references.

Battery degradation and learned corrections must be versioned and traceable.

The Performance stage must not silently change altitude. Any altitude change must be an explicit, validated trajectory-affecting change.

## 8. 4D trajectory semantics

`TrajectorySet` is authoritative for conflict verification.

Each trajectory point contains:

```text
(lat, lon, alt, t, state, uavId)
```

Time is monotonic per trajectory. Launch delay, hold, recovery and other temporal states are represented explicitly.

The Conflict Engine must consume the supplied `TrajectorySet`; it must not reconstruct timing independently.

## 9. Conflict semantics

A 2D geometric crossing is not automatically a conflict.

Conflict classification considers:

- horizontal proximity;
- temporal overlap;
- vertical separation;
- required separation;
- uncertainty and margins;
- operational/shared volumes.

Required states:

- `NO_CONFLICT`
- `WARNING`
- `CONFLICT`
- `UNRESOLVED`

`UNRESOLVED` is blocking because the system cannot demonstrate admissible separation.

## 10. Conflict resolution semantics

Conflict resolution is pre-execution planning logic, not an in-flight collision-avoidance controller.

Priority:

1. spatial regeneration;
2. temporal correction;
3. permitted vertical correction.

Temporal correction is bounded:

```text
0 ≤ delay ≤ 5 seconds
```

A correction is accepted only after fleet-wide 4D re-verification.

Vertical correction requires all applicable altitude, airspace, aircraft, mission, performance, authorization and conflict constraints to pass.

Every accepted correction records before/after values and links to the reverified trajectory and conflict report.

## 11. Final verification

`FinalCheckResult` is the final technical planning gate.

It verifies the latest linked versions of:

- mission/coverage;
- constrained space;
- zones;
- assignments;
- UAV readiness/capability;
- routes;
- performance/energy/reserve;
- trajectories;
- conflict verification;
- unresolved conflicts;
- traceability;
- version consistency.

`RELEASE_ELIGIBLE` means that the BlueSky technical planning gate has passed. It does not itself constitute regulatory authorization.

## 12. Determinism and replay

For identical:

- mission inputs;
- source versions;
- UAV configuration;
- environmental references;
- algorithm versions;
- configuration versions;

the planning state and verification result shall be reproducible.

A correction creates a new version rather than mutating the previous result.

## 13. Implementation boundary

This schema defines contracts, not executable implementation.

The repository currently contains the architecture/documentation foundation but does not yet contain the executable Qt/C++/QML planning core. Implementation shall consume these contracts rather than introduce an incompatible parallel data model.

## 14. Acceptance baseline

The first executable integration target is:

```text
2 UAV
↓
1 constrained operational area
↓
2 verified non-overlapping zones
↓
1 UAV per zone
↓
routes contained in zones
↓
wind/performance
↓
4D trajectories
↓
NO_CONFLICT
↓
FINAL_CHECK_PASS
↓
RELEASE_ELIGIBLE
```

The next mandatory negative/regression cases are:

- invalid restricted geometry;
- insufficient feasible UAVs;
- route outside assigned zone;
- insufficient energy reserve;
- 4D conflict;
- uncertainty → UNRESOLVED;
- feasible temporal correction ≤5 s;
- correction causing a secondary conflict;
- source-version change invalidating downstream PASS;
- deterministic replay.

## 15. Relationship to certification and assurance

These contracts are designed to support evidence generation and traceability. They do not themselves constitute certification, regulatory approval or a claim of compliance with a specific jurisdictional standard.

Certification-specific requirements remain allocated through the Assurance, Regulatory and Operational Validation contours.
