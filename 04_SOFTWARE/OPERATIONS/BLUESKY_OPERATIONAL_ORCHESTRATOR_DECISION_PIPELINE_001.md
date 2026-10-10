# BlueSky PRO — Operational Orchestrator Decision Pipeline 001

**Status:** IMPLEMENTATION BASELINE — P0 composition contract  
**Scope:** Composition of runtime gating, authority resolution, next-action selection and decision evidence

## 1. Purpose

Provide one deterministic composition point for the already-baselined Operational Orchestrator contracts. The pipeline coordinates existing semantics; it does not replace or redefine them.

## 2. Processing boundary

```text
RUNTIME DECISION CONTEXT
        ↓
RUNTIME ORDINARY-SELECTION GATE
        ↓
ACTION GRAPH / ACTIONABLE SET
        ↓
AUTHORITY + AUTOMATION RESOLUTION
        ↓
DETERMINISTIC NEXT-ACTION SELECTION
        ↓
DECISION EVIDENCE
```

The Runtime State Machine remains authoritative for execution state, event classification, recovery and replanning triggers.

## 3. Composition rules

1. Runtime gating is evaluated before ordinary action selection.
2. Existing `eligible_next_actions` and `select_next_action` provide action eligibility and deterministic ordering.
3. Existing authority/automation resolution remains the sole authority decision mechanism.
4. A human-required result is preserved as an operational outcome; the pipeline does not convert it into automatic execution.
5. Decision evidence records the selected/considered action and resolved authority/automation.
6. No action is executed by this pipeline.

## 4. Runtime gate

Ordinary selection is blocked when the RuntimeDecisionContext is stale, inconsistent, incomplete, requires immediate safety/recovery action, or fails the authoritative energy gate. The pipeline does not invent an alternative continuation in these cases.

## 5. Evidence

The pipeline populates the existing DecisionEvidence structure. It does not create a second evidence model. Runtime-blocked and no-selection outcomes remain valid evidence outcomes.

## 6. Non-goals

- no second runtime state machine;
- no FCS/autopilot commands;
- no C2 command implementation;
- no safety/regulatory gate bypass;
- no numerical Energy Model parameters;
- no mission-package upload;
- no UI orchestration semantics;
- no real-UAV, HIL or certification claim.

## 7. Verification

ARCH-OPS-011 verifies composition only through deterministic C++ tests covering runtime blocking, executable selection and preservation of human-required actions. It does not claim subsystem integration or operational verification.
