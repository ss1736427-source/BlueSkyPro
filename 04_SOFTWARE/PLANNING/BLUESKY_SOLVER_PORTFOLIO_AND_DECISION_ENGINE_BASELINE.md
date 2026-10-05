# BlueSky PRO — Solver Portfolio and Decision Engine Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## 1. Purpose

Define the execution architecture for rapid evaluation of multiple route-planning algorithms and selection of the best admissible solution for the actual mission.

The user supplies the mission objective. BlueSky determines the planning strategy, algorithm set, vehicle allocation and final solution without requiring the user to select an algorithm.

## 2. Core principle

```text
MISSION OBJECTIVE
      ↓
PROBLEM CLASSIFICATION
      ↓
CONSTRAINT / CAPABILITY FILTER
      ↓
SOLVER PORTFOLIO
      ↓
PARALLEL CANDIDATE GENERATION
      ↓
FAST FEASIBILITY FILTER
      ↓
MULTI-CRITERIA EVALUATION
      ↓
BEST ADMISSIBLE SOLUTION
      ↓
EXPLANATION
```

The orchestrator is a coordinator, not a solver. It shall not become a serial bottleneck.

## 3. Solver portfolio

The implementation shall support a portfolio rather than one universal routing algorithm.

Candidate families may include:

- Dijkstra / graph shortest-path methods for discrete constrained routing;
- A* for fast goal-directed graph search;
- weighted or multi-objective graph search where appropriate;
- D* Lite / incremental search for changing environments;
- coverage-path planning methods for area inspection;
- vehicle-routing / assignment methods for multi-UAV task allocation;
- trajectory optimization for vehicle-dynamics-aware refinement;
- specialized domain solvers introduced as independently versioned plugins.

No solver is mandatory for every mission. Applicability is determined by the problem classifier and constraints.

## 4. Problem classification

The classifier shall derive the planning problem from the mission objective and available data.

Relevant dimensions include:

- mission type;
- geometry: point-to-point, corridor, area, volume, network;
- number of UAVs;
- vehicle capabilities;
- equipment capabilities;
- terrain/obstacle model;
- airspace constraints;
- timing constraints;
- energy constraints;
- wind/environmental uncertainty;
- required output quality;
- required arrival accuracy;
- equipment operating constraints.

## 5. Hard constraints

A candidate violating a mandatory safety or operational constraint is inadmissible regardless of score.

Hard constraints include, as applicable:

- regulatory restrictions;
- geofencing;
- vehicle limits;
- equipment limits;
- minimum energy reserve;
- recovery feasibility;
- terrain/obstacle clearance;
- mandatory waypoints/segments;
- required timing constraints;
- C2 requirements.

## 6. Objective priorities

Mission objectives determine the relative priorities used for admissible candidate comparison.

Examples:

| Mission class | Primary objective | Secondary objectives |
|---|---|---|
| Inspection | task coverage / required observation quality | energy, time, route efficiency |
| Photo / video | image quality / geometry | energy, time, coverage |
| 3D / volumetric | geometric coverage / reconstruction quality | energy, time |
| Delivery | successful delivery / arrival condition | energy, time, route efficiency |
| Long endurance | endurance / reserve margin | coverage, time |
| Fast mission | completion time | energy, quality |
| Precise arrival | arrival accuracy / timing | energy, route efficiency |

These profiles are configurable and versioned. Safety constraints remain above objective optimization.

## 7. Heterogeneous fleet

Different UAVs may use different solvers for different subtasks.

```text
MISSION
 ├── UAV-01 → Solver A → subtask 1
 ├── UAV-02 → Solver B → subtask 2
 └── UAV-03 → Solver C → subtask 3
```

The orchestrator evaluates the complete fleet solution, not only individual route quality.

## 8. Evaluation sequence

To avoid preparation delays, evaluation shall be staged:

### Stage 1 — Cheap rejection

Immediately eliminate candidates failing static capability, geometry, regulatory, energy or other hard constraints.

### Stage 2 — Fast feasibility

Run the fastest suitable solver(s) to establish feasible solutions quickly.

### Stage 3 — Parallel portfolio evaluation

Run applicable alternative solvers concurrently when resources permit.

### Stage 4 — Refinement

Apply more expensive optimization only to the best feasible candidates.

### Stage 5 — Final validation

Validate the selected solution against authoritative constraints and current environmental data.

This prevents expensive algorithms from delaying the basic mission solution.

## 9. Energy-first admissibility

Energy is a hard feasibility criterion, not merely a score component.

The evaluation shall use the authoritative energy model and account for:

- battery state of charge;
- battery degradation;
- propulsion configuration;
- equipment configuration;
- predicted consumption;
- wind;
- recovery/return requirement;
- required reserve.

Among otherwise admissible solutions, the decision engine may optimize energy consumption and reserve margin according to the mission profile.

## 10. Wind application

The planning process remains staged:

```text
BASE SOLUTION
     ↓
CURRENT WIND DATA
     ↓
WIND-AWARE RECALCULATION
     ↓
FEASIBILITY / OBJECTIVE CHECK
```

Where wind is sufficiently uncertain or dynamic, incremental/local replanning may be used instead of rebuilding the entire solution.

## 11. Decision scoring

Candidate solutions shall first pass hard constraints. Only then are soft objectives evaluated.

Conceptually:

```text
if HARD_CONSTRAINTS_FAIL:
    REJECT
else:
    SCORE(objectives, energy, time, quality, efficiency)
```

The scoring model shall be explicit, versioned and reproducible.

## 12. Resource control

The orchestrator shall have bounded computation budgets.

Required controls:

- time budget;
- CPU/GPU budget where applicable;
- memory budget;
- solver concurrency limit;
- early termination threshold;
- candidate count limit;
- fallback solver.

A solver that exceeds its budget shall return a partial/timeout status and shall not block the mission if an admissible solution already exists.

## 13. Best-so-far strategy

The system shall maintain a continuously available `best_admissible_solution`.

```text
candidate → feasible? → better than current best?
                         ↓
                     replace best
```

The operator should not wait for every theoretical solver to finish when a sufficiently validated solution is already available.

## 14. Algorithm selection

Selection shall be based on measured problem characteristics and solver capability metadata, including:

- supported problem class;
- constraint handling;
- expected solution quality;
- computational complexity/performance;
- deterministic behaviour;
- ability to support incremental replanning;
- suitability for the mission geometry;
- validation status.

Historical performance may be used as a secondary selection signal only when it is traceable and does not override safety or mandatory constraints.

## 15. Explanation to pilot

The system shall not expose algorithmic complexity by default.

It shall provide concise, decision-oriented explanations, for example:

> **Маршрут выбран так, потому что:** встречный ветер на прямом участке увеличивает расход энергии; выбранный вариант сохраняет необходимый резерв и обеспечивает требуемое качество съёмки.

The explanation shall identify the material reason for the selected solution without requiring the pilot to understand the internal solver portfolio.

## 16. Provenance

For every selected solution, retain:

- problem classification;
- candidate solvers considered;
- rejected candidates and rejection reason;
- solver versions;
- input data versions/timestamps;
- scoring model version;
- selected solution;
- final validation result;
- explanation basis.

## 17. Safety isolation

The orchestrator may select and compare solutions but cannot override mandatory safety gates.

The final executable Mission Package must pass the independent validation and release process before execution.

## 18. Acceptance criteria

The Decision Engine is accepted when:

1. the user can specify a mission without selecting an algorithm;
2. the system classifies the planning problem automatically;
3. unsuitable solvers are filtered before expensive computation;
4. applicable solvers can run in parallel;
5. a best admissible solution is available progressively;
6. hard constraints reject unsafe candidates;
7. energy reserve is treated as a mandatory feasibility condition;
8. mission-specific priorities influence ranking;
9. different UAVs may use different suitable solvers;
10. the selected result is reproducible and explainable;
11. computation budgets prevent orchestration from becoming a preparation bottleneck;
12. the final result enters Mission Package validation rather than bypassing it.

## 19. Implementation boundary

Solver implementations are independent plugins/services behind a common solver contract. The orchestrator owns classification, scheduling, candidate lifecycle and decision policy; individual solvers own their mathematical optimization.
