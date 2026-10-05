# BlueSky PRO — Fleet / Vehicle / Equipment Model

**Status:** WORKING BASELINE
**Purpose:** Freeze the product-level model before implementing universal adapters.

## 1. Principle

BlueSky PRO is fleet-agnostic. A customer selects and operates a fleet; BlueSky supplies the integration profile, capability mapping, configuration baseline and verification status required for supported operation.

## 2. Hierarchy

```text
Customer / Operator
       ↓
Fleet
       ↓
Vehicle
       ├── Flight Control System / Autopilot
       ├── Navigation
       ├── C2 interfaces
       ├── Sensors
       ├── Energy / propulsion
       ├── Equipment
       └── Safety configuration
```

## 3. Fleet object

A Fleet groups vehicles under a common operational owner and configuration policy. It contains:

- fleet identity;
- operator/organization;
- vehicles;
- approved vehicle profiles;
- common operational policies;
- communication resources;
- equipment catalogue;
- authorization/airspace context;
- maintenance/resource records;
- software/configuration baselines.

## 4. Vehicle object

A Vehicle is a unique operational aircraft instance, not merely a model.

Required identity:

- vehicle ID;
- manufacturer;
- model;
- serial/registration identifiers as applicable;
- vehicle type/configuration;
- FCS/autopilot;
- firmware version;
- configuration baseline;
- current operational state;
- capability set;
- health/readiness state;
- C2 state;
- installed equipment;
- maintenance/resource state;
- verification/approval status.

## 5. Vehicle profile vs vehicle instance

```text
VEHICLE PROFILE
manufacturer + model + FCS + supported equipment
+ limits + capabilities + verified integration
             │
             ├── UAV-001
             ├── UAV-002
             └── UAV-003
```

The profile describes what a configuration is expected to support. The instance records the actual aircraft, firmware, parameters, installed equipment and current status.

## 6. Capability model

Capabilities shall be explicit and machine-readable. Examples:

- mission upload;
- mission modification;
- takeoff/land;
- RTL;
- guided/goto;
- geofence;
- rally points;
- failsafe configuration;
- parameter read/write;
- log download;
- camera capture/recording;
- gimbal control;
- thermal/IR control;
- LiDAR operation;
- RTK/PPK;
- offboard/external control where supported;
- traffic information;
- redundant C2.

Each capability has at least:

`SUPPORTED | NOT_SUPPORTED | VERIFIED | NOT_VERIFIED | DEGRADED`

## 7. Readiness is separate from capability

```text
CAPABILITY
Can this configuration perform the function?
        ↓
READINESS
Can it safely perform it now?
        ↓
AUTHORIZATION
Is it permitted to perform it now?
```

These states must never be collapsed into one boolean.

## 8. Equipment model

An Equipment item is an equipment instance installed on or associated with a vehicle.

Required information:

- equipment ID;
- manufacturer/model;
- serial/identity;
- equipment class;
- interface/protocol;
- capabilities;
- configuration/calibration;
- power/state;
- telemetry;
- command set;
- recording/data outputs;
- vehicle compatibility;
- verification status.

## 9. Equipment semantic interface

BlueSky mission logic shall use normalized semantic actions:

`CAPTURE`, `START_RECORDING`, `STOP_RECORDING`, `SET_CAMERA_MODE`, `SET_SENSOR_MODE`, `SET_GIMBAL`, `TRIGGER_EQUIPMENT`.

The equipment adapter translates these into the concrete device protocol.

## 10. Performance model

Vehicle profiles shall provide the parameters required by flight planning and energy calculations, including where applicable:

- mass;
- equipment mass;
- airspeed limits;
- climb/descent limits;
- endurance/energy model;
- propulsion characteristics;
- battery capacity and degradation;
- aerodynamic configuration;
- installed-equipment effects;
- environmental operating limits.

Operational calculations shall use the verified profile/instance values and record the configuration version used for the calculation.

## 11. Configuration baseline

A baseline binds the actual vehicle to:

`Vehicle + FCS + Firmware + Parameters + Sensors + Equipment + C2 + Safety + Software compatibility`.

Any change capable of affecting mission execution or safety shall create a new configuration version and invalidate affected verification status until revalidated.

## 12. Discovery and onboarding

```text
ADD / DISCOVER VEHICLE
        ↓
IDENTIFY VEHICLE + FCS
        ↓
MATCH PROFILE
        ↓
CHECK FIRMWARE
        ↓
READ CONFIGURATION
        ↓
DETECT EQUIPMENT / COMPONENTS
        ↓
CAPABILITY DISCOVERY
        ↓
BASELINE / COMPATIBILITY CHECK
        ↓
VERIFICATION STATUS
        ↓
READY FOR FLEET
```

## 13. Multi-UAV

The fleet model shall support heterogeneous fleets and coordinated missions. A mission may contain different vehicle profiles, but each vehicle receives a capability-validated execution plan and retains its own vehicle identity, state, regulatory relationship and Flight Record.

## 14. Adapter independence

Vehicle profiles and instances shall not contain protocol-specific mission logic. Protocol translation belongs to integration adapters. This allows new autopilots, aircraft and equipment to be added without redesigning the BlueSky operational model.

## 15. Gate

**Phase 3 is complete only when the data model can represent a heterogeneous customer fleet, its actual aircraft instances, installed equipment, capabilities, configuration baselines, readiness and verification state without introducing manufacturer-specific objects into the core BlueSky mission model.**
