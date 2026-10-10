# BlueSky PRO — Universal Integration Layer

**Status:** WORKING BASELINE
**Phase:** 4 — Universal Integration

## 1. Principle

BlueSky PRO shall integrate with the customer's fleet through a universal integration layer. The user receives a product ready to operate the supported fleet; protocol-specific integration is a BlueSky product responsibility.

Universality applies to UAVs, autopilots/FCS, C2/data links, payloads, video, GNSS/RTK, traffic, airspace/ATM, weather, GIS/terrain, companion computers, ground infrastructure and enterprise/data interfaces.

## 2. Architecture

```text
BLUE SKY CORE
 Mission / Fleet / Safety / AI / Flight
              │
              ▼
 UNIVERSAL INTEGRATION CONTRACTS
              │
      ┌───────┼────────┬────────┐
      ▼       ▼        ▼        ▼
 Autopilot   C2      Payload   Data
 Adapter     Adapter  Adapter   Adapters
      │       │        │        │
 Protocol / Transport / SDK / API
              │
              ▼
 External systems / UAV / FCS
```

## 3. Responsibilities

**BlueSky Core:** mission semantics, fleet model, planning/optimization, system-level safety, capability evaluation, readiness, authorization state, normalized telemetry/state, mission state, Flight Record and audit.

**Integration Layer:** discovery, connection/session management, protocol translation, transport selection, message normalization, command/acknowledgement mapping, version compatibility, capability mapping, timeout/retry semantics and diagnostics.

**External system:** authoritative execution/state belonging to that system, such as onboard flight-control execution, physical radio transport, payload device operation or an external authorization decision.

## 4. Universal contracts

- Vehicle Contract
- Autopilot Contract
- C2 Contract
- Payload Contract
- Navigation/GNSS Contract
- Traffic Contract
- Weather Contract
- Geospatial Contract
- ATM/Regulatory Contract
- Log/Replay Contract
- Ground Infrastructure Contract
- Enterprise/API Contract

## 5. Common adapter lifecycle

```text
DISCOVER → CONNECT → IDENTIFY → CAPABILITY DISCOVERY
→ LOAD PROFILE → SYNCHRONIZE → VERIFY → OPERATIONAL
→ MONITOR → DEGRADED/LOST → RECOVER → DISCONNECT
```

Every adapter must preserve external diagnostics and map them to canonical BlueSky states.

## 6. Autopilot/FCS adapter

Reference integrations: **ArduPilot** and **PX4**, with MAVLink as the first common protocol where applicable. OEM integrations use documented and legally available SDK/API/protocol interfaces.

The adapter exposes, where supported:

- system/component discovery;
- vehicle type and firmware;
- capabilities;
- heartbeat/state and telemetry;
- command dispatch and acknowledgement;
- mission upload/download/start/pause/resume/abort;
- parameter access and configuration synchronization;
- geofence/rally/safety configuration;
- failsafe state;
- time synchronization;
- log acquisition;
- diagnostics.

Protocol compatibility alone is insufficient: BlueSky must determine actual capabilities and version compatibility.

## 7. Command contract

Each command contains target, parameters, preconditions, authorization, timestamp, correlation ID, timeout/retry policy, acknowledgement/result, resulting state and audit record. Transport success is not execution success.

## 8. Mission contract

```text
BlueSky Mission
      ↓
Semantic validation
      ↓
Vehicle capability check
      ↓
Adapter Mission Compiler
      ↓
Target mission
      ↓
Upload
      ↓
Read-back
      ↓
Semantic verification
      ↓
Approved mission instance
```

Unsupported mission semantics produce an explicit incompatibility/correction state.

## 9. C2 abstraction

```text
BlueSky C2 Service
       ↓
C2 Link Manager
       ↓
Link 1 / Link 2 / Link 3 ...
       ↓
Radio / IP / Cellular / Ethernet / Relay / Vendor transport
```

Track availability, quality, latency, loss, failover and recovery. Distinguish loss of C2, telemetry, video, payload control and total communication. Onboard FCS failsafe remains authoritative for aircraft safety.

## 10. Payload abstraction

Semantic operations:
`CAPTURE | START_RECORDING | STOP_RECORDING | SET_CAMERA_MODE | SET_SENSOR_MODE | SET_GIMBAL | TRIGGER_PAYLOAD`.

Payload adapters translate these to device-specific protocols/APIs and normalize state, faults and metadata.

## 11. External data

External weather, airspace, terrain, traffic and GNSS-correction data shall carry source identity and, where applicable, timestamp, coordinate reference, validity, freshness, quality/confidence, version and transformation history.

## 12. Error model

Canonical classes:
`CONNECTION | TIMEOUT | AUTHENTICATION | UNSUPPORTED | INVALID_STATE | INVALID_COMMAND | ACK_FAILURE | DATA_STALE | VERSION_MISMATCH | DEVICE_FAULT | EXTERNAL_SERVICE_FAILURE`.

## 13. Version compatibility

Evaluate at least:
`BlueSky × Adapter × Protocol × UAV Model × FCS/Firmware × Payload Configuration`.

## 14. Verification

Each adapter requires automated tests for discovery, identification, capabilities, state normalization, command/ACK, mission translation, read-back, configuration synchronization, failure/recovery, logging and version compatibility.

Reference integrations progress through simulation, HIL/bench and representative real-aircraft verification before operational status.

## 15. Phase 4 Gate

Phase 4 is complete when the universal BlueSky contracts and adapter lifecycle/error/version rules are frozen, and reference ArduPilot/PX4 integration can be implemented and verified without manufacturer-specific logic entering BlueSky Core.
