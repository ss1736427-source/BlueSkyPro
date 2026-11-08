# BlueSky PRO — Mission Objective Profiles

**Status:** ARCHITECTURE BASELINE — orchestration decision contract

## 1. Purpose

Mission Objective Profiles define how BlueSky determines what constitutes the best admissible solution for a mission type. The pilot states the operational task; BlueSky selects the strategy, algorithms and vehicle-specific solutions automatically.

## 2. Fundamental rule

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

## 3. Priority hierarchy

The evaluation shall use a hierarchical model rather than allowing a low-level optimization gain to compensate for a failed mandatory requirement.

### Level 0 — Hard admissibility

The candidate is rejected if it violates mandatory safety, regulatory, vehicle, C2, separation, payload or minimum-energy-reserve requirements.

### Level 1 — Primary mission objective

The objective that defines operational success for the mission type.

### Level 2 — Secondary mission objectives

Quality, completeness, time, energy efficiency, propulsion resource and other relevant objectives.

### Level 3 — Tie-breakers

Route length, smoothness, computational cost and other secondary factors may be used when higher-priority objectives are effectively equivalent.

## 4. Canonical profiles

### 4.1 Survey / mapping

Primary: required coverage and data quality.

Relevant objectives:

- coverage completeness;
- GSD/ground sampling requirements;
- image overlap;
- flight geometry;
- sensor operating constraints;
- energy efficiency;
- time.

### 4.2 3D / volumetric reconstruction

Primary: reconstruction quality and geometric completeness.

Relevant objectives:

- surface/volume coverage;
- observation geometry;
- overlap;
- multi-angle acquisition;
- occlusion reduction;
- sensor constraints;
- energy;
- time.

### 4.3 Search / reconnaissance

Primary: probability and completeness of detection/observation.

Relevant objectives:

- search coverage;
- sensor performance;
- observation geometry;
- revisit/uncertainty reduction;
- energy reserve;
- C2 continuity;
- time.

### 4.4 Delivery

Primary: successful delivery to the required location/condition.

Relevant objectives:

- delivery feasibility;
- route safety;
- energy reserve for mission and recovery;
- payload constraints;
- ETA;
- propulsion resource;
- route efficiency.

### 4.5 Inspection

Primary: acquisition of the required inspection information.

Relevant objectives:

- required viewpoints;
- sensor geometry;
- resolution/quality;
- surface completeness;
- obstacle/clearance constraints;
- energy;
- time.

### 4.6 Monitoring

Primary: continuity and quality of observation.

Relevant objectives:

- observation continuity;
- required sensor geometry;
- revisit interval;
- C2 continuity;
- energy reserve;
- time/resource efficiency.

### 4.7 Long-endurance / BVLOS

Primary: robust completion of the mission with adequate energy and operational margins.

Relevant objectives:

- energy reserve;
- route robustness;
- C2 availability;
- recovery feasibility;
- propulsion resource;
- mission completion;
- ETA.

### 4.8 Time-critical arrival

Primary: achievement of the required arrival window.

Relevant objectives:

- ETA accuracy;
- wind robustness;
- energy reserve;
- route safety;
- propulsion/resource constraints.

### 4.9 Multi-UAV coordinated mission

Primary: successful coordinated completion of the parent task.

Relevant objectives:

- task completeness;
- temporal/spatial coordination;
- conflict avoidance;
- individual UAV energy margins;
- heterogeneous vehicle capabilities;
- C2 continuity;
- overall mission time.

## 5. Dynamic weighting

Profiles shall support context-dependent objective priorities. The effective priority may change with environmental conditions, vehicle state, regulatory restrictions, mission phase or detected risk.

Examples:

- rising headwind increases the importance of energy margin;
- degraded battery increases energy conservatism;
- a time-critical mission increases ETA priority only within safety and energy limits;
- poor imaging geometry increases acquisition quality priority;
- loss of a UAV in a group mission changes allocation priorities.

## 6. Algorithm orchestration

The profile does not select a single fixed algorithm. It supplies mission semantics and constraints to the Algorithm Orchestrator.

The orchestrator may select different methods for different subtasks or UAVs and may combine them.

The selection criterion is the quality and admissibility of the resulting operational plan, not the number or prestige of algorithms used.

## 7. Pilot-facing explanation

The pilot normally sees only the selected route and relevant operational alerts.

If the route is non-obvious, BlueSky shall explain the principal reason in concise terms:

- wind;
- energy reserve;
- safety/airspace restriction;
- task quality;
- UAV capability;
- coordination requirement;
- timing requirement.

Example:

> **Route changed:** direct segment has stronger headwind. The selected route preserves the required reserve and task coverage.

The system shall not expose algorithmic complexity unless explicitly requested or required for an engineering/verification workflow.

## 8. Acceptance criteria

An objective profile implementation is acceptable when:

1. the pilot can state the task without selecting an algorithm;
2. the system automatically determines the applicable profile;
3. mandatory constraints are never traded against soft objectives;
4. task-specific quality has explicit acceptance criteria;
5. energy reserve remains a mandatory gate;
6. different UAVs can receive different algorithmic solutions;
7. environmental changes can alter effective priorities;
8. the selected solution has a concise explainable operational rationale;
9. the decision is reproducible and traceable.


## 9. Mapping the 13 approved templates to objective profiles

The operator-facing template ID remains stable. Objective profiles are reusable evaluation models and may be shared by several templates. The algorithm framework in `BLUESKY_TEMPLATE_ALGORITHM_FRAMEWORK.md` defines the reusable method combinations, inputs and acceptance evidence.

| Template | Primary objective profile | Primary success measure | Main algorithm capabilities |
|---|---|---|---|
| MT-01 Mapping | Survey / mapping | AOI coverage, GSD and overlap | Cellular decomposition, coverage path planning, sortie sequencing |
| MT-02 3D reconstruction | 3D / volumetric reconstruction | Surface coverage, geometric quality and occlusion | Viewpoint sampling, visibility, set-cover/ILP, TSP |
| MT-03 Inspection | Inspection | Required evidence/viewpoint coverage and quality | Viewpoint coverage, visibility, set-cover, graph/TSP |
| MT-04 Construction monitoring | Monitoring + survey/3D | Registered change/volume accuracy against versioned baseline | Repeatable coverage, BIM-aware viewpoints, registration, change detection |
| MT-05 Perimeter monitoring | Persistent monitoring | Maximum revisit interval, dwell and observation continuity | Patrol graph, cyclic routing, orienteering, revisit scheduling |
| MT-06 Search and rescue | Search / reconnaissance | Probability of detection and residual search uncertainty | Bayesian search, sensor PoD, probability-weighted coverage, information gain |
| MT-07 Fire / emergency | Search / reconnaissance + monitoring | Priority hazard observation within time/risk limits | Hotspot prioritization, orienteering, thermal detection, short-horizon replanning |
| MT-08 Environmental monitoring | Monitoring + survey | Representative sampling and repeatable indicator quality | Stratified sampling, coverage, adaptive sampling, time-series/change detection |
| MT-09 Agriculture | Survey / mapping + operation-specific treatment | Crop information quality or treatment coverage/dose | Coverage planning, field decomposition, headland/turn optimization, prescription zoning |
| MT-10 Cargo delivery | Delivery | Delivery feasibility/success and safe recovery reserve | Constrained graph routing, VRP/time windows, payload/energy optimization |
| MT-11 Communications relay | Long-endurance / BVLOS + connectivity service | Required link availability, capacity and network connectivity | A2G link budget, relay placement, graph connectivity, joint trajectory/network optimization |
| MT-12 Aerial media | Survey/acquisition quality + time-critical shot plan | Required framing/content and feasible smooth trajectory | Shot sequencing, spline/minimum-jerk trajectory, camera/gimbal control |
| MT-13 C-UAS detection | Search / reconnaissance (sensor detection) | Detection/localization/tracking performance and calibrated confidence | Sensor detection, data association, multi-target tracking, validated sensor fusion |

### 9.1 Profile composition rules

- A template may compose more than one profile when its operational task genuinely has multiple outcomes. MT-04, MT-07, MT-08 and MT-09 are explicit examples.
- Composition does not permit a secondary objective to compensate for failure of a hard gate or the primary acceptance threshold.
- MT-13 is a sensor-intelligence task, not a route-coverage task by default. A patrol/coverage route may be added only if sensor deployment geometry requires it.
- MT-11 connectivity is a service-level objective, not simply a waypoint or line-of-sight objective.
- MT-09 must distinguish sensing/monitoring from spraying, spreading or seeding because the payload physics and constraints differ.
- MT-12 framing/smoothness is subordinate to vehicle dynamics, obstacle clearance, airspace and reserve.
- For every composed profile, the orchestrator records the primary objective, secondary objectives, thresholds, source of each threshold and the reason for composition.

### 9.2 Required evidence

An implementation is not accepted because it emits a route. It must produce template-specific quality metrics, coverage/evidence accounting, uncertainty, hard-gate results, data provenance and a reproducible decision trace. Missing sensor calibration, geometry, baseline or environmental data must produce a clarification or explicitly reduced-scope result.
