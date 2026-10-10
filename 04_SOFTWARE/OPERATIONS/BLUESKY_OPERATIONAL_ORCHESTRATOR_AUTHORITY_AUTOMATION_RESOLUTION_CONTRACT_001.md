# BlueSky PRO — Operational Orchestrator Authority / Automation Resolution Contract 001

**Status:** IMPLEMENTATION BASELINE — P0
**Parent:** `04_SOFTWARE/OPERATIONS/BLUESKY_OPERATIONAL_ORCHESTRATOR_INTEGRATION_CONTRACT_001.md`

## 1. Purpose

Define the deterministic boundary between an actionable operational action and the authority required to execute it.

The contract composes the existing action `AutomationLevel` and authorization gates. It does not redefine mission state, readiness, safety, regulatory or energy ownership.

## 2. Resolution sequence

```text
ACTIONABLE ACTION
      ↓
SAFETY / REGULATORY / ENERGY GATES
      ↓
ACTION AUTHORIZATION
      ↓
REQUESTED AUTOMATION LEVEL
      ↓
AUTHORITY CONTEXT
      ↓
EXECUTABLE
  or REQUIRES HUMAN
  or BLOCKED
```

## 3. Authority context

The minimal implementation context distinguishes:

- `system_authorized` — the action is within the approved system automation envelope;
- `role_authorized` — an authorized human role may perform the action;
- `human_decision_granted` — the required explicit human decision has been made.

A boolean context is intentionally used at this stage; concrete identity, role catalogues and authentication services remain owned by the applicable security/identity contracts.

## 4. Resolution semantics

### AUTOMATIC

If the system automation authority is present, the action is executable under `SYSTEM` authority.

If system authority is absent, the action does not silently execute. It becomes `REQUIRES_HUMAN` with a guided human path.

### HUMAN_GUIDED

The system presents and verifies the required human action. A role-authorized human is required. Without role authority, the action is blocked.

### HUMAN_DECISION

The system prepares the decision and evidence. Until the explicit decision is granted, the action remains `REQUIRES_HUMAN`. After the decision is granted, the action is executable under explicit human-decision authority.

## 5. Mandatory gates

Resolution shall fail closed when any applicable mandatory gate fails:

- safety;
- regulatory;
- energy;
- action authorization.

Authority resolution shall never bypass these gates.

## 6. Boundary rules

1. `AutomationLevel` remains owned by the operational action contract.
2. Authority resolution determines whether the requested mode is executable in the current authority context.
3. Authentication and concrete role identity remain outside this minimal contract.
4. Safety, regulatory and energy contracts remain authoritative.
5. UI may present the resolved state but cannot create authority.
6. AI and optimization cannot elevate authority.
7. Engineering/service authority remains governed by its applicable role and security contracts.
8. Every consequential resolution shall be suitable for later evidence recording.

## 7. Verification requirements

The implementation shall test:

- automatic execution with system authority;
- automatic action falling back to human-guided when system authority is absent;
- human-guided execution with and without role authority;
- human-decision pending and granted states;
- safety, regulatory and energy blocks;
- explicit action-authorization block.

## 8. Non-goals

This contract does not implement:

- authentication;
- user/role directory;
- safety policy;
- regulatory policy;
- Energy Model parameters;
- execution itself;
- UI authorization;
- FCS control.

## 9. Acceptance

ARCH-OPS-006 is implementation-complete when the resolver and its contract test are built by CI and all relevant CTest cases pass on the PR version.
