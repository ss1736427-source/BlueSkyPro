# Multi-UAV Start-Delay Sequencing

**ID:** PLAN-MULTI-003
**Status:** BASELINED
**Scope:** deterministic conflict resolution after 4D trajectory calculation.

## Purpose

Resolve a detected multi-UAV temporal conflict by changing only start timing, when a configured start delay can remove the conflict without changing route geometry, wind/performance inputs, or spatial constraints.

## Contract

`MultiUavSequencer` consumes:
- versioned 4D trajectories;
- per-UAV separation requirements;
- per-UAV initial delay, positive delay step, and maximum delay;
- calculation version.

It produces:
- versioned start-delay schedule;
- dependency identity;
- resolved/unresolved status;
- unresolved conflict evidence when timing alone cannot resolve the conflict.

The operation is deterministic. UAV IDs provide the stable tie-breaker when selecting which conflicting UAV is delayed.

## Dependency and recalculation

A start-delay change does **not** alter the spatial route or the wind/performance model. It creates a new temporal trajectory identity. The shifted trajectories are re-evaluated by the existing multi-UAV conflict detector.

Downstream consumers must use the new versioned trajectories. No unaffected spatial route or wind calculation is repeated.

## Boundaries

This component:
- does not change route geometry;
- does not modify authorization, safety, readiness, or execution state;
- does not grant permission to operate;
- does not silently relax separation requirements;
- does not use AI to make the authoritative decision.

If the configured delay budget cannot resolve the conflict, the result remains unresolved and must proceed to another deterministic planning operation (for example local route replanning or task redistribution).

## Relationship to mission planning

The canonical sequence is:

`ROUTE CANDIDATES
→ UAV ASSIGNMENT
→ WIND/PERFORMANCE
→ 4D TRAJECTORIES
→ MULTI-UAV CONFLICT DETECTION
→ START-DELAY SEQUENCING
→ CONFLICT RECHECK
→ CANDIDATE COMPARISON/OPTIMIZATION
`

A fixed one-second delay is not a universal rule. Delay step and maximum delay are mission/UAV constraints and must be supplied explicitly.
