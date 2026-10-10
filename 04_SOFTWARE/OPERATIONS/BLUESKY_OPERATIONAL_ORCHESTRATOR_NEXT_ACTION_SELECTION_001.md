# BlueSky PRO — Operational Orchestrator Next-Action Selection Contract 001

**Status:** IMPLEMENTATION BASELINE — P0
**Parent:** `04_SOFTWARE/OPERATIONS/BLUESKY_OPERATIONAL_ORCHESTRATOR_INTEGRATION_CONTRACT_001.md`

## Purpose

Define deterministic selection of the next operational action after dependency, readiness and authority/automation evaluation.

## Contract

The selector consumes the existing `OperationalSnapshot` and `AuthorityContext`. It does not redefine action semantics, execute UAV commands, or bypass safety, regulatory, energy or authorization gates.

```text
CURRENT STATE
 ↓
ACTIONABLE SET
 ↓
AUTHORITY / AUTOMATION RESOLUTION
 ↓
ELIGIBLE SET
 ↓
DETERMINISTIC ORDER
 ↓
NEXT ACTION / PARALLEL SET
```

Selection uses the existing action priority and stable action ID ordering. Blocked actions are excluded. An action requiring human participation remains selectable as a human-required path; it is not silently discarded.

## Parallel execution

Actions with the highest eligible priority may form a parallel set. Existing dependency and authority evaluation remains authoritative for each action.

## Boundary rules

1. Existing `Action` and `AutomationLevel` semantics remain authoritative.
2. Authority resolution remains owned by ARCH-OPS-006.
3. Safety, regulatory, energy and authorization gates cannot be bypassed.
4. Selection is deterministic for identical inputs.
5. Selection does not execute an action.
6. UI consumes the selection result and does not redefine selection semantics.
7. AI/optimization cannot elevate authority or bypass gates.

## Verification

CTest verifies executable selection, human-required selection, deterministic parallel selection and the empty-selection case when all actions are blocked.
