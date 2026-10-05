# BlueSky PRO — Canonical Mission Model

**Priority:** P0  
**Status:** BASELINED — core domain contract

## 1. Purpose

The Mission Model is the canonical representation of an operational task from user intent through planning, validation, authorization, execution and post-flight recording.

It is independent of any autopilot, C2 transport, payload protocol or external regulatory system.

## 2. Principle

The pilot expresses the operational objective and relevant task information. BlueSky determines the appropriate planning strategy, vehicle allocation, algorithms and route within mandatory constraints.

The pilot does not select the optimization algorithm.

## 3. Mission hierarchy

```text
MISSION
 ├── Mission Intent
 ├── Task Definition
 ├── Objective Profile
 ├── Constraints
 ├── Environment Snapshot(s)
 ├── Vehicle Assignment(s)
 ├── Payload Assignment(s)
 ├── Route / Trajectory Candidate(s)
 ├── Selected Solution
 ├── Regulatory Plan(s)
 ├── Authorization
 ├── Executable Mission Package(s)
 └── Execution / Flight Reference(s)
```

## 4. Required identity

Every mission and material revision shall have:

- immutable mission ID;
- mission version;
- creation/update timestamp;
- organization/operator identity;
- mission type;
- operational status;
- parent mission ID for coordinated sub-missions where applicable.

## 5. Task definition

The task shall describe what must be achieved, not how it must be flown.

Examples:

- survey an area;
- inspect an asset;
- acquire imagery for 3D reconstruction;
- search an area;
- deliver a payload;
- monitor an object;
- arrive within a required time window.

## 6. Constraints

Constraints are separated into mandatory and optimization criteria.

Mandatory constraints include, where applicable:

- regulatory/airspace restrictions;
- terrain/obstacle clearance;
- vehicle envelope;
- payload limitations;
- C2 requirements;
- separation/conflict requirements;
- minimum energy reserve;
- recovery/return feasibility;
- operational safety limits.

A candidate violating a mandatory constraint is inadmissible.

## 7. Environment snapshots

Planning inputs shall be versioned snapshots of external data, including where applicable:

- map/GIS;
- terrain/DEM;
- airspace;
- NOTAM/operational restrictions;
- weather;
- wind;
- traffic;
- navigation data.

Each input records source, timestamp, validity/freshness and quality where available.

## 8. Vehicle and payload assignment

A mission may use one or many vehicles. Each assignment references a verified vehicle configuration and payload configuration.

For multi-UAV operations, the parent mission retains common task identity while each aircraft receives its own vehicle-specific execution representation.

## 9. Planning and orchestration

```text
TASK
 ↓
CAPABILITY ANALYSIS
 ↓
TASK DECOMPOSITION
 ↓
VEHICLE / PAYLOAD ALLOCATION
 ↓
ALGORITHM ORCHESTRATION
 ↓
CANDIDATE PLANS
 ↓
HARD CONSTRAINT GATE
 ↓
OBJECTIVE EVALUATION
 ↓
SELECTED SOLUTION
```

The selected solution shall retain its calculation inputs, algorithm/model versions and principal decision rationale.

## 10. Wind and energy sequence

BlueSky shall support the agreed planning sequence:

1. construct the task-appropriate route/coverage solution;
2. allocate the solution to suitable vehicles;
3. apply current environmental conditions, including wind;
4. recalculate predicted trajectory, time and energy;
5. reject or modify candidates that fail energy reserve or other mandatory constraints;
6. select the best admissible solution.

Wind is therefore a dynamic planning input, not merely a display item.

## 11. Quality objective

Mission success criteria depend on mission type. For imaging/3D/inspection tasks, data quality may outrank route length or elapsed time. For delivery, successful delivery and energy/recovery margin dominate. For time-critical tasks, ETA is optimized within safety and energy limits.

The objective profile is selected internally by BlueSky from the operational task.

## 12. Mission versioning

A material change to task, route, vehicle, payload, environment assumptions, regulatory state or safety-critical configuration creates a new mission version.

Only a validated version may proceed to release.

## 13. Regulatory relationship

A mission may reference one or more regulatory plans/authorizations. Regulatory status is external authority state and is not equivalent to internal BlueSky validation.

## 14. Executable mission package

The selected validated mission is compiled into one or more vehicle-specific executable packages containing only semantics supported and verified by the target vehicle/autopilot/payload integration.

```text
Canonical Mission Version
        ↓
Vehicle-specific compilation
        ↓
Executable Package
        ↓
Upload / Read-back
        ↓
Semantic Verification
        ↓
Flight Release
```

## 15. Execution reference

During flight the mission version remains linked to:

- vehicle;
- autopilot/FCS;
- C2 session;
- commands and acknowledgements;
- telemetry;
- payload/video data;
- events/alerts;
- logs;
- regulatory state.

## 16. Post-flight closure

The completed mission shall resolve into a Flight Record linking the planned and actual execution data. Replay and deviation analysis can then generate controlled corrections for future planning.

## 17. Acceptance criterion

The model is complete when the same canonical Mission Version can be traced through:

`TASK → PLAN → VALIDATE → AUTHORIZE → EXECUTE → MONITOR → COMPLETE → RECORD → REPLAY`

without requiring a vendor-specific mission structure in the BlueSky core.
