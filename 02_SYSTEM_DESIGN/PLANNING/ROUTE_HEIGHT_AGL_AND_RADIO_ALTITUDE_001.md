# BlueSky PRO — Route Height AGL and Radio-Altimeter Semantics

**ID:** PLAN-ALT-001  
**Status:** BASELINED  
**Scope:** Flight planning, terrain following, telemetry integration and route validation

## Requirement

The planned route height is specified relative to the local ground surface (AGL). During flight, the radio altimeter measures the actual height between the aircraft and the surface directly below it.

## Separate data concepts

1. **Target AGL height** — the route/profile setpoint used by planning and mission definition.
2. **Measured AGL height** — a time-stamped radio-altimeter measurement received through the vehicle/autopilot telemetry adapter.
3. **Terrain elevation/profile** — versioned terrain data used to predict the ground surface along future route segments before the aircraft reaches them.
4. **Terrain/obstacle clearance** — a safety condition assessed against terrain and obstacle geometry and applicable limits; it is not synonymous with target AGL.

## Planning and execution boundary

The planner uses terrain data and route geometry to assess whether the target AGL profile can be followed over the planned route. The radio altimeter supplies in-flight feedback and evidence of achieved height, but cannot replace terrain data ahead of the aircraft.

The flight-control system/autopilot remains responsible for direct vehicle control. BlueSky may request or supervise height targets only through a declared, verified vehicle capability and an approved interface. The system must not assume all UAVs carry a radio altimeter or support direct AGL control.

## Data quality and failure handling

- Radio-altimeter measurements must carry source identity, timestamp, validity/health state and units through the telemetry adapter.
- Stale, invalid, unavailable or capability-incompatible measurements must not be interpreted as a valid height.
- The vehicle-specific safety/contingency policy defines the response to loss or degradation of height measurement; the planning UI must report the condition and must not silently mark the mission ready.
- Missing, stale or insufficient terrain data for a route requiring terrain following blocks the relevant terrain validation.
- Structural route validation alone does not establish terrain clearance or flight readiness.

## Verification boundary

Tests shall distinguish target AGL from measured AGL and terrain elevation, verify stale/invalid measurement handling, and confirm that a structural validator pass cannot bypass terrain/obstacle validation or release gates.
