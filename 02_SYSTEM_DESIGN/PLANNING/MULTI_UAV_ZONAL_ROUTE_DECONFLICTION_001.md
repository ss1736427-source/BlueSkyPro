# Multi-UAV Zonal Route Deconfliction

**ID:** PLAN-MULTI-006
**Status:** BASELINED

## Purpose

Prevent multi-UAV trajectory intersections at route-generation time by assigning each participating UAV a distinct operational zone/sector and generating its route only inside that zone.

## Priority

ZONE PARTITION → UAV ZONE ASSIGNMENT → ROUTE GENERATION INSIDE ZONE → WIND/PERFORMANCE → 4D VERIFICATION → CONFLICT-RESOLUTION FALLBACK

Each UAV should operate in a clean zone/sector belonging to that UAV for the planned mission interval. Zone boundaries are planning boundaries and do not constitute regulatory authorization.

## Fallback

If non-overlapping zoning cannot satisfy mission coverage, geometry, constraints or other hard feasibility requirements, the planner invokes the established ground-only conflict-resolution policy:

1. minimum necessary start delay, maximum 5 s;
2. continuous 4D conflict recheck;
3. if required and applicable, vertical trajectory correction according to the established LZP right/left rule;
4. return to the original profile after the affected conflict segment;
5. continuous 4D recheck;
6. unresolved conflict blocks readiness.

## Constraints

Zoning must respect the same constrained-open-space environment as ordinary route planning. Zoning cannot remove or relax regulatory restrictions, authorization requirements, terrain, obstacles, altitude limits, UAV performance limits or safety constraints.

## Recalculation

Zone assignment is a versioned planning result. Changes invalidate only affected zones and their downstream routes/trajectories/conflict results. Unaffected zones remain reusable under the calculation-reuse baseline.

## Mission composition entry point

Automatic and manual mission creation converge on the same planning pipeline.

- **Automatic path:** user task → system derives/assigns applicable templates → composed mission/coverage.
- **Manual path:** operator selects one or more templates → each contributes a planning scaffold on the Flight Chart → operator combines and edits the scaffolds into one mission composition.
- **Convergence:** the resulting mission/coverage enters the same constrained-open-space, zoning, UAV assignment, in-zone routing, wind/performance, 4D trajectory verification and conflict-resolution flow.

The only intended difference is the template-selection stage: system-selected for automatic creation, operator-selected for manual creation. Manual composition does not bypass any downstream constraint, validation, safety gate or approval. Template scaffolds are planning inputs and must be reconciled into one coherent mission before downstream planning.

## Architecture

MISSION/COVERAGE → CONSTRAINED OPEN SPACE → ZONE PARTITION → UAV ZONE ASSIGNMENT → ROUTE-IN-ZONE → 4D TRAJECTORY → 4D VERIFY → CONFLICT RESOLUTION FALLBACK → CANDIDATE COMPARISON
