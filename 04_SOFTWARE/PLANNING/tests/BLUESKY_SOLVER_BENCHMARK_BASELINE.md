# BlueSky PRO — Solver Benchmark Baseline

**Status:** VERIFICATION BASELINE — P0

## Purpose

Provide a repeatable first benchmark for solvers implementing the common Solver Contract.

## First benchmark

The initial test compares **A*** and **Dijkstra** on the same normalized planning graph, with identical start and goal nodes.

The test verifies:

- both solvers are eligible for the same point-to-point problem;
- both return a feasible candidate;
- both produce the same route cost;
- both produce the same route on the deterministic test graph;
- both respect the common execution budget;
- the common candidate evaluator can select an admissible result.

## Important interpretation

The benchmark is a **correctness and integration test**, not yet a production performance claim. Runtime measured on a small synthetic graph cannot establish which solver is faster for operational missions.

## Production benchmark requirements

Future benchmark sets shall vary:

- graph size;
- obstacle density;
- route complexity;
- mission geometry;
- number of vehicles;
- wind scenarios;
- energy constraints;
- solver time budgets.

Metrics shall include:

- wall-clock preparation time;
- nodes/edges evaluated;
- memory consumption;
- candidate quality;
- energy estimate;
- reserve margin;
- feasibility rate;
- timeout rate;
- reproducibility.

## Decision rule

Benchmark data informs solver selection and orchestration, but never overrides hard safety constraints. Solver selection is workload-dependent; no global statement that one algorithm is universally best is permitted.

## Current test

Source:

`astar_dijkstra_benchmark.cpp`

Expected result:

`Benchmark: PASS`

The first implementation uses a zero heuristic for A*, making it equivalent to Dijkstra in search ordering. This is intentional for the initial shared-correctness test; a subsequent benchmark shall use an admissible spatial heuristic to measure the expected A* advantage on suitable route graphs.
