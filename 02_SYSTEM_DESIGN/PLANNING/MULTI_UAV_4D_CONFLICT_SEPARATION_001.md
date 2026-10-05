# Multi-UAV 4D Conflict / Separation

**ID:** PLAN-CALC-002
**Status:** BASELINED

## Purpose

Evaluate separation using the already calculated 4D trajectories. This component does not recalculate wind, vehicle performance or route geometry.

Inputs:
- feasible 4D trajectories;
- UAV identities;
- horizontal and vertical separation envelopes;
- temporal tolerance;
- calculation version.

For each UAV pair, the detector evaluates overlapping time intervals and samples trajectory endpoints/waypoints within the overlap, interpolating position and altitude at those times.

A conflict is a hard feasibility finding when both horizontal and vertical separation requirements are simultaneously violated.

## Dependency reuse

The dependency identity contains each trajectory dependency identity, separation parameters and calculation version. A start-delay correction changes temporal overlap/conflict evaluation without requiring route or wind/performance recalculation.

## Authority boundary

The detector:
- does not execute start delays;
- does not modify routes;
- does not authorize flights;
- does not change readiness/safety state.

Conflict resolution is a separate planning operation. Candidate sequencing or local replanning must produce a new versioned result and then re-run only affected downstream calculations.


## Continuous separation

Conflict evaluation is performed continuously over overlapping linear trajectory segments in time. For each segment pair, the algorithm evaluates the interval satisfying the vertical-separation condition and finds the minimum horizontal separation within that interval. Waypoint sampling alone is not sufficient as a safety boundary.
