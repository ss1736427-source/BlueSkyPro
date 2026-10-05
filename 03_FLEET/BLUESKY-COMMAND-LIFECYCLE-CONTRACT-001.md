---
id: BLUESKY-COMMAND-LIFECYCLE-CONTRACT-001
type: command_lifecycle_contract
status: controlled_working_draft
system: BlueSky PRO
implementation_stub: 04_SOFTWARE/PLANNING/command_lifecycle_contract_stub.hpp
test_stub: 04_SOFTWARE/PLANNING/command_lifecycle_contract_stub_test.cpp
authority_gate_contract: BLUESKY-COMMAND-AUTHORITY-GATE-CONTRACT-001
---

# BlueSky PRO — Command Lifecycle Contract 001

## 1. Purpose

Define the technology-neutral lifecycle boundary for commands between BlueSky planning/runtime logic and an external Vehicle/Equipment adapter.

This contract is a controlled implementation baseline. It does not constitute operational, HIL, flight-test, or certification evidence.

## 2. Lifecycle

```text
REQUESTED
  ↓
VALIDATING
  ├── REJECTED
  ↓
DISPATCHED
  ↓
ACKNOWLEDGED
  ↓
EXECUTING
  ├── COMPLETED
  ├── FAILED
  ├── CANCELLED
  └── TIMEOUT

UNKNOWN may represent an unresolved execution state and shall not be interpreted as success.
```

## 3. Required distinctions

The implementation shall preserve the distinction between:

```text
command accepted by BlueSky
command validated for dispatch authority
command dispatched to adapter
command acknowledged by Vehicle
command execution confirmed
```

Acceptance, authorization, dispatch, acknowledgement and execution confirmation are separate states/events and shall not be collapsed into a single success indication.

## 4. Command request identity

Each command request shall carry:

```text
command_id
mission_id
target_vehicle_id
command_class
parameters
authority_source
requested_at_ms
correlation_id
```

Identifiers shall remain stable through the lifecycle and must be sufficient for deterministic correlation of responses/events.

## 5. Transition requirements

| Current | Allowed next state(s) | Rule |
|---|---|---|
| REQUESTED | VALIDATING | validation begins |
| VALIDATING | REJECTED, DISPATCHED | reject or pass validation and applicable authority gate |
| DISPATCHED | ACKNOWLEDGED, TIMEOUT, UNKNOWN | external acknowledgement or unresolved outcome |
| ACKNOWLEDGED | EXECUTING, FAILED, CANCELLED, TIMEOUT | execution phase or terminal/timeout outcome |
| EXECUTING | COMPLETED, FAILED, CANCELLED, TIMEOUT, UNKNOWN | execution outcome |
| COMPLETED | — | terminal |
| FAILED | — | terminal |
| CANCELLED | — | terminal |
| TIMEOUT | — | terminal for this lifecycle instance |
| REJECTED | — | terminal |
| UNKNOWN | — | unresolved; no success inference |

The concrete adapter may implement additional protocol-specific intermediate events, but those events shall be mapped to the canonical lifecycle without changing the canonical meaning.

## 6. Safety and authority boundary

The lifecycle contract does not grant command authority.

```text
Command request
→ lifecycle validation
→ authority / safety gate
→ adapter dispatch
→ Vehicle acknowledgement
→ execution state
→ execution confirmation
```

The controlled authority boundary is defined by `BLUESKY-COMMAND-AUTHORITY-GATE-CONTRACT-001`.

An adapter, Registry or lifecycle implementation shall not bypass the applicable authority/safety gate.

`ACKNOWLEDGED` is not equivalent to `EXECUTING` and neither is equivalent to `COMPLETED`. None of these states substitutes for an `ALLOWED` authority decision.

## 7. Error and timeout semantics

Failures shall remain explicit. A timeout shall not be converted to successful completion. `UNKNOWN` shall be used when the system cannot establish the external execution state and shall require the higher-level recovery logic to determine the safe response.

Authority gate `REJECTED` and `DEFERRED` decisions shall not be interpreted as dispatch permission.

## 8. Implementation mapping

The controlled software stub exposes the canonical lifecycle operations required by this contract:

```text
validate()      → VALIDATING
 dispatch()      → DISPATCHED
 acknowledge()   → ACKNOWLEDGED
 execute()       → EXECUTING
 complete()      → COMPLETED
 fail()          → FAILED
 cancel()        → CANCELLED
 timeout()       → TIMEOUT
 unknown()       → UNKNOWN
```

The `execute()` transition is explicit so that `ACKNOWLEDGED` cannot be implicitly treated as execution confirmation.

## 9. Verification boundary

The current implementation is a stub:

```text
implementation = STUB
execution = NOT EXECUTED
real hardware = NOT CONNECTED
real-flight evidence = NOT AVAILABLE
verification = UNVERIFIED
```

The contract test `command_lifecycle_contract_stub_test` is the deterministic software-level verification fixture for the currently defined interface surface. Its execution result shall be recorded separately from real integration evidence.

## 10. Integration handoff

The concrete adapter implementation shall consume this contract through the controlled integration chain:

```text
BLUESKY-ADAPTER-REGISTRY-BOUNDARY-001
→ BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
→ Command Lifecycle Contract
→ Command Authority Gate Contract
→ concrete adapter
```

The registry resolves the compatible adapter boundary; the canonical Vehicle/Equipment schema supplies the domain semantics; the authority/safety gate decides whether the specific command may proceed to dispatch. Neither registration nor lifecycle acknowledgement grants safety or execution authority.

No vendor-specific protocol is selected by this document.

## 11. Change rule

If an actual integration gap requires a change to the canonical lifecycle, the change shall follow:

```text
GAP
→ IMPACT ANALYSIS
→ CONTRACT CHANGE
→ TEST UPDATE
→ IMPLEMENTATION UPDATE
→ TRACEABILITY UPDATE
```

**Status: CONTROLLED WORKING DRAFT — CANONICAL COMMAND LIFECYCLE AND AUTHORITY GATE BOUNDARIES DEFINED; PRODUCT SAFETY POLICY, REAL INTEGRATION AND TEST EVIDENCE PENDING.**
