# BlueSky PRO — Mission Objective Profiles

**Status:** ARCHITECTURE BASELINE — orchestration decision contract

## Purpose

Mission Objective Profiles define how BlueSky determines what constitutes the best admissible solution for a mission type. The pilot states the operational task; BlueSky selects the strategy, algorithms and vehicle-specific solutions automatically.

## Fundamental rule

The profile is an internal decision model. It is not a pilot checklist and does not require the pilot to select algorithms or manually tune optimization weights.

```text
USER TASK
   ↓
TASK CLASSIFICATION
   ↓
OBJECTIVE PROFILE
   ↓
ALGORITHM ORCHESTRATOR
   ↓
CANDIDATE SOLUTIONS
   ↓
HARD SAFETY / ENERGY GATE
   ↓
OBJECTIVE EVALUATION
   ↓
BEST ADMISSIBLE SOLUTION
```

## Priority hierarchy

The evaluation uses a hierarchical model rather than allowing a low-level optimization gain to compensate for a failed mandatory requirement.

### Level 0 — Hard admissibility

The candidate is rejected if it violates mandatory safety, regulatory, vehicle, C2, separation, equipment or minimum-energy-reserve requirements.

### Level 1 — Primary mission objective

The objective that defines operational success for the mission type.

### Level 2 — Secondary mission objectives

Quality, completeness, time, energy efficiency, propulsion resource and other relevant objectives.

### Level 3 — Tie-breakers

Route length, smoothness, computational cost and other secondary factors may be used when higher-priority objectives are effectively equivalent.

## Canonical profiles

### Survey / mapping

Primary: required coverage and data quality.

Relevant objectives: coverage completeness, GSD, overlap, flight geometry, sensor constraints, energy efficiency, time.

### 3D / volumetric reconstruction

Primary: reconstruction quality and geometric completeness.

Relevant objectives: surface/volume coverage, observation geometry, overlap, multi-angle acquisition, occlusion reduction, sensor constraints, energy, time.

### Search / reconnaissance

Primary: probability and completeness of detection/observation.

Relevant objectives: search coverage, sensor performance, observation geometry, revisit/uncertainty reduction, energy reserve, C2 continuity, time.

### Delivery

Primary: successful delivery to the required location/condition.

Relevant objectives: delivery feasibility, route safety, energy reserve for mission and recovery, equipment constraints, ETA, propulsion resource, route efficiency.

### Inspection

Primary: acquisition of the required inspection information.

Relevant objectives: required viewpoints, sensor geometry, resolution/quality, surface completeness, obstacle/clearance constraints, energy, time.

### Monitoring

Primary: continuity and quality of observation.

Relevant objectives: observation continuity, required sensor geometry, revisit interval, C2 continuity, energy reserve, time/resource efficiency.

### Long-endurance / BVLOS

Primary: robust completion of the mission with adequate energy and operational margins.

Relevant objectives: energy reserve, route robustness, C2 availability, recovery feasibility, propulsion resource, mission completion, ETA.

### Time-critical arrival

Primary: achievement of the required arrival window.

Relevant objectives: ETA accuracy, wind robustness, energy reserve, route safety, propulsion/resource constraints.

### Multi-UAV coordinated mission

Primary: successful coordinated completion of the parent task.

Relevant objectives: task completeness, temporal/spatial coordination, conflict avoidance, individual UAV energy margins, heterogeneous vehicle capabilities, C2 continuity, overall mission time.

## Dynamic priorities

Profiles support context-dependent priority changes. Effective priorities may change with environmental conditions, vehicle state, regulatory restrictions, mission phase or detected risk.

Examples:

- stronger headwind increases the importance of energy margin;
- degraded battery increases energy conservatism;
- time-critical operations increase ETA priority only within safety and energy limits;
- poor imaging geometry increases acquisition-quality priority;
- loss of one UAV changes allocation and coordination priorities.

## Algorithm orchestration

The profile does not select a single fixed algorithm. It supplies mission semantics and constraints to the Algorithm Orchestrator.

The orchestrator may select different methods for different subtasks or UAVs and may combine them. Selection is based on operational outcome, admissibility, computational cost and robustness—not on the number or prestige of algorithms used.

## Pilot-facing explanation

The pilot normally sees the selected route and relevant operational alerts. When a route is non-obvious, BlueSky explains the principal operational reason in concise terms:

- wind;
- energy reserve;
- safety/airspace restriction;
- task quality;
- UAV capability;
- coordination requirement;
- timing requirement.

Example:

> **Route changed:** direct segment has stronger headwind. The selected route preserves the required reserve and task coverage.

Internal algorithm names and mathematical details remain outside the normal pilot workflow.

## Acceptance criteria

1. The pilot can state the task without selecting an algorithm.
2. The system determines the applicable profile automatically.
3. Mandatory constraints are never traded against soft objectives.
4. Task-specific quality has explicit acceptance criteria.
5. Required energy reserve remains a hard gate.
6. Different UAVs can receive different algorithmic solutions.
7. Environmental changes can alter effective priorities.
8. Non-obvious results have concise operational reasoning.
9. Decisions are reproducible and traceable.

## Relationship to orchestration architecture

This profile is an input contract for the BlueSky Algorithm Orchestrator and must remain independent of UI implementation. The same profile must be usable during planning, replanning, simulation, validation and replay analysis.
