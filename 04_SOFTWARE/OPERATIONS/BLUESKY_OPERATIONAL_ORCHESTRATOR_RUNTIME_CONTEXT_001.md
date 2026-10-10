# BlueSky PRO — Operational Orchestrator Runtime Decision Context 001

**Status:** IMPLEMENTATION BASELINE — P0 integration contract
**Scope:** Operational Orchestrator ↔ Runtime State Machine boundary

## 1. Purpose

Define the canonical hand-off of runtime execution facts to the Operational Orchestrator without creating a second runtime state machine.

The Runtime State Machine remains authoritative for execution state, event classification, recovery transitions and replanning triggers. The Operational Orchestrator consumes those facts to select operational strategy and next actions.

## 2. Boundary

```text
ACTUAL UAV / FCS / C2 / ENVIRONMENT
            ↓
     RUNTIME STATE MACHINE
            ↓
   RuntimeDecisionContext
            ↓
 OPERATIONAL ORCHESTRATOR
            ↓
 next action / replanning strategy / recovery coordination
```

The context is a projection of authoritative runtime facts. It is not a second source of mission, vehicle, C2 or execution state.

## 3. Canonical context

The minimum context contains:

- runtime state version;
- active Mission Package version;
- actual vehicle state version;
- runtime freshness indicator;
- active-package consistency indicator;
- runtime coordination mode;
- safety-action-required indicator;
- authoritative energy-gate result;
- C2 operational-state indication.

`Continue`, `Recalculate`, `Replan`, `Recover` and `AwaitRuntime` are coordination modes. Their underlying state-transition semantics remain owned by Runtime.

## 4. Selection gate

Ordinary Operational Orchestrator selection is permitted only when:

1. the runtime context is current;
2. runtime, package and actual-vehicle references are present;
3. the active package is consistent with runtime state;
4. no immediate safety/recovery action is required;
5. the authoritative energy gate permits continued ordinary orchestration.

A blocked ordinary-selection gate does not authorize the orchestrator to invent a continuation. Runtime/recovery policy remains authoritative.

## 5. Replanning boundary

When Runtime classifies an event as requiring recalculation or replanning, the orchestrator consumes that classification and coordinates the applicable planning/adaptation path. It does not duplicate event thresholds or transition rules.

```text
RUNTIME EVENT
 ↓
RUNTIME CLASSIFICATION
 ↓
RUNTIME DECISION CONTEXT
 ↓
ORCHESTRATOR STRATEGY
 ├─ continue
 ├─ recalculate
 ├─ replan
 └─ coordinate recovery
```

## 6. Package/state consistency

Every active execution references one active Mission Package version. Runtime supplies the authoritative active-package reference and actual vehicle-state reference. The orchestrator must not treat planned state as actual state.

A replacement Mission Package is versioned and validated before it can become the active package according to the existing Runtime contract.

## 7. Energy boundary

This contract consumes the authoritative Energy Model result as a gate. It introduces no numerical reserve, degradation, consumption, wind or uncertainty parameters.

## 8. C2 boundary

The context reports normalized C2 operational state. C2 loss/recovery semantics remain governed by the existing runtime and C2 contracts. The orchestrator does not issue low-level C2 commands through this context.

## 9. Safety and authority invariants

- Runtime owns execution state transitions.
- Autopilot/FCS owns immediate flight-control execution within its validated boundary.
- Operational Orchestrator owns operational coordination and next-action selection.
- Safety, regulatory and authoritative energy gates cannot be bypassed.
- AI/recommendation logic cannot elevate authority.
- UI does not redefine runtime semantics.
- No stale or inconsistent runtime context may be used for ordinary action selection.
- Recovery conditions are not converted into ordinary optimization decisions.

## 10. Verification scope

ARCH-OPS-010 verifies the contract boundary and deterministic gating only. It does not claim Runtime State Machine implementation, autopilot integration, Energy Model qualification, SIL/HIL or real-UAV verification.

Acceptance evidence is limited to compile/test coverage of context usability and ordinary-selection gating.
