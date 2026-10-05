# BlueSky PRO — Solver Benchmark Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## Purpose

Measure solver performance and solution quality on the same normalized planning problem. The benchmark is an engineering measurement tool for the orchestrator; it is not a safety approval mechanism.

## A* comparison modes

A* shall be measured with:

1. zero heuristic — correctness/reference mode;
2. spatial admissible heuristic — performance mode.

The spatial heuristic is admissible only when the graph cost model guarantees that edge cost is not less than the corresponding geometric lower bound.

## Dijkstra baseline

Dijkstra is the optimal-cost reference for non-negative edge costs. Its result is used to verify that A* with an admissible heuristic preserves optimality on the benchmark graph.

## Metrics

For each solver run record:

- wall-clock time;
- nodes expanded;
- edges evaluated;
- candidate generation time;
- route cost;
- route length where available;
- estimated energy;
- estimated reserve;
- feasibility state;
- timeout/cancellation state.

## Test classes

The benchmark shall include at least:

- small graph — functional correctness;
- medium graph — normal planning workload;
- large graph — scalability;
- constrained graph — obstacle/restricted-edge filtering;
- sparse graph;
- dense graph;
- repeated start/goal pairs;
- cases where the heuristic strongly reduces search;
- cases where the heuristic provides little benefit.

## Acceptance

A benchmark run shall verify:

- A* and Dijkstra return the same optimal route cost where the A* heuristic is admissible;
- zero-heuristic A* agrees with Dijkstra on route cost;
- spatial-heuristic A* agrees with Dijkstra on route cost under its admissibility condition;
- the spatial heuristic reduces or does not materially increase search work for applicable graphs;
- timeout and cancellation do not produce a false `Feasible` result;
- results remain reproducible for identical inputs and solver versions.

## Orchestrator use

Benchmark measurements feed solver-selection metadata and performance models. They must not override hard constraints or final safety validation.

The orchestrator may prefer A* over Dijkstra when measured performance is better for the current problem class, while retaining Dijkstra as a reference/fallback where appropriate.
