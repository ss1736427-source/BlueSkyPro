# BlueSky PRO — Solver Contract Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## 1. Purpose

Define the stable interface between the BlueSky Algorithm Orchestrator and individual planning solvers. A solver is an independently replaceable planning component; the orchestrator owns selection, scheduling, comparison and release of candidates.

## 2. Contract boundary

```text
Mission Problem
      ↓
Orchestrator
      ↓
Solver Contract
 ┌────┼────────┬─────────┐
 ↓    ↓        ↓         ↓
A*  Dijkstra  D* Lite  Coverage...
 └────┴────────┴─────────┘
      ↓
Candidate Solution
      ↓
Evaluation / Validation
```

## 3. Solver lifecycle

```text
AVAILABLE
   ↓
ELIGIBILITY_CHECK
   ↓
STARTED
   ↓
RUNNING
   ├── CANDIDATE
   ├── PARTIAL
   ├── COMPLETED
   ├── TIMEOUT
   ├── CANCELLED
   └── FAILED
```

A solver must never control mission execution directly.

## 4. Solver metadata

Each solver shall expose machine-readable metadata including:

- unique solver ID;
- version;
- supported problem classes;
- supported constraints;
- required input data;
- optional input data;
- supported vehicle/trajectory models;
- deterministic/non-deterministic characteristics;
- incremental-replanning capability;
- expected computational cost;
- validation/qualification status.

## 5. Input contract

The orchestrator provides a normalized planning problem containing, as applicable:

- mission objective;
- geometry and coverage requirements;
- assigned UAV(s);
- vehicle capabilities;
- equipment capabilities;
- environmental data;
- airspace/regulatory constraints;
- terrain/obstacle data;
- energy model inputs;
- timing constraints;
- mandatory route elements;
- computation budget.

The solver shall treat the supplied hard constraints as mandatory.

## 6. Output contract

A solver returns a candidate with:

- candidate ID;
- solver ID/version;
- route/trajectory;
- assigned vehicle(s);
- mission actions;
- estimated execution time;
- estimated energy consumption;
- estimated reserve margin;
- quality/objective metrics;
- constraint status;
- warnings;
- computation statistics;
- provenance.

## 7. Feasibility result

Every candidate shall have an explicit feasibility state:

`FEASIBLE / INFEASIBLE / UNCERTAIN / PARTIAL`

`INFEASIBLE` candidates cannot enter normal objective ranking.

`UNCERTAIN` candidates require the applicable validation path before they can become admissible.

## 8. Hard-constraint contract

The solver may report violations but cannot waive them.

At minimum the contract supports evaluation of:

- regulatory restrictions;
- geofence;
- terrain/obstacle clearance;
- vehicle limits;
- equipment limits;
- minimum energy reserve;
- recovery feasibility;
- C2 requirements;
- mandatory waypoints/segments;
- timing requirements.

## 9. Progressive results

The solver may return candidates before its complete search terminates.

The orchestrator may evaluate each candidate immediately and update `best_admissible_solution` without waiting for other solvers.

This is mandatory for preparation-time performance.

## 10. Cancellation and budget

The orchestrator shall be able to cancel a running solver.

A solver receives bounded:

- wall-clock time;
- compute budget;
- memory budget;
- result/candidate budget.

Timeout or cancellation is a normal controlled outcome, not a system fault by itself.

## 11. Determinism and reproducibility

A solver shall report the information required to reproduce its result, including:

- solver version;
- relevant configuration;
- input dataset/version identifiers;
- random seed where applicable;
- termination condition;
- computation budget.

## 12. Safety separation

The solver produces candidates. The independent validation/release layer determines whether a candidate may become an executable Mission Package.

No solver result may bypass:

```text
constraint validation
→ mission package validation
→ release gate
```

## 13. Error model

Errors shall be structured and distinguish at least:

- invalid input;
- unsupported problem class;
- unsupported constraint;
- missing data;
- computation timeout;
- resource exhaustion;
- numerical/algorithmic failure;
- internal implementation failure.

The orchestrator uses these states to select fallback or alternative solvers where appropriate.

## 14. Solver portfolio example

```text
Point-to-point constrained route → A* / Dijkstra
Dynamic route environment       → D* Lite
Area inspection                 → Coverage Planner
Multi-UAV allocation            → Assignment / VRP solver
Vehicle-dynamics refinement     → Trajectory Optimizer
```

The mapping is advisory metadata, not a hard-coded global algorithm choice. The orchestrator may combine solvers when the problem decomposition benefits from it.

## 15. Multi-UAV use

A solver may operate on:

- one UAV;
- a subset of UAVs;
- the complete fleet.

The contract shall explicitly declare its scope so that the orchestrator can combine results without ambiguity.

## 16. Acceptance criteria

The Solver Contract is accepted when:

1. a solver can be added without modifying orchestrator semantics;
2. metadata allows automatic eligibility selection;
3. inputs are normalized and versioned;
4. candidates are independently identifiable;
5. feasibility is explicit;
6. hard constraints cannot be waived by the solver;
7. progressive candidates are supported;
8. execution budgets and cancellation are supported;
9. results are reproducible;
10. structured failures support fallback;
11. multi-UAV scope is explicit;
12. all candidates enter the common validation path.
