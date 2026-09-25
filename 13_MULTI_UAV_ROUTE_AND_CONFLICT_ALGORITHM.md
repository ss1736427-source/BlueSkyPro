# BlueSky PRO — Zone-Based Multi-UAV Route and 4D Conflict Algorithm

## Purpose

This algorithm defines the planning order for multi-UAV coverage missions. Its primary objective is to prevent trajectory intersections by spatial decomposition before applying temporal or vertical conflict resolution.

## Authoritative sequence

```
MISSION / COVERAGE
        ↓
CONSTRAINED OPEN SPACE
        ↓
ZONE PARTITION
        ↓
UAV ↔ ZONE ASSIGNMENT
        ↓
ROUTE-IN-ZONE
        ↓
WIND + PERFORMANCE
        ↓
4D TRAJECTORY
        ↓
4D CONFLICT VERIFY
        ↓
IF ZONAL RESOLUTION IS IMPOSSIBLE
        ↓
GROUND CONFLICT RESOLUTION
        ├─ delay 0–5 s
        └─ vertical correction
        ↓
FINAL CHECK
```

## Rules

### 1. Mission / Coverage

The mission defines the coverage geometry, operational boundaries, required resolution/coverage pattern, aircraft fleet and mission constraints.

### 2. Constrained Open Space

Remove all regions that cannot be used for the planned operation. The resulting geometry is the only geometry available to the zone partitioner.

### 3. Zone Partition

Partition the usable geometry into zones/sectors. The preferred result is a **clean zone per UAV**, with boundaries selected so that planned routes do not cross between UAV zones.

The partition should account for:

- aircraft capability;
- payload and coverage requirements;
- operational restrictions;
- launch/recovery geometry;
- required route pattern;
- expected wind/performance;
- workload balance.

### 4. UAV ↔ Zone Assignment

Assign each UAV to one operational zone. Assignment is a planning decision and must be verified against aircraft capability and mission constraints.

### 5. Route-in-Zone

Generate the coverage route entirely within the assigned zone whenever geometrically possible.

The planner must not intentionally create intersecting trajectories and depend on later collision resolution if the same coverage objective can be achieved through zonal decomposition.

### 6. Wind + Performance

Evaluate the route against current/forecast environmental conditions and aircraft performance. This stage may modify:

- ground speed;
- heading;
- altitude;
- segment timing;
- energy consumption;
- reserve requirements.

### 7. 4D Trajectory

Convert each route into a time-dependent trajectory:

```
T = f(latitude, longitude, altitude, time)
```

The conflict engine operates on these 4D trajectories rather than on 2D route lines alone.

### 8. 4D Conflict Verify

Verify fleet-wide separation and identify:

- spatial intersections;
- temporal overlap at intersections;
- insufficient separation margins;
- convergence into the same operational volume;
- conflicts introduced by wind/performance timing changes.

A geometric crossing is not automatically an operational conflict if the required separation is maintained in time and/or altitude; the 4D verification determines the actual conflict state.

### 9. Ground Conflict Resolution

If a conflict remains and cannot be eliminated by changing the zones/routes, apply bounded planned corrections:

**Temporal correction**
- delay range: **0–5 seconds**.

**Vertical correction**
- change altitude only when permitted by mission constraints, aircraft capability, restricted-airspace limits and applicable operational rules.

The term **Ground Conflict Resolution** refers to resolution performed by the planning/ground system before execution. It does not mean an instruction to improvise an unsafe maneuver in flight.

### 10. Final Check

After every correction, rerun:

- constraints;
- route validity;
- aircraft capability;
- wind/performance;
- energy/reserve;
- 4D conflict verification.

Only a plan that passes the complete final check may proceed to the next mission/authorization stage.

## Conflict-resolution priority

1. **Spatial separation by zone partition**
2. **Route separation inside assigned zones**
3. **4D verification**
4. **Temporal correction up to 5 s**
5. **Permitted vertical correction**
6. **Re-verification**
7. **Reject/hold the plan if unresolved**

This ordering keeps conflict handling deterministic, auditable and traceable.

## 11. Formal Processing Model

The planning pipeline is defined as deterministic processing stages. Each stage has explicit inputs, outputs and a release gate.

### Stage A — Mission / Coverage
**Inputs:** mission geometry, coverage objective, operational boundaries, coverage pattern/resolution, UAV fleet, mission constraints.  
**Output:** MissionCoverageModel.  
**Gate:** mission geometry and objective are valid and complete.

### Stage B — Constrained Open Space
**Inputs:** MissionCoverageModel, restricted/prohibited/unavailable regions, altitude limits, mandatory corridors and exclusion buffers.  
**Output:** ConstrainedOpenSpace.  
**Gate:** usable geometry exists and is valid.

### Stage C — Zone Partition
**Inputs:** ConstrainedOpenSpace, UAV capabilities, coverage requirements, launch/recovery constraints, workload-balancing criteria.  
**Output:** ZoneSet.

Each zone has a unique identifier and explicit geometry. Zone boundaries are planning constraints.

**Gate:** required coverage is assigned or explicitly marked uncovered; forbidden geometry is excluded.

### Stage D — UAV ↔ Zone Assignment
**Inputs:** ZoneSet, UAVCapabilitySet, aircraft readiness/availability.  
**Output:** ZoneAssignmentSet.

Each assignment records UAV_ID → Zone_ID → capability validation → assignment status.

**Gate:** every assigned UAV is capable of its zone workload and constraints.

### Stage E — Route-in-Zone
**Inputs:** ZoneAssignmentSet, coverage pattern, route constraints.  
**Output:** RouteSet.

Each route references exactly one assigned zone.

**Gate:** route remains inside its assigned zone except explicitly defined launch/recovery transitions.

### Stage F — Wind + Performance
**Inputs:** RouteSet, forecast/observed wind, aircraft performance model, payload/configuration, degradation/energy history.  
**Output:** PerformanceAdjustedRouteSet.

This stage produces timing and energy estimates; it does not silently change the mission objective.

### Stage G — 4D Trajectory
**Inputs:** PerformanceAdjustedRouteSet.  
**Output:** TrajectorySet.

Each trajectory contains at minimum position, altitude, time, UAV_ID and route/zone reference.

### Stage H — 4D Conflict Verify
**Inputs:** TrajectorySet, applicable separation criteria, operational constraints.  
**Output:** ConflictReport.

Each conflict records UAV pair, trajectory segment(s), location/volume, time interval, separation deficit and source stage.

**Gate:** no unresolved conflict, or an eligible conflict is passed to Stage I.

### Stage I — Ground Conflict Resolution
**Inputs:** ConflictReport and mutable planning parameters.

**Allowed corrections:**
- temporal delay: 0–5 s;
- permitted vertical correction.

**Output:** ResolvedTrajectorySet or UnresolvedConflict.

Every correction is explicit and traceable to the conflict that caused it.

### Stage J — Final Check
Rerun all relevant gates:

constraints → route validity → capability → wind/performance → energy/reserve → 4D conflict verification.

**Output:** FINAL_CHECK_PASS or FINAL_CHECK_FAIL.

FINAL_CHECK_FAIL blocks release of the multi-UAV plan.

## 12. Data Objects

| Object | Role |
|---|---|
| MissionCoverageModel | Mission and coverage definition |
| ConstrainedOpenSpace | Usable planning geometry |
| Zone | One operational sector |
| ZoneSet | Complete partition |
| ZoneAssignment | UAV-to-zone relationship |
| Route | Coverage path inside a zone |
| PerformanceAdjustedRoute | Route with environmental/performance timing |
| Trajectory4D | Time-dependent aircraft trajectory |
| Conflict | One detected fleet conflict |
| ConflictReport | Fleet conflict verification result |
| ConflictResolution | Planned temporal/vertical correction |
| FinalCheckResult | Release gate result |

## 13. State Transitions

```
DRAFT
  ↓
CONSTRAINED
  ↓
PARTITIONED
  ↓
ASSIGNED
  ↓
ROUTED
  ↓
PERFORMANCE_ADJUSTED
  ↓
TRAJECTORIZED
  ↓
CONFLICT_VERIFIED
  ↓
RESOLUTION_REQUIRED → RESOLVED
                         ↓
                    FINAL_CHECK
                         ↓
                      RELEASED
```

If any mandatory gate fails:

```
CURRENT_STATE → BLOCKED
```

A blocked plan cannot be released without returning to the appropriate planning stage.

## 14. Traceability Requirements

Every generated route and correction should be attributable to:
- mission version;
- planning run/version;
- UAV configuration;
- zone version;
- environmental data version/time;
- performance model version;
- conflict-verification run;
- correction reason;
- final-check result.

This creates an audit chain for engineering verification and certification work.

## 15. Implementation Boundary

The algorithm should be implemented as separable services/modules:

```
Mission Model
     ↓
Constraint Engine
     ↓
Zone Partition Engine
     ↓
Assignment Engine
     ↓
Route Generator
     ↓
Wind / Performance Engine
     ↓
4D Trajectory Engine
     ↓
4D Conflict Engine
     ↓
Conflict Resolution Engine
     ↓
Final Verification Gate
```

The UI consumes planning objects and verification states. It does not independently implement conflict-resolution logic.

## 16. Determinism and Auditability

For identical inputs and identical model/configuration versions, the planning pipeline should produce reproducible results or explicitly record any source of nondeterminism.

Each planning run should have:
- unique planning_run_id;
- input/configuration version references;
- algorithm version;
- timestamp;
- result state;
- verification evidence.

## 17. Zone Partition Engine — Formal Model

### Objective

The Zone Partition Engine converts the constrained mission area into operational zones assigned to individual UAVs. The primary objective is to eliminate route intersections by construction while preserving mission coverage and respecting aircraft and operational constraints.

The engine is a planning component, not an in-flight collision-avoidance controller.

### 17.1 Inputs

The engine consumes:
- ConstrainedOpenSpace geometry;
- number of available UAVs;
- UAV capability profiles;
- launch/recovery locations;
- coverage pattern and required density;
- altitude constraints;
- mandatory corridors/waypoints;
- no-go and restricted buffers;
- expected wind/performance envelope;
- workload/coverage balancing parameters.

### 17.2 Geometry model

Let O = constrained open operational space, Z_i = zone assigned to UAV i, and U = set of participating UAVs.

The partition must satisfy Z_i ⊆ O. For distinct zones, Z_i ∩ Z_j should be empty except for explicitly defined transition, boundary or recovery areas.

The mission coverage requirement is the union of all Z_i covering C_required. If complete coverage cannot be achieved without violating constraints, the uncovered region must be explicitly reported rather than silently assigned.

### 17.3 Hard constraints

The following are hard constraints: prohibited/restricted geometry; mandatory operational boundaries; aircraft capability limits; altitude restrictions; mandatory waypoints/corridors where applicable; launch/recovery feasibility; minimum required separation geometry where specified; mission safety constraints. A candidate partition violating a hard constraint is rejected.

### 17.4 Soft objectives

Among valid partitions, the engine may optimize balanced workload, route length, estimated energy, flight time, boundary complexity, route transitions, wind exposure and launch/recovery burden. The objective is multi-criteria rather than simple geometric area equality.

### 17.5 Capability-aware partitioning

Equal-area partitioning is not mandatory. For UAVs with different capabilities, target workload may be weighted by range, endurance, payload, speed and constraints. Different zone sizes are therefore valid when coverage and operational constraints remain satisfied.

### 17.6 Candidate partition generation

Preferred hierarchy: 1) simple contiguous sectors; 2) sweep-oriented sectors aligned with the dominant coverage direction; 3) capability-weighted sectors; 4) constrained polygon decomposition; 5) fallback strategies for fragmented operational space.

The engine should avoid unnecessary fragmented zones.

### 17.7 Partition scoring

Each valid candidate can be evaluated with a configurable multi-criteria objective covering coverage, workload balance, energy, time, wind and geometric complexity. Exact coefficients are configuration parameters and must be versioned with the planning run.

The selected candidate and evaluation evidence are retained for traceability.

### 17.8 Boundary and route-crossing principle

The partitioner should prefer boundaries following the natural coverage sweep direction. The objective is to minimize cross-zone transitions, repeated boundary crossings, converging entry/exit paths and unnecessary common corridors.

### 17.9 Fragmented open space

If the usable space consists of disconnected components, identify connected operational components first. Each component is then treated as a partitioning problem subject to fleet assignment constraints. A UAV must not be assigned a disconnected workload unless the aircraft and mission model explicitly support that transition.

### 17.10 Impossible partition

If no valid partition satisfies all hard constraints, the engine returns PARTITION_FAILED with structured reasons such as insufficient UAV capability, insufficient operational space, incompatible altitude constraints, unreachable zone, mandatory corridor conflict or coverage exceeding fleet capacity.

PARTITION_FAILED blocks automatic release and requires replanning or human intervention.

### 17.11 Output

The engine produces ZoneSet containing at least: zone_id; geometry; assigned UAV candidate(s); coverage workload; estimated route length; estimated time; estimated energy; constraint references; partition version; score/evaluation metadata.

### 17.12 Determinism

For identical mission geometry, constraint set, fleet, configuration and algorithm version, the same partition should be reproducible. If future optimization uses stochastic methods, the planning run must record the seed and algorithm configuration.

### 17.13 Partition verification

Before UAV assignment, verify:

ZONE ⊂ CONSTRAINED OPEN SPACE
ZONE OVERLAP = 0
COVERAGE = COMPLETE or EXPLICITLY UNAVAILABLE
HARD CONSTRAINTS = PASS
ZONE GEOMETRY = VALID

Only a verified ZoneSet can enter UAV ↔ ZONE ASSIGNMENT.

## 18. Separation from 4D Conflict Resolution

Zone partitioning is the preventive spatial planning mechanism. 4D conflict verification is the verification mechanism. Ground conflict resolution is the bounded planning correction mechanism.

PARTITION → ROUTE GENERATION → 4D VERIFY → CORRECT IF NECESSARY → VERIFY AGAIN

The conflict-resolution stage must not be used as a substitute for defective zone partitioning.
