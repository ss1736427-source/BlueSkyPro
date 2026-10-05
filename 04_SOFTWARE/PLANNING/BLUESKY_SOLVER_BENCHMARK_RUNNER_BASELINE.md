# BlueSky PRO — Solver Benchmark Runner Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## Purpose

Provide a deterministic test harness for comparing planning solvers on identical normalized planning problems.

The benchmark is a development and qualification tool. Its measurements must not be used as an operational safety decision without the independent validation layer.

## Test principle

```text
ONE INPUT PROBLEM
       ↓
 ┌─────┴─────┐
 ↓           ↓
A*        Dijkstra
 ↓           ↓
candidate  candidate
 └─────┬─────┘
       ↓
common validator
       ↓
common metrics
       ↓
comparison report
```

## Identical inputs

Every solver invocation shall receive the same:

- normalized planning graph;
- start/goal;
- constraints;
- vehicle configuration;
- equipment configuration;
- energy-model inputs;
- environmental data;
- computation budget;
- objective profile.

No solver may receive hidden advantages through solver-specific preprocessing unless that preprocessing is explicitly part of the solver contract and measured.

## Required metrics

For every run record:

- solver ID/version;
- problem ID/version;
- result state;
- feasibility;
- route cost;
- route length where applicable;
- estimated execution time;
- estimated energy consumption;
- reserve margin;
- objective metrics;
- nodes expanded;
- wall-clock time;
- peak resource usage where available;
- timeout/cancellation state;
- validation result.

## Correctness reference

For non-negative graph costs, Dijkstra is the baseline shortest-path reference. A* shall be checked against the same optimum when its heuristic is admissible and the problem assumptions permit that guarantee.

A benchmark shall distinguish:

`SAME_OPTIMUM`

from

`BETTER_HEURISTIC_PERFORMANCE`

so that a faster result is not confused with a different objective.

## Benchmark cases

The runner shall support at least:

1. small sparse graph;
2. small dense graph;
3. medium graph;
4. large graph;
5. graph with many dead ends;
6. graph with alternative equal-cost paths;
7. graph with disconnected regions;
8. graph with constrained edges;
9. repeated runs for variance measurement;
10. timeout-limited runs.

## Test invariants

For each valid case:

- both solvers receive equivalent inputs;
- an infeasible result is never ranked as a valid optimum;
- route endpoints are correct;
- every route edge exists in the input graph;
- total route cost equals the sum of traversed edge costs within the defined numerical tolerance;
- a cancelled solver does not publish a completed result;
- timeout does not become a successful completion;
- benchmark records preserve solver provenance.

## Decision use

Benchmark data feeds solver metadata and future orchestrator policy only after qualification.

The orchestrator may use measured performance to prefer a solver for a problem class, but must retain a safe fallback and must not assume historical performance guarantees current feasibility.

## Acceptance

The benchmark runner is accepted when it can execute A* and Dijkstra against the same normalized test set, validate their outputs through a common path, collect the defined metrics, and produce a reproducible comparison report.
