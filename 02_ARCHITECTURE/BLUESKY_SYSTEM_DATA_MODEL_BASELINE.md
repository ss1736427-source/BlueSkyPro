# BlueSky PRO — System Data Model & Architecture Baseline

**Status:** WORKING BASELINE — PHASE 2 GATE
**Governing document:** `00_MASTER/BLUESKY_PRODUCT_LIFECYCLE_MASTER_PLAN.md`

## 1. Purpose

This document freezes the principal system objects and their relationships before detailed implementation of UI, autopilot adapters and operational engines.

The objective is to prevent rework caused by designing modules independently and later discovering that mission, vehicle, authorization, telemetry and Flight Record data cannot be joined consistently.

## 2. Architectural rule

BlueSky core shall operate on normalized domain objects. External systems are integrated through adapters. No core business logic shall depend directly on ArduPilot, PX4, MAVLink, a particular C2 transport, equipment protocol, map provider or regulatory interface.

## 3. Core domain objects

```text
ORGANIZATION
   │
   ├── USER / ROLE
   │
   └── FLEET
          │
          └── VEHICLE
                │
                ├── FCS / AUTOPILOT
                ├── CONFIGURATION BASELINE
                ├── C2 LINKS
                ├── NAVIGATION / SENSORS
                ├── EQUIPMENT
                └── MAINTENANCE / RESOURCES

MISSION
   │
   ├── TASK
   ├── ROUTE / TRAJECTORY
   ├── CONSTRAINTS
   ├── VEHICLE ASSIGNMENT
   ├── EQUIPMENT ASSIGNMENT
   ├── EXTERNAL DATA SNAPSHOTS
   ├── REGULATORY PLAN(S)
   └── AUTHORIZATION

FLIGHT
   │
   ├── MISSION VERSION
   ├── VEHICLE
   ├── C2 SESSION(S)
   ├── TELEMETRY
   ├── COMMANDS / ACKS
   ├── EVENTS / ALERTS
   ├── EQUIPMENT DATA
   ├── LOGS
   └── FLIGHT RECORD
```

## 4. Entity definitions

### 4.1 Vehicle

Represents a physical aircraft and its stable identity.

Minimum identity: internal BlueSky vehicle ID; manufacturer/model; registration/board number where applicable; serial number where legally/technically available; vehicle type; configuration version; operational status.

### 4.2 FCS / Autopilot

Represents the flight-control system installed on a vehicle: FCS family; vendor; firmware/version; protocol/transport; component identity; capabilities; compatibility status; verification status.

### 4.3 Vehicle Profile

Versioned operational description combining vehicle characteristics and approved configuration needed by BlueSky.

### 4.4 Equipment

Physical mission equipment installed on or associated with a vehicle. Equipment profile defines capabilities, controls, status, interfaces and data outputs.

### 4.5 C2 Link

One communication path between BlueSky infrastructure and the vehicle/FCS. It contains link identity, transport, status, quality, priority, security state and failover role.

### 4.6 Mission

The operational task definition independent of a particular autopilot representation. A Mission may contain multiple vehicle assignments and multiple regulatory plans.

### 4.7 Mission Version

Immutable operational snapshot used for validation, authorization binding, mission transfer and Flight Record traceability. Any material change creates a new version.

### 4.8 Flight

An actual execution instance of a Mission Version by a specific vehicle or coordinated vehicle set.

### 4.9 Regulatory Plan / Authorization

External regulatory objects linked to the applicable Mission Version and, where required, individual vehicles/flights.

### 4.10 Telemetry / Event / Command

All operational state changes and externally exchanged control actions shall carry at minimum timestamp, vehicle identity, source/component, sequence or correlation ID where applicable, value/state and quality/status where applicable.

### 4.11 Flight Record

The authoritative post-flight operational record joining the mission, aircraft, configuration, authorization, telemetry/events, commands, logs and equipment references.

## 5. Relationships that must never be lost

```text
Customer
  → Fleet
  → Vehicle
  → Configuration/FCS/Equipment
  → Mission Version
  → Regulatory Authorization
  → Flight
  → Telemetry/Events/Commands
  → Logs/Equipment Data
  → Flight Record
```

A downstream object shall never be stored without the upstream identity required for traceability.

## 6. State model

BlueSky shall distinguish three independent concepts:

- **Capability:** what the integrated vehicle/system can technically perform.
- **Readiness:** whether it is currently acceptable to perform the planned operation.
- **Authorization:** whether the operation is legally/regulatorily permitted.

```text
CAPABILITY = CAN IT?
READINESS  = IS IT READY NOW?
AUTHORITY  = IS IT AUTHORIZED?
```

`READY FOR FLIGHT` requires all mandatory conditions to be satisfied.

## 7. External interface boundary

```text
                    BLUE SKY DOMAIN
                          │
                ┌─────────▼─────────┐
                │ Normalized Models │
                └─────────┬─────────┘
                          │
                  Adapter Contracts
                          │
       ┌──────────┬───────┼───────┬──────────┐
       ▼          ▼       ▼       ▼          ▼
    FCS/C2     Equipment GNSS    ATM      GIS/Weather/
   adapters    adapters adapter adapter    Traffic
       │          │       │       │          │
       └──────────┴───────┴───────┴──────────┘
                          │
                     External World
```

## 8. Data ownership

- BlueSky owns canonical mission/task versions and operational records.
- Vehicle/FCS remains authoritative for aircraft-level flight-control state and onboard safety behavior.
- External regulatory systems remain authoritative for submitted/accepted authorization state.
- External weather/GIS/traffic providers remain authoritative for their source data; BlueSky records source, timestamp and quality.
- Equipment devices remain authoritative for device-local status and raw data; BlueSky associates and indexes it operationally.

## 9. Configuration versioning

The following must be versioned and traceable: BlueSky software build; vehicle profile; FCS/firmware; parameters/configuration baseline; equipment profile; C2 configuration; mission version; regulatory submission/authorization; AI/correction model or rule version.

A material change invalidates the relevant verification/readiness state until revalidated.

## 10. Multi-UAV model

A single operational Mission may contain multiple vehicle assignments while each vehicle retains its own identity, FCS, C2 session, telemetry, equipment, regulatory relationship and Flight Record data. The mission coordinator maintains common task identity and coordination state.

```text
MISSION
 ├── UAV-01 → Mission Segment A
 ├── UAV-02 → Mission Segment B
 └── UAV-03 → Mission Segment C
```

## 11. Phase 2 dependency order

Implementation shall follow:

```text
Domain objects
      ↓
State machines
      ↓
Canonical data contracts
      ↓
Adapter contracts
      ↓
Persistence/versioning
      ↓
Services/engines
      ↓
UI
```

No vendor-specific adapter shall define the canonical BlueSky mission or vehicle model.

## 12. Phase 2 exit gate

Phase 2 is complete when:

1. all lifecycle-critical domain objects are defined;
2. ownership and authority are defined;
3. relationships and traceability are defined;
4. versioning rules are defined;
5. capability/readiness/authorization are separated;
6. multi-UAV relationships are defined;
7. external boundaries are defined through contracts;
8. no planned implementation requires a vendor-specific change to the core domain model.

## 13. Next development step

After this gate, proceed to **PHASE 3 — Fleet / Vehicle / Equipment Model implementation specification**, followed by universal adapter contracts.
