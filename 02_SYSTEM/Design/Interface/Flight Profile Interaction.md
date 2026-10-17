# Flight Profile Interaction

This document describes the baseline interaction between the flight profile, route, map, waypoint table, and mission state.

The approved requirement **Interactive Flight Profile — Data Synchronization and Recalculation** defines the controlling requirements. If this supporting document conflicts with that requirement, the approved requirement takes precedence.

## Flight Profile Display

The interface displays the calculated flight profile for each UAV, taking account of its route and assigned task.

The profile is associated with the corresponding UAV, its waypoints, and the applicable time parameters.

## Route Profile

For the selected UAV, the interface displays the principal route parameters:

- Departure;
- first working waypoint;
- intermediate working waypoints;
- last working waypoint;
- Destination;
- altitude;
- speed;
- course;
- ETO;
- EET;
- ETA;
- Trip Time;
- Total Time.

The values must correspond to the selected route and its current calculation result. Preview values must not be represented as validated planning output.

## 2D / 3D Interaction

The operator can inspect the route in 2D and 3D representations.

- The 2D representation supports route inspection on the map.
- The 3D representation supports inspection of spatial route geometry and altitude changes along the route.

The map and profile are representations of the same route state; they must not maintain independent authoritative geometry.

## Waypoint Interaction

Waypoints may be edited directly on the active map.

The operator can select a waypoint and drag it to the required position. When the drag ends, the proposed position is captured as a route edit candidate.

Waypoint altitude can be edited in the flight profile. The altitude change applies to the corresponding route waypoint, not to an unrelated point.

A route edit is not accepted merely because the operator changed its geometry. It must pass the required validation and recalculation sequence before being shown as validated.

## Automatic Revalidation

After a change to route inputs or a waypoint, the system initiates route revalidation.

Changes to waypoint position or altitude trigger revalidation and, when required, recalculation. The affected route must be checked against applicable:

- route constraints and restrictions;
- task requirements;
- UAV performance limits;
- time-related parameters;
- conflicts with routes assigned to other UAVs.

The interface must distinguish an edited candidate from the last valid route and must not silently present stale calculation results as current.

## Table Synchronization

Route changes made on the map are reflected in the waypoint table.

Changes to supported route parameters made in the table are reflected in the corresponding map and profile representations.

The map, waypoint table, and flight profile must display the same current route-point identity, sequence, position, and altitude. Calculated values must refer to the same calculation result and version.

## Profile and Mission State

The profile reflects the current mission and selected-UAV context.

Changes to route geometry, applicable telemetry, battery state, or mission execution state update the corresponding displayed values when the relevant data source is connected and valid. Missing or stale inputs must be identified rather than silently substituted.

## Pilot Control

The interface allows the operator to edit supported parameters of the working route.

An edit must not directly force mission readiness, bypass validation, or authorize execution. Each operational change enters the controlled candidate, validation, and recalculation flow defined by the approved requirement.

## Archive Protection

Editing the working route or profile must not overwrite previously archived mission or route versions.

When an edited candidate is accepted under the canonical route-versioning rules, the accepted result is retained as a traceable version for subsequent analysis and audit.