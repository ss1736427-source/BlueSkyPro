# BlueSky PRO — GAP Execution Plan

**Status:** ACTIVE
**Purpose:** turn the architecture GAP audit into an ordered implementation program and prevent rework.

## 1. Governing rule

The system is considered complete only when a capability progresses through:

`SPECIFIED → IMPLEMENTED → INTEGRATED → TESTED → VERIFIED`

No UI feature shall be treated as a substitute for an underlying operational contract.

## 2. Dependency-first execution

```text
COMMON MISSION MODEL
        ↓
VEHICLE / PAYLOAD CAPABILITY MODEL
        ↓
AUTOPILOT ABSTRACTION
        ↓
C2 ABSTRACTION
        ↓
MISSION PACKAGE
        ↓
SAFETY + ENERGY GATE
        ↓
SIL / SIMULATION
        ↓
REAL AUTOPILOT ADAPTERS
        ↓
HIL
        ↓
ATM / REGULATORY ADAPTERS
        ↓
RUNTIME / TELEMETRY
        ↓
REAL UAV VALIDATION
```

## 3. Phase 1 — Common contracts

### 1.1 Mission Model

Define the canonical BlueSky mission representation independent of UI, autopilot and regulatory provider.

Must contain at minimum:

- mission identity and version;
- task type and objective;
- area/geometry;
- required quality;
- constraints;
- timing;
- UAV allocation;
- payload requirements;
- route/trajectory;
- energy/reserve requirements;
- regulatory state;
- execution state.

**Exit criterion:** one canonical mission object can be created, validated, versioned and serialized.

### 1.2 Vehicle/Payload Capability Model

Define normalized capabilities for UAV, autopilot, propulsion, battery, payload, camera, gimbal, sensors and communications.

**Exit criterion:** the planner can determine whether a concrete UAV configuration is capable of performing a mission before route optimization.

## 4. Phase 2 — Execution contracts

### 2.1 Autopilot API

Define a normalized command/state interface independent of ArduPilot, PX4 or OEM implementation.

Required functions include:

- connection/state;
- capability discovery;
- mission upload;
- upload acknowledgement;
- mission version verification;
- arm/disarm state;
- start/pause/resume/abort;
- waypoint/trajectory execution;
- telemetry;
- failsafe/recovery state;
- command acknowledgement;
- error reporting.

### 2.2 C2 API

Define the normalized communication service independently of physical transports.

Support:

- primary/secondary channels;
- link quality;
- channel selection;
- loss detection;
- failover;
- reconnect;
- synchronization;
- degraded/offline operation.

### 2.3 Mission Package

Define the immutable executable package produced from the approved mission.

It shall identify:

- mission version;
- vehicle;
- payload;
- route;
- commands;
- safety parameters;
- configuration/model versions;
- integrity information;
- regulatory/authorization reference where applicable.

## 5. Phase 3 — Deterministic planning safety

Implement the mandatory gates before optimization trade-offs:

```text
candidate
   ↓
SAFETY HARD CONSTRAINTS
   ↓
ENERGY SUFFICIENCY + RESERVE
   ↓
only admissible candidates
   ↓
QUALITY / RESOURCE / TIME optimization
```

Energy calculations must use the concrete vehicle configuration, payload, wind/environment, battery condition and reserve policy.

## 6. Phase 4 — Algorithm Orchestrator

Implement BAO as a dispatcher and composition layer, not as a serial algorithm chain.

Requirements:

- task classification;
- algorithm suitability selection;
- parallel candidate calculation where useful;
- bounded calculation time;
- result caching;
- incremental recalculation;
- multi-UAV algorithm composition;
- deterministic validation of every candidate;
- mission-specific quality evaluation;
- concise pilot-facing rationale for non-obvious decisions.

The user specifies the operational task; BlueSky determines the appropriate algorithmic strategy.

## 7. Phase 5 — Simulation and verification

Create the minimum testable execution environment before broad hardware integration.

### SIL

Validate mission generation, adapters, state transitions, safety gates and orchestration against a simulated vehicle/autopilot.

### HIL

Validate real autopilot interfaces, timing, acknowledgements, telemetry and failure behaviour against simulated flight dynamics.

### Replay

Use recorded logs to reproduce execution and compare predicted versus actual behaviour.

## 8. Phase 6 — Real integrations

First target adapters:

1. ArduPilot;
2. PX4;
3. OEM adapter contract.

Then establish real C2 transports and target ATM/regulatory integrations.

The core shall remain independent of all provider-specific protocols.

## 9. Phase 7 — Runtime

Implement the execution state machine:

```text
READY
 ↓
ARM / START
 ↓
EXECUTING
 ↓
MONITORING
 ↓
CORRECTION / REPLAN
 ↓
CONTINUE
 ↓
COMPLETE
```

Failure branches must cover at minimum:

- C2 loss;
- autopilot fault;
- energy degradation;
- navigation degradation;
- payload failure;
- regulatory/environmental change;
- mission abort;
- recovery/return.

## 10. Phase 8 — Operational evidence

For each released mission retain the evidence needed to reconstruct the release decision and execution, subject to applicable security and retention rules.

Minimum linkage:

`Mission → Vehicle → Payload → Algorithm/Model versions → Environment → Regulatory status → C2 → Validation → Mission Package → Execution → Logs → Replay/Corrections`

## 11. Current priorities

| Priority | Work | State |
|---|---|---|
| P0 | Mission Model | NEXT |
| P0 | Vehicle/Payload Model | BLOCKED BY 1 |
| P0 | Autopilot API | BLOCKED BY 1/2 |
| P0 | C2 API | BLOCKED BY 1 |
| P0 | Mission Package | BLOCKED BY 1/3 |
| P0 | Safety/Energy Gate | BLOCKED BY 1/2 |
| P0 | SIL | BLOCKED BY 3–6 |
| P0 | ArduPilot/PX4 adapters | BLOCKED BY 3 |
| P0 | HIL | BLOCKED BY SIL + adapters |
| P0 | ATM/Regulatory implementation | contract first, then provider |
| P0 | Runtime/Telemetry | BLOCKED BY execution contracts |
| P0 | Real UAV validation | final integration stage |
| P1 | advanced replay/analytics | after P0 baseline |
| P1 | expanded algorithm portfolio | after orchestrator baseline |

## 12. Definition of Done

A BlueSky contour is not marked complete when its specification is written. It is complete only when:

- implementation exists;
- required external interface is integrated;
- automated/repeatable tests pass;
- operational evidence exists;
- the result is traceable to the exact configuration and version used.

## 13. Anti-rework rule

Before adding or substantially changing UI panels, freeze the underlying data contracts needed by that panel.

Before implementing an external adapter, freeze the normalized BlueSky interface that the adapter implements.

Before optimizing algorithms, freeze the mission model, vehicle/payload model and safety/energy validation contracts.

Before real UAV trials, complete the SIL/HIL path for the applicable integration.

This document is the execution sequence for closing the architectural GAPs identified in `BLUESKY_ARCHITECTURE_GAP_AUDIT.md`.
