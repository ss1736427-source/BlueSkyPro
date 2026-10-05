# BlueSky PRO — System Requirements Baseline

**Status:** WORKING BASELINE — PHASE 1
**Governing document:** `00_MASTER/BLUESKY_PRODUCT_LIFECYCLE_MASTER_PLAN.md`

## 1. Purpose

This document is the controlled starting point for requirements work. It defines the product-level requirements that must be stable before detailed implementation of interfaces, planning logic and UI is expanded.

The purpose is to prevent rework: architecture, interfaces, safety logic, mission model and verification shall be derived from the same requirement baseline.

## 2. Product definition

BlueSky PRO is a mission and flight-operations system for UAV operations. It manages the operational lifecycle from fleet configuration and mission preparation through flight supervision, post-flight data and controlled improvement.

BlueSky PRO is not itself the aircraft flight-control firmware. The onboard Flight Control System (FCS)/autopilot remains responsible for direct flight-control execution within its defined authority. BlueSky supervises, coordinates and exchanges commands/data through defined interfaces.

## 3. Fundamental requirements

### SYS-001 — Universal fleet interoperability
BlueSky shall support heterogeneous UAV fleets through a universal internal vehicle model and external adapters.

### SYS-002 — User-ready integration
A supported customer aircraft shall be delivered as a configured, verified BlueSky integration. The customer shall not be required to develop protocol integrations.

### SYS-003 — Protocol independence
Core BlueSky mission and operational logic shall not depend on a specific autopilot, C2 transport, payload, map, weather, traffic or regulatory protocol.

### SYS-004 — Capability-driven behavior
BlueSky shall discover or load the capabilities of the selected aircraft/system and shall not issue or promise unsupported functions.

### SYS-005 — Traceability
Safety-, mission- and authorization-relevant operations shall be traceable to requirements, software/configuration versions, events and verification evidence.

## 4. Operational lifecycle requirements

BlueSky shall support the following controlled lifecycle:

`Fleet → Preparation → Mission → External data → Planning → Validation → Regulatory/ATM → Readiness → Mission transfer → Flight → Contingency → Completion → Logs/Data → Flight Record → Maintenance/Analysis → Corrections → Next mission`.

Each transition shall have defined inputs, outputs and acceptance conditions.

## 5. Vehicle / autopilot requirements

### SYS-AUT-001
The system shall identify the connected FCS/autopilot, firmware/version and vehicle type.

### SYS-AUT-002
The system shall discover or validate available capabilities.

### SYS-AUT-003
The system shall normalize telemetry, health, navigation, energy, mode and mission state into the BlueSky vehicle model.

### SYS-AUT-004
The system shall translate BlueSky mission semantics to the target FCS representation.

### SYS-AUT-005
Mission transfer shall include upload, read-back and semantic verification.

### SYS-AUT-006
Commands shall have explicit acknowledgement/result handling, timeout and failure semantics.

### SYS-AUT-007
Vehicle configuration shall support snapshot, comparison, controlled change, confirmation/read-back and baseline association.

### SYS-AUT-008
The architecture shall permit adapters for ArduPilot, PX4 and OEM/proprietary FCS without modification of the core mission model.

ArduPilot's own documentation notes that supported mission commands vary by vehicle type and firmware, and that parameter sets change between vehicle types and firmware releases. Therefore compatibility shall be capability/version based rather than protocol-presence based. citeturn0search3turn0search1

## 6. C2 requirements

### SYS-C2-001
BlueSky shall abstract command-and-control transport from mission logic.

### SYS-C2-002
The system shall support multiple available communication paths where the aircraft/network permits.

### SYS-C2-003
The system shall represent link state at minimum as connected, degraded, lost and recovering.

### SYS-C2-004
The system shall implement defined primary/backup selection and failover policies.

### SYS-C2-005
The system shall distinguish C2 loss from telemetry-only, video-only and payload-control loss.

### SYS-C2-006
Link changes, failures and recovery shall be logged and associated with the Flight Record.

## 7. Vehicle / payload requirements

### SYS-VP-001
Every supported aircraft shall have a versioned vehicle profile.

### SYS-VP-002
Every supported payload shall have a versioned payload profile.

### SYS-VP-003
Payload capabilities, commands, status and data association shall be exposed through a normalized interface.

### SYS-VP-004
BlueSky shall validate aircraft/payload compatibility before flight release.

### SYS-VP-005
Payload data shall be associated with vehicle, mission, time and available navigation state.

## 8. External data requirements

BlueSky shall provide adapter-based ingestion and validation for applicable:

- airspace/restriction data;
- aeronautical information/NOTAM sources;
- weather and wind;
- maps/GIS/DEM/terrain;
- traffic/ADS-B;
- GNSS/RTK/PPK/NTRIP;
- ground infrastructure;
- enterprise/customer data interfaces.

External data shall carry source, timestamp/freshness and quality state where technically available.

## 9. Mission planning requirements

The mission model shall be independent of the target autopilot and shall support:

- mission templates;
- waypoints and route geometry;
- flight profile;
- altitude/speed/time constraints;
- terrain/obstacle constraints;
- airspace constraints;
- aircraft performance;
- payload constraints;
- energy/battery degradation;
- aerodynamic configuration;
- wind;
- mission objective optimization;
- multi-UAV task decomposition and coordination.

## 10. AI / Corrections requirements

AI may recommend or calculate corrections, optimization and decision support, but safety-critical constraints shall remain deterministic and enforceable outside the AI decision path.

Each correction shall be traceable to:

`input state + algorithm/model/rule version + reason + resulting change + disposition`.

Offline operation and synchronization shall preserve version integrity and auditability.

## 11. ATM / regulatory requirements

The regulatory integration shall support the applicable jurisdiction-specific lifecycle:

`Prepare → Validate → Submit → Rejected/Correction → Resubmit → Accepted/Authorized → Bind authorization to mission version → Flight Release → Close/Report where required`.

A successful network transaction shall not itself constitute authorization.

For multi-UAV operations, BlueSky shall support relationships between one operational mission and one or more regulatory flight plans/authorizations.

## 12. Readiness requirements

BlueSky shall automatically evaluate, as applicable:

- aircraft identity;
- FCS/firmware compatibility;
- configuration baseline;
- C2;
- navigation/GNSS;
- energy/battery;
- payload;
- mission feasibility;
- airspace/authorization;
- weather;
- terrain/obstacles;
- safety/failsafe state;
- required human checks.

The output shall be an explicit `READY`, `NOT READY` or defined degraded state with blocking reasons.

## 13. Flight execution requirements

The system shall provide normalized supervisory monitoring of:

- position;
- altitude;
- speed;
- attitude where available;
- mission progress;
- flight mode;
- energy;
- navigation quality;
- health;
- C2 state;
- safety/failsafe state;
- payload/video state;
- alerts;
- operator commands/events.

Multi-UAV operations shall maintain vehicle identity and mission/task association for every state and event.

## 14. Contingency requirements

For every defined abnormal condition BlueSky shall specify:

`Trigger → Detection → Responsible subsystem → Action → Timeout → Recovery / escalation → Termination state → Record`.

At minimum the model shall cover C2 loss, telemetry loss, GNSS degradation/loss, energy limits, weather deterioration, aircraft health degradation, payload failure, airspace conflict and communication recovery.

The onboard autopilot's safety behavior remains authoritative for autonomous aircraft-level failsafe actions unless an explicit validated interface defines otherwise.

## 15. Post-flight requirements

BlueSky shall acquire and preserve, as available:

- autopilot logs;
- telemetry;
- commands/acknowledgements;
- C2 events;
- safety events;
- payload data and metadata;
- mission version;
- aircraft configuration;
- authorization references;
- Flight Record.

Logs shall support replay and post-flight analysis.

## 16. Verification requirements

Critical requirements shall be verified progressively:

`Unit → Integration → SIL/SITL → HIL → Bench/Ground → Real UAV → Operational Release`.

Simulation shall be used before real flight wherever practical. ArduPilot SITL supports simulation of wind, sensor malfunctions and RC failures; PX4 supports SITL and HIL workflows for testing flight code and hardware interaction. citeturn0search0turn0search5turn0search2

## 17. Requirement status

| Area | Baseline | Implementation | Evidence |
|---|---|---|---|
| Product boundary | DONE-DOC | NOT DONE | NOT DONE |
| Universal integration principle | DONE-DOC | IN PROGRESS | NOT DONE |
| Autopilot abstraction | DONE-DOC | IN PROGRESS | NOT DONE |
| C2 abstraction | DONE-DOC | IN PROGRESS | NOT DONE |
| Vehicle/payload model | DONE-DOC | IN PROGRESS | NOT DONE |
| External data interfaces | IN PROGRESS | NOT DONE | NOT DONE |
| Mission model | IN PROGRESS | IN PROGRESS | NOT DONE |
| AI/Corrections | IN PROGRESS | NOT DONE | NOT DONE |
| ATM/regulatory | IN PROGRESS | NOT DONE | NOT DONE |
| Readiness gate | IN PROGRESS | NOT DONE | NOT DONE |
| Flight supervision | IN PROGRESS | IN PROGRESS | NOT DONE |
| Contingency | IN PROGRESS | NOT DONE | NOT DONE |
| Post-flight/Flight Record | IN PROGRESS | NOT DONE | NOT DONE |
| SIL/SITL/HIL validation | DONE-DOC | NOT DONE | NOT DONE |

## 18. Phase 1 exit gate

Phase 1 is complete only when each requirement that can affect architecture or interface design has:

1. unique identifier;
2. clear statement;
3. responsible subsystem/interface;
4. verification method;
5. acceptance criterion;
6. traceability to applicable regulatory/certification basis where required.

Until this gate is passed, detailed implementation that would lock the architecture shall be treated as provisional.
