# IF-AUTOPILOT — Universal Vehicle / Autopilot Interface

**Priority:** P0  
**Status:** BASELINED — implementation contract

## 1. Purpose

Provide a stable BlueSky interface to heterogeneous onboard flight-control systems without coupling the core mission model to one autopilot implementation.

## 2. External integration classes

- ArduPilot;
- PX4;
- OEM/proprietary FCS through documented and legally usable interfaces;
- future FCS through the same adapter contract.

## 3. Layer boundary

```text
BlueSky Mission / Safety / Runtime
              ↓
      Universal Vehicle API
              ↓
       ADAPTER REGISTRY
              ↓
       Autopilot Adapter
              ↓
 MAVLink / OEM protocol / SDK
              ↓
          Autopilot
              ↓
        Flight Controller
```

The canonical BlueSky model is authoritative for mission semantics. The autopilot remains authoritative for aircraft-level flight-control state and onboard safety behavior.

The registry resolves the appropriate adapter by controlled adapter/vehicle/equipment compatibility data; it does not authorize execution.

## 4. Connection lifecycle

```text
DISCONNECTED
 ↓
CONNECTING
 ↓
IDENTIFYING
 ↓
CAPABILITY_DISCOVERY
 ↓
CONFIGURATION_SYNC
 ↓
HEALTH_CHECK
 ↓
READY / DEGRADED / REJECTED
```

## 5. Required normalized state

The adapter shall expose, where supported:

- position;
- altitude;
- velocity/speed;
- attitude;
- navigation/GNSS quality;
- battery/energy;
- propulsion/health;
- flight mode;
- mission state/progress;
- failsafe state;
- C2/link state;
- Equipment state;
- timestamp and source quality.

Unsupported fields shall be explicitly marked `UNSUPPORTED` or `UNKNOWN`; they shall never be fabricated.

## 6. Required operations

- connect/disconnect;
- discover/identify;
- capability discovery;
- configuration snapshot/read/write/synchronize/compare;
- arm/disarm;
- takeoff/land;
- hold/pause/resume;
- RTL/return;
- guided/goto where supported;
- mission upload/download/start/pause/resume/abort where supported;
- parameter operations;
- safety/geofence/rally configuration where supported;
- log acquisition;
- time synchronization.

Every operation is capability-gated. Unsupported operations are reported explicitly.

## 7. Command contract

Every command shall contain:

- command ID;
- mission/flight correlation ID where applicable;
- target vehicle/component;
- command type and parameters;
- authorization context;
- precondition/result state;
- sequence/correlation data;
- timestamp.

The adapter shall return normalized states such as `ACCEPTED`, `EXECUTING`, `COMPLETED`, `REJECTED`, `TIMEOUT`, `FAILED`, `CANCELLED`, or `UNKNOWN` as applicable.

## 8. Mission transfer contract

```text
Approved Mission Version
          ↓
Capability / compatibility gate
          ↓
Vehicle-specific mission translator
          ↓
External mission representation
          ↓
UPLOAD
          ↓
ACK / transfer result
          ↓
READ-BACK
          ↓
Semantic verification
          ↓
MISSION VERIFIED
```

Transport success alone never establishes mission validity.

The read-back verification shall confirm the approved mission identity/version and all safety- and task-critical semantics representable by the target autopilot.

## 9. Configuration contract

Configuration operations shall support:

`DISCOVER → SNAPSHOT → COMPARE → VALIDATE → APPLY → READ-BACK → VERIFY → BASELINE`

Where rollback is technically supported:

`BASELINE → CHANGE → FAILURE → ROLLBACK → VERIFY`

Safety-critical configuration changes invalidate affected readiness/verification state until revalidated.

## 10. Safety reconciliation

BlueSky safety constraints and autopilot safety mechanisms shall be compared before release.

At minimum, where applicable:

- geofence;
- rally/return points;
- failsafe actions;
- flight-mode restrictions;
- minimum/critical battery behavior;
- navigation-loss behavior;
- C2-loss behavior.

BlueSky shall not silently override onboard safety behavior.

## 11. Manual authority

The integration shall explicitly model control authority between:

- automated mission execution;
- BlueSky supervisory commands;
- RC/manual control;
- onboard autopilot failsafe behavior.

Authority conflicts shall have deterministic precedence and audit records.

## 12. Reference protocol

MAVLink is the first reference protocol for ArduPilot/PX4 integration. The implementation shall use its applicable microservices rather than treating MAVLink as one undifferentiated interface.

The adapter boundary must remain protocol-neutral so additional OEM integrations do not modify BlueSky mission semantics.

## 13. Verification

Adapter verification shall cover:

- connect/disconnect;
- identification;
- capability discovery;
- configuration synchronization;
- pre-arm/health mapping;
- command ACK/result/error handling;
- mission upload/read-back/semantic verification;
- telemetry normalization;
- link loss/recovery;
- safety reconciliation;
- manual override/authority arbitration;
- log acquisition;
- version compatibility.

## 14. Equipment integration boundary

Equipment is a first-class canonical BlueSky object and is integrated through the universal Vehicle/Equipment adapter boundary.

```text
External Vehicle / Equipment representation
                    ↓
              Adapter mapping
                    ↓
Canonical Vehicle / Equipment Schema
                    ↓
             Core services
```

The detailed canonical Equipment model and adapter obligations are defined in:

```text
03_FLEET/BLUESKY_CANONICAL_VEHICLE_EQUIPMENT_SCHEMA_001.md
03_FLEET/BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001.md
03_FLEET/BLUESKY-ADAPTER-CONFORMANCE-MATRIX-001.md
03_FLEET/BLUESKY-ADAPTER-REGISTRY-BOUNDARY-001.md
```

If an external protocol uses legacy terminology such as `payload`, it is treated only as an external protocol term and normalized to `Equipment` at the BlueSky boundary.

## 15. Acceptance criterion

A representative supported aircraft can complete:

`CONNECT → IDENTIFY → CAPABILITY → CONFIG → HEALTH → MISSION UPLOAD → READ-BACK VERIFY → COMMAND/CONTROL → TELEMETRY → CONTINGENCY → LAND/RTL → LOG ACQUISITION`

without BlueSky core depending on autopilot-specific data structures.

Equipment integration shall preserve the same adapter/configuration/verification boundary without introducing vendor-specific objects into the BlueSky core model.
