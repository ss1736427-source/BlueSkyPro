# BlueSky PRO — Constrained Open-Space Model

**ID:** PLAN-ARCH-002  
**Status:** BASELINED  
**Scope:** Universal route planning

## 1. Principle

BlueSky PRO resolves the current environment before route search and constructs routes only inside the resulting permitted/open spatial domain.

This is universal for mission, reconnaissance, survey, inspection, transit/repositioning, return/RTH, alternate-return, emergency/contingency, recovery and multi-UAV individual routes.

Route purpose changes the objective or generation parameters, not the spatial-feasibility architecture.

## 2. Environment input

The model consumes a versioned ConstrainedEnvironmentSnapshot. It may contain normalized restrictions from NOTAM, airspace restrictions, temporary restrictions, terrain/obstacles, altitude limits and other applicable hard spatial constraints.

Upstream adapters own source-specific normalization and freshness. A missing or incomplete environment is not interpreted as open space.

## 3. Spatial feasibility

Each candidate edge is evaluated against active restrictions whose altitude band overlaps the edge altitude.

Polygon restrictions use segment intersection; endpoint-only checks are insufficient. Circle restrictions use segment-to-center distance.

The spatial result contains allowed/rejected state, environment snapshot identity/version, calculation input version, dependency identity and blocking restriction identifiers.

## 4. Planning relationship

EXTERNAL SOURCES → NORMALIZED ENVIRONMENT SNAPSHOT → CONSTRAINED OPEN-SPACE MODEL → ROUTE SEARCH → ROUTE CANDIDATES

The model is a feasibility layer, not an authorization or execution layer. It does not mutate mission, readiness, safety, authorization or execution state.

## 5. Recalculation

Changes to restrictions or the environment snapshot invalidate only affected spatial results and downstream route candidates. Unchanged results remain reusable under PLAN-ARCH-001.

## 6. Boundary

The model must not grant authorization, change READY state, execute commands, silently treat unavailable restriction data as no restriction, or duplicate wind/performance/conflict calculations.

## 7. Current implementation

The deterministic core currently supports normalized polygon/circle restrictions and altitude bands. Source-specific NOTAM parsing and complete terrain/obstacle/airspace adapters remain upstream integration work.
