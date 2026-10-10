# Multi-UAV Conflict Resolution

**ID:** PLAN-MULTI-005
**Status:** BASELINED

Deterministic pre-flight resolution of calculated multi-UAV conflicts.

## Contract

Conflict resolution occurs on the ground, before the affected UAVs start. The component consumes versioned 4D trajectories and an already calculated conflict result through the existing detector.

Resolution order:
1. retain a clear plan;
2. temporal conflicts may be resolved by the existing start-delay sequencer;
3. for a same-altitude crossing that remains conflicting, apply the vertical profile rule;
4. right-by-course UAV: +1 m;
5. left-by-course UAV: -1 m;
6. apply the offset only to the affected pre-planned conflict segment;
7. return to the original profile after the segment;
8. re-run continuous 4D conflict detection;
9. unresolved residual conflict remains a hard planning/readiness blocker.

The 1 m value is the vertical maneuver step. For equal initial altitudes it produces a nominal 2 m vertical difference. It is not a universal regulatory separation requirement.

The resolver does not issue in-flight commands and does not modify authorization, safety, readiness or execution state. It only produces versioned corrected trajectory candidates. A correction is accepted only if altitude limits are satisfied and the continuous conflict recheck is clear.

Ambiguous right/left course geometry is not guessed; it produces an unresolved result.

## Lineage

Each corrected trajectory retains its original dependency identity and appends a deterministic vertical-resolution dependency. Superseded trajectory variants remain historical; only the current consolidated result is an active planning input.

## Architecture

4D TRAJECTORIES → CONFLICT DETECTION → CONFLICT RESOLUTION → CONFLICT RECHECK → CANDIDATE COMPARISON → SELECTED ROUTE SET → FLIGHT PROFILE → FINAL INTEGRITY → READY


## Right/left determination

For a same-altitude conflict, right/left is determined by the **signed course angle from the LZP (Line of Intended Path / planned course)** to the line of sight from the UAV to the conflicting UAV at the calculated conflict time. Positive clockwise course angle denotes the other UAV on the right; negative denotes the other UAV on the left. A zero/ambiguous angle is not guessed and remains unresolved.


## Route intersection and collision-resolution timing

A route intersection point is an existing point defined by the route geometry; the resolver does not create or calculate a new intersection point as a separate planning object.

When routes intersect at that defined point, the planner checks whether the affected 4D trajectories create a collision. If a collision exists, it is resolved on the ground before any affected UAV starts.

The maximum permitted start-time correction for resolving a calculated collision is **5 seconds**. The resolver shall use the minimum delay required to remove the conflict, with the configured upper bound of 5 s. After every timing correction, the affected 4D conflict result is recalculated. If the collision remains, the deterministic vertical trajectory correction is applied according to the established right/left LZP rule, followed by another 4D conflict check.

The 5-second value is a maximum planning correction window, not a mandatory delay and not a regulatory separation requirement.
