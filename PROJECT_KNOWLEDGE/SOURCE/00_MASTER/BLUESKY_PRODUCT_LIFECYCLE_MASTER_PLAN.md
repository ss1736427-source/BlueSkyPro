# BlueSky PRO — MASTER PRODUCT LIFECYCLE & DEVELOPMENT PLAN

**Status:** MASTER BASELINE — WORKING DOCUMENT
**Purpose:** Single governing document for the complete BlueSky PRO product lifecycle, development sequence, completion status and integration dependencies.

---

## 1. Governing principle

> **UNIVERSALITY FOR MARKET EXPANSION:** BlueSky PRO shall be delivered as a ready-to-operate product for the customer's existing UAV fleet and shall integrate with heterogeneous UAVs, autopilots, C2 links, equipment and external aviation systems through a universal integration architecture.

The customer shall not be required to develop the integrations personally.

Universality is an architectural property of the entire product, not a feature limited to autopilot support.

> **SINGLE CURRENT RESULT / PROCESS EVOLUTION:** BlueSky PRO development is a continuous controlled evolution. When a requirement, algorithm, architecture or document is corrected and consolidated, the corrected result becomes the only current working baseline for subsequent development. Intermediate superseded variants are not development references.

> **MINIMUM RECALCULATION / NO DUPLICATION:** Every calculation is performed once at the architectural level where all inputs required for that result are available. Its versioned result is reused by downstream stages. A calculation is repeated only when an input or dependency capable of changing its result has changed.

> **DEPENDENCY-DRIVEN RECALCULATION:** Changes propagate only through the affected dependency graph. Unaffected calculations and candidates remain reusable.

These principles apply to requirements, architecture, planning algorithms, validation, AI orchestration, safety, readiness, execution and evidence.

---

## 2. What this document controls

This is the **master sequence** for the project. Existing specialized documents remain detailed subordinate specifications. They must not create a second independent development sequence.

The master plan answers four questions for every stage:

1. What has to exist?
2. What has already been defined/documented?
3. What remains to be developed/verified?
4. What is the exit criterion before moving forward?

Status terminology:

- **DONE-DOC** — requirement/architecture is documented and accepted as a working baseline.
- **IN PROGRESS** — partially defined or requires consolidation/refinement.
- **NOT DONE** — implementation/specification/evidence is still required.
- **GATE** — cannot be considered complete until its acceptance evidence exists.

---


### 2.1 Current-baseline rule

For all future work:

1. use the latest corrected and consolidated document/result as the source of truth;
2. do not reintroduce superseded intermediate variants;
3. when a correction changes an earlier decision, update the governing document and dependent specifications together;
4. downstream implementation must use only the corrected contract;
5. historical commits/PRs remain audit history, not active design alternatives.

### 2.2 Calculation reuse rule

Planning and optimization shall use a dependency-aware calculation graph.

Each material calculation result should carry, as applicable:

- input snapshot/version;
- calculation/algorithm version;
- dependency hash or equivalent identity;
- affected object/segment/candidate identifiers;
- calculation timestamp;
- provenance/evidence references.

The system shall reuse a result when its relevant dependencies are unchanged. Recalculation is required only for the affected result and its downstream dependents.

This rule prevents duplicated terrain, restriction, wind, performance and conflict calculations and is part of the master architecture, not an implementation optimization.

## 3. Product lifecycle — one continuous chain

```text
BUSINESS / OPERATIONAL NEED
          ↓
PRODUCT SCOPE & REQUIREMENTS
          ↓
SYSTEM / SOFTWARE ARCHITECTURE
          ↓
AIRCRAFT & FLEET MODEL
          ↓
EXTERNAL INTEGRATION ARCHITECTURE
          ↓
UAV / AUTOPILOT / C2 / EQUIPMENT ADAPTERS
          ↓
MISSION & FLIGHT PLANNING
          ↓
AI / OPTIMIZATION / CORRECTIONS
          ↓
AIRSPACE / ATM / REGULATORY WORKFLOW
          ↓
PRE-FLIGHT READINESS
          ↓
MISSION TRANSFER TO AUTOPILOT
          ↓
FLIGHT / SUPERVISORY CONTROL
          ↓
C2 / TELEMETRY / SAFETY / TRAFFIC / WEATHER
          ↓
CONTINGENCY / REPLANNING
          ↓
LAND / RTL / MISSION COMPLETE
          ↓
LOGS / EQUIPMENT DATA / FLIGHT RECORD
          ↓
POST-FLIGHT / MAINTENANCE / ANALYSIS
          ↓
CORRECTIONS / CONTROLLED LEARNING
          ↓
RELEASE / NEXT MISSION
          ↺
```

This chain is the primary definition of completeness. A planning-only implementation is not a complete BlueSky PRO product.

---

# 4. DEVELOPMENT MASTER SEQUENCE

## PHASE 0 — Product boundary and operating concept

### Objective
Define what BlueSky PRO is responsible for and where responsibility passes to external systems.

### Required outputs
- product concept;
- operational concept;
- system boundary;
- roles: Administrator / Pilot / Technician / Operator as applicable;
- mission types;
- fleet concept;
- safety philosophy;
- universal interoperability principle.

### Status
**DONE-DOC / baseline exists.**

### Gate
Product boundary approved and no unresolved ambiguity about BlueSky vs autopilot vs external systems.

---

## PHASE 1 — Requirements and certification baseline

### Objective
Create the controlled requirements baseline from operational needs and applicable standards/regulations.

### Required outputs
- system requirements;
- software requirements;
- safety requirements;
- external interface requirements;
- cybersecurity requirements;
- human-machine interface requirements;
- certification traceability;
- applicable Russian aviation/UAS standards and regulatory basis;
- verification criteria.

### Status
**IN PROGRESS.** Existing certification and requirements work exists, but the complete lifecycle must remain traceable to requirements and verification evidence.

### Gate
Every safety/mission-critical function has an owner, requirement ID, verification method and acceptance criterion.

---

## PHASE 2 — System architecture

### Objective
Freeze the high-level modular architecture before implementation expands.

### Core BlueSky blocks

```text
ADMINISTRATOR
      │
      ├───────────────┐
      ▼               ▼
    FLEET           CONFIG
      │
      └──────┬────────┘
             ▼
        FLIGHT / MISSION
             │
      ┌──────┼──────────┐
      ▼      ▼          ▼
   Planning  AI       Safety
      │      │          │
      └──────┼──────────┘
             ▼
            HUB
             │
      UNIVERSAL INTEGRATION
             │
 ┌───────────┼───────────────┐
 ▼           ▼               ▼
Autopilot   C2             Equipment
 ▼           ▼               ▼
UAV/FCS    Links          Equipment
```

### Status
**DONE-DOC / baseline exists.**

### Gate
All external boundaries are explicitly represented; no direct coupling of core mission logic to one vendor protocol.

---

## PHASE 3 — Fleet / Vehicle / Equipment model

### Objective
Make a user's heterogeneous fleet a first-class BlueSky object.

### Required
- aircraft identity and registration;
- vehicle type/configuration;
- autopilot/FCS and firmware;
- capabilities;
- sensors/navigation;
- propulsion/energy/performance data;
- C2 links;
- equipment;
- limits;
- safety configuration;
- configuration baseline;
- compatibility/verification status;
- maintenance/resource state.

### Status
**IN PROGRESS.** Vehicle/equipment integration framework defined; full executable profile model and product database still required.

### Gate
A real fleet member can be represented completely enough for planning, compatibility, readiness, flight and post-flight record.

---

## PHASE 4 — Universal Integration Layer

### Objective
Provide one stable internal BlueSky interface and adapters for heterogeneous external systems.

### Architecture

```text
                         BLUE SKY PRO
                              │
                    UNIVERSAL INTERNAL API
                              │
       ┌──────────────┬───────┼────────┬─────────────┐
       ▼              ▼       ▼        ▼             ▼
   AUTOPILOT          C2    EQUIPMENT VIDEO       EXTERNAL
   ADAPTERS         ADAPTER ADAPTER   ADAPTER      ADAPTERS
       │              │       │        │             │
 ArduPilot/PX4/OEM  RF/IP  Cameras   Streams   ATM/Weather/GIS/
       │                                      Traffic/etc.
       ▼
      UAV
```

### Required external domains

**P0**
- Autopilot/FCS;
- MAVLink where applicable;
- commands;
- missions;
- parameters/configuration;
- normalized vehicle state;
- C2;
- safety/failsafe;
- ATM/airspace/FPL/authorization.

**P1**
- equipment;
- video;
- companion computer;
- GNSS/RTK;
- traffic;
- GIS/terrain;
- weather;
- logs;
- peripheral buses;
- ground infrastructure.

**P2**
- maintenance;
- security;
- enterprise/customer APIs.

### Status
**IN PROGRESS — architecture documented; implementation not complete.**

### Gate
At least one representative heterogeneous UAV integration completes the entire connect → mission → flight → log lifecycle; adapter contract is reusable for additional vehicles.

Detailed documents:
- `AUTOPILOT_INTEGRATION_FRAMEWORK.md`
- `C2_CONNECTIVITY_FRAMEWORK.md`
- `VEHICLE_EQUIPMENT_INTEGRATION_FRAMEWORK.md`
- `ATM_REGULATORY_INTEGRATION_FRAMEWORK.md`
- `OPERATIONAL_VALIDATION_FRAMEWORK.md`
- `INTEGRATION_GAP_TRACEABILITY_MATRIX.md`

---

## PHASE 5 — Autopilot integration

### Objective
BlueSky must operate with different existing autopilots without changing the core mission model.

### Reference integrations
- ArduPilot;
- PX4;
- OEM/proprietary FCS through documented and legally usable interfaces;
- future FCS through the same adapter contract.

### Required sequence

```text
CONNECT
 ↓
DISCOVER
 ↓
IDENTIFY FCS / FIRMWARE / VEHICLE
 ↓
CAPABILITY DISCOVERY
 ↓
CONFIGURATION SYNC
 ↓
HEALTH / PRE-ARM STATE
 ↓
MISSION TRANSLATION
 ↓
UPLOAD
 ↓
READ-BACK
 ↓
SEMANTIC VERIFY
 ↓
COMMAND / CONTROL
 ↓
EXECUTION
 ↓
LOG ACQUISITION
```

### Required adapter functions
- session/transport;
- discovery;
- capability discovery;
- telemetry normalization;
- command translation;
- ACK/result handling;
- timeout/retry;
- mission translation;
- read-back verification;
- parameter synchronization;
- safety/failsafe mapping;
- log transfer;
- version compatibility;
- security;
- automated tests.

### Status
**IN PROGRESS.** Architecture exists. Protocol-level implementation, concrete adapters, compatibility matrix and verification evidence remain.

### Gate
Mission Planner/QGroundControl-class fundamental connection/configuration/mission/telemetry/log lifecycle is covered for the selected reference autopilot, while BlueSky remains independent of that autopilot.

Mission Planner explicitly covers connection, mission planning, mandatory setup, configuration/tuning, simulation and live data/video; its logs also include telemetry recording and analysis. citeturn0search0turn0search1turn0search2

---

## PHASE 6 — C2 / Connectivity

### Objective
Make communication transport-independent and resilient.

### Required
- multiple C2 channels;
- primary/backup links;
- link health;
- latency/bandwidth;
- degraded state;
- loss detection;
- automatic failover;
- recovery;
- safe command handling;
- prioritization;
- security;
- audit trail.

### Status
**IN PROGRESS — architecture documented; implementation and real-link validation remain.**

### Gate
Defined link-loss scenarios behave deterministically in simulation and real hardware, including recovery/failover and association with the Flight Record.

---

## PHASE 7 — Equipment / sensor / video integration

### Objective
Treat mission equipment as an integrated operational subsystem rather than an external camera attached to the UAV.

### Required
- equipment profiles;
- camera/EO/IR/thermal/LiDAR/gimbal abstractions;
- commands;
- status/telemetry;
- recording state;
- metadata;
- time/position association;
- device compatibility;
- video transport;
- mission events.

### Status
**IN PROGRESS — framework documented; concrete device adapters and end-to-end validation remain.**

### Gate
Mission actions can command the selected equipment and resulting data is associated with the correct UAV, mission, time and Flight Record.

---

## PHASE 8 — Navigation / GNSS / RTK / terrain / weather / traffic

### Objective
Provide the external data required for safe and optimized planning and flight supervision.

### Required
- GNSS state and quality;
- RTK/PPK/NTRIP where applicable;
- fallback navigation state;
- maps;
- DEM/terrain;
- obstacles/restrictions;
- weather forecast and actual data;
- wind profile;
- traffic/ADS-B sources where applicable;
- source/freshness/quality metadata.

### Status
**IN PROGRESS.** Concepts and interface domains identified; complete source adapters, freshness policies, validation and operational fallback logic remain.

### Gate
Planning and readiness calculations can distinguish valid, stale, unavailable and degraded external data.

---

## PHASE 9 — Mission planning and optimization

### Objective
Convert an operational task into a validated executable mission.

### Required
- mission templates;
- real-coordinate map;
- route/waypoint editing;
- flight profile;
- terrain/obstacle constraints;
- airspace restrictions;
- vehicle capability constraints;
- equipment constraints;
- wind;
- battery degradation;
- aerodynamic configuration;
- engine/resource constraints;
- mission objectives: endurance, speed, punctuality, equipment, etc.;
- multi-UAV decomposition;
- collision/separation logic;
- staggered starts;
- unified mission/data association.

### Status
**SUBSTANTIALLY DEFINED / IN PROGRESS.** The planning concept and many operational rules exist; full executable optimization engine and verification remain.

### Gate
A mission generated by BlueSky is internally consistent, feasible for the selected fleet and convertible into verified vehicle-specific missions.

---

## PHASE 10 — AI / Corrections / decision support

### Objective
Use AI to reduce operator workload without making uncontrolled safety-critical decisions.

### Required
- offline-capable agent architecture;
- online synchronization/update mechanism;
- corrections learned from operator actions;
- explainable corrections presented as operational reasons;
- controlled versioning;
- auditability;
- separation between recommendation and authorized command;
- deterministic safety constraints around AI.

### Status
**IN PROGRESS — concept defined; implementation, governance and verification remain.**

### Gate
Every AI-derived correction has a traceable input, rule/model version, reason and operator/system disposition.

---

## PHASE 11 — ATM / regulatory integration

### Objective
Connect BlueSky's operational mission to applicable airspace and regulatory procedures.

### Required
- airspace/restriction information;
- NOTAM/AIP-related sources as applicable;
- FPL generation;
- submission;
- acceptance/rejection;
- correction loop;
- authorization;
- validity;
- mission/authorization version binding;
- multi-UAV regulatory relationships;
- closure/reporting where required;
- audit trail.

### Status
**IN PROGRESS.** Lifecycle is defined; concrete jurisdiction-specific interfaces and production integration remain.

### Gate
A flight cannot reach READY FOR FLIGHT without the applicable positive regulatory/authorization state.

---

## PHASE 12 — Pre-flight readiness

### Objective
Move maximum preparation from people to the system while keeping mandatory human responsibilities explicit.

### Required checks

```text
VEHICLE
  ↓
FCS / FIRMWARE
  ↓
CONFIGURATION
  ↓
C2
  ↓
GNSS / NAVIGATION
  ↓
BATTERY / ENERGY
  ↓
EQUIPMENT
  ↓
MISSION
  ↓
AIRSPACE / AUTHORIZATION
  ↓
WEATHER
  ↓
SAFETY / FAILSAFE
  ↓
PILOT / TECHNICIAN CHECKS
  ↓
READY
```

### Status
**IN PROGRESS.** The desired minimalist division between system/pilot/technician is defined conceptually; complete executable checklist and evidence model remain.

### Gate
System automatically produces READY / NOT READY with explicit blocking reasons and records the release state.

---

## PHASE 13 — Mission transfer and flight release

### Objective
Guarantee that the approved mission is the mission loaded into the actual aircraft.

### Required
- mission package identity/version;
- capability check;
- translation;
- upload;
- read-back;
- semantic comparison;
- authorization binding;
- release decision;
- final configuration snapshot.

### Status
**IN PROGRESS.** Architecture defined; implementation and test evidence remain.

### Gate
No flight release without positive mission verification and all mandatory readiness gates.

---

## PHASE 14 — Flight execution

### Objective
Operate one or many UAVs with minimum operator workload and clear supervisory control.

### Required
- telemetry;
- flight mode;
- health;
- battery/energy;
- C2 state;
- mission progress;
- route/profile;
- alerts;
- equipment/video;
- operator commands;
- multi-UAV coordination;
- data aggregation;
- event timeline.

### Status
**IN PROGRESS.** UI/operational concepts are substantially defined; real integration and flight validation remain.

### Gate
Representative aircraft completes mission under BlueSky supervision with all critical state/event data captured.

---

## PHASE 15 — Contingency / degraded operations

### Objective
Handle abnormal conditions without ambiguous responsibility between BlueSky and the onboard autopilot.

### Required scenarios
- C2 loss;
- telemetry loss;
- video loss;
- GNSS degradation/loss;
- weather deterioration;
- aircraft health degradation;
- battery/energy limit;
- route/airspace conflict;
- equipment failure;
- operator intervention;
- communication recovery;
- return-to-home / alternate return policy;
- mission resume/replan where supported.

### Status
**IN PROGRESS.** Core philosophy is defined; complete state machines, policies and verification evidence remain.

### Gate
Every defined contingency has a deterministic owner, trigger, action, timeout and recovery/termination condition.

---

## PHASE 16 — Landing / mission completion

### Objective
Close the operational mission safely and unambiguously.

### Required
- landing/RTL state;
- mission completion detection;
- aircraft state confirmation;
- C2 confirmation;
- equipment recording closure;
- data integrity check;
- log availability;
- Flight Record closure trigger.

### Status
**IN PROGRESS.**

### Gate
Mission cannot be closed as successful without the required completion evidence.

---

## PHASE 17 — Post-flight / Flight Record / data

### Objective
Create the authoritative digital record of the flight.

### Required
- telemetry logs;
- autopilot logs;
- mission version;
- actual trajectory;
- commands/ACKs;
- alerts/events;
- C2 events;
- equipment/video references;
- regulatory references;
- configuration snapshot;
- software/adapter versions;
- timestamps;
- operator actions;
- anomalies;
- result/status.

### Status
**IN PROGRESS.** Log/Flight Record architecture exists; complete data model, ingestion and replay implementation remain.

### Gate
A flight can be reconstructed sufficiently for operational review, incident investigation and regulatory/customer records.

---

## PHASE 18 — Maintenance / resources / fleet readiness

### Objective
Return post-flight information into fleet readiness.

### Required
- battery cycles/health;
- engine/resource counters where applicable;
- component hours/cycles;
- faults;
- maintenance actions;
- inspections;
- configuration changes;
- aircraft availability state.

### Status
**IN PROGRESS.** Maintenance integration is identified; complete implementation and workflow remain.

### Gate
Aircraft status for the next mission reflects actual post-flight condition and maintenance restrictions.

---

## PHASE 19 — Validation / verification / release

### Objective
Prove that BlueSky works as an integrated system, not merely as isolated software modules.

### Validation chain

```text
REQUIREMENTS
    ↓
UNIT TEST
    ↓
INTEGRATION TEST
    ↓
SIL / SITL
    ↓
HIL
    ↓
BENCH / GROUND
    ↓
REAL UAV
    ↓
OPERATIONAL TEST
    ↓
RELEASE
```

### Required
- automated tests;
- interface conformance tests;
- SITL/SIL scenarios;
- HIL;
- real UAV tests;
- C2 loss/recovery tests;
- mission upload/read-back tests;
- equipment tests;
- regulatory workflow tests;
- log replay;
- regression suite;
- requirements traceability;
- evidence archive.

### Status
**NOT DONE — framework documented; evidence-producing validation system must be built.**

### Gate
Operational release only after defined requirements and integration tests pass with retained evidence.

---

# 5. Master status snapshot

| Domain | Current state | Next required work |
|---|---|---|
| Product concept / boundary | DONE-DOC | controlled requirements baseline |
| System architecture | DONE-DOC | architecture review |
| Universal integration principle | DONE-DOC | enforce in all modules |
| Autopilot abstraction | IN PROGRESS | API + adapter implementation |
| ArduPilot integration | IN PROGRESS | reference adapter + SITL + real UAV |
| PX4 integration | IN PROGRESS | reference adapter + SITL + real UAV |
| OEM integration | IN PROGRESS | generic adapter contract + first OEM |
| C2 multi-link | IN PROGRESS | state machine + failover implementation |
| Vehicle profiles | IN PROGRESS | executable profile/configuration model |
| Equipment integration | IN PROGRESS | first equipment adapters |
| Video | IN PROGRESS | transport/recording integration |
| GNSS/RTK | IN PROGRESS | source adapters + fallback |
| GIS/terrain | IN PROGRESS | source/version/freshness model |
| Weather | IN PROGRESS | provider adapters + validation |
| Traffic | IN PROGRESS | source adapters + operational logic |
| ATM/FPL/authorization | IN PROGRESS | jurisdiction-specific production adapters |
| Mission planning | IN PROGRESS | implementation + validation |
| Optimization | IN PROGRESS | calculation engine + test set |
| AI/Corrections | IN PROGRESS | agent + governance + verification |
| Pre-flight readiness | IN PROGRESS | executable gates |
| Flight execution | IN PROGRESS | autopilot/C2 integration |
| Contingency | IN PROGRESS | complete state machines + tests |
| Logs / Flight Record | IN PROGRESS | canonical data model + replay |
| Maintenance | IN PROGRESS | workflow implementation |
| SIL/SITL | IN PROGRESS | integrated scenarios |
| HIL | NOT DONE | HIL environment |
| Real UAV validation | NOT DONE | representative aircraft test program |
| Regression/evidence system | NOT DONE | automated evidence pipeline |
| Certification evidence | IN PROGRESS | requirements-to-test traceability |

---

# 6. What must NOT happen

1. Do not build BlueSky directly around ArduPilot commands.
2. Do not make the customer responsible for integration work.
3. Do not declare compatibility because a connection succeeds.
4. Do not release a mission based only on successful upload; perform read-back and semantic verification.
5. Do not treat C2, equipment, ATM, weather, traffic or logs as optional add-ons to the lifecycle.
6. Do not allow AI to bypass deterministic safety gates.
7. Do not move to certification evidence while the operational lifecycle has unresolved interface ownership.
8. Do not create separate parallel architectures for different autopilots; use one universal internal contract and adapters.

---

# 7. Definition of DONE for BlueSky PRO

BlueSky PRO reaches the complete-product gate when a customer's representative fleet can pass through the complete lifecycle:

**REGISTER → CONFIGURE → PLAN → VALIDATE → AUTHORIZE → READY → CONNECT → TRANSFER → VERIFY → FLY → MONITOR → HANDLE CONTINGENCY → LAND/RTL → COLLECT DATA → CLOSE FLIGHT RECORD → MAINTAIN → READY FOR NEXT MISSION**

and the same critical interfaces are covered by repeatable verification in simulation, HIL where applicable, and real aircraft testing.

The final product shall hide protocol complexity from the operator while retaining full technical traceability underneath.
