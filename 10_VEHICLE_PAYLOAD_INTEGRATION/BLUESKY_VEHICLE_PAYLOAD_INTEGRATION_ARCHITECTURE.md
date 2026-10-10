# BlueSky PRO — Vehicle / Payload Integration Architecture

**Status:** ACCEPTED — architecture baseline

## 1. Purpose

BlueSky PRO shall be able to operate with a heterogeneous fleet without rebuilding the mission-planning core for each UAV, autopilot, payload or equipment combination.

The integration layer converts the real capabilities and current state of each vehicle and payload into a common BlueSky model used by planning, Algorithm Orchestration, validation and execution.

Core principle:

> The user selects the task and the available fleet. BlueSky determines how the task should be executed using the actual capabilities, limitations, energy state and payload configuration of each vehicle.

## 2. Separation of responsibilities

```text
                         BLUESKY CORE
                              │
                COMMON VEHICLE / PAYLOAD MODEL
                              │
                 INTEGRATION CAPABILITY LAYER
                              │
        ┌────────────────────┼────────────────────┐
        ↓                    ↓                    ↓
   VEHICLE ADAPTER      PAYLOAD ADAPTER      AUTOPILOT ADAPTER
        │                    │                    │
     UAV model          camera/gimbal/etc.    ArduPilot/PX4/OEM
```

The BlueSky core must not contain manufacturer-specific assumptions. Manufacturer-specific protocol, parameter mapping and capability quirks belong in adapters and profiles.

## 3. Vehicle profile

Every supported UAV shall have a machine-readable profile containing, as applicable:

- aircraft type and configuration;
- dimensions and mass limits;
- propulsion characteristics;
- motor/engine characteristics and resource state;
- propeller/rotor configuration;
- aerodynamic/performance model;
- cruise, climb, descent and maximum operating speeds;
- operating altitude limits;
- range/endurance model;
- battery/fuel model;
- degradation coefficients;
- payload capacity and configuration;
- navigation and positioning capabilities;
- C2 capabilities and supported links;
- autopilot family/version;
- supported mission/command capabilities;
- failsafe capabilities;
- environmental operating limits.

The profile is not merely descriptive: relevant parameters become inputs to feasibility and optimization calculations.

## 4. Battery and energy model

Energy availability is a first-class planning constraint.

BlueSky shall distinguish:

- nominal battery capacity;
- current state of charge;
- usable capacity;
- battery health/degradation;
- temperature effects where modelled;
- predicted consumption;
- reserve requirement;
- uncertainty margin.

A mission is not considered feasible solely because nominal energy is sufficient.

Conceptual gate:

```text
AVAILABLE ENERGY
        ↓
MISSION ENERGY
 + CONTINGENCY
 + REQUIRED RESERVE
        ↓
      GATE
   PASS / REJECT
```

The reserve policy shall be explicit, configurable and traceable to the applicable operational/safety requirements. The optimizer may prefer a larger reserve, but may not violate the minimum required reserve.

## 5. Propulsion / engine resource

BlueSky shall model resource consumption where reliable data are available.

Relevant factors may include:

- engine/motor operating time;
- operating regime;
- thermal/load limits;
- expected wear/resource state;
- propeller/rotor configuration;
- maintenance/resource restrictions.

Resource optimization is secondary to safety and required energy reserve, but may influence selection among otherwise feasible plans.

## 6. Payload model

Payloads are independent capability objects associated with a vehicle configuration.

Examples include:

- RGB camera;
- thermal camera;
- multispectral/hyperspectral sensor;
- LiDAR;
- video system;
- gimbal;
- delivery mechanism;
- other mission equipment.

Each payload profile may define:

- mass and dimensions;
- power consumption;
- operating modes;
- field of view;
- resolution/GSD requirements;
- minimum/maximum operating altitude;
- required speed range;
- pointing/stabilization requirements;
- image/video capture constraints;
- trigger capabilities;
- storage/data-rate requirements;
- environmental limits;
- supported control protocol;
- dependencies on companion computer or autopilot component.

## 7. Mission capability matching

Before route optimization BlueSky determines whether a vehicle/payload combination is capable of performing the requested task.

```text
MISSION REQUIREMENTS
        ↓
CAPABILITY MATCHING
        ↓
┌──────────────┬───────────────┐
│ CAPABLE      │ NOT CAPABLE   │
└──────┬───────┴───────────────┘
       ↓
 eligible UAVs
       ↓
 Algorithm Orchestrator
```

Unsupported or partially supported capabilities must be visible and must never be silently assumed.

## 8. Configuration integrity

A UAV profile shall correspond to an actual configuration, not only to a generic aircraft type.

Changes such as:

- different payload;
- different battery;
- different propeller;
- equipment installation;
- firmware/autopilot change;
- mass/center-of-gravity change;

may invalidate or alter the performance model and therefore trigger recalculation or validation.

## 9. Autopilot capability discovery

The integration layer shall determine the actual capabilities of the connected autopilot rather than assuming that the vehicle supports every BlueSky function.

```text
CONNECT UAV
    ↓
IDENTIFY AUTOPILOT
    ↓
CAPABILITY DISCOVERY
    ↓
SUPPORTED / PARTIAL / UNSUPPORTED
    ↓
CREATE OPERATIONAL CAPABILITY SET
```

The same BlueSky mission model may therefore be executed through different adapters while preserving a common planning architecture.

## 10. Payload integration boundary

Payload control shall be separated from flight-control logic.

```text
                 BLUE SKY
                    │
              MISSION STATE
             ┌──────┴──────┐
             ↓             ↓
        FLIGHT CONTROL   PAYLOAD CONTROL
             ↓             ↓
         AUTOPILOT       PAYLOAD ADAPTER
                            ↓
                    CAMERA / GIMBAL /
                    SENSOR / ACTUATOR
```

BlueSky may coordinate payload actions with flight events, but the integration layer must handle protocol-specific implementation.

## 11. Mission quality is payload-dependent

Mission quality requirements must enter planning through the payload/task model.

For imaging missions, for example, relevant constraints can include:

- required ground sampling distance;
- overlap;
- camera orientation;
- exposure/trigger conditions where available;
- image acquisition geometry;
- speed and altitude constraints.

For 3D reconstruction, the model may additionally require suitable observation geometry and multiple viewpoints.

For delivery missions, payload mass, delivery mechanism state, release conditions and post-release energy requirements become relevant.

Therefore the optimizer must not treat all missions as shortest-path problems.

## 12. Algorithm Orchestration input

The Vehicle/Payload Integration layer supplies the Algorithm Orchestrator with a normalized capability set:

```text
TASK
  + VEHICLE CAPABILITIES
  + PAYLOAD CAPABILITIES
  + CURRENT STATE
  + ENERGY STATE
  + C2 STATE
  + ENVIRONMENT
  + REGULATORY CONSTRAINTS
        ↓
ALGORITHM ORCHESTRATOR
        ↓
SELECT / COMBINE ALGORITHMS
```

Different UAVs in the same mission may therefore receive different planning methods or optimization strategies when their dynamics, payloads, energy characteristics or operating constraints differ.

## 13. Runtime state versus static profile

BlueSky shall distinguish between:

**Static/configuration data:** vehicle type, propulsion, payload, nominal limits, protocol capabilities.

**Dynamic state:** position, velocity, attitude, battery state, temperature where available, C2 quality, payload state, current mission item, active faults and environmental information.

Planning shall use the current validated state rather than relying only on the nominal profile.

## 14. Data confidence

Every critical input used for planning should have a known source/state, such as:

- configured;
- measured;
- received from vehicle;
- received from payload;
- calculated;
- estimated;
- stale;
- unavailable.

Critical unknowns must not silently become valid numerical assumptions.

## 15. Universal fleet principle

A customer should be able to connect BlueSky to an existing fleet and receive a usable operational model through configuration and adapters rather than redesigning the planning core.

The architecture therefore supports:

```text
                  BLUESKY
                     │
        ┌────────────┼────────────┐
        ↓            ↓            ↓
       UAV          UAV          UAV
     ArduPilot       PX4          OEM
        │            │            │
     payload       payload      payload
        │            │            │
     battery       battery      battery
        │            │            │
    propulsion    propulsion   propulsion
```

## 16. Validation before use

A new or changed vehicle/payload configuration must pass the applicable validation process before being marked operationally ready.

Validation shall cover, as applicable:

1. profile consistency;
2. autopilot capability mapping;
3. payload command/control;
4. telemetry/state mapping;
5. energy model sanity;
6. performance model sanity;
7. mission upload/execution compatibility;
8. failsafe interaction;
9. simulation/SITL/HIL;
10. bench and real-UAV verification.

## 17. Design rule

> **BlueSky shall adapt to the user's fleet, not require the user's fleet to adapt to BlueSky.**

The vehicle and payload integration layer is therefore a mandatory product layer and a prerequisite for universal autopilot and fleet interoperability.
