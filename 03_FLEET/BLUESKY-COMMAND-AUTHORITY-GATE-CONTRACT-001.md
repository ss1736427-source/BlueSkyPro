---
id: BLUESKY-COMMAND-AUTHORITY-GATE-CONTRACT-001
type: command_authority_gate_contract
status: controlled_working_draft
system: BlueSky PRO
lifecycle_contract: BLUESKY-COMMAND-LIFECYCLE-CONTRACT-001
registry_boundary: BLUESKY-ADAPTER-REGISTRY-BOUNDARY-001
implementation_stub: 04_SOFTWARE/PLANNING/command_authority_gate_contract_stub.hpp
test_stub: 04_SOFTWARE/PLANNING/command_authority_gate_contract_stub_test.cpp
---

# BlueSky PRO — Command Authority Gate Contract 001

## 1. Purpose

Define the technology-neutral boundary between command lifecycle processing and the authority/safety decision required before an external command may be dispatched.

This contract defines an interface and safety boundary only. It does not define product-specific safety limits, regulatory policy, mission release criteria, or operational approval rules.

## 2. Boundary

```text
Command Lifecycle
    ↓
AUTHORITY / SAFETY GATE
    ↓
AdapterServiceResolver / Adapter
    ↓
External Vehicle / Equipment
```

The authority/safety gate is upstream of adapter dispatch. Registry lookup and lifecycle processing shall not grant command execution authority. The Registry boundary remains a separate compatibility/lookup boundary defined by `BLUESKY-ADAPTER-REGISTRY-BOUNDARY-001`.

## 3. Decision vocabulary

The gate shall expose a deterministic decision with three canonical outcomes:

```text
ALLOWED
REJECTED
DEFERRED
```

`ALLOWED` means the gate has positively authorized the specific command request for the evaluated context.

`REJECTED` means the gate has determined that the command shall not be dispatched.

`DEFERRED` means the gate cannot establish an applicable authorization decision at the current evaluation point. It shall not be interpreted as permission to dispatch.

## 4. Decision identity

Each evaluation shall preserve:

```text
command_id
mission_id
target_vehicle_id
command_class
authority_source
policy_version
evaluated_at_ms
correlation_id
```

The decision shall be traceable to the command request and evaluation context.

## 5. Mandatory invariants

The integration chain shall preserve these rules:

```text
REJECTED  → no adapter dispatch
DEFERRED  → no adapter dispatch
ALLOWED   → permits progression to dispatch, subject to the remaining command lifecycle rules
```

`ACKNOWLEDGED`, `EXECUTING` and `COMPLETED` shall never be treated as substitutes for an `ALLOWED` authority decision.

The adapter, Registry and command lifecycle shall not bypass the authority/safety gate.

## 6. Separation of concerns

The authority/safety gate owns the authorization decision boundary. It does not own:

- adapter discovery;
- adapter compatibility resolution;
- protocol-specific command translation;
- Vehicle acknowledgement;
- execution confirmation.

Those functions remain in their respective controlled boundaries.

## 7. Safety-policy neutrality

The contract intentionally leaves the following as external controlled inputs or policies:

- applicable safety constraints;
- regulatory restrictions;
- mission release rules;
- operator authority model;
- operational mode;
- vehicle state limits;
- environmental constraints;
- configuration validity.

No new policy is implied by this interface.

## 8. Error semantics

A gate evaluation failure to establish a valid authorization result shall not silently become `ALLOWED`.

Where the higher-level system cannot establish a safe decision, the result shall be represented as `DEFERRED` or another controlled non-dispatch condition defined by the applicable policy.

## 9. Verification boundary

Current implementation status:

```text
contract = DEFINED
implementation = STUB
real execution = NOT EXECUTED
real hardware = NOT CONNECTED
real-flight evidence = NOT AVAILABLE
verification = UNVERIFIED
```

The software contract test verifies only the deterministic interface invariants. It is not evidence of operational authorization performance.

## 10. Integration handoff

The controlled command chain is:

```text
Command Request
→ Command Lifecycle
→ Authority / Safety Gate
→ Adapter Service Resolver
→ Adapter
→ External Vehicle / Equipment
→ acknowledgement / execution state
```

The gate is a mandatory boundary before dispatch. A resolved adapter is not an authorized adapter command, and a valid lifecycle state is not an authorization decision.

## 11. Change rule

Changes to the canonical authority boundary shall follow:

```text
GAP
→ IMPACT ANALYSIS
→ CONTRACT CHANGE
→ TEST UPDATE
→ IMPLEMENTATION UPDATE
→ TRACEABILITY UPDATE
```

**Status: CONTROLLED WORKING DRAFT — TECHNOLOGY-NEUTRAL AUTHORITY/SAFETY DISPATCH GATE DEFINED; PRODUCT SAFETY POLICY AND REAL INTEGRATION EVIDENCE PENDING.**
