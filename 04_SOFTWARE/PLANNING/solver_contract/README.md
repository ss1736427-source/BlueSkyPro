# BlueSky PRO — Solver Contract Implementation

This directory defines the first executable-facing contract for planning solvers.

## Runtime flow

```text
Mission Problem
      ↓
Eligibility
      ↓
Solver.run()
      ↓
Candidate stream
      ↓
Hard-constraint gate
      ↓
Decision Engine
      ↓
best_admissible_solution
```

## Required solver interface

A solver implementation must provide:

- metadata;
- eligibility evaluation;
- bounded execution;
- candidate production;
- cancellation;
- structured completion/error state.

The orchestrator must be able to run several solvers independently and consume candidates as they arrive.

## Safety boundary

The solver cannot waive hard constraints and cannot release a mission for execution.

## Performance boundary

The implementation must support:

- parallel solver execution;
- time budgets;
- candidate limits;
- cancellation;
- best-so-far evaluation.
