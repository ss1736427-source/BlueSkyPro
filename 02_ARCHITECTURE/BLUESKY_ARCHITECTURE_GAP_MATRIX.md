# BlueSky PRO — Architecture GAP Matrix

**Status:** BASELINE — controlled integration audit

## 1. Purpose

This matrix checks whether the BlueSky product lifecycle has a continuous path from mission intent to real UAV execution and back to verified operational evidence.

A document describing an interface is not treated as implementation. Each capability progresses through:

`SPECIFIED → CONTRACT BASELINED → IMPLEMENTED → INTEGRATED → TESTED → VERIFIED`

## 2. End-to-end control chain

```text
MISSION INTENT
   ↓
MISSION MODEL
   ↓
OBJECTIVE / TASK INTERPRETATION
   ↓
CAPABILITY + RESOURCE DISCOVERY
   ↓
TASK / COVERAGE MODEL
   ↓
ALGORITHM ORCHESTRATION
   ↓
CANDIDATE SOLUTIONS
   ↓
SAFETY + REGULATORY + ENERGY GATE
   ↓
OPERATIONAL ORCHESTRATOR
   ↓
READINESS / AUTHORIZATION / RELEASE
   ↓
C2
   ↓
AUTOPILOT ADAPTER
   ↓
MISSION PACKAGE
   ↓
OPERATIONAL VALIDATION
   ↓
READY FOR FLIGHT
   ↓
UAV EXECUTION
   ↓
TELEMETRY / EVENTS / LOGS
   ↓
RUNTIME STATE + NEXT-ACTION DECISION
   ↓
REPLANNING / RECOVERY
   ↓
RESULT VERIFICATION
   ↓
VIRTUAL ↔ REAL ↔ REPLAY ANALYSIS
   ↓
INTERNAL EXPERIENCE / CORRECTIONS
   ↓
EXTERNAL / TECHNOLOGY / COMPETITIVE INTELLIGENCE
   ↓
BENCHMARK / GAP / OPPORTUNITY ANALYSIS
   ↓
HYPOTHESIS / EXPERIMENT / VALIDATION
   ↓
APPROVED MODEL / ALGORITHM / KNOWLEDGE
   ↓
NEXT PREDICTION / PLANNING CYCLE
```

## 3. Current GAP classification

| Capability | Architectural contract | Implementation | Integration / verification | Priority |
|---|---|---|---|---|
| Mission Model | BASELINED | IMPLEMENTED | pending | P0 |
| Mission Objective Profiles | BASELINED | IMPLEMENTED | pending | P0 |
| Vehicle Capability Model | BASELINED | IMPLEMENTED | pending | P0 |
| Payload Capability Model | BASELINED | IMPLEMENTED | pending | P0 |
| Algorithm Orchestration | BASELINED | IMPLEMENTED | pending | P0 |
| **Operational Orchestrator** | **BASELINED** | **IMPLEMENTED** | **pending** | **P0** |
| **Dynamic Readiness / Action Graph** | **existing requirements + additive orchestration boundary** | **IMPLEMENTED** | **pending** | **P0** |
| **Resource/mission adaptation decision path** | **BASELINED by orchestration policy** | **IMPLEMENTED** | **pending** | **P0** |
| Universal Autopilot API | BASELINED | **IMPLEMENTED** | pending | P0 |
| ArduPilot adapter | required | **IMPLEMENTED** | pending | P0 |
| PX4 adapter | required | **IMPLEMENTED** | pending | P0 |
| OEM adapter framework | required | pending | pending | P0 |
| C2 abstraction | BASELINED | pending | pending | P0 |
| Channel management / failover | BASELINED | pending | pending | P0 |
| Mission Package | BASELINED | pending | pending | P0 |
| Mission upload + read-back verification | BASELINED | pending | pending | P0 |
| Regulatory adapter layer | BASELINED concept | pending | pending | P0 |
| FPL / authorization workflows | required | pending | pending | P0 |
| SIL | specified | pending | pending | P0 |
| HIL | specified | pending | pending | P0 |
| Automated pre-flight validation | specified | pending | pending | P0 |
| Telemetry normalization | BASELINED | pending | pending | P0 |
| Runtime state machine | specified | pending | pending | P0 |
| Recovery / failsafe coordination | specified | pending | pending | P0 |
| Replanning | specified | pending | pending | P0 |
| Mission result verification | **additive contract required by operational orchestration** | pending | pending | P0 |
| Intelligence Core / Intelligence Contract | BASELINED | pending | pending | P1 |
| Internal operational experience | BASELINED | pending | pending | P1 |
| Virtual/Real/Actual-condition Replay comparison | BASELINED | pending | pending | P1 |
| Difference + cause analysis | BASELINED | pending | pending | P1 |
| External Technology Intelligence | BASELINED | pending | pending | P1 |
| Competitive Intelligence | BASELINED | pending | pending | P1 |
| Comparison / Benchmark Engine | BASELINED | pending | pending | P1 |
| Hypothesis / Experiment lifecycle | BASELINED | pending | pending | P1 |
| Controlled learning / model promotion | BASELINED | pending | pending | P1 |
| Knowledge provenance / governance | BASELINED | pending | pending | P1 |
| Log / replay | specified | pending | pending | P1 |
| Predicted-vs-actual analysis | specified | pending | pending | P1 |
| Corrections / model learning | specified | pending | pending | P1 |

## 4. Remaining P0 implementation gaps

The following P0 items remain implementation or integration blockers after the operational-orchestrator and reference-autopilot-adapter work. They are not considered closed merely because an architectural contract exists.

- OEM adapter framework
- C2 abstraction
- Channel management / failover
- Mission Package
- Mission upload + read-back verification
- Regulatory adapter layer
- FPL / authorization workflows
- SIL
- HIL
- Automated pre-flight validation
- Telemetry normalization
- Runtime state machine
- Recovery / failsafe coordination
- Replanning
- Mission result verification

Universal Autopilot API, ArduPilot reference adapter, PX4 MAVLink2 reference adapter, Operational Orchestrator, Dynamic Readiness / Action Graph, Resource/mission adaptation decision path, and Mission Objective Profiles are implemented at the current contract/test level; their integration and verification remain pending.

### 4.1 Dependency-audit conclusion

The reference autopilot abstraction chain is now executable at contract level for the Universal API, ArduPilot and PX4. The PX4 reference adapter was closed by ARCH-OPS-014 after its adapter consistency, configure, build and test gates passed. No duplicate Universal API implementation is required before proceeding.

The next implementation work shall therefore be selected from the remaining P0 chain based on dependency order, not simply by the order of the GAP list. In particular, the C2 abstraction, Mission Package, telemetry normalization and runtime state-machine boundaries must be checked for prerequisite relationships before introducing mission upload/read-back or real integration behavior.

This audit does **not** grant integration, SIL/HIL, real-UAV or certification verification status.

## 5. Additive orchestration gaps

The following are architectural additions, not replacements for existing contracts:

### 5.1 Task objective → acceptable outcomes

The mission objective shall be represented independently from the selected route, vehicle allocation and algorithm. A mission may define multiple acceptable outcomes and operational priorities. The orchestrator evaluates candidate solutions against mandatory acceptance conditions and then optimizes applicable objectives.

### 5.2 Resource availability → mission adaptation

Resource selection shall be able to identify unavailable/available UAVs and equipment and feed that state into planning. If the preferred resource is unavailable, the system shall evaluate feasible alternatives including another UAV, multiple UAVs, changed route/profile, task splitting or multiple sorties where permitted.

### 5.3 Preparation → dynamic next action

Preparation shall be represented as a dependency-aware graph. Independent actions may proceed in parallel; blocked actions remain unavailable until prerequisites are satisfied. The orchestrator selects the next actionable step or coordinated parallel set rather than exposing a static checklist.

### 5.4 Role-aware execution

The common preparation graph shall support one person performing multiple roles or several people performing distributed roles. Each role receives only its applicable human actions while the system retains the global readiness state.

### 5.5 Automation → human authority

Actions shall be classified as automatable, human-guided or human-decision. Automation is permitted only within an explicit authority boundary. Mandatory configuration and safety/regulatory constraints cannot be silently changed.

### 5.6 Mission adaptation envelope

Active mission adaptation shall distinguish pre-authorized automatic changes from changes requiring human decision. The adaptation envelope shall be traceable to mission authorization, safety policy and verified vehicle/FCS capability.

### 5.7 Next-action selection objective

When several actions are executable, selection shall advance successful mission completion while respecting hard safety/regulatory constraints, protected energy reserve, task quality and operational timing/resource constraints.

### 5.8 Result-based completion

`LANDED` and `MISSION COMPLETE` shall remain distinct states. Mission completion requires the mission-specific result, data integrity and quality/output acceptance conditions to be satisfied.
