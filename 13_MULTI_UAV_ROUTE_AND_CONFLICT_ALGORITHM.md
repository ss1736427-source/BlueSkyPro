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

## 19. UAV ↔ Zone Assignment Engine — Formal Model

### Objective

The UAV ↔ Zone Assignment Engine selects a specific aircraft for each verified operational zone. Assignment is performed only after Zone Partition verification and before route generation. The engine must preserve independent aircraft accountability while selecting the fleet configuration that can execute the partition within mission constraints.

The engine separates **feasibility** from **optimization**: an infeasible UAV-zone pair is rejected; optimization is performed only among feasible candidates.

### 19.1 Inputs

The engine consumes:
- verified ZoneSet;
- available UAV fleet and unique UAV_ID values;
- UAV capability profiles;
- readiness and availability state;
- payload and equipment configuration;
- launch/recovery locations;
- authorization and operational restrictions;
- C2/communication availability and required link conditions;
- range, endurance and reserve requirements;
- estimated wind/performance conditions;
- maintenance/service status;
- mission-specific assignment constraints;
- assignment objective and configuration version.

### 19.2 Hard feasibility constraints

A UAV-zone candidate is feasible only when all mandatory conditions pass:
1. UAV is available and in the required readiness state.
2. Aircraft capability is sufficient for the zone workload and required coverage pattern.
3. Payload/equipment configuration is compatible with the mission.
4. Range/endurance and required energy reserve are sufficient for the assigned workload and launch/recovery profile.
5. Required altitude, speed and operational envelope are supported.
6. Launch and recovery are feasible for the selected UAV and zone.
7. Required authorization and operational restrictions permit the assignment.
8. Required C2/communication conditions are available.
9. No blocking maintenance or technical condition exists.
10. The assignment does not violate a mission-specific mandatory UAV/zone relationship.

A candidate failing any hard constraint is marked INFEASIBLE and is not passed to optimization.

### 19.3 Candidate matrix

For each UAV u and zone z, create an auditable candidate record:

```text
A[u,z] = {
  feasibility,
  failed_constraints[],
  capability_margin,
  energy_margin,
  time_margin,
  wind_margin,
  launch_recovery_margin,
  authorization_status,
  c2_status,
  score,
  evidence_refs[]
}
```

The matrix must retain both feasible and rejected candidates so that the assignment decision can be reconstructed.

### 19.4 Assignment rules

The default rule is one primary UAV per operational zone and one primary zone per assigned UAV.

If the number of available UAVs exceeds the number of zones, unassigned aircraft remain available/reserve and are not forced into a zone.

If the number of zones exceeds the number of available feasible UAVs, the mission is not silently compressed. The affected zones are reported as UNASSIGNED and the assignment stage returns ASSIGNMENT_FAILED unless the mission model explicitly supports sequential execution or another approved workload model.

One UAV may not be assigned to multiple simultaneous primary zones unless the mission model explicitly defines a non-overlapping sequential workload and the resulting schedule is verified before release.

### 19.5 Assignment optimization

Among feasible candidates, the engine may optimize a configurable multi-criteria objective using:
- capability fit and operational margin;
- energy/reserve margin;
- expected flight time;
- route/transition burden;
- wind exposure;
- launch/recovery burden;
- workload balance across the fleet;
- preservation of future conflict-free geometry;
- use of specialized aircraft where their capabilities materially improve mission feasibility.

Exact weights are configuration parameters. The selected configuration and score evidence must be versioned with the planning run.

The engine must not optimize by nominal area alone. A larger zone may be assigned to a higher-endurance UAV when this improves operational margins, while a smaller or more constrained zone may be assigned to a UAV with the required specialized capability.

### 19.6 Assignment states

```text
CANDIDATE
   ↓
FEASIBILITY_CHECK
   ├─ INFEASIBLE
   └─ FEASIBLE
          ↓
     OPTIMIZATION
          ↓
       ASSIGNED
```

If no valid complete assignment exists:

```text
ASSIGNMENT_FAILED → BLOCKED
```

### 19.7 Assignment verification

Before entering Route-in-Zone, verify:

```text
ALL REQUIRED ZONES = ASSIGNED or EXPLICITLY UNASSIGNED
NO UAV = DOUBLE-ASSIGNED TO SIMULTANEOUS PRIMARY ZONES
CAPABILITY = PASS
READINESS = PASS
PAYLOAD = PASS
ENERGY / RESERVE = PASS
LAUNCH / RECOVERY = PASS
AUTHORIZATION = PASS
C2 = PASS
MISSION CONSTRAINTS = PASS
ASSIGNMENT RECORDS = COMPLETE
```

Only a verified ZoneAssignmentSet enters route generation.

### 19.8 Failure handling

If assignment fails, the engine returns `ASSIGNMENT_FAILED` with structured reasons. Typical reasons include:
- insufficient feasible UAVs;
- UAV capability mismatch;
- insufficient endurance or reserve;
- payload incompatibility;
- unavailable launch/recovery option;
- authorization restriction;
- C2/communication failure;
- maintenance/readiness block;
- mandatory UAV-zone constraint conflict;
- partition requiring a workload model not supported by the current fleet.

Failure must return the planning process to the appropriate earlier stage. The system must not silently substitute an unsuitable aircraft.

### 19.9 Determinism and traceability

For identical ZoneSet, fleet state, mission constraints, configuration and algorithm version, assignment results should be reproducible.

Each assignment run records:
- planning_run_id;
- assignment_run/version;
- ZoneSet/partition version;
- fleet snapshot and UAV configuration references;
- readiness/availability snapshot;
- environmental/performance data references;
- assignment configuration and objective weights;
- candidate matrix or its immutable evidence reference;
- selected assignments;
- rejected alternatives and reasons;
- result state;
- timestamp.

### 19.10 Assignment output

Each ZoneAssignment should contain at minimum:
- assignment_id;
- UAV_ID;
- zone_id;
- assignment status;
- capability validation result;
- readiness/availability result;
- energy/reserve margin;
- expected time margin;
- authorization/C2 status;
- selected score/evaluation metadata;
- constraint/evidence references;
- assignment algorithm/configuration version.

The resulting ZoneAssignmentSet is the authoritative input to Route-in-Zone generation.

### 19.11 Relationship to later stages

The assignment engine must not generate the final route or resolve 4D conflicts itself.

```text
VERIFIED ZONESET
      ↓
UAV ↔ ZONE ASSIGNMENT
      ↓
VERIFIED ZONEASSIGNMENTSET
      ↓
ROUTE-IN-ZONE
      ↓
WIND + PERFORMANCE
      ↓
4D TRAJECTORY
      ↓
4D CONFLICT VERIFY
```

This boundary keeps aircraft selection, route generation and conflict resolution independently testable and auditable.

## 20. Route-in-Zone Generator — Formal Model

### Objective

The Route-in-Zone Generator converts each verified UAV ↔ Zone assignment into a coverage route that remains inside the assigned operational zone wherever geometrically possible. Its primary purpose is to complete the assigned coverage workload without intentionally creating cross-zone trajectory intersections.

The generator operates on verified planning inputs. It does not select UAVs, partition zones, perform final wind/performance optimization or resolve fleet conflicts.

### 20.1 Inputs

The generator consumes:
- verified ZoneAssignmentSet;
- verified ZoneSet and zone geometry;
- mission coverage objective and required coverage density/resolution;
- selected coverage pattern;
- mandatory waypoints and corridors;
- altitude and route constraints;
- launch/recovery geometry;
- turn-radius, speed and maneuverability limits;
- aircraft-specific route constraints;
- configurable route-generation parameters;
- route algorithm/configuration version.

### 20.2 Hard route constraints

Every generated route must satisfy:
1. Route geometry remains inside its assigned zone except explicitly defined launch/recovery or transition segments.
2. Restricted/prohibited geometry is not entered.
3. Mandatory waypoints/corridors are respected where applicable.
4. Altitude and route-volume constraints are respected.
5. Aircraft maneuverability, minimum turn radius and segment constraints are respected.
6. Required coverage geometry is reachable by the assigned UAV.
7. Launch and recovery transitions are feasible.
8. Route does not contain invalid geometry, discontinuities or impossible segments.
9. Mission-specific route constraints are satisfied.

A route violating a hard constraint is rejected rather than repaired implicitly.

### 20.3 Coverage pattern generation

The generator should select or receive a coverage pattern appropriate to the mission objective, for example:
- lawnmower/sweep coverage;
- corridor-following coverage;
- perimeter/contour coverage;
- point-to-point inspection;
- structured grid/photogrammetry coverage;
- custom waypoint sequence.

The pattern is subordinate to the assigned zone: coverage generation must not expand into another UAV's zone merely to simplify the path.

### 20.4 Route construction

For sweep/grid missions, the generator should:
1. determine the usable coverage region inside the zone;
2. select a sweep direction based on zone geometry, mission requirements and configured optimization criteria;
3. generate parallel coverage passes;
4. connect passes using aircraft-feasible turns;
5. remove redundant or unreachable segments;
6. add required entry and exit transitions;
7. validate complete coverage;
8. retain route evidence and configuration.

For non-sweep missions, the same principle applies: generate the required task path inside the assigned zone and validate every segment against the route constraints.

### 20.5 Boundary handling

Zone boundaries are treated as operational planning boundaries, not merely visual map lines.

The generator should use configurable boundary margins where required by mission safety or route-generation precision. A boundary margin must not silently reduce required coverage; any resulting uncovered region is explicitly reported.

Shared boundary segments between zones must not become common flight corridors unless explicitly permitted and subsequently verified by the 4D conflict engine.

### 20.6 Route separation principle

Routes should be constructed so that separate UAVs remain spatially separated by zone design. The generator must not intentionally create route crossings and rely on the later 0–5 s conflict-resolution stage when an alternative in-zone route is feasible.

If a crossing or shared operational volume is unavoidable because of mission geometry, mandatory corridors or launch/recovery transitions, it must be explicitly marked in the route data for subsequent 4D verification.

### 20.7 Route optimization

Among valid routes, the generator may optimize:
- coverage completeness;
- route length;
- number and severity of turns;
- transition distance;
- boundary complexity;
- expected energy burden;
- expected flight time;
- wind exposure where environmental data is available;
- launch/recovery burden;
- future 4D conflict exposure.

Coverage and hard constraints take priority over optimization objectives.

### 20.8 Route validation

Before a route enters Wind + Performance, verify:

```text
ROUTE ⊂ ASSIGNED ZONE = PASS or EXPLICIT TRANSITION
RESTRICTED GEOMETRY = CLEAR
MANDATORY WAYPOINTS / CORRIDORS = SATISFIED
ALTITUDE / ROUTE CONSTRAINTS = PASS
AIRCRAFT MANEUVERABILITY = PASS
LAUNCH / RECOVERY = PASS
COVERAGE = COMPLETE or EXPLICITLY UNAVAILABLE
GEOMETRY = VALID
ROUTE EVIDENCE = COMPLETE
```

Only a verified RouteSet proceeds to environmental/performance processing.

### 20.9 Route states

```text
ASSIGNMENT_VERIFIED
        ↓
ROUTE_GENERATING
        ↓
ROUTE_VALIDATING
   ├─ ROUTE_REJECTED
   └─ ROUTE_VALID
          ↓
   PERFORMANCE_PENDING
```

If route validation fails, the route returns to generation or the appropriate earlier planning stage. It must not be released as a partially valid route.

### 20.10 Failure handling

Typical structured failure reasons include:
- insufficient coverage reachability;
- route exits assigned zone;
- restricted geometry intersection;
- mandatory waypoint/corridor incompatibility;
- impossible turn geometry;
- aircraft maneuverability limitation;
- launch/recovery transition failure;
- incomplete coverage;
- invalid route geometry;
- boundary margin makes required coverage infeasible.

Failure is recorded against the route and planning run. If no valid route can be generated for a required zone, the multi-UAV plan is blocked pending replanning.

### 20.11 Route data model

Each Route should contain at minimum:
- route_id;
- UAV_ID;
- zone_id;
- ordered route segments/waypoints;
- altitude/profile constraints;
- coverage pattern/reference;
- coverage completeness result;
- entry/exit transition references;
- route length estimate;
- turn count/turn metrics;
- validation result;
- constraint/evidence references;
- route algorithm/configuration version.

### 20.12 Determinism and traceability

For identical ZoneAssignmentSet, mission geometry, coverage configuration, route constraints and algorithm version, route generation should be reproducible.

Each generation run records:
- planning_run_id;
- route_generation_run/version;
- source ZoneAssignmentSet version;
- source ZoneSet/partition version;
- coverage configuration;
- route-generation parameters;
- selected pattern and sweep direction where applicable;
- generated route version;
- validation result;
- timestamp.

### 20.13 Relationship to later stages

The generator produces geometry and route intent. Environmental and aircraft performance processing subsequently converts this route into performance-adjusted timing and energy estimates.

```text
VERIFIED ZONEASSIGNMENTSET
          ↓
ROUTE-IN-ZONE GENERATOR
          ↓
VERIFIED ROUTESET
          ↓
WIND + PERFORMANCE
          ↓
4D TRAJECTORY
          ↓
4D CONFLICT VERIFY
```

This boundary prevents route-generation logic from being mixed with aircraft assignment or fleet conflict-resolution logic.

## 21. Wind + Performance Engine — Formal Model

### Objective

The Wind + Performance Engine converts a verified RouteSet into performance-adjusted route estimates using environmental conditions and the configured aircraft performance model. It determines segment timing, ground-speed effects, energy consumption and reserve margins without silently changing the mission objective or violating verified zone/route constraints.

The engine separates environmental/performance estimation from route generation and 4D conflict verification. Any material timing change must propagate to 4D Trajectory and trigger re-verification.

### 21.1 Inputs

- verified RouteSet;
- UAV configuration and payload/equipment state;
- aerodynamic/performance model;
- battery state and degradation model;
- approved historical performance corrections;
- forecast and observed wind data;
- atmospheric inputs where supported;
- planned altitude;
- speed limits and operating envelope;
- energy reserve policy;
- launch/recovery and transition energy model;
- environmental/performance model versions.

### 21.2 Performance model

The model accounts for airspeed/ground-speed relationship, wind vector, aircraft mass and configuration, payload, characterized frontal-drag penalty, propulsion performance, battery state/degradation, altitude/atmospheric effects where supported, maneuver/turn burden, climb/descent and launch/recovery energy, and configured reserve.

Where detailed aerodynamic data is unavailable, an approved averaged aircraft-class model may be used. Model version and uncertainty assumptions are retained.

### 21.3 Wind treatment

Wind is a primary uncertainty affecting timing and energy. Where spatial or altitude variation is available, wind is evaluated per route segment rather than as one mission-wide correction.

Forecast and observed wind remain separate traceable inputs. Observed values must not silently overwrite forecast values.

### 21.4 Segment calculation

For every route segment calculate at minimum:
- distance and heading/course;
- planned airspeed;
- wind vector;
- resulting ground-speed estimate;
- segment time;
- estimated energy;
- cumulative time and energy;
- remaining reserve margin.

Where supported, climb/descent and turn energy are explicit contributors.

### 21.5 Battery degradation and approved learning

A degradation coefficient or approved historical correction may represent observed battery behavior. Corrections must have a defined source, version, validity bounds and review status, and remain distinguishable from the baseline model.

An observed correction does not automatically replace the baseline model.

### 21.6 Energy model

Energy is accumulated across transit, coverage, turns, climb/descent, launch/recovery and reserve basis. The implementation may be more detailed, but contributors remain identifiable.

Predicted consumption plus required reserve must remain within the usable energy envelope. An insufficient margin produces a structured blocking result and returns the plan to the appropriate planning stage.

### 21.7 Timing propagation

Performance adjustment produces a time-aware route representation. Changes in ground speed, wind or performance can alter arrival times and fleet separation.

```text
ROUTESET
   ↓
WIND + PERFORMANCE
   ↓
PERFORMANCE-ADJUSTED ROUTESET
   ↓
4D TRAJECTORY
   ↓
4D CONFLICT VERIFY
```

A material timing change invalidates the previous 4D verification.

### 21.8 Operational margins

Retain explicit margins for energy/reserve, flight time, wind uncertainty, performance uncertainty, route transitions and launch/recovery. Margin configuration is versioned. A breach becomes a warning or blocking condition according to the configured assurance policy.

### 21.9 States

```text
ROUTE_VALID
    ↓
PERFORMANCE_EVALUATING
    ↓
PERFORMANCE_VALIDATING
    ├─ PERFORMANCE_BLOCKED
    └─ PERFORMANCE_VALID
            ↓
       TRAJECTORY_PENDING
```

### 21.10 Forecast versus actual feedback

After execution compare forecast versus actual wind, predicted versus actual ground speed, segment time, energy consumption and reserve at key checkpoints. This produces approved correction data for future planning and feeds the internal learning layer with traceability to the originating flight record.

### 21.11 Failure handling

Typical failures: insufficient energy/reserve; performance model outside validity envelope; inadequate wind data; wind outside aircraft operating envelope; payload/configuration mismatch; impossible speed/altitude requirement; launch/recovery energy failure; uncertainty margin exceeded.

The engine returns structured results and does not silently relax hard constraints.

### 21.12 Output

Each PerformanceAdjustedRoute contains at minimum route/UAV identity, segment timing, wind source/time, performance model version, aircraft/payload configuration, segment and total energy estimates, reserve margin, uncertainty values, warnings/blocking conditions, correction references, performance run/version and timestamp.

### 21.13 Verification

```text
ROUTE VALID = PASS
PERFORMANCE MODEL VALID = PASS
UAV CONFIGURATION = PASS
WIND DATA VALIDITY = PASS
SPEED / ALTITUDE ENVELOPE = PASS
ENERGY / RESERVE = PASS
UNCERTAINTY MARGINS = PASS
TIMING DATA = COMPLETE
TRACEABILITY = COMPLETE
```

Only a verified PerformanceAdjustedRouteSet enters the 4D Trajectory Engine.

### 21.14 Determinism and traceability

Identical RouteSet, UAV configuration, environmental dataset, performance model and configuration should produce reproducible estimates. Record planning_run_id, performance run/version, source RouteSet version, UAV/payload snapshot, environmental dataset references, performance model version, battery/degradation model version, margin configuration, results, warnings/blocking conditions and timestamp.

### 21.15 Relationship to AI learning layer

The Wind + Performance Engine is an operational calculation component. The AI learning layer may propose or maintain approved corrections from historical flight data, but it does not silently modify operational calculations.

```text
FLIGHT RECORD
     ↓
FORECAST vs ACTUAL COMPARISON
     ↓
APPROVED CORRECTION
     ↓
VERSIONED PERFORMANCE INPUT
     ↓
WIND + PERFORMANCE ENGINE
```

This preserves separation between operational computation and learned knowledge.

## 22. 4D Trajectory Engine — Formal Model

### Objective

The 4D Trajectory Engine converts each verified PerformanceAdjustedRoute into a time-dependent trajectory containing position, altitude and time for the assigned UAV. It provides the authoritative representation used by fleet-wide 4D conflict verification.

The engine does not redesign the mission, reassign UAVs or resolve conflicts. Its responsibility is to produce a consistent temporal representation of the already validated route and performance estimate.

### 22.1 Inputs

- verified PerformanceAdjustedRouteSet;
- route waypoints and segment geometry;
- segment ground-speed and timing estimates;
- altitude/profile constraints;
- climb/descent performance;
- UAV_ID and configuration;
- launch/recovery transition timing;
- temporal corrections already approved by the planning pipeline;
- trajectory sampling/resolution configuration;
- trajectory algorithm/configuration version.

### 22.2 Trajectory representation

For each UAV, trajectory is represented as:

```text
T_u = {(lat, lon, alt, t, state)}
```

where each trajectory point has at minimum position, altitude, timestamp and UAV identity. The state may identify phases such as launch, transit, coverage, turn, climb, descent, recovery or hold.

The trajectory must preserve the ordering of route segments and their calculated timing.

### 22.3 Temporal construction

The engine calculates cumulative time from the validated segment timing:

```text
T_start = mission/launch reference time
T_i+1 = T_i + Δt_i
```

For each segment, timing must remain consistent with the PerformanceAdjustedRoute. If an explicit launch delay or approved temporal correction exists, it is represented as an explicit temporal event rather than hidden inside an arbitrary segment duration.

### 22.4 Spatial interpolation

Between route waypoints, the engine generates a time-position representation appropriate to the aircraft model and configured trajectory resolution.

Interpolation must respect:
- route geometry;
- altitude profile;
- speed constraints;
- turn transitions;
- climb/descent limits;
- configured trajectory resolution.

The engine must not introduce spatial shortcuts that were absent from the verified route.

### 22.5 State and phase model

Each trajectory should identify operational phases where available:

```text
LAUNCH
  ↓
TRANSITION
  ↓
COVERAGE / MISSION
  ↓
TURN / MANEUVER
  ↓
TRANSITION
  ↓
RECOVERY
```

Additional states such as HOLD, DELAY or ABORT may be represented when explicitly generated by the planning system.

### 22.6 Altitude profile

Altitude is part of the trajectory, not an independent 2D route attribute.

The engine must preserve validated altitude constraints and represent climbs, descents and level segments with their associated timing. A vertical correction introduced by the conflict-resolution stage must create a new trajectory version and trigger re-verification.

### 22.7 Launch and recovery transitions

Launch/recovery segments must be represented explicitly when they affect fleet separation or route timing.

If multiple UAVs use a common launch/recovery area, their temporal occupancy is represented in the trajectory so that the 4D conflict engine can verify the shared operational volume.

### 22.8 Temporal uncertainty and resolution

Trajectory generation should retain configured uncertainty/resolution information rather than implying false precision.

The representation must distinguish:
- calculated nominal time;
- configured timing tolerance/uncertainty;
- actual observed time when available.

The nominal trajectory is the planning reference; uncertainty metadata is used by subsequent verification according to the configured separation policy.

### 22.9 Trajectory validation

Before entering 4D Conflict Verify, validate:

```text
ROUTE REFERENCE = VALID
TIME ORDER = MONOTONIC
POSITION DATA = VALID
ALTITUDE PROFILE = VALID
SEGMENT TIMING = CONSISTENT
SPEED LIMITS = PASS
CLIMB / DESCENT LIMITS = PASS
LAUNCH / RECOVERY = REPRESENTED
UAV_ID = PRESENT
TRACEABILITY = COMPLETE
```

A trajectory failing validation is rejected and returned to the appropriate earlier stage.

### 22.10 State transitions

```text
PERFORMANCE_VALID
        ↓
TRAJECTORY_GENERATING
        ↓
TRAJECTORY_VALIDATING
   ├─ TRAJECTORY_BLOCKED
   └─ TRAJECTORY_VALID
             ↓
      CONFLICT_VERIFY_PENDING
```

### 22.11 Failure handling

Typical failures include:
- inconsistent segment timing;
- non-monotonic timestamps;
- invalid or missing altitude profile;
- speed constraint violation;
- climb/descent capability violation;
- missing UAV identity;
- invalid route reference;
- unsupported temporal event;
- insufficient trajectory resolution for configured verification.

A failure blocks downstream conflict verification until corrected.

### 22.12 Output

Each Trajectory4D should contain at minimum:
- trajectory_id;
- UAV_ID;
- route_id;
- zone_id;
- ordered time-position-altitude samples;
- operational phase/state;
- nominal start/end time;
- timing uncertainty metadata;
- altitude profile reference;
- source PerformanceAdjustedRoute version;
- trajectory algorithm/configuration version;
- validation result;
- traceability references.

The TrajectorySet is the authoritative input to 4D Conflict Verify.

### 22.13 Determinism and traceability

For identical PerformanceAdjustedRouteSet, timing data, trajectory configuration and algorithm version, trajectory generation should be reproducible.

Each run records:
- planning_run_id;
- trajectory_run/version;
- source PerformanceAdjustedRouteSet version;
- trajectory resolution/configuration;
- temporal correction references;
- generated trajectory version;
- validation result;
- timestamp.

### 22.14 Relationship to conflict verification

The 4D Conflict Engine must consume the generated TrajectorySet rather than reconstructing timing independently from route geometry.

```text
PERFORMANCE-ADJUSTED ROUTE
          ↓
4D TRAJECTORY ENGINE
          ↓
VERIFIED TRAJECTORYSET
          ↓
4D CONFLICT VERIFY
```

This establishes one authoritative temporal representation for subsequent fleet conflict analysis.

## 23. 4D Conflict Verification Engine — Formal Model

### Objective

The 4D Conflict Verification Engine determines whether any pair of UAV trajectories violates the configured operational separation criteria in space and time. It is a verification component, not an autonomous collision-avoidance controller.

A 2D geometric crossing is not by itself a conflict. A conflict exists only when the trajectories occupy an insufficiently separated operational volume during overlapping time intervals according to the applicable separation model.

### 23.1 Inputs

- verified TrajectorySet;
- horizontal separation criteria;
- vertical separation criteria;
- temporal overlap criteria;
- operational-volume definitions;
- altitude restrictions;
- configurable safety/assurance margins;
- launch/recovery shared-volume definitions;
- mission-specific separation constraints;
- conflict algorithm/configuration version.

### 23.2 Verification principle

For each relevant UAV pair, determine whether their trajectory segments can simultaneously occupy an insufficiently separated spatial volume.

Conceptually:

```text
SPATIAL PROXIMITY
       +
TEMPORAL OVERLAP
       +
VERTICAL SEPARATION CHECK
       ↓
CONFLICT STATE
```

A spatial intersection with non-overlapping occupancy times may therefore be `NO_CONFLICT`.

### 23.3 Pair selection

The engine should first identify candidate trajectory pairs using spatial/temporal bounding information to avoid unnecessary all-to-all detailed calculations. The optimization must not change the verification result.

Every relevant pair must remain auditable, including the reason a pair was excluded from detailed verification when exclusion is mathematically proven safe by the configured broad-phase method.

### 23.4 Segment-level verification

For candidate trajectory segments, evaluate:
- closest horizontal separation during temporal overlap;
- vertical separation during temporal overlap;
- duration of insufficient separation;
- location/operational volume of the event;
- trajectory states involved;
- uncertainty/margin values;
- applicable separation criteria.

Where the trajectory representation is sampled, the configured resolution and interpolation method must be sufficient to detect a potential minimum-separation event. Sampling must not be treated as proof of safety if the configured verification model requires continuous-segment evaluation.

### 23.5 Conflict classification

Each detected event should be classified at minimum as:

```text
NO_CONFLICT
WARNING
CONFLICT
UNRESOLVED
```

The distinction between WARNING and CONFLICT is configuration-driven and traceable to the applicable separation margins. `UNRESOLVED` is used when verification cannot establish the required state with sufficient evidence.

### 23.6 Conflict record

Each conflict record contains at minimum:
- conflict_id;
- UAV_A_ID and UAV_B_ID;
- trajectory IDs and versions;
- route/zone references;
- segment references;
- conflict location/volume;
- start/end time of overlap;
- minimum horizontal separation;
- minimum vertical separation;
- required separation values;
- separation deficit;
- uncertainty/margin values;
- conflict classification;
- source planning/verification run;
- evidence references.

### 23.7 Fleet-level verification

The engine verifies all relevant UAV pairs and shared operational volumes, including common launch/recovery areas where applicable.

The result is a fleet-level `ConflictReport` containing:
- verified trajectory set/version;
- number of evaluated pairs;
- candidate-pair count;
- conflict/warning/unresolved counts;
- detailed conflict records;
- verification configuration/version;
- evidence references;
- overall gate result.

### 23.8 Verification gate

The default release gate is:

```text
CONFLICTS = 0
UNRESOLVED = 0
REQUIRED EVIDENCE = COMPLETE
        ↓
CONFLICT_VERIFICATION_PASS
```

If a conflict exists and is eligible for Stage I Ground Conflict Resolution, the state becomes:

```text
RESOLUTION_REQUIRED
```

If verification is inconclusive, the state is `UNRESOLVED` and release is blocked until sufficient evidence or replanning exists.

### 23.9 Relationship to 0–5 second temporal correction

The conflict engine does not itself apply the temporal correction. It reports the conflict and the feasible correction range required by the next planning stage.

For a temporal correction, the verification layer should expose whether a delay within the configured **0–5 s** range can restore the required separation. The correction engine then applies the selected delay and generates a new trajectory for re-verification.

```text
CONFLICT
   ↓
FEASIBLE DELAY WINDOW
   ↓
GROUND CONFLICT RESOLUTION
   ↓
NEW TRAJECTORY
   ↓
4D RE-VERIFY
```

### 23.10 Vertical separation

Vertical separation is evaluated as part of the same 4D event, not as an independent 2D route rule.

A vertical correction is eligible only when the mission, aircraft, altitude constraints, restricted airspace and applicable operational rules permit it. The correction engine must record the before/after altitude profile and re-run full verification.

### 23.11 Shared operational volumes

Some locations may be intentionally shared, such as defined launch/recovery areas or explicitly authorized transition corridors.

For each shared volume, the model should define:
- geometry/volume;
- allowed UAV states;
- allowed occupancy times or sequencing rules;
- required separation;
- responsible planning rule.

Shared-volume conflicts are verified using the same authoritative trajectory representation.

### 23.12 Verification uncertainty

The engine must distinguish nominal separation from separation including configured uncertainty margins.

If uncertainty prevents establishing compliance, the event is `UNRESOLVED` rather than being assumed safe.

The uncertainty model and margin configuration are versioned with the verification run.

### 23.13 Determinism and traceability

For identical TrajectorySet, separation model, configuration and algorithm version, the verification result should be reproducible.

Each verification run records:
- planning_run_id;
- conflict_verification_run/version;
- trajectory set/version;
- separation criteria version;
- uncertainty/margin configuration;
- candidate-pair method/version;
- detailed result;
- evidence references;
- timestamp.

### 23.14 State transitions

```text
TRAJECTORY_VALID
       ↓
CONFLICT_ANALYZING
       ↓
CONFLICT_VERIFICATION
   ├─ PASS
   ├─ RESOLUTION_REQUIRED
   └─ UNRESOLVED → BLOCKED
```

After an approved correction:

```text
RESOLUTION_REQUIRED
       ↓
CORRECTION_APPLIED
       ↓
NEW TRAJECTORY
       ↓
CONFLICT_VERIFICATION
```

A previous PASS is invalidated whenever a trajectory-affecting correction changes the verified trajectory.

### 23.15 Output

`ConflictReport` is the authoritative output and contains the fleet-level gate result plus all required evidence and conflict records.

Only `CONFLICT_VERIFICATION_PASS` or an explicitly eligible `RESOLUTION_REQUIRED` state may proceed to Ground Conflict Resolution. `UNRESOLVED` blocks release.

### 23.16 Boundary with Ground Conflict Resolution

The separation is explicit:

```text
4D CONFLICT ENGINE
= detects + classifies + evidences

GROUND CONFLICT RESOLUTION
= selects + applies bounded correction

4D CONFLICT ENGINE
= verifies corrected trajectory again
```

This prevents the verification engine from becoming an uncontrolled maneuver generator and preserves a clear assurance boundary.
