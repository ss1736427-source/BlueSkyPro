# ARCH-OPS-015 — P0 Dependency Audit after PX4 Reference Adapter

**Status:** AUDIT BASELINE

## Purpose

Reconcile the controlled Architecture GAP Matrix after ARCH-OPS-014 and determine the next implementation boundary without duplicating already implemented contracts or advancing into integration prematurely.

## Verified state entering the audit

- Mission Model — implemented at current contract/test level.
- Mission Objective Profiles — implemented at current contract/test level.
- Vehicle Capability Model — implemented at current contract/test level.
- Payload Capability Model — implemented at current contract/test level.
- Algorithm Orchestration — implemented at current contract/test level.
- Operational Orchestrator — implemented at current contract/test level.
- Dynamic Readiness / Action Graph — implemented at current contract/test level.
- Resource/mission adaptation decision path — implemented at current contract/test level.
- Universal Autopilot API — implementation baseline exists; no duplicate implementation required.
- ArduPilot reference adapter — executable contract-level implementation exists.
- PX4 MAVLink2 reference adapter — ARCH-OPS-014 implementation is merged and its contract/build/test gates passed.

## Remaining P0 boundaries

1. OEM adapter framework
2. C2 abstraction
3. Channel management / failover
4. Mission Package
5. Mission upload + read-back verification
6. Regulatory adapter layer
7. FPL / authorization workflows
8. SIL
9. HIL
10. Automated pre-flight validation
11. Telemetry normalization
12. Runtime state machine
13. Recovery / failsafe coordination
14. Replanning
15. Mission result verification

## Dependency interpretation

The remaining work must be ordered by executable dependency rather than by the visual order of the GAP Matrix.

The operational path requires a stable boundary for external vehicle control, communications, normalized runtime observations and the versioned mission artifact before upload/read-back and runtime execution behavior can be integrated coherently.

Therefore the next planning pass shall explicitly trace:

```text
C2 abstraction
   +
Channel management / failover
   ↓
Telemetry normalization
   ↓
Runtime state machine
   ↓
Mission Package
   ↓
Upload + read-back verification
   ↓
Operational validation / SIL / HIL
```

This is a dependency hypothesis, not a claim that every item above must be implemented in exactly that sequence. The next implementation PR shall be chosen only after the repository contracts for these boundaries are inspected and prerequisite blockers are identified.

## Boundary rules

- Do not duplicate the Universal Autopilot API.
- Do not add MAVLink wire/transport code to BlueSky Core.
- Do not introduce numerical Energy Model parameters.
- Do not introduce UI semantics into the orchestration core.
- Do not claim SIL/HIL/real-UAV verification from fixture or contract tests.
- Do not advance to real integration while prerequisite contracts are unresolved.
- Preserve planned-vs-actual separation and Runtime ownership of execution state transitions.

## Exit criterion

ARCH-OPS-015 is complete when the GAP Matrix accurately reflects the implemented reference adapter chain and the next P0 implementation boundary is selected from a dependency trace rather than stale GAP ordering.
