# Multi-UAV Ground Conflict Resolution

**ID:** PLAN-MULTI-004  
**Status:** BASELINED  
**Scope:** deterministic pre-flight resolution of calculated multi-UAV trajectory conflicts.

## Principle

All planned multi-UAV conflicts must be resolved **on the ground before any affected UAV starts**. The system does not treat an unresolved calculated conflict as an in-flight maneuver instruction.

The execution package contains already coordinated 4D trajectories and start times. A conflict remaining after planning is a hard condition preventing the affected multi-UAV plan from becoming READY.

## Resolution order

1. Calculate versioned wind/performance-adjusted 4D trajectories.
2. Run continuous 4D conflict detection.
3. If no conflict exists, retain the trajectories.
4. If a dangerous temporal conflict exists, first evaluate a start-time correction within configured limits.
5. Re-run continuous conflict detection.
6. If trajectories intersect at the same altitude and timing correction is insufficient, apply the deterministic vertical profile rule:
   - UAV on the right by course: **+1 m**;
   - UAV on the left by course: **-1 m**.
7. The vertical offset is applied only to the pre-planned conflict segment/profile.
8. After the conflict zone, each UAV returns to its required original flight profile.
9. Re-run continuous 4D conflict detection.
10. If a conflict remains, the plan is unresolved and cannot proceed to READY. Another deterministic planning alternative must be calculated.

## Vertical profile rule

The 1 m value is the maneuver step, not a universal regulatory separation requirement. For a two-UAV same-altitude crossing, the resulting nominal vertical difference is **2 m**.

The vertical maneuver is part of the planned trajectory:

ORIGINAL PROFILE → ±1 m CONFLICT SEGMENT → ORIGINAL PROFILE

It is not an instruction generated reactively during flight.

Before accepting the vertical profile, the planning system must verify applicable altitude, terrain/obstacle, airspace, UAV performance and other hard constraints. A maneuver that violates a hard constraint is rejected rather than forced.

## Start-delay correction

Start delay is a planning correction, not an in-flight action.

The planner may delay one UAV by the minimum configured amount needed to remove the calculated temporal conflict, subject to the mission's permitted delay range. The resulting start time becomes part of the versioned execution plan.

A start delay invalidates and recalculates only affected temporal/conflict downstream results; unchanged route geometry and unaffected calculations are reused.

## Required final condition

Before READY:

MULTI-UAV PLAN → CONTINUOUS 4D CONFLICT CHECK → ZERO UNRESOLVED CONFLICTS

UNRESOLVED or RESIDUAL CONFLICT is a hard planning/readiness blocker for the affected operation.

## Architecture

4D TRAJECTORIES → CONFLICT DETECTION → CONFLICT RESOLUTION → CONFLICT RECHECK → CANDIDATE COMPARISON → SELECTED ROUTE SET → FLIGHT PROFILE → FINAL INTEGRITY → READY

MultiUavConflictDetector remains detection-only. Conflict resolution is a separate deterministic planning component. It does not grant authorization, change safety authority, or execute commands.

## Traceability

Each correction carries the source trajectory identities, conflict evidence, correction type, correction parameters, calculation version and resulting trajectory/start-time identity. Superseded variants remain historical; only the consolidated current result is an active planning input.
