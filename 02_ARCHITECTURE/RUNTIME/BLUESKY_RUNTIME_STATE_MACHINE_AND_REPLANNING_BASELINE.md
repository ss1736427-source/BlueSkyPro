# BlueSky PRO — Runtime State Machine and Replanning Baseline

**Status:** ARCHITECTURE BASELINE — P0 integration contract

## 1. Purpose

Define deterministic behaviour from mission release through execution, degradation, replanning, recovery and completion. The runtime consumes normalized data from C2, vehicle/autopilot, payload, environment and regulatory services and must never assume that the planned state equals the actual aircraft state.

## 2. Runtime chain

```text
RELEASED MISSION PACKAGE
        ↓
EXECUTION PREPARATION
        ↓
READY
        ↓
EXECUTING
        ↓
MONITORING
   ┌────┼───────────────┐
   ↓    ↓               ↓
NORMAL  DEGRADATION   EMERGENCY
   ↓    ↓               ↓
   └→ REPLAN / RECOVER ←┘
          ↓
      VALIDATION
          ↓
       RESUME
          ↓
 COMPLETED / ABORTED
```

## 3. Authoritative state

The runtime shall maintain separate but correlated states for:

- mission;
- vehicle;
- autopilot;
- C2;
- payload;
- navigation;
- energy;
- regulatory constraints;
- active mission package.

Actual vehicle/autopilot state is authoritative for execution status. BlueSky planning state is authoritative for the approved intended mission. Differences between them shall be detected and reconciled.

## 4. Core states

Minimum mission execution states:

`RELEASED → PREPARING → READY → EXECUTING → PAUSED/DEGRADED → REPLANNING → VALIDATING → RESUMING → COMPLETED/ABORTED`

Emergency handling may transition directly to an approved recovery/abort path.

## 5. Event-driven transitions

Transitions shall be triggered by defined events rather than by arbitrary UI actions.

Examples:

- C2 degradation/loss/recovery;
- significant wind change;
- energy prediction crossing a threshold;
- battery/propulsion anomaly;
- navigation degradation;
- obstacle or restricted-area change;
- payload failure;
- vehicle state deviation;
- missed waypoint or timing condition;
- multi-UAV conflict/degradation;
- regulatory/authorization change.

## 6. Replanning trigger policy

Not every telemetry change shall trigger replanning.

The runtime shall distinguish:

**Monitor-only change** — update prediction/state, no new plan.

**Recalculation change** — update calculations while retaining the current executable route if still valid.

**Replanning trigger** — current plan may no longer satisfy the required objective or constraints.

**Immediate safety action** — execute approved recovery/failsafe behaviour without waiting for full optimization.

This prevents the optimizer from creating unnecessary computational load or route oscillation.

## 7. Wind update principle

The planned mission is initially optimized using available environmental information. Current wind data is then applied to the calculated solution and, when material, the route is recalculated using the actual/latest wind field.

```text
TASK
 ↓
OPTIMIZE ROUTE / OPTIONS
 ↓
ALLOCATE UAVs
 ↓
APPLY CURRENT WIND
 ↓
RECALCULATE
 ↓
ENERGY + SAFETY + QUALITY GATE
 ↓
RELEASE
```

During execution, new wind observations update the prediction. Replanning occurs only when the change is operationally material.

## 8. Energy gate

Energy is a mandatory feasibility constraint, not merely a preference.

The runtime shall continuously compare:

`estimated remaining usable energy` versus `energy required for current plan + approved recovery + mandatory reserve`.

If the margin becomes insufficient, the current mission plan becomes invalid for continued execution and the runtime shall enter the appropriate recovery/replanning state.

## 9. Replanning workflow

```text
TRIGGER
  ↓
CAPTURE ACTUAL STATE
  ↓
FREEZE SAFETY-CRITICAL FACTS
  ↓
UPDATE ENVIRONMENT / VEHICLE STATE
  ↓
ALGORITHM ORCHESTRATOR
  ↓
CANDIDATE PLAN(S)
  ↓
HARD SAFETY / REGULATORY / ENERGY GATE
  ↓
MISSION OBJECTIVE EVALUATION
  ↓
NEW MISSION PACKAGE VERSION
  ↓
VALIDATE
  ↓
TRANSFER
  ↓
READ-BACK / CONFIRM
  ↓
RESUME
```

## 10. No-plan condition

If no admissible continuation exists, BlueSky shall not force an optimized answer. It shall select the approved recovery/abort procedure applicable to the current vehicle and operational context.

## 11. C2 interaction

If C2 is degraded, runtime behaviour depends on the actual communication state:

- with usable control: continue/replan according to policy;
- with insufficient control: stop issuing unconfirmed commands and rely on vehicle failsafe behaviour;
- after recovery: read actual vehicle and mission state before resuming or changing the plan.

## 12. Autopilot interaction

Runtime communicates with the autopilot only through the universal Autopilot Adapter.

The adapter provides normalized:

- vehicle state;
- navigation state;
- mission progress;
- command result;
- failsafe state;
- capability/state changes.

Native protocol semantics remain outside the runtime core.

## 13. Multi-UAV runtime

Each UAV has an independent execution state. The parent mission coordinator maintains the aggregate state.

A single-UAV failure may trigger:

- reassignment of remaining subtasks;
- coordinated replanning;
- mission degradation;
- mission termination.

The decision shall respect the same hard safety and energy gates.

## 14. Algorithm orchestration at runtime

The runtime shall not invoke the full optimization stack for every event. A trigger classification determines the minimum required computation.

```text
EVENT
 ↓
CLASSIFY IMPACT
 ├─ LOW      → update only
 ├─ MEDIUM   → local recalculation
 └─ HIGH     → full replanning
```

The orchestrator may use different algorithms for different subtasks and vehicles. Computational cost is itself a planning-system constraint, but it may never justify violation of a mandatory safety condition.

## 15. Explainable operational decision

When the selected route differs materially from the previous route, the pilot receives a short operational explanation, for example:

> Route changed: wind increased on the direct leg; the new route preserves energy reserve and required inspection coverage.

The explanation shall describe the principal operational reason, not expose unnecessary algorithmic internals.

## 16. State consistency and version control

Every active execution shall reference exactly one active mission package version. A replacement package shall have a new version and shall not silently mutate the active package.

The runtime shall correlate:

- package version;
- vehicle state;
- telemetry timestamps;
- environmental inputs;
- regulatory inputs;
- replanning event;
- validation result;
- operator actions.

## 17. Acceptance criteria

The runtime/replanning contract is complete when:

1. all mission execution states are explicit;
2. state transitions are event-driven and deterministic;
3. planned and actual states are distinguishable;
4. wind changes can trigger material recalculation/replanning;
5. energy reserve is continuously enforceable as a hard gate;
6. C2 degradation/loss/recovery has defined transitions;
7. autopilot state is obtained through the adapter;
8. replanning produces a new validated Mission Package version;
9. read-back/confirmation precedes resumption where required;
10. no admissible plan causes transition to the approved recovery/abort path;
11. multi-UAV execution states remain independent and coordinated;
12. all safety-critical transitions are traceable and testable.

## 18. Verification sequence

The implementation shall be verified progressively:

`unit state-transition tests → planner/replanner tests → SIL → C2 fault injection → autopilot integration → HIL → real UAV`

Evidence shall be retained against the exact software, model and mission-package versions used.
