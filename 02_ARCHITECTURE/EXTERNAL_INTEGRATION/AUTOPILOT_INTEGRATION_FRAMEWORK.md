# BlueSky PRO — Autopilot Integration Framework

**Status:** WORKING BASELINE — UNIVERSAL AUTOPILOT INTEROPERABILITY

## 1. Objective

BlueSky PRO shall be delivered as a ready-to-operate product for a customer's existing UAV fleet. The customer shall not be required to develop an integration for each aircraft.

The architecture therefore separates the BlueSky operational model from any specific Flight Control System (FCS), autopilot, manufacturer or transport protocol.

## 2. Supported integration classes

The framework shall support, subject to technical and legal interface availability:

- standardized autopilots/protocols (e.g. ArduPilot and PX4 through MAVLink);
- proprietary OEM autopilots through documented SDK/API/protocol;
- vehicle-specific integrations where a manufacturer provides an accessible interface;
- future protocols without changes to the BlueSky operational domain model.

## 3. Layered architecture

```text
                         BLUE SKY PRO
                              │
                   Operational Domain Model
                              │
                 Universal Vehicle Interface
                              │
                       ADAPTER REGISTRY
                              │
                ┌─────────────┴─────────────┐
                │     Integration Layer     │
                ├────────────┬──────────────┤
                │            │              │
         Autopilot Adapter  C2 Adapter  Equipment Adapter
                │            │              │
          Protocol Adapter  Transport    Device/API
                │
       ┌────────┼────────┬─────────┐
       ▼        ▼        ▼         ▼
   ArduPilot    PX4     OEM-FCS   Future-FCS
       │        │        │         │
     MAVLink  MAVLink  Native/API  ...
```

The Adapter Registry is a resolution and compatibility boundary only. It does not grant execution authority and does not bypass safety controls.

## 4. BlueSky Universal Vehicle Interface

The internal contract shall expose normalized objects and operations independent of the external implementation:

### State

- position and navigation state;
- attitude and motion state;
- altitude and speed;
- battery/energy state;
- GNSS/navigation quality;
- health and readiness;
- flight mode;
- failsafe state;
- mission state;
- communication state;
- Equipment state.

### Operations

- connect/disconnect;
- discover/identify;
- synchronize configuration;
- arm/disarm;
- takeoff/land;
- hold/pause/resume;
- RTL/return;
- guided/goto where supported;
- mission upload/download;
- mission start/stop/modify where supported;
- parameter read/write;
- geofence/rally/safety configuration;
- log acquisition;
- time synchronization.

Every operation has a capability requirement. Unsupported operations shall be explicitly reported rather than silently emulated.

## 5. Adapter responsibilities

Each adapter shall implement:

1. protocol/session establishment;
2. vehicle/system/component discovery;
3. capability discovery;
4. message and command translation;
5. acknowledgement and result mapping;
6. timeout/retry semantics;
7. state normalization;
8. configuration synchronization;
9. mission translation;
10. safety/failsafe mapping;
11. log acquisition;
12. protocol-specific diagnostics;
13. version/compatibility handling;
14. security requirements;
15. automated verification tests.

## 6. Capability-driven operation

BlueSky shall never assume that all aircraft implement the same functions.

```text
Connect
  ↓
Identify FCS / firmware / vehicle
  ↓
Discover capabilities
  ↓
Load verified vehicle profile
  ↓
Check mission + Equipment + safety requirements
  ↓
READY / NOT READY with explicit reasons
```

## 7. ArduPilot/PX4 reference implementation

MAVLink shall be the first common reference integration because it is used by both ArduPilot and PX4 ecosystems. The implementation shall use the relevant MAVLink microservices rather than treating MAVLink as a single undifferentiated interface.

Minimum services for the reference adapter:

- Heartbeat / system identification;
- Command Protocol;
- Mission Protocol;
- Parameter Protocol;
- telemetry/state messages;
- timesync where required;
- log transfer where supported;
- component/capability discovery.

## 8. Mission translation

The BlueSky Mission Model shall remain richer and independent of MAVLink mission items.

```text
BlueSky Mission
      ↓
Capability check
      ↓
Vehicle-specific mission compiler/translator
      ↓
External mission representation
      ↓
Upload
      ↓
Read-back
      ↓
Canonical comparison
      ↓
MISSION VERIFIED
```

A mission shall not be considered uploaded merely because a transport operation completed. The vehicle-side mission shall be read back and verified against the approved mission identity/version and supported semantics.

## 9. Parameter management

Parameters shall be treated as controlled configuration, not as unrestricted implementation details.

```text
DISCOVER
  ↓
SNAPSHOT
  ↓
COMPARE
  ↓
VALIDATE
  ↓
APPLY
  ↓
READ-BACK
  ↓
VERIFY
  ↓
BASELINE
```

Safety-critical configuration changes shall invalidate affected readiness/verification state until revalidated.

## 10. Equipment integration

Equipment is a first-class canonical BlueSky object and is resolved through the Adapter Registry and universal Vehicle/Equipment adapter boundary.

```text
External Vehicle / Equipment
          ↓
Equipment Adapter
          ↓
Canonical Vehicle / Equipment Schema
          ↓
BlueSky Core Services
```

External legacy terminology such as `payload` remains an external protocol term only and is normalized to canonical `Equipment` at the adapter boundary.

## 11. Safety boundary

The adapter and registry shall not grant execution authority.

```text
Operational Request
        ↓
Authority + Safety Gate
        ↓
Adapter Registry / Resolution
        ↓
Vehicle / Equipment Adapter
        ↓
External protocol
```

No adapter may bypass mandatory BlueSky safety or regulatory gates.

## 12. Verification

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
- version compatibility;
- Equipment identification, capability and state normalization.

## 13. Implementation status

The reference architecture and integration boundary are defined. Concrete vendor implementation, hardware connectivity and real-flight verification remain pending.

No real-test evidence or certification claim is made by this framework.

## 14. Acceptance criterion

A representative supported aircraft can complete:

`CONNECT → IDENTIFY → CAPABILITY → CONFIG → HEALTH → MISSION UPLOAD → READ-BACK VERIFY → COMMAND/CONTROL → TELEMETRY → CONTINGENCY → LAND/RTL → LOG ACQUISITION`

without BlueSky core depending on autopilot-specific data structures.

Equipment integration shall preserve the same adapter/configuration/verification boundary without introducing vendor-specific objects into the BlueSky core model.
