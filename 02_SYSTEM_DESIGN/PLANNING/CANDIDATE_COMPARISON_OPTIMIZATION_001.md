# Candidate Comparison and Optimization

**ID:** PLAN-OPT-001
**Status:** BASELINED

## Purpose

Select one feasible route candidate from already calculated candidate results without repeating spatial, wind, performance, 4D trajectory, or multi-UAV calculations.

## Inputs

- versioned feasible/infeasible candidate solutions;
- declared objective priority order;
- calculation-input version.

Supported objective dimensions at this boundary:
- `time`: lower is preferred;
- `energy`: lower is preferred;
- `reserve`: higher is preferred;
- `objective_score`: lower is preferred.

The objective order is explicit mission configuration, not an implicit product preference.

## Determinism

Mandatory feasibility is evaluated before objective ordering. Infeasible candidates are rejected from comparison. Equal objective values use stable `candidate_id` ordering.

## Recalculation boundary

This component consumes existing candidate metrics. It performs no terrain, restriction, wind, UAV-performance, trajectory, or conflict calculation. If a dependency changes, the affected candidate must be recalculated upstream and supplied as a new version.

## Authority boundary

This component selects a planning candidate only. It does not authorize, mark READY, execute, modify safety constraints, or override regulatory/physical restrictions.

Canonical pipeline:

`ROUTE CANDIDATES → UAV ASSIGNMENT → WIND/PERFORMANCE → 4D TRAJECTORIES → MULTI-UAV CONFLICT → SEQUENCING → CANDIDATE COMPARISON/OPTIMIZATION → SELECTED ROUTE SET`

## Mandatory prohibited-zone avoidance

The optimizer may compare only candidates that have passed the current versioned restriction checks. Candidate generation may explore both lateral detours and vertical bypasses above a restriction's known upper boundary, subject to a compatible altitude datum, aircraft limits, terrain/obstacle constraints, and required safety margin. The complete vertical envelope of each segment matters: a climb or descent through the restricted band inside the horizontal footprint is not a valid bypass. No objective improvement (distance, time, or energy) may override a prohibited-zone constraint. If all candidates intersect a prohibited zone or the restriction cannot be evaluated reliably, selection must fail closed with a reason; it must not return an intersecting candidate as feasible.
