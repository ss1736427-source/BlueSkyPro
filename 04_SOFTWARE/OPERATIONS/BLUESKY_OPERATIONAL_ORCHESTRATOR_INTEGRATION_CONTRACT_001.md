# BlueSky PRO — Operational Orchestrator Integration Contract 001

**Status:** IMPLEMENTATION BASELINE — P0 integration contract
**Parent architecture:** `02_ARCHITECTURE/BLUESKY_OPERATIONAL_ORCHESTRATOR_ENERGY_POLICY_BASELINE.md`
**Lifecycle:** `02_ARCHITECTURE/BLUESKY_END_TO_END_OPERATIONAL_LIFECYCLE_BASELINE.md`

## 1. Purpose

Define the implementation boundary for the BlueSky PRO Operational Orchestrator.

The Operational Orchestrator is the coordination layer for the full UAV mission lifecycle. It consumes authoritative contracts from existing subsystems and determines the next executable operational action without taking ownership of subsystem algorithms or onboard flight-control loops.

This document is an implementation contract. It does not replace existing Mission Model, Objective Profiles, Capability Model, Algorithm Orchestration, C2, Runtime State Machine, Mission Package, Validation, Energy Model or Intelligence contracts.

## 2. Architectural position

```text
MISSION / TASK
      ↓
OPERATIONAL ORCHESTRATOR
      ├── Mission Model
      ├── Objective Profiles
      ├── Capability / Resources
      ├── Planning / Algorithm Orchestrator
      ├── Weather / Environment
      ├── Safety / Regulatory
      ├── Energy
      ├── Readiness / Action Graph
      ├── C2
      ├── Runtime State Machine
      ├── Mission Package
      ├── Result Verification
      └── Evidence / Traceability
               ↓
        FCS INTEGRATION
        ├── ArduPilot
        ├── PX4
        └── OEM FCS
               ↓
              UAV
```

ArduPilot, PX4 and OEM FCS remain execution contours. The Operational Orchestrator does not become a second flight controller.

## 3. Inputs

The orchestrator shall consume normalized, versioned state and contract outputs from:

- mission and task definition;
- mission objective profile;
- vehicle and payload capabilities;
- available resources and crew/role state;
- planning and solver results;
- weather and environmental state;
- airspace/regulatory constraints;
- safety state;
- energy state, prediction and uncertainty;
- C2 state;
- runtime state;
- readiness/action graph state;
- mission package/release state;
- telemetry/events/logs;
- mission-result and data-quality state.

The orchestrator shall not silently invent missing authoritative values. Missing or stale inputs become explicit state/constraint conditions.

## 4. Normalized operational state

The implementation shall maintain a deterministic operational snapshot containing, at minimum:

- mission state;
- current objective and required outcome;
- current UAV/resource allocation;
- readiness state;
- blocked and actionable actions;
- active constraints;
- authorization/release state;
- C2 state;
- runtime state;
- energy feasibility state;
- active contingency state;
- mission-result verification state;
- evidence/provenance references;
- orchestrator policy/version.

The snapshot is a coordination view. Subsystem-owned authoritative state remains authoritative in its own contract.

## 5. Action model

Every executable operational action shall have:

- stable action ID;
- action type;
- owning subsystem/role;
- prerequisites;
- blocking conditions;
- authorization requirement;
- automation level;
- safety classification;
- expected state transition;
- completion evidence;
- failure/timeout handling;
- provenance/version.

Supported automation levels:

1. `AUTOMATIC` — BlueSky may execute without human intervention within the authorized policy envelope.
2. `HUMAN_GUIDED` — BlueSky presents the exact required action and verifies completion.
3. `HUMAN_DECISION` — BlueSky prepares evidence/options; an authorized human decision is required.

## 6. Next-action selection

The orchestrator shall continuously derive the actionable set from current state and dependencies.

```text
CURRENT STATE
    ↓
DEPENDENCIES
    ↓
BLOCKED / ACTIONABLE SET
    ↓
AUTOMATABLE / HUMAN-REQUIRED
    ↓
AUTHORITY + SAFETY + REGULATORY GATE
    ↓
FEASIBILITY + ENERGY GATE
    ↓
MISSION OBJECTIVE / RESULT IMPACT
    ↓
TIME / RESOURCE COST
    ↓
NEXT ACTION OR PARALLEL ACTION SET
    ↓
EXECUTE / REQUEST HUMAN ACTION
    ↓
VERIFY COMPLETION
    ↓
UPDATE STATE
```

When multiple independent actions are available, the implementation shall permit parallel execution where dependencies, resources and authority allow it.

The selection policy shall not force the operator to choose an action merely because the action is visible. It shall expose the currently actionable human work and suppress blocked or system-owned work from the primary operator workflow.

## 7. Human authority boundary

The orchestrator shall distinguish:

- actions BlueSky may perform automatically;
- actions requiring human confirmation;
- actions reserved for authorized engineering/service roles;
- actions prohibited from normal operational workflow.

Safety-critical constraints, regulatory prohibitions, validated vehicle limits and authoritative energy gates cannot be overridden by optimization or AI recommendations.

## 8. Resource and mission adaptation

If the preferred resource is unavailable, the orchestrator shall request or initiate a controlled adaptation evaluation.

Candidate adaptations may include:

- alternate UAV;
- alternate payload/equipment;
- multi-UAV allocation;
- task redistribution;
- route/profile change;
- speed/altitude adjustment;
- mission split into sorties;
- schedule change.

An adapted mission must pass the same applicable safety, regulatory, energy, readiness and result-quality gates before release.

## 9. Runtime coordination

During execution the orchestrator consumes the Runtime State Machine rather than duplicating it.

Runtime events may cause:

- continuation;
- local/incremental replanning;
- full replanning;
- resource redistribution;
- contingency activation;
- return/recovery coordination;
- mission termination.

The orchestrator selects the operational strategy. The FCS remains responsible for immediate flight-control execution and onboard safety functions within its validated boundary.

## 10. C2 loss policy interface

The orchestrator shall consume normalized C2 state and apply the approved contingency policy:

```text
C2 LOST
  ↓
RECOVERY POSSIBLE?
  ├─ YES → recovery/continuation policy
  └─ NO
       ↓
SAFE AUTONOMOUS CONTINUATION?
  ├─ YES → continue under approved envelope
  └─ NO
       ↓
RETURN / RECOVERY
       ↓
LAND / APPROVED RECOVERY
```

The implementation shall preserve UAV safety and collected mission data as primary operational objectives, subject to the approved safety/regulatory policy.

## 11. Multi-UAV failure interface

When an active UAV becomes unavailable:

1. mark the resource unavailable;
2. preserve and record collected data;
3. recalculate remaining mission requirements;
4. evaluate redistribution among available resources;
5. recalculate safety/energy/time/result feasibility;
6. continue if the adapted mission is admissible;
7. otherwise enter the configured human-decision or contingency path.

## 12. Mission completion

`LANDED` is not equivalent to `MISSION COMPLETE`.

The orchestrator shall request/consume result verification for:

- required data collected;
- data integrity;
- required coverage/completeness;
- quality criteria;
- required outputs;
- unresolved mission obligations.

Only an accepted result state may transition the operational lifecycle to `MISSION COMPLETE`.

## 13. Evidence and traceability

Every consequential orchestration decision shall be traceable to:

- orchestrator policy/version;
- input state versions;
- selected action/candidate;
- applicable constraints;
- authority context;
- human intervention, if any;
- resulting state transition;
- completion/failure evidence.

This evidence must support replay and predicted-versus-actual analysis.

## 14. Error handling

The implementation shall fail closed for missing mandatory safety/regulatory/authorization/energy information where the applicable contract requires a gate.

The orchestrator shall distinguish at least:

- blocked action;
- unavailable resource;
- stale/invalid input;
- rejected candidate;
- failed execution;
- timeout;
- human decision pending;
- contingency active;
- result verification failed.

These conditions must remain visible in traceable state rather than being silently collapsed into a generic failure.

## 15. Interface invariants

1. Existing authoritative subsystem contracts remain authoritative.
2. The orchestrator coordinates; it does not duplicate subsystem ownership.
3. Hard safety/regulatory/validated energy constraints are feasibility gates.
4. AI/optimization cannot override mandatory gates.
5. Human authority is explicit and auditable.
6. Runtime state is consumed, not reimplemented in the orchestrator.
7. FCS performs immediate flight control; BlueSky performs operational orchestration.
8. Mission completion requires verified mission result, not landing alone.
9. Every consequential decision is replayable from versioned evidence.
10. UI consumes this contract and does not redefine orchestration semantics.

## 16. Implementation decomposition

The first implementation shall be decomposed into independently testable services/components:

- `OperationalStateAggregator`;
- `ActionGraphEvaluator`;
- `ActionEligibilityGate`;
- `AutomationAuthorityResolver`;
- `NextActionSelector`;
- `MissionAdaptationCoordinator`;
- `OperationalDecisionRecorder`;
- `ResultVerificationCoordinator`.

These names are implementation identifiers, not additional product-level modules. The final deployment topology may combine them while preserving the contractual boundaries.

## 17. Acceptance criteria

The integration baseline is accepted when:

1. a normalized operational snapshot can be assembled from authoritative subsystem contracts;
2. actionable and blocked work can be distinguished deterministically;
3. automatic, human-guided and human-decision actions are distinguished;
4. independent actions can execute in parallel where permitted;
5. next-action selection is deterministic and traceable;
6. resource/mission adaptation can be invoked without bypassing safety gates;
7. runtime events can invoke the appropriate continuation/replanning/contingency path;
8. C2-loss coordination follows the approved policy boundary;
9. multi-UAV resource loss is handled without losing mission evidence;
10. landing and mission completion are distinct states;
11. mission-result verification is part of lifecycle closure;
12. all consequential decisions carry sufficient provenance for replay;
13. no subsystem contract is redefined by the orchestrator.

## 18. Implementation sequence / anti-rework rule

Implement in this order:

1. normalized operational snapshot;
2. action graph and dependency evaluation;
3. authority/automation resolution;
4. deterministic next-action selection;
5. operational decision evidence;
6. resource/mission adaptation interface;
7. runtime integration;
8. result verification integration;
9. SIL tests;
10. real-system validation.

Do not introduce UI-specific orchestration logic before items 1–4 are contract-tested.

Do not introduce numerical Energy Model parameters here. Energy values, degradation parameters, reserve thresholds and uncertainty qualification remain governed by the existing Energy Model qualification path.
