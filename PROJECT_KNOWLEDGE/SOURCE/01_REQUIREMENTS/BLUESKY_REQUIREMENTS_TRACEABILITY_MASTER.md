# BlueSky PRO — Requirements Traceability Master

**Status:** PHASE 1 — WORKING BASELINE
**Parent:** `00_MASTER/BLUESKY_PRODUCT_LIFECYCLE_MASTER_PLAN.md`
**Purpose:** Prevent architectural and implementation rework by defining, before detailed implementation, what each critical requirement belongs to, what interface it uses, how it will be implemented, and how it will be verified.

## 1. Rule

No critical requirement is considered closed merely because it is documented or coded.

The controlled chain is:

`Requirement → Owner/Sub-system → Interface → Implementation → Verification → Evidence → Gate`

A requirement affecting an external interface or safety-critical behavior must be stabilized before the dependent implementation is treated as final.

## 2. Status legend

- `DONE-DOC` — requirement/architecture defined.
- `IN PROGRESS` — partially defined or being refined.
- `NOT DONE` — implementation/evidence missing.
- `BLOCKED` — dependent decision/interface is unresolved.
- `GATE` — acceptance evidence required before proceeding.

## 3. Master traceability matrix

| ID | Requirement / capability | Owner / subsystem | External interface | Implementation needed | Verification | Current status |
|---|---|---|---|---|---|---|
| SYS-001 | Universal heterogeneous fleet interoperability | Core / Integration | Vehicle Adapter API | Universal vehicle model + adapters | Multi-vendor integration test | IN PROGRESS |
| SYS-002 | Customer receives ready integration | Integration / Fleet | Profiles + adapters | Verified integration packages | Acceptance test on representative fleet | IN PROGRESS |
| SYS-003 | Core independent of vendor protocols | Architecture | All external APIs | Adapter boundary enforcement | Architecture/code review | DONE-DOC |
| SYS-004 | Capability-driven behavior | Vehicle Integration | FCS discovery/API | Capability registry + runtime checks | Unsupported-capability tests | IN PROGRESS |
| SYS-005 | Full operational traceability | Core / Flight Record | All critical interfaces | Event/audit model | Traceability/replay test | IN PROGRESS |
| SYS-AUT-001 | Identify FCS, firmware, vehicle | Autopilot Adapter | MAVLink/OEM | Discovery service | Connection test | IN PROGRESS |
| SYS-AUT-002 | Discover/validate capabilities | Autopilot Adapter | MAVLink/OEM | Capability mapper | Capability matrix test | IN PROGRESS |
| SYS-AUT-003 | Normalize telemetry/health/state | Vehicle State | MAVLink/OEM | Canonical state model | Telemetry integration/replay | IN PROGRESS |
| SYS-AUT-004 | Translate mission semantics | Mission / Adapter | Mission protocol | Mission compiler/translator | Semantic conversion test | IN PROGRESS |
| SYS-AUT-005 | Upload/read-back/verify mission | Mission Transfer | Mission protocol | Transfer + canonical comparison | End-to-end mission test | IN PROGRESS |
| SYS-AUT-006 | Command ACK/result/timeout/failure | Command Service | Command protocol | Command state machine | Fault injection | IN PROGRESS |
| SYS-AUT-007 | Parameter snapshot/change/baseline | Configuration | Parameter API | Config service | Read/write/rollback test | IN PROGRESS |
| SYS-AUT-008 | ArduPilot/PX4/OEM adapters | Integration | MAVLink/native APIs | Reference adapters | Real/SIL integration | IN PROGRESS |
| SYS-C2-001 | C2 transport independent of mission logic | C2 Manager | RF/IP/vendor | Link abstraction | Integration test | IN PROGRESS |
| SYS-C2-002 | Multiple C2 paths | C2 Manager | Radio/cellular/IP/etc. | Multi-link manager | Link switching test | IN PROGRESS |
| SYS-C2-003 | Connected/degraded/lost/recovering states | C2 Manager | Link telemetry | State machine | Fault injection | IN PROGRESS |
| SYS-C2-004 | Primary/backup failover | C2 Manager | Multiple links | Failover policy engine | Loss/recovery scenarios | NOT DONE |
| SYS-C2-005 | Distinguish C2/telemetry/video/equipment loss | C2/Data | Separate streams | Independent link states | Failure isolation test | NOT DONE |
| SYS-C2-006 | Record link events | Flight Record | C2 events | Event logging | Replay/audit test | IN PROGRESS |
| SYS-VP-001 | Versioned vehicle profile | Fleet | Vehicle metadata | Fleet database/profile schema | Profile validation | IN PROGRESS |
| SYS-VP-002 | Versioned equipment profile | Fleet / Equipment | Device API | Equipment registry | Device compatibility test | IN PROGRESS |
| SYS-VP-003 | Normalized equipment commands/status/data | Equipment Adapter | Camera/gimbal/etc. | Equipment API | End-to-end equipment test | NOT DONE |
| SYS-VP-004 | Vehicle/equipment compatibility | Planning / Readiness | Profiles | Capability/compatibility engine | Blocking tests | NOT DONE |
| SYS-VP-005 | Associate equipment data | HUB / Flight Record | Video/data/telemetry | Metadata correlation | Data association test | NOT DONE |
| EXT-AIR-001 | Airspace/restriction ingestion | Airspace | External source | Airspace adapter | Fresh/stale/unavailable tests | IN PROGRESS |
| EXT-AIR-002 | NOTAM/aeronautical information | Airspace | External source | Aeronautical data adapter | Data validity tests | NOT DONE |
| EXT-WX-001 | Weather/wind ingestion | Weather | Weather provider | Weather adapter | Freshness/quality tests | IN PROGRESS |
| EXT-GIS-001 | Map/DEM/terrain | GIS | Map/DEM provider | GIS adapter | Coordinate/terrain tests | IN PROGRESS |
| EXT-TRF-001 | Traffic/ADS-B | Traffic | Traffic source | Traffic adapter | Replay/data quality tests | NOT DONE |
| EXT-NAV-001 | GNSS/RTK/PPK/NTRIP | Navigation | Receiver/network | Navigation adapter | Degraded-navigation tests | NOT DONE |
| EXT-ATM-001 | FPL generation | ATM | Regulatory system | FPL generator | Golden-data validation | IN PROGRESS |
| EXT-ATM-002 | Submission/status/rejection/correction | ATM | API/portal/exchange | Regulatory adapter | End-to-end workflow test | NOT DONE |
| EXT-ATM-003 | Authorization linked to mission version | Flight Release | ATM | Authorization object + binding | Version integrity test | NOT DONE |
| EXT-ATM-004 | Multi-UAV regulatory relationship | ATM / Fleet | ATM/FPL | Group-to-individual plan model | Scenario test | IN PROGRESS |
| MIS-001 | Mission model independent of autopilot | Mission | Mission Adapter | Canonical mission schema | Cross-autopilot conversion | IN PROGRESS |
| MIS-002 | Mission templates | Mission Planning | Internal | Template engine | Template acceptance tests | IN PROGRESS |
| MIS-003 | Feasibility against vehicle/equipment | Planning | Fleet profiles | Constraint engine | Boundary tests | NOT DONE |
| MIS-004 | Wind/energy/aerodynamic optimization | Planning / AI | Weather + vehicle model | Optimization engine | Simulation + flight comparison | IN PROGRESS |
| MIS-005 | Multi-UAV decomposition | Fleet Mission | Multiple adapters | Task allocation/mission compiler | Multi-UAV simulation | IN PROGRESS |
| AI-001 | AI recommendations/corrections | AI | Internal data | Agent + controlled interface | Scenario/regression tests | IN PROGRESS |
| AI-002 | Deterministic safety boundary | Safety | AI ↔ Safety | Safety gate outside AI | Fault/unsafe recommendation tests | NOT DONE |
| AI-003 | Offline/online synchronization | AI / Sync | Network | Versioned sync | Offline/online replay | NOT DONE |
| AI-004 | Explainable correction record | AI / Flight Record | Internal | Input/reason/version/result record | Audit test | NOT DONE |
| RDY-001 | Automatic pre-flight readiness | Readiness | Fleet/C2/Nav/Equipment/ATM/WX | Readiness engine | Full checklist simulation | IN PROGRESS |
| RDY-002 | READY/NOT READY with reasons | HMI / Readiness | Internal | Gate aggregation | UI + logic tests | IN PROGRESS |
| RDY-003 | Human checks only where required | Operations | Pilot/Technician | Responsibility matrix | Procedure validation | IN PROGRESS |
| FLT-001 | Supervisory flight monitoring | Flight | Autopilot/C2/Equipment | Flight state service + UI | Real UAV test | IN PROGRESS |
| FLT-002 | Multi-UAV coordinated operation | Fleet / Flight | Multiple vehicles | Fleet coordinator | SIL/HIL/flight tests | IN PROGRESS |
| FLT-003 | Data aggregation from multiple UAVs | HUB | Video/photo/log streams | Correlation/merge pipeline | Multi-UAV data test | IN PROGRESS |
| CNT-001 | Defined contingency state machine | Safety / Flight | Autopilot/C2/WX/Traffic | Trigger-action-recovery engine | Fault injection | IN PROGRESS |
| CNT-002 | C2 loss behavior | Safety / C2 | C2 + autopilot | Policy + handoff model | SIL/HIL/real UAV | IN PROGRESS |
| CNT-003 | GNSS degradation/loss | Navigation / Safety | FCS/Nav | Detection + policy | Simulation + HIL | NOT DONE |
| CNT-004 | Energy/battery emergency | Safety | Vehicle telemetry | Energy prediction + gate | SIL/flight test | IN PROGRESS |
| CNT-005 | Weather/airspace conflict | Safety / Planning | Weather/ATM | Replan/abort policy | Scenario test | NOT DONE |
| LOG-001 | Acquire autopilot/telemetry logs | HUB | FCS log protocol | Log adapters | Real vehicle test | IN PROGRESS |
| LOG-002 | Replay operational data | Analysis | Flight Record | Replay engine | Deterministic replay | IN PROGRESS |
| LOG-003 | Complete Flight Record | Flight Record | All critical systems | Immutable/versioned record | Audit reconstruction | IN PROGRESS |
| VAL-001 | Unit/integration verification | V&V | Internal | Automated test suite | CI/regression | IN PROGRESS |
| VAL-002 | SIL/SITL | V&V | Autopilot simulator | Simulation harness | Scenario suite | NOT DONE |
| VAL-003 | HIL | V&V | Real FCS hardware | HIL harness | Hardware scenarios | NOT DONE |
| VAL-004 | Bench/ground | V&V | Real system | Test procedures | Test evidence | NOT DONE |
| VAL-005 | Real UAV validation | V&V | Complete integrated stack | Flight-test program | Flight-test evidence | NOT DONE |
| VAL-006 | Regression after interface changes | V&V | Adapter/config/build | Impact analysis + CI | Regression gate | NOT DONE |
| SEC-001 | Secure external interfaces | Security | C2/APIs | Identity/integrity/key handling | Security verification | NOT DONE |
| MNT-001 | Maintenance/resource state | Fleet | Aircraft/battery/engine | Resource model | Maintenance workflow test | NOT DONE |

## 4. Dependency order — anti-rework rule

The matrix must be executed in dependency order, not in UI order:

```text
REQUIREMENTS
   ↓
DOMAIN / DATA MODELS
   ↓
SYSTEM BOUNDARIES
   ↓
UNIVERSAL INTERFACES
   ↓
ADAPTER CONTRACTS
   ↓
REFERENCE ADAPTERS
   ↓
MISSION / SAFETY / READINESS ENGINES
   ↓
C2 / EQUIPMENT / EXTERNAL DATA
   ↓
UI / OPERATOR WORKFLOW
   ↓
SIL/SITL
   ↓
HIL
   ↓
BENCH
   ↓
REAL UAV
   ↓
OPERATIONAL RELEASE
```

UI implementation shall not be used to freeze an interface that has not passed its architecture/interface gate.

## 5. Phase 1 Gate

Phase 1 can close only when all architecture-affecting requirements have:

- stable identifier;
- defined ownership;
- defined interface dependency;
- implementation target;
- verification method;
- acceptance criterion;
- certification/regulatory trace where applicable.

## 6. Verification & Evidence linkage

The verification evidence framework is implemented under `04_SOFTWARE/AUTOPILOT_ADAPTER/verification/` as a reusable prototype mechanism. It provides a stable path for future physical-test data without changing the requirement model:

`Requirement → Test Method → Test Case → Configuration → Execution → Raw Data → Processed Data → Result → Evidence Package → Certification Reference`

Current prepared evidence mappings:

| Requirement | Evidence mechanism | Present state |
|---|---|---|
| `SYS-005` | Event stream + immutable/versioned record contract | IN PROGRESS |
| `SYS-C2-006` | C2/link events captured as timestamped records | IN PROGRESS |
| `LOG-001` | Source-log ingestion point reserved | IN PROGRESS |
| `LOG-002` | Raw event stream retained for deterministic replay | IN PROGRESS |
| `LOG-003` | Test record + manifest + report | IN PROGRESS |
| `VAL-001` | Automated recorder regression test in CI | IN PROGRESS |
| `VAL-002..005` | Same record structure reserved for staged tests | NOT DONE — future test execution |
| `VAL-006` | CI remains the software regression gate | IN PROGRESS |

The framework records preparation capability now; it does not claim physical verification results.

## 7. Next controlled action

With the evidence mechanism established, continue implementation in dependency order toward the demonstrable prototype while preserving the same requirement/verification/evidence chain.
