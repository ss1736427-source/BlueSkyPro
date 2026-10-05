# BlueSky PRO — Calculation Reuse and Current-Baseline Principle

**ID:** PLAN-ARCH-001
**Status:** BASELINED
**Scope:** Mission / Flight Planning / Route Optimization / Validation
**Authority:** subordinate to the Master Product Lifecycle Plan

## 1. Purpose

Establish one controlled planning result as the current source of truth and prevent duplicated calculations between planning stages.

## 2. Current-result rule

BlueSky PRO is developed as a continuous controlled process.

When a requirement, algorithm, architecture or document is corrected and consolidated:

- the corrected result becomes the current working baseline;
- subsequent development uses only that corrected result;
- intermediate superseded variants remain historical audit material only;
- implementation and dependent documentation are updated against the current result.

No active design decision may be taken from an intermediate superseded variant.

## 3. Single-calculation rule

Each calculation is owned by the architectural stage where all inputs necessary for that calculation are available.

The stage produces one versioned result.

Downstream stages consume that result instead of independently reproducing the same calculation.

Examples:

- constrained spatial search produces route geometry and spatial feasibility;
- wind/performance produces ground speed, traversal time, energy and 4D trajectory;
- conflict/separation consumes the 4D trajectory;
- candidate comparison consumes already calculated feasibility, time, energy and trajectory data;
- flight-profile compilation consumes the selected trajectory/performance result.

## 4. Dependency-driven recalculation

A change invalidates only the affected result and its downstream dependents.

| Change | Affected calculation chain |
|---|---|
| NOTAM / restricted geometry | spatial candidates → downstream dependents |
| terrain / obstacle | affected spatial candidates → downstream dependents |
| route altitude | affected performance → 4D trajectory → conflict → comparison |
| wind | performance → 4D trajectory → conflict → comparison |
| UAV configuration | assignment-dependent performance → 4D trajectory → conflict → comparison |
| start delay | timing/conflict → comparison |
| objective profile | comparison/selection |
| presentation-only UI change | none |

Unchanged calculations remain reusable.

## 5. Calculation result identity

A reusable material result should contain:

- input snapshot/version;
- algorithm/calculation version;
- dependency identity/hash;
- affected object/segment/candidate IDs;
- calculation timestamp;
- provenance/evidence references.

A result is reusable only while the dependencies relevant to its output remain valid.

## 6. Universal route scope

The constrained-open-space rule is universal across route purposes. It applies to mission, reconnaissance, survey, inspection, transit/repositioning, return, return-to-home, alternate-return, emergency/contingency and recovery routes, including individual routes within multi-UAV operations.

The route purpose may change objectives and generation parameters, but it does not create a separate spatial-planning architecture.

Before route search, BlueSky resolves the current environment into a versioned constrained-open-space model. NOTAM, airspace restrictions, terrain, obstacles, altitude limits and other applicable hard spatial constraints are therefore inputs to route construction rather than a duplicated post-route screening step.

If the environment snapshot changes, dependency-driven invalidation affects only the route results and downstream calculations that depend on the changed input.

## 6. Planning pipeline

```text
MISSION / COVERAGE
        ↓
CONSTRAINED SPATIAL SEARCH
        ↓
ROUTE CANDIDATES
        ↓
UAV CAPABILITY / ASSIGNMENT
        ↓
WIND + VEHICLE PERFORMANCE
        ↓
4D TRAJECTORIES
        ↓
MULTI-UAV CONFLICT / SEPARATION
        ↓
CANDIDATE COMPARISON / OPTIMIZATION
        ↓
SELECTED ROUTE SET
        ↓
FLIGHT PROFILE
        ↓
FINAL INTEGRITY / CHANGE-IMPACT CHECK
```

Hard spatial restrictions are incorporated into the feasible search environment so that prohibited corridors are not generated as selectable route candidates.

## 7. No duplicate validation

Validation remains necessary at defined contract boundaries, but it must not recreate upstream calculations.

A validator verifies the result and its contract, identity, provenance and residual conditions within its responsibility.

It does not independently rebuild:

- the terrain model;
- the restricted-area graph;
- wind/performance calculations;
- 4D trajectories;
- multi-UAV conflict analysis.

## 8. Recalculation trigger

Recalculation is permitted only when:

1. a relevant input changes;
2. a required source becomes stale/invalid;
3. the algorithm/calculation version changes;
4. a dependency is invalidated;
5. a material correction changes the result domain.

A presentation-only change must not trigger planning recalculation.

## 9. Determinism and traceability

The same input snapshot, algorithm version and dependency state must reproduce the same deterministic result within the defined numerical tolerances.

Each correction and recalculation records:

- reason;
- affected dependency;
- previous current result reference;
- new result reference;
- algorithm version;
- input snapshot/version;
- resulting downstream invalidations.

This preserves development and operational traceability without treating historical variants as active alternatives.

## 10. Architectural consequence

This principle is mandatory for subsequent BlueSky PRO planning implementation.

The planning system shall prefer:

**one calculation → one authoritative result → downstream reuse → dependency-driven invalidation → selective recalculation**

over:

**independent module checks → duplicated calculations → repeated validation → full recomputation.**

This document is the current architectural reference for calculation reuse and process evolution.
