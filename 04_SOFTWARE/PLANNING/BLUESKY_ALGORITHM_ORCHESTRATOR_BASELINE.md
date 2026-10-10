# BlueSky PRO — Algorithm Orchestrator Baseline

**Status:** ARCHITECTURE / IMPLEMENTATION BASELINE — P0

## 1. Purpose

The Algorithm Orchestrator selects and combines planning algorithms according to the actual mission, vehicle capabilities, equipment, constraints and optimization priorities.

The operator states the task. BlueSky determines the appropriate computational strategy and explains the resulting decision concisely.

## 2. Core principle

```text
MISSION OBJECTIVE
      ↓
TASK CLASSIFICATION
      ↓
CONSTRAINTS + CAPABILITIES + ENVIRONMENT
      ↓
ALGORITHM CANDIDATES
      ↓
FAST PARALLEL / STAGED EVALUATION
      ↓
FEASIBILITY GATE
      ↓
MULTI-CRITERIA RANKING
      ↓
BEST SAFE SOLUTION
      ↓
EXPLANATION / CORRECTIONS
```

The orchestrator is a coordinator, not a new route-planning algorithm. Its job is to select the appropriate solver(s), allocate computational effort and compare results.

## 3. No universal algorithm

BlueSky shall maintain a portfolio of algorithms. The system shall select the most appropriate method for each planning subproblem instead of forcing every mission through one algorithm.

Candidate families may include, as applicable:

- Dijkstra — graph shortest path and constrained network routing;
- A* — goal-directed graph search;
- Theta* / any-angle search — reduced waypoint/path geometry where appropriate;
- D* Lite / incremental search — dynamic environments and repeated replanning;
- RRT / RRT* — continuous-space motion planning where graph methods are insufficient;
- optimization methods — trajectory, energy and multi-objective optimization;
- assignment algorithms — allocation of subtasks to heterogeneous UAVs;
- coverage-planning methods — area, corridor and volumetric coverage;
- scheduling methods — timing, sequencing and multi-UAV coordination.

The exact portfolio is implementation-controlled and shall be validated for each supported operational domain.

## 4. Algorithm selection

Selection shall consider:

- mission type;
- geometry/topology of the planning space;
- static or dynamic constraints;
- required solution quality;
- available computation time;
- vehicle capabilities;
- equipment requirements;
- energy model;
- safety margins;
- number of UAVs;
- need for incremental replanning.

The orchestrator shall prefer the simplest algorithm that adequately solves the current subproblem within the required safety and quality limits.

## 5. Hard constraints first

Safety and mandatory constraints are feasibility gates, not soft optimization preferences.

The decision sequence is mandatory:

```text
MANDATORY REQUIREMENTS
      ↓
ADMISSIBILITY CHECK
      ↓
REJECT NON-ADMISSIBLE CANDIDATES
      ↓
COMPARE REMAINING CANDIDATES
      ↓
BEST SAFE SOLUTION
```

A candidate violating a mandatory constraint is rejected before economic, quality, time or other optimization scoring. A candidate shall not become admissible merely because it has a better optimization score.

Typical hard constraints:

- prohibited/restricted airspace;
- terrain/obstacles;
- vehicle operating envelope;
- equipment limitations;
- minimum energy reserve;
- mandatory regulatory conditions;
- C2 requirements;
- navigation requirements;
- mission-specific safety constraints.

The normalized solver result shall carry any detected mandatory-constraint violations in `constraint_violations`. The orchestrator uses this field as a feasibility gate: a candidate with one or more reported violations is not eligible for ranking or final selection.

## 6. Mission-specific priorities

The orchestrator shall derive optimization priorities from the mission objective rather than requiring the operator to manually specify algorithm weights.

Examples:

### Inspection

Primary: coverage and required observation quality.
Secondary: safety, energy, time and equipment constraints.

### High-quality imaging

Primary: image/observation quality and required geometry.
Secondary: safety, energy and time.

### Volumetric / 3D task

Primary: geometric coverage and completeness.
Secondary: observation quality, safety, energy and time.

### Delivery

Primary: successful delivery and required arrival conditions.
Secondary: safety, energy reserve, route/time and vehicle wear.

These profiles are configurable and versioned. Mandatory safety constraints always override mission preferences.

## 7. Heterogeneous fleet

Different UAVs may use different algorithms for different subtasks of the same mission.

```text
MISSION
 ├─ Subtask A → UAV-01 → Solver A
 ├─ Subtask B → UAV-02 → Solver B
 └─ Subtask C → UAV-03 → Solver C
```

The orchestrator evaluates the resulting fleet solution as a whole. It shall account for inter-UAV dependencies, timing, separation and resource constraints.

## 8. Parallel evaluation without preparation slowdown

Algorithm evaluation shall be designed as a bounded computational pipeline.

Where computational resources permit, independent candidates shall run in parallel. The orchestrator shall use early rejection, staged evaluation and configurable time budgets.

```text
FAST FEASIBILITY
      ↓
REJECT CLEAR LOSERS
      ↓
PARALLEL QUALITY EVALUATION
      ↓
DETAILED OPTIMIZATION ONLY FOR SURVIVORS
```

The system shall not wait for every theoretically possible algorithm when a sufficiently good safe solution has already been established under the configured decision policy.

## 9. Candidate scoring

After hard-constraint filtering, candidates are compared using mission-derived criteria. At minimum the framework shall support:

- safety margin;
- energy reserve / predicted consumption;
- mission quality;
- flight time;
- engine/propulsion resource impact where modelled;
- route length;
- operational complexity;
- robustness to uncertainty;
- computation time.

The ranking model must preserve hard constraints separately from optimization scores.

## 10. Energy priority

Energy is a first-class planning variable. A candidate with insufficient mandatory reserve is infeasible regardless of image quality, time or route efficiency.

Among feasible candidates, the orchestrator shall prefer solutions that preserve an adequate energy margin, subject to the mission objective.

## 11. Wind integration

The base route/coverage solution is generated from the mission geometry and constraints. Current wind data is then applied to the candidate solution and the affected flight parameters are recalculated.

Where wind materially changes feasibility or ranking, the orchestrator shall trigger the required local or full recalculation.

```text
MISSION GEOMETRY
      ↓
BASE SOLUTION
      ↓
CURRENT WIND
      ↓
FLIGHT-PARAMETER RECALCULATION
      ↓
RE-RANK / REPLAN IF REQUIRED
```

## 12. Corrections / explanation

The operator is not required to select the algorithm.

When the selected route differs from an intuitive direct route, BlueSky shall provide concise, non-fatiguing reasoning, for example:

- "Route selected to preserve energy reserve."
- "Route avoids restricted airspace."
- "Route selected for required imaging geometry."
- "Headwind increases consumption on the direct segment."
- "UAV-02 assigned this section because its equipment/capability is better suited."

The explanation is evidence of the decision basis, not a replacement for formal logs.

## 13. Algorithm result contract

Every solver participating in orchestration shall return a normalized result containing:

- candidate ID;
- solver ID/version;
- execution time;
- feasibility status;
- constraint violations;
- route/trajectory;
- predicted energy/resource use;
- mission-quality metrics;
- uncertainty/robustness indicators where available;
- explanation inputs;
- provenance.

## 14. Deterministic decision policy

For identical controlled inputs and configuration, the orchestrator shall produce a reproducible decision, subject to explicitly documented stochastic solver behaviour and controlled random seeds where required.

Tie-breaking rules shall be explicit.

## 15. Runtime orchestration

The same architecture shall operate during execution. Runtime events may trigger selection of a faster incremental solver instead of repeating the complete initial planning pipeline.

Example:

```text
SMALL CHANGE → local / incremental solver
LARGE CHANGE → full planner portfolio
CRITICAL CHANGE → recovery / failsafe policy
```

## 16. Safety boundary

The orchestrator cannot override mandatory safety constraints, regulatory restrictions, validated vehicle limits or minimum energy requirements.

A faster or more economical algorithm is never selected solely because it is faster if its resulting solution is not adequately validated.

## 17. Acceptance criteria

The orchestrator is accepted when:

1. the operator can state the mission without selecting an algorithm;
2. algorithms are selected according to the actual planning subproblem;
3. hard constraints reject infeasible candidates before scoring;
4. heterogeneous UAVs may use different appropriate solvers;
5. parallel/staged evaluation prevents unnecessary preparation delay;
6. energy reserve is a mandatory feasibility criterion;
7. wind is applied to the prepared solution and can trigger recalculation;
8. mission-specific quality priorities are represented;
9. the selected decision has concise operator-facing reasoning;
10. solver results and selection decisions are traceable and replayable;
11. runtime replanning can use an appropriate incremental/full solver;
12. the complete process is measurable in SIL/HIL and real-UAV verification.

## 18. Implementation boundary

Solver implementations are separate modules. The orchestrator owns selection, scheduling, feasibility gating, comparison and decision provenance. Individual solvers do not decide which solver should run next.
