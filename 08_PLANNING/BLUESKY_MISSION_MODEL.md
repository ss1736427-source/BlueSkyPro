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
