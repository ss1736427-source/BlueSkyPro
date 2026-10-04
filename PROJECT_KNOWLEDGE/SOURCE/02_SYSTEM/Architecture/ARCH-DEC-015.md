---
id: ARCH-DEC-015
type: architecture_decision
title: Map Terrain and Spatial Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-015 — Map / Terrain / Spatial Architecture

## Decision

BlueSky shall provide a common spatial architecture for maps, terrain, airspace, geofences, mission geometry, route planning and spatial validation.

Spatial information shall be available to Mission Graph, Route Planner, Mission Validation, Safety Engine, Mission AI, Simulation and QML.

## Spatial Architecture

The primary spatial flow shall be:

MAP
→ TERRAIN
→ AIRSPACE / GEOFENCE
→ SPATIAL MODEL
→ MISSION GRAPH
→ ROUTE PLANNER
→ VALIDATION / SAFETY

## Map Layer

The map layer shall provide the spatial reference required for:

- mission planning;
- route visualization;
- UAV position;
- fleet visualization;
- geofence visualization;
- airspace visualization;
- terrain visualization;
- conflict visualization;
- simulation visualization.

The map implementation shall remain replaceable through an appropriate abstraction.

## Terrain

Terrain data shall support applicable:

- elevation;
- terrain surface;
- obstacles;
- restricted terrain;
- terrain clearance;
- terrain-dependent route constraints.

Terrain information shall be available to route planning and validation.

## Airspace

The spatial architecture shall support representation of applicable airspace constraints.

Airspace information may include:

- restricted areas;
- controlled areas;
- altitude restrictions;
- temporary restrictions;
- operational corridors;
- other applicable spatial constraints.

## Geofence

Geofences shall be represented as explicit spatial constraints.

The system shall support:

- inclusion zones;
- exclusion zones;
- altitude limits;
- time-dependent restrictions where applicable;
- dynamic geofence changes.

Geofence constraints shall be available to Mission Validation and Safety Engine.

## Spatial Coordinates

The architecture shall support spatial representation using:

- X;
- Y;
- Z;
- Time.

X and Y represent horizontal position.

Z represents altitude or vertical position.

Time represents the temporal validity of the spatial state.

## Four-Dimensional Spatial Model

The system shall support four-dimensional mission analysis:

X + Y + Z + TIME

This model shall be used where required for:

- route conflicts;
- UAV separation;
- airspace conflicts;
- temporal constraints;
- fleet coordination;
- dynamic validation.

## Mission Graph Integration

Mission Graph nodes and transitions may contain spatial and temporal context.

Applicable graph elements may include:

- position;
- altitude;
- area;
- route segment;
- timing;
- spatial condition;
- geofence condition;
- terrain condition;
- airspace condition.

Mission Graph behaviour shall remain connected to the spatial model.

## Route Planner Integration

Route Planner shall consume the common spatial model.

Route planning shall consider applicable:

- terrain;
- airspace;
- geofence;
- altitude;
- timing;
- UAV capabilities;
- payload requirements;
- communication;
- energy;
- fleet coordination;
- optimization profile.

## Spatial Validation

Mission Validation shall validate spatial constraints before execution.

Spatial validation shall consider applicable:

- X;
- Y;
- Z;
- Time;
- terrain;
- airspace;
- geofence;
- route constraints;
- separation requirements.

## Conflict Detection

The spatial architecture shall support detection of conflicts including:

- horizontal intersection;
- vertical separation violation;
- temporal conflict;
- route segment conflict;
- airspace conflict;
- geofence conflict;
- terrain conflict;
- fleet separation conflict.

A spatial conflict may be passed to Conflict Resolution.

## Conflict Resolution Integration

Conflict Resolution may generate alternatives involving:

- route;
- route segment;
- altitude;
- time;
- UAV role;
- resource allocation.

Any selected alternative shall be revalidated.

## Safety Integration

Safety Engine shall be able to consume spatial information relevant to safety decisions.

Safety-critical spatial constraints shall not be bypassed by:

- Mission AI;
- Route Planner;
- Mission Optimization;
- QML;
- Simulation.

## Mission AI Integration

Mission AI may use spatial information for:

- route planning;
- sector planning;
- fleet allocation;
- conflict prediction;
- conflict resolution;
- optimization;
- adaptation.

Mission AI recommendations remain subordinate to spatial safety constraints.

## Simulation Integration

Simulation / Digital Twin shall use the same logical spatial model as real mission execution.

Simulation shall be able to model:

- terrain;
- airspace;
- geofence;
- routes;
- altitude;
- timing;
- fleet separation;
- spatial conflicts.

The same mission geometry shall be usable in:

PLANNED
→ SIMULATED
→ ACTUAL

analysis.

## Dynamic Spatial Changes

The architecture shall support changes during mission execution, including:

- new geofence;
- airspace restriction;
- terrain or obstacle information update;
- route restriction;
- UAV position change;
- fleet position change;
- temporal restriction.

Material changes may trigger:

EXECUTING
→ REVALIDATE
→ ADAPT

or:

EXECUTING
→ REQUIRES_REPLAN

according to applicable safety and mission rules.

## Spatial Data Sources

Spatial information may originate from:

- map providers;
- terrain databases;
- airspace sources;
- geofence systems;
- UAV telemetry;
- C2 systems;
- mission configuration;
- Simulation / Digital Twin;
- approved external systems.

External spatial data shall enter the Core through defined interfaces.

## QML Integration

QML shall use the spatial model for presentation of:

- maps;
- routes;
- UAV positions;
- fleet positions;
- terrain;
- airspace;
- geofences;
- conflicts;
- mission graph spatial elements.

QML shall not independently determine authoritative safety or spatial validation results.

## Spatial Versioning

Material changes to spatial data relevant to a mission shall be associated with:

- data source;
- timestamp;
- mission revision;
- applicable spatial dataset version;
- resulting validation state.

## Traceability

Spatial decisions and validation results shall remain traceable to:

- mission revision;
- spatial dataset;
- terrain data;
- airspace data;
- geofence data;
- route;
- altitude;
- time;
- detected conflict;
- validation result;
- safety decision;
- selected resolution.

## Rationale

A common spatial model prevents separate and inconsistent representations of map, terrain, airspace, geofence and mission geometry. It allows the same spatial information to support planning, validation, simulation, execution monitoring and safety decisions.


