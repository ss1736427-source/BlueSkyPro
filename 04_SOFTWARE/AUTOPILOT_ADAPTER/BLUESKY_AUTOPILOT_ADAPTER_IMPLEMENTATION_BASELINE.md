# BlueSky PRO — Autopilot Adapter Implementation Baseline

**Priority:** P0  
**Status:** IMPLEMENTATION BASELINE  
**Scope:** ArduPilot / PX4 / OEM

## 1. Goal

Provide one executable integration boundary between the BlueSky core and any supported flight-control system (FCS).

BlueSky core shall operate only on normalized commands, state, capabilities and mission semantics. Protocol-specific details remain inside the adapter.

## 2. Runtime chain

```text
MISSION ENGINE / SAFETY / OPERATOR
              │
              ▼
       UNIVERSAL VEHICLE API
              │
              ▼
       AUTOPILOT ADAPTER
              │
      ┌───────┼────────┐
      ▼       ▼        ▼
   ArduPilot  PX4     OEM
      │       │        │
   MAVLink  MAVLink  Native/API
      └───────┼────────┘
              ▼
        FLIGHT CONTROLLER
              │
              ▼
        UAV / EQUIPMENT
```

## 3. Adapter lifecycle

```text
DISCOVER
   ↓
IDENTIFY
   ↓
AUTHENTICATE
   ↓
CONNECT
   ↓
CAPABILITY DISCOVERY
   ↓
VEHICLE PROFILE MATCH
   ↓
CONFIGURATION SYNC
   ↓
HEALTH / PRE-ARM STATE
   ↓
MISSION COMPILE
   ↓
UPLOAD
   ↓
READ-BACK
   ↓
CANONICAL VERIFY
   ↓
READY
   ↓
EXECUTE / MONITOR
   ↓
CONTINGENCY
   ↓
LAND / RTL / ABORT
   ↓
LOG ACQUISITION
   ↓
CLOSE / ARCHIVE
```

## 4. Mandatory normalized API

### Identity

- `getVehicleIdentity()`
- `getAutopilotIdentity()`
- `getFirmwareVersion()`
- `getProtocolVersion()`

### Capabilities

- `getCapabilities()`
- `getSupportedCommands()`
- `getSupportedMissionFeatures()`
- `getSupportedEquipmentFeatures()`

### Connection

- `connect()`
- `disconnect()`
- `getConnectionState()`
- `getLinkMetrics()`

### State

- `getNavigationState()`
- `getFlightMode()`
- `getHealthState()`
- `getFailsafeState()`
- `getEnergyState()`
- `getMissionState()`
- `getEquipmentState()`

### Commands

- `arm()`
- `disarm()`
- `takeoff()`
- `land()`
- `hold()`
- `resume()`
- `rtl()`
- `goto()`
- `startMission()`
- `pauseMission()`
- `abortMission()`

Every command returns a normalized result containing at minimum:

```text
command_id
vehicle_id
accepted / rejected
execution_state
acknowledgement
reason
source_timestamp
adapter_timestamp
```

### Mission

- `compileMission(BlueSkyMission, VehicleCapabilities)`
- `uploadMission()`
- `downloadMission()`
- `readBackMission()`
- `compareMission()`
- `startMission()`
- `pauseMission()`
- `resumeMission()`
- `abortMission()`

## 5. Mission compiler

The adapter shall NOT receive raw BlueSky internal objects and pass them directly to the autopilot.

```text
BlueSky Mission Model
        ↓
Semantic validation
        ↓
Vehicle capability check
        ↓
Adapter mission compiler
        ↓
Protocol-specific representation
        ↓
Upload
        ↓
Read-back
        ↓
Semantic comparison
```

Unsupported mission semantics are explicit errors. Silent degradation is prohibited for safety-critical semantics.

## 6. Parameter/configuration service

The adapter shall support a controlled configuration lifecycle:

```text
READ BASELINE
    ↓
COMPARE
    ↓
VALIDATE
    ↓
WRITE
    ↓
READ-BACK
    ↓
VERIFY
```

Safety-critical configuration changes shall invalidate the applicable readiness state until revalidated.

## 7. Autopilot authority

BlueSky is the mission-planning and supervisory layer. The onboard flight controller remains authoritative for real-time stabilization and onboard failsafe execution.

BlueSky shall never assume that loss of its own connection transfers flight-control authority to the ground system.

Authority states shall be represented explicitly, including where supported:

- autonomous mission;
- guided/manual-supervised operation;
- failsafe;
- return/recovery;
- emergency/abort;
- communication lost.

## 8. ArduPilot / PX4 reference adapter

The first common adapter shall target MAVLink-based ArduPilot and PX4 implementations.

The adapter shall isolate MAVLink message/service details from the normalized BlueSky API. At minimum it shall cover the required heartbeat/identity, command acknowledgement, mission transfer, parameters, telemetry/state, time synchronization and log-transfer functions supported by the selected implementation.

Protocol capabilities shall be discovered rather than assumed.

## 9. OEM adapter contract

OEM integrations use the same normalized API.

```text
OEM SDK / API / Protocol
          ↓
      OEM Adapter
          ↓
Universal Vehicle API
          ↓
BlueSky Core
```

OEM-specific limitations, unsupported functions and certification restrictions shall be declared in the adapter capability manifest.

## 10. Capability manifest

Each adapter shall expose a machine-readable capability manifest containing at least:

```text
adapter_id
vendor
protocol
protocol_version
fcs_name
fcs_version
vehicle_classes
supported_commands
supported_mission_features
supported_parameter_features
telemetry_features
equipment_features
failsafe_features
log_features
security_features
known_limitations
verification_status
```

## 11. Error model

Normalized adapter errors shall distinguish:

- NOT_CONNECTED
- AUTHENTICATION_FAILED
- UNSUPPORTED
- INCOMPATIBLE_VERSION
- CAPABILITY_MISSING
- INVALID_STATE
- SAFETY_REJECTED
- COMMAND_REJECTED
- TIMEOUT
- LINK_LOST
- PROTOCOL_ERROR
- MISSION_MISMATCH
- CONFIGURATION_MISMATCH
- INTERNAL_ADAPTER_ERROR

## 12. Acceptance test sequence

For each adapter, automated tests shall cover:

1. discovery;
2. identification;
3. authentication/session;
4. capability discovery;
5. profile matching;
6. configuration read-back;
7. health/pre-arm state;
8. mission compile;
9. mission upload;
10. mission read-back;
11. semantic comparison;
12. command ACK/result;
13. telemetry normalization;
14. link-loss behavior;
15. reconnect/resynchronization;
16. contingency command behavior;
17. log acquisition;
18. clean close.

## 13. Product readiness

An adapter is `PRODUCT_READY` only when:

```text
CONTRACT BASELINED
       ↓
IMPLEMENTED
       ↓
AUTOMATED TESTS PASS
       ↓
SIL PASS
       ↓
HIL PASS (where applicable)
       ↓
REPRESENTATIVE UAV PASS
       ↓
VERIFIED
```

Until then its status shall remain explicit and it shall not be presented to the operator as universally operational.

## 14. Anti-coupling rule

No BlueSky planning, optimization or HMI module may directly depend on ArduPilot, PX4, MAVLink or an OEM protocol type. Such dependencies are permitted only inside the corresponding adapter/protocol implementation boundary.
