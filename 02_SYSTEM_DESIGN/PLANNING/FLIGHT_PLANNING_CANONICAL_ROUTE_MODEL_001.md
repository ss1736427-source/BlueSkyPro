# BlueSky PRO — Canonical Route Model

**ID:** PLAN-DATA-001
**Status:** BASELINED
**Scope:** Phase B — Flight Planning Core

## Purpose

Define the canonical route representation used between mission planning, constraint validation, optimization, flight-profile calculation and later vehicle-specific compilation.

## Authority

The Route model is a deterministic planning-domain object. It does not authorize flight, change readiness, bypass safety, or command an aircraft.

## Relationship

`MISSION VERSION → ROUTE CANDIDATE(S) → VALIDATED ROUTE → FLIGHT PROFILE → VEHICLE-SPECIFIC COMPILATION`

A route is always traceable to the Mission Version that produced it.

## Multi-UAV mission allocation

A mission may contain multiple selected templates and multiple participating UAVs. The mission-planning/allocation layer produces explicit task-assignment records before vehicle-specific route compilation. Each assignment identifies the UAV, source template/task, role or spatial sector, and its route candidate/version. One template may produce assignments for several UAVs; the assignments must describe distinct work scopes where applicable.

The allocator handles routine initial distribution automatically. If it cannot produce a complete feasible allocation, it returns a structured exception for operator intervention. Routine allocation does not constitute flight authorization and does not bypass canonical route validation, restrictions, vehicle feasibility, safety gates, or required approvals.

Each participating UAV has its own route candidate and vehicle-specific planning context. The map presents the combined mission-wide set of routes; selecting a UAV filters/emphasizes its route in the table and flight profile without removing other routes from the map. UI-local preview data is not authoritative route or assignment storage.

## Minimum model

- immutable route identity and version;
- Mission ID and Mission Version;
- ordered waypoints;
- explicit target height above local ground (AGL) per waypoint;
- mandatory waypoint flag;
- route segments with source/target identity and geometry summary;
- hard route constraints;
- terrain/airspace/weather/wind snapshot references;
- generator and generator-version lineage;
- calculation-input version.

## Vertical reference and radio-altimeter semantics

The planned route height is defined as **AGL (height above the local ground surface)**. The existing `RouteWaypoint::altitude_m` field therefore represents a target AGL height in metres; it is not AMSL or ellipsoidal altitude.

The radio altimeter provides a measured AGL value relative to the surface directly below the aircraft during flight. It is feedback for monitoring/control and verification of the achieved height; it does not provide advance knowledge of the terrain ahead and must not be treated as the terrain database used for route planning.

Before flight, the planner uses a versioned terrain snapshot to estimate the ground surface along the route and check terrain-following feasibility. In flight, measured radio-altimeter data is compared with the target AGL and applicable limits. The actual control loop belongs to the aircraft/autopilot integration and its declared capabilities; BlueSky must not assume every vehicle has a radio altimeter or can accept direct height-control commands.

A route cannot be considered terrain/obstacle validated merely because target AGL values are present or a radio-altimeter measurement is available. Terrain coverage/freshness, obstacle data, vehicle capability, measurement validity and applicable clearance rules remain separate validation inputs. Missing or invalid required inputs must produce an explicit blocked/rejected result, not an implicit pass.

The `minimum_altitude_m` and `maximum_altitude_m` constraints bound target AGL height. `minimum_clearance_m` is a separate terrain/obstacle safety constraint to be verified by the relevant validators; it must not be silently equated with target AGL or with a radio-altimeter reading.

## Design rules

1. A route candidate may be generated before it is validated.
2. Mandatory constraints are feasibility conditions, not optimization weights.
3. External data is referenced by versioned snapshots; stale or missing inputs are not silently substituted.
4. A route change that materially changes geometry, target AGL height, constraints or planning inputs creates a new route version.
5. Wind remains an explicit environmental reference and may trigger recalculation or re-optimization.
6. Vehicle-specific executable semantics are created only after route validation and later compilation.
7. AI may propose route variants later, but the canonical Route object is accepted only through deterministic validation and the existing proposal/safety/authorization boundaries.

## Interactive profile synchronization

The flight profile, waypoint table and map are synchronized editing surfaces over the canonical mission/route data. Profile edits must update the canonical route candidate, trigger validation and the required recalculation, then publish a consistent result to all dependent views. UI-local state and display preferences are not authoritative storage for route constraints.

All route points use one sequential route numbering, from start to finish, shared by the table, map and profile. Mandatory status is an attribute of a route point, not a separate numbering system: a mandatory point retains its route number and is additionally marked as mandatory. A newly inserted mandatory point becomes a route node and receives its position in the same sequence.

See the approved [Interactive Flight Profile — Data Synchronization and Recalculation](../../02_SYSTEM/Design/Interface/Interactive%20Flight%20Profile%20%E2%80%94%20Data%20Synchronization%20and%20Recalculation.md) requirement.

## Next block

Implement deterministic Route Constraint Validation against the canonical Route model, followed by terrain/obstacle and airspace adapters.
