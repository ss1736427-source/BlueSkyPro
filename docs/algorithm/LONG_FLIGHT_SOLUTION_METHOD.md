# BlueSky PRO — LONG FLIGHT Solution Method

**Status:** ALGORITHM BASELINE  
**Date:** 2026-10-04  
**Role:** Template-specific solution method built on the authoritative Planning Kernel and Optimization Layer.

## 1. Purpose

LONG FLIGHT solves an A→B or multi-leg flight whose primary objective is to complete the required mission with maximum useful range/endurance while preserving the required reserve and all safety constraints.

The template does not mean simply “choose the shortest route”. It evaluates the complete mission state:

- operator task requirements;
- UAV performance;
- current battery state and degradation;
- wind and forecast uncertainty;
- terrain/obstacles/restrictions;
- altitude profile;
- payload;
- departure/arrival requirements;
- required reserve;
- optional intermediate tasks.

## 2. Operator task definition

The operator's preparation data is mandatory input to the Task Model.

Minimum inputs:

- start point;
- destination or required route;
- ETD, if specified;
- ETA/deadline, if specified;
- selected UAV or UAV group;
- payload;
- required altitude / altitude range;
- required energy reserve;
- mission priority;
- route preference;
- acceptable operational risk;
- optional intermediate targets/tasks;
- operator-defined restrictions;
- desired completion conditions.

The system must preserve these values throughout planning.

The operator may select:

- **ECONOMY** — minimum predicted energy;
- **FAST** — minimum feasible time;
- **BALANCED** — energy/time compromise;
- **PRIORITY** — deadline/arrival objective;
- **CUSTOM** — explicit operator weights.

## 3. Task Model

The formal model contains:

`LONG_FLIGHT = {START, DESTINATION, ETD, ETA/DEADLINE, UAV, PAYLOAD, ALTITUDE, RESERVE, PRIORITY, OBJECTIVE, OPTIONAL_TASKS}`

Derived data is kept separate:

`ENVIRONMENT = {NOTAM, AIRSPACE, TERRAIN, OBSTACLES, WIND, WEATHER}`

Vehicle state:

`UAV_STATE = {MASS, BATTERY_SOC, BATTERY_SOHealth, DEGRADATION, PERFORMANCE, ENDURANCE, PAYLOAD, NAVIGATION_CAPABILITY}`

The planner must not replace operator requirements with defaults when the operator supplied an explicit value.

## 4. Feasibility first

The Planning Kernel creates the feasible search space before optimization.

Hard gates:

1. airspace / NOTAM;
2. regulatory restrictions;
3. terrain and obstacle clearance;
4. UAV performance;
5. navigation/communication limits where applicable;
6. mandatory energy reserve;
7. weather/operational limits;
8. multi-UAV separation where applicable.

A route that fails a hard gate is not an optimization candidate.

## 5. Base route generation

The initial route is generated through the existing route engine.

The base route shall consider:

- legal airspace;
- restricted/forbidden zones;
- terrain;
- obstacles;
- altitude limits;
- UAV capabilities.

The base route is then segmented for energy and wind evaluation.

No optimization method is allowed to bypass this stage.

## 5A. Interactive mandatory passage points

The map and flight profile are interactive controls for mission definition.

The operator may place mandatory passage points on the map and/or directly on the flight profile. These points become hard route constraints in the LONG FLIGHT Task Model.

For each point the system stores:

- position;
- required altitude or altitude band, if specified;
- order, if specified;
- passage tolerance/corridor;
- optional dwell/observation requirement;
- operator-defined reason/type.

The route is therefore represented as ordered constrained legs:

START → MANDATORY_1 → MANDATORY_2 → … → DESTINATION

The planner may optimize each leg and the complete mission, but cannot delete, reorder, or bypass a mandatory point.

A point placed by the operator is subject to the same feasibility checks as every other route element. The system validates legality, terrain/obstacle clearance, altitude, UAV performance, energy/reserve and applicable multi-UAV constraints.

The map and profile are bidirectionally synchronized. Moving a point in either view updates the shared Task Model and invalidates the affected route/trajectory calculations until recalculation is complete.

For an infeasible mandatory point, the planner returns a structured failure and identifies the affected point/leg. It must not silently move the point or route around it.

## 6. Wind-aware directed graph

Long-flight routing uses a directed graph.

For each transition:

`Cost(A→B) != Cost(B→A)`

because wind changes:

- ground speed;
- required propulsion energy;
- flight time;
- reserve margin.

For each candidate segment calculate at minimum:

- distance;
- heading;
- wind vector;
- relative airspeed;
- ground speed;
- segment time;
- estimated energy;
- reserve consumption.

The planner should prefer routes that reduce headwind exposure when the additional distance does not violate the selected objective.

## 7. Energy model

Energy planning is based on the actual UAV profile and current battery state.

The model includes:

- battery usable capacity;
- current SOC;
- battery degradation coefficient;
- payload;
- expected power/energy consumption;
- airspeed;
- wind;
- climb/descent;
- temperature/environmental corrections when supported;
- reserve requirement.

The battery degradation coefficient is an explicit planning variable. A degraded battery must not be treated as a nominal new battery.

The output is:

`E_required = E_route + E_contingency + E_reserve`

Mission is feasible only if:

`E_available >= E_required`

The exact energy model is UAV-specific and remains outside the template.

## 8. Reserve policy

Reserve is a hard constraint.

The planner must distinguish:

- mission energy;
- contingency energy;
- mandatory landing reserve.

Operator-defined reserve overrides the template default when it is stricter.

If no safe feasible route exists with the required reserve, the planner must return **NOT FEASIBLE** and explain the limiting factor.

It must not silently reduce the reserve to obtain a route.

## 9. Route alternatives

For a long flight, the planner should generate a bounded set of feasible alternatives where practical:

- shortest feasible;
- lowest-energy feasible;
- wind-favorable;
- balanced;
- reserve-maximizing.

Each alternative is passed through the same safety gates.

The Optimization Layer may then rank or retain Pareto alternatives.

## 10. Optimization policy

LONG FLIGHT normally uses:

**Primary:** energy minimization  
**Secondary:** flight time  
**Secondary:** distance  
**Hard:** reserve and safety

Canonical objective:

`J = wE*Energy + wT*Time + wD*Distance + wR*Risk + wC*Complexity`

For ECONOMY, `wE` dominates.

For FAST, `wT` dominates but feasibility and reserve remain hard constraints.

For BALANCED, energy and time are weighted comparably.

The optimizer may use GA/ACO/SA where route complexity justifies it, but a deterministic feasible baseline must always exist before heuristic improvement.

## 11. Opportunistic tasks

If the operator permits intermediate tasks, the planner may test insertion into the long-flight route.

For each candidate:

- added distance;
- added time;
- added energy;
- risk/cost;
- task priority/value;
- impact on reserve.

A task is inserted only when the resulting mission remains feasible and the selected optimization policy accepts the trade-off.

P0 Mandatory tasks cannot be removed by optimization.

## 12. Replanning

LONG FLIGHT supports state-based replanning.

Replanning triggers include:

- significant wind deviation;
- unexpected energy consumption;
- battery degradation/state deviation;
- obstacle/restriction change;
- weather deterioration;
- UAV performance degradation;
- delayed departure;
- route blockage;
- multi-UAV conflict.

The replanner does not start from zero unnecessarily.

Preferred sequence:

`CURRENT_TRAJECTORY → UPDATED_STATE → LOCAL/ROUTE REPAIR → ENERGY CHECK → SAFETY RE-VERIFY`

A full global re-optimization is used when local repair cannot preserve feasibility or objective quality.

## 13. Multi-UAV long flight

For multiple UAVs, LONG FLIGHT may optimize:

- UAV assignment;
- departure staggering;
- task distribution;
- route selection;
- arrival synchronization.

Conflict resolution remains authoritative.

The existing BlueSky PRO TICAS rule and conflict-resolution mechanism are applied after candidate generation and again after optimization.

## 14. Outputs

The template produces:

- selected route;
- 4D trajectory;
- ETA/ETD;
- predicted flight time;
- predicted energy;
- predicted reserve at destination;
- battery/degradation assumptions;
- wind assumptions;
- safety/constraint status;
- optimization profile;
- alternative routes, if retained;
- reason for rejection when no feasible route exists.

## 15. Failure states

The planner must return a structured failure reason, including at minimum:

- NO_LEGAL_ROUTE;
- INSUFFICIENT_ENERGY;
- INSUFFICIENT_RESERVE;
- UAV_PERFORMANCE_LIMIT;
- WEATHER_LIMIT;
- OBSTACLE_OR_TERRAIN_LIMIT;
- AIRSPACE_RESTRICTION;
- MULTI_UAV_CONFLICT_UNRESOLVED.

A failure is not converted into a degraded “best effort” route when a hard constraint is violated.

## 16. Verification sequence

Before READY:

1. validate operator requirements;
2. validate environment data;
3. validate route legality;
4. validate terrain/obstacle clearance;
5. validate UAV performance;
6. calculate wind-aware energy;
7. validate reserve;
8. validate 4D trajectory;
9. validate multi-UAV separation;
10. run Optimization Layer;
11. repeat complete safety/constraint verification;
12. compile UAV-specific mission;
13. final release gate.

## 17. Relationship to Optimization Layer

LONG FLIGHT does not implement its own independent optimizer.

It supplies the Optimization Layer with:

- feasible candidate routes;
- directed transition costs;
- energy/time metrics;
- task priorities;
- operator objective;
- reserve constraints;
- UAV state.

The Optimization Layer returns candidate ranking/Pareto solutions.

The Planning Kernel remains authoritative.

## 18. Engineering rule

**Operator task → Task Model → deterministic feasible route → wind/energy evaluation → optimization → complete re-verification → UAV-specific mission**

The objective of LONG FLIGHT is not merely maximum distance. It is **maximum useful mission endurance within the declared task, reserve, safety and vehicle constraints**.

## 19. Template status

This document is the baseline solution method for the LONG FLIGHT mission template.

Subsequent implementation shall add:

- formal parameter schema;
- UAV-specific energy model interface;
- benchmark scenarios;
- verification cases;
- deterministic acceptance thresholds;
- performance measurements for the Optimization Layer.
