# BlueSky PRO — Common Mission Model

**Status:** ARCHITECTURE BASELINE — implementation contract

## 1. Purpose

The Common Mission Model is the canonical data contract between the BlueSky planning core and all downstream execution, regulatory, communication, vehicle, payload and validation contours.

The user defines the operational task. BlueSky derives the required mission strategy, selects suitable algorithms and vehicles, constructs candidate plans, validates them and produces an executable mission package. The pilot is not required to select algorithms or manually construct the optimization criteria.

## 2. Core principle

```text
USER TASK
   ↓
COMMON MISSION MODEL
   ↓
TASK / CONSTRAINT ANALYSIS
   ↓
ALGORITHM ORCHESTRATION
   ↓
CANDIDATE PLANS
   ↓
SAFETY + ENERGY + QUALITY VALIDATION
   ↓
BEST ADMISSIBLE PLAN
   ↓
REGULATORY / C2 / AUTOPILOT
```

The Mission Model describes **what must be accomplished and under what conditions**. It does not prescribe a particular planning algorithm.

## 3. Mission identity and lifecycle

Every mission shall have:

- unique mission identifier;
- mission version;
- parent mission identifier for derived/replanned versions;
- creation/update timestamps;
- operational status;
- operator/company context;
- mission type/objective profile;
- selected fleet/configuration references;
- data/model versions used for calculation.

Recommended lifecycle states:

`DRAFT → CALCULATING → VALIDATING → READY → SUBMITTED → AUTHORIZED → RELEASED → EXECUTING → PAUSED/REPLANNING → COMPLETED/ABORTED`

The exact state transitions shall be controlled by system rules and external regulatory status.

## 4. Mission intent

The model shall capture the user's operational intent without forcing the user to specify implementation details.

Examples of intent:

- survey an area;
- inspect an object;
- produce imagery or a 3D model;
- search an area;
- monitor an object;
- deliver cargo;
- arrive at a specified time;
- perform a long-endurance mission;
- perform a multi-UAV coordinated operation.

## 5. Objective profile

BlueSky shall derive or assign the applicable Mission Objective Profile from the mission intent and available operational parameters.

The profile defines the relative objectives and acceptance criteria for the mission. Examples include:

- safety and regulatory admissibility;
- required task quality;
- coverage completeness;
- imaging geometry/overlap/GSD requirements where applicable;
- detection probability where applicable;
- delivery success;
- ETA/time constraints;
- energy efficiency;
- battery reserve;
- propulsion/engine resource;
- C2 continuity;
- fleet coordination.

Hard safety, regulatory and minimum-energy-reserve constraints cannot be traded away for better mission quality or shorter time.

## 6. Environment

The mission references the environmental state required for planning and validation, including where applicable:

- geographic operating area;
- terrain/elevation;
- obstacles;
- airspace restrictions;
- weather;
- wind field;
- temperature;
- relevant environmental uncertainty;
- operational time window.

Environmental data shall carry source, timestamp/version and validity information where available.

## 7. Vehicle and payload references

The mission shall reference the selected vehicle configuration rather than embedding an assumed generic UAV.

Each vehicle reference may include:

- UAV identity/configuration;
- autopilot type and adapter;
- propulsion configuration;
- battery/SOH state;
- mass and payload configuration;
- performance envelope;
- communications configuration;
- installed sensors/cameras;
- payload/payload-control adapter;
- applicable operational limitations.

This permits different UAVs in the same mission to receive different route solutions and algorithm combinations.

## 8. Task geometry and coverage

The model shall support the geometry required to express the operational task, including:

- points;
- lines;
- polygons;
- corridors;
- volumes/3D areas;
- inspection surfaces/objects;
- search regions;
- delivery locations;
- required approach/departure geometry;
- mandatory and prohibited areas.

Task-specific quality constraints belong to the mission objective profile rather than to the map UI.

## 9. Constraints

Constraints are represented explicitly and classified at minimum as:

### Hard constraints

Violation makes a candidate plan inadmissible. Examples:

- regulatory/airspace restrictions;
- aircraft operating envelope;
- obstacle/terrain clearance;
- mandatory operational limits;
- minimum energy reserve;
- C2 requirements;
- safety separation;
- payload limitations.

### Soft objectives

Used to compare admissible candidates. Examples:

- mission quality;
- energy consumption above the required reserve;
- propulsion resource consumption;
- flight time;
- route length;
- smoothness;
- workload/complexity.

## 10. Algorithm Orchestrator interface

The Mission Model supplies the orchestrator with task intent, environment, vehicle capabilities, constraints and objective profile.

The orchestrator decides:

- which algorithms to use;
- whether to combine algorithms;
- which algorithm is appropriate for each subtask/UAV;
- whether candidates should be calculated in parallel;
- when incremental recalculation is sufficient;
- which admissible candidate is preferable.

The Mission Model does not expose internal algorithm selection as a pilot requirement.

## 11. Wind and recalculation principle

The planning process shall support staged calculation:

```text
TASK / BASE CONDITIONS
       ↓
BASE PLAN / CANDIDATES
       ↓
CURRENT WIND
       ↓
ENERGY + PERFORMANCE RECALCULATION
       ↓
CANDIDATE RE-EVALUATION
       ↓
BEST ADMISSIBLE PLAN
```

A change that does not invalidate unchanged portions of a plan should trigger incremental recalculation rather than rebuilding the complete mission from zero.

## 12. Multi-UAV mission

A parent mission may contain multiple coordinated vehicle plans.

```text
PARENT MISSION
      ↓
TASK DECOMPOSITION
      ↓
UAV ALLOCATION
 ┌────┼────┐
 ↓    ↓    ↓
UAV1 UAV2 UAV3
 ↓    ↓    ↓
PLAN PLAN PLAN
 └────┼────┘
      ↓
COORDINATED MISSION
```

Each child plan shall retain its own vehicle, autopilot, payload, C2, energy and regulatory references while remaining linked to the parent mission.

## 13. Mission package boundary

The Common Mission Model is the source from which BlueSky generates an executable Mission Package.

The package shall contain only the data required by the target execution chain and shall include:

- mission/version identity;
- target vehicle;
- executable route/commands;
- required payload actions;
- relevant parameters;
- contingency/recovery instructions where supported;
- integrity/version information;
- references required for execution and validation.

The package is then translated by the Autopilot Adapter for ArduPilot, PX4 or OEM execution interfaces.

## 14. Validation outputs

Validation results shall attach to a specific mission version and include at minimum:

- safety result;
- regulatory result;
- vehicle/payload compatibility;
- C2 readiness;
- energy prediction and reserve result;
- resource result;
- task-quality result;
- multi-UAV conflict result;
- data freshness result;
- blocking findings;
- warnings;
- validation engine/model versions.

## 15. Pilot-facing result

BlueSky shall present the selected result, not the internal calculation complexity.

When the selected route is non-obvious, the system shall provide concise operational reasoning, for example:

> **Route changed:** stronger headwind on the direct segment.
>
> **Reason:** preserves required battery reserve while maintaining task coverage.

The explanation shall be short, operational and understandable. Internal algorithm names and mathematical details remain outside the normal pilot workflow.

## 16. Runtime linkage

During execution the Mission Model remains linked to:

- actual vehicle state;
- telemetry;
- C2 state;
- payload state;
- environmental updates;
- mission events;
- regulatory state;
- replanning versions.

This creates continuity between planning, execution, replay and post-flight analysis.

## 17. Traceability

A mission version shall be reproducible from its references to:

- objective profile;
- vehicle/payload configuration;
- environmental data;
- regulatory data/status;
- algorithms and versions;
- optimization parameters;
- validation results;
- mission package;
- operator/system actions.

## 18. Implementation rule

All internal modules and external adapters shall consume or produce defined Mission Model contracts. UI components shall not become authoritative owners of mission logic.

Changes to the Mission Model are controlled architectural changes because they affect planning, validation, regulatory, C2, autopilot and operational evidence chains.

## 19. Acceptance criteria

The Common Mission Model is considered implementation-ready when:

1. a mission can be created without selecting an algorithm;
2. a mission can reference multiple heterogeneous UAV configurations;
3. objective profiles can represent different task priorities;
4. hard and soft constraints are machine-readable;
5. wind/environment versions can be attached to calculations;
6. candidate plans can be associated with their calculation context;
7. validation results are tied to a mission version;
8. an executable Mission Package can be generated without bypassing the validation gate;
9. a replan creates a traceable new mission version;
10. the same model can pass through planning, simulation, regulatory, execution and replay contours.

## 20. Relationship to GAP audit

This document closes the architectural specification gap for:

- Mission/task model;
- Mission Objective Profiles;
- task geometry/coverage representation;
- explicit hard/soft constraints;
- multi-UAV parent/child mission structure;
- Mission Package boundary;
- traceability across the lifecycle.

Implementation, integration, testing and verification remain separate maturity states in the GAP audit.

## 21. Interactive map and flight-profile control of the trajectory

The map and the flight profile are two synchronized operator-control views of one canonical mission trajectory. They are not independent route editors and neither view is authoritative over the other. The Planning Core owns the canonical route model; the map and profile submit edit commands and render the resulting route version.

### 21.1 Responsibilities of the views

- **Map:** spatial editing and inspection of route geometry, including moving a waypoint, inserting/removing a waypoint, adjusting a segment/corridor and inspecting NOTAM/terrain context. Map pan, zoom, layer visibility and selection are view operations and must never modify mission geometry.
- **Flight profile:** editing and inspection of vertical and time-related route parameters, including waypoint altitude and supported segment/flight parameters. A profile edit must update the same canonical route displayed on the map.
- A change made in either view is immediately reflected in the other view after the Planning Core accepts the edit. Both views display the same route ID, route version and validation state.
- The map may render route geometry but must not calculate or silently rewrite route geometry in JavaScript/WebGL. Geometry changes are commands to the Planning Core.

### 21.2 Operator feedback record

Every accepted operator edit shall be recorded as a versioned route-edit event containing at minimum:

- unique edit/event ID and monotonically ordered sequence;
- mission ID/version, route ID and base route version;
- source view (MAP, FLIGHT_PROFILE or another identified control);
- operation (MOVE_WAYPOINT, INSERT_WAYPOINT, REMOVE_WAYPOINT, SET_ALTITUDE, SET_SEGMENT_PARAMETER, PIN_SEGMENT, SET_PREFERENCE, etc.);
- target waypoint/segment/object ID;
- before and after values, coordinate/altitude datum and units;
- operator intent: hard constraint (pinned/mandatory) or soft preference;
- timestamp, operator identity/reference and reason/comment when supplied;
- resulting route version and validation status.

Edits must be persisted with the mission and included in traceability/audit history. A new optimizer run must consume the current edit set and its version, not reconstruct intent from the currently displayed geometry.

### 21.3 Optimizer contract and conflict handling

- **Hard operator constraints** (for example, a pinned waypoint or segment) must be preserved by optimization unless they make the mission infeasible. The optimizer must report the exact conflict; it must not silently relax or delete the constraint.
- **Soft preferences** influence candidate generation/ranking but may be traded off against higher-priority mission objectives and safety constraints. The resulting deviation must be explained.
- A manual edit invalidates route geometry/validation evidence that depends on the changed data. The edited route becomes RECALCULATION_REQUIRED / unvalidated until the Planning Core regenerates complete trajectory geometry and all applicable validators accept the exact new route and input versions.
- Every replan creates a traceable route/mission revision with parent-version linkage and retains the operator edit history. The optimizer may propose alternatives, but must not replace the operator's current route without an explicit selection/acceptance action.
- Safety, NOTAM, terrain/obstacle, vehicle-envelope, weather, energy/reserve and regulatory gates remain mandatory. Operator edits never override these gates.
- If new NOTAM/weather/vehicle data invalidates a pinned segment, preserve the operator's edit record, block readiness, identify the conflict and offer safe alternatives. Do not erase the edit.

### 21.4 Current implementation boundary and gap

The current map implementation supports mission-geometry capture by map clicks and displays the route supplied by the Planning Core. Its route overlay is currently render-only; it does not yet provide canonical route-waypoint drag/edit commands. The current RouteController exposes waypoint-altitude editing and invalidates prior trajectory geometry/validation, but the full versioned operator-edit event contract, map-to-core route editing, synchronized map/profile editing, persistence and optimizer consumption of hard/soft operator intent are not yet demonstrated end-to-end.

These are implementation gaps, not completed capabilities. They must be closed before interactive trajectory control is considered integrated.

### 21.5 Acceptance tests

1. Move a route waypoint on the map; the same canonical route version and waypoint position appear in the flight profile.
2. Change a waypoint altitude in the flight profile; the map reflects the same route revision and vertical data.
3. Insert/remove a waypoint or change a supported segment parameter; the edit event records source, target, before/after values and sequence.
4. Start a new optimizer run; it consumes the saved hard constraints and soft preferences from the current edit revision.
5. Verify pinned geometry is preserved when feasible; when infeasible, the optimizer reports a blocking conflict and does not silently relax it.
6. Verify every edit invalidates dependent validation evidence and that only matching route/input-version validation can restore VALIDATED.
7. Verify a new NOTAM/weather snapshot triggers re-evaluation without deleting the operator's edit history.
8. Reopen the mission and verify the last accepted route revision and operator edits are restored.

## 22. Mission lifecycle gates and evidence

The lifecycle state sequence in Section 3 is the top-level mission status. The following ordered gates define the work required to move between those states. A state transition is permitted only when its required evidence is present and tied to the same mission/version lineage.

1. **Mission definition (DRAFT):** task intent/template, AOI/route geometry, mission operations, origin and operating window are defined. Missing task-specific inputs remain explicit blockers; the system must not invent them.
2. **Vehicle and equipment readiness:** assigned UAV configuration, performance envelope, equipment compatibility, calibration/verification and operator-approved custom equipment are resolved for the exact configuration revision.
3. **Environment and airspace acquisition:** terrain/obstacle and applicable airspace/NOTAM data are acquired for the mission AOI and time window. The mission NOTAM snapshot is independent of the map viewport snapshot. Unknown or ambiguous applicable restrictions block planning.
4. **Initial route generation:** generate admissible geometric route candidates against task geometry, vehicle constraints and NOTAM restrictions. This is the pre-weather baseline; do not rank it as the final weather-aware solution.
5. **Weather assessment:** after baseline route generation, acquire/cache forecast data for the candidate route corridors, altitude bands and estimated time window. Apply wind to time/energy estimates and candidate ranking; do not download unrelated global data.
6. **Weather-aware re-optimization:** calculate corrected candidate trajectories, then re-check NOTAM applicability using the corrected segment times. Preserve the source/version lineage for weather and airspace snapshots.
7. **Operator review and editing:** present synchronized map/profile controls, candidate comparison, warnings and explanations. Persist operator feedback and feed it into any subsequent optimization as specified in Section 21.
8. **Integrated validation (VALIDATING):** validate complete flown-path/turn geometry, NOTAM and other airspace restrictions, terrain/obstacles, vertical datum/clearance, dynamic UAV limits, payload/equipment, task quality/coverage, weather, energy/reserve, C2 and applicable regulatory requirements. All results must reference the exact route, mission, vehicle and input versions.
9. **Ready (READY):** all required technical gates pass; no unresolved blocker remains. READY is not synonymous with external authorization or permission to fly.
10. **Submission and authorization (SUBMITTED, AUTHORIZED):** produce the required review package and capture external approvals/authorizations with their authority, scope, validity and version references.
11. **Release (RELEASED):** create and integrity-check the executable mission package for the assigned vehicle/autopilot. Release is allowed only when required approvals and validation evidence remain current.
12. **Execution (EXECUTING):** link telemetry, vehicle state, C2, payload, operator actions and deviations to the released mission version.
13. **Pause/replanning:** a changed condition or operator intervention creates a controlled replan revision. Re-run all affected gates; do not treat an old READY or RELEASED result as valid for changed inputs.
14. **Completion/abort and replay:** close the mission with actual execution results, deviations, incidents, data products and audit evidence; retain links to planned, released and executed versions.

### 22.1 Lifecycle invariants

- No stage may silently substitute missing, stale or unverified input data.
- A new mission/route/input revision invalidates only the validation and approval evidence dependent on the changed inputs; unaffected source records remain traceable.
- READY, AUTHORIZED and RELEASED are distinct states with distinct evidence.
- Mission package generation must not bypass validation or external authorization gates.
- Every state transition records actor/system, timestamp, previous/new state, reason and evidence references.
- A blocked transition returns machine-readable blocker IDs and operator-readable explanations.

### 22.2 Lifecycle acceptance tests

- Attempt each transition with one required input/evidence missing and verify it is blocked with a specific reason.
- Verify a valid mission progresses through the ordered gates without skipping NOTAM-before-route and weather-after-baseline-route ordering.
- Change mission geometry, UAV configuration, NOTAM, weather or operator route edits and verify dependent validation becomes stale.
- Verify authorization and release cannot be inferred from technical READY.
- Verify execution, replanning and post-flight replay preserve complete mission/route version lineage.
