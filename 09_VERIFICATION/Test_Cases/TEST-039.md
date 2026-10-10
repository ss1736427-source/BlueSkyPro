---
id: TEST-039
type: test_case
title: Capability-Based Task Allocation Test
status: draft
verifies:
  - "[[SYS-REQ-077]]"
verification_method: test
result: not_run
---

# TEST-039 — Capability-Based Task Allocation Test

## Objective

Verify [[SYS-REQ-077]] by controlled task-allocation testing using heterogeneous UAV capabilities and current state.

## Preconditions

- Mission Planner is available.
- Multiple UAV candidates are available.
- UAV capability and current-state data can be provided to Mission Planner.
- Candidate UAVs differ in at least one relevant capability or state parameter.
- Representative mission tasks with different priorities and requirements are available.

## Test Procedure

1. Configure multiple candidate UAVs.
2. Provide capability information for each candidate.
3. Provide current availability for each candidate.
4. Provide current health for each candidate.
5. Provide current energy for each candidate.
6. Provide range information for each candidate.
7. Provide payload capabilities for each candidate.
8. Provide communication capabilities and constraints.
9. Configure applicable airspace constraints.
10. Configure task risk and task priority.
11. Create mission tasks with different capability and state requirements.
12. Run Mission Planner task allocation.
13. Record the selected executor for each task.
14. Change one relevant UAV capability or state parameter.
15. Re-run task allocation.
16. Verify that allocation changes when the changed capability or state affects task suitability.
17. Verify that allocation is not based exclusively on UAV type or identifier.

## Expected Result

Mission Planner allocates tasks according to the relevant UAV capabilities and current state.

Capability, availability, health, energy, range, payload, communication, airspace constraints, risk and task priority are considered where applicable.

Changing a relevant capability or state parameter can change the selected executor.

Task allocation is not based exclusively on UAV type or identifier.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- UAV capability/state inputs.
- Mission task definitions.
- Allocation decisions.
- Allocation logs.
- Before/after allocation results.
- Evidence of changed selection following changed capability/state.

## Traceability
- [[SYS-REQ-077]]
- [[ARCH-027]]



