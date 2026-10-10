# BlueSky PRO — Constraint Visualization on Flight Chart

**ID:** PLAN-HMI-001  
**Status:** BASELINED  
**Scope:** Flight Planning / Map-Centric HMI

## 1. Principle

The Flight Chart is the primary planning instrument. The current constrained environment must be visually represented on the map before and during route planning.

The visualization is a presentation of the same versioned environment used by the Constrained Open-Space Model. It must not maintain an independent restriction dataset or perform a second spatial calculation.

## 2. What is displayed

As applicable, the Flight Chart shall visualize:
- NOTAM-defined temporary restrictions;
- prohibited/restricted/danger areas;
- controlled airspace and other relevant airspace boundaries;
- terrain and obstacle information;
- altitude-dependent restrictions;
- temporary or time-dependent restrictions;
- other hard spatial constraints used by planning.

The exact visual encoding is configurable by layer, but the underlying restriction identity and source/version remain traceable.

## 3. Single source of truth

The flow is:

ENVIRONMENT SOURCES
→ NORMALIZED ENVIRONMENT SNAPSHOT
→ CONSTRAINED OPEN-SPACE MODEL
→ ROUTE SEARCH
→ FLIGHT CHART VISUALIZATION

The Flight Chart consumes the same normalized snapshot. It does not independently re-parse NOTAM or rebuild restriction geometry.

## 4. Visual state

The map should distinguish, at minimum:
- active blocking restriction;
- active non-blocking/relevant airspace information;
- altitude-dependent restriction;
- time-dependent restriction;
- stale/uncertain/unavailable source state;
- selected/inspected restriction.

A restriction inspection action should expose its identity, source, validity interval, altitude limits, snapshot/version and applicability information.

## 5. Route interaction

The operator can see the spatial relationship between:
- planned route;
- open space;
- restrictions;
- terrain/obstacles;
- mandatory points;
- launch/recovery points.

Route construction remains constrained by the planning core. Map visualization does not authorize a route and does not override constraints.

## 6. Version consistency

The displayed environment snapshot and the planning calculation input version must be identifiable. If the map snapshot becomes stale relative to the planning state, the UI must indicate the state rather than silently displaying an apparently current restriction picture.

## 7. Performance and usability

Visualization is presentation-only and may use rendering simplification, tiling, level-of-detail or cached geometry. Such optimization must not alter the authoritative spatial model used for planning.

The map remains the main workspace; side panels and overlays must not obscure the operator's view of the relevant restricted/open space.

## 8. Boundary

The visualization layer:
- does not grant authorization;
- does not change readiness;
- does not execute flight commands;
- does not replace deterministic spatial planning;
- does not create a second restriction calculation.

Its purpose is immediate spatial awareness and traceable operator interaction with the current planning environment.
