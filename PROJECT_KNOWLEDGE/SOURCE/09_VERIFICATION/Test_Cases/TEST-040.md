---
id: TEST-040
type: test_case
title: Concurrent Multi-UAV Mission Test
status: draft
verifies:
  - "[[SYS-REQ-078]]"
verification_method: test
result: not_run
---

# TEST-040 — Concurrent Multi-UAV Mission Test

## Objective

Verify [[SYS-REQ-078]] by executing multiple concurrent UAV tasks as parts of one mission.

## Preconditions

- Mission Planner is available.
- HUB Core is available.
- Multiple UAV participants are available or represented by test stubs.
- A common mission containing multiple coordinated tasks is available.
- Mission state and task execution state can be observed through HUB Core.

## Test Procedure

1. Create a mission containing multiple coordinated tasks.
2. Configure multiple UAV participants.
3. Assign different mission roles to the UAVs, including search, confirmation, observation, communications relay and reserve where applicable.
4. Start the mission.
5. Verify that multiple UAVs execute different parts of the same mission concurrently.
6. Verify that each UAV receives and executes its assigned task.
7. Verify that task execution states are reported to HUB Core.
8. Verify that Mission Planner can obtain the execution state of the tasks through HUB Core.
9. Verify that concurrent task execution does not corrupt the common mission state.
10. Record task assignments, execution states, events and completion results.

## Expected Result

Multiple UAVs concurrently execute different parts of the same mission.

UAVs can perform different mission roles, including search, confirmation, observation, communications relay and reserve as applicable.

Mission Planner receives the execution state of the distributed tasks through HUB Core.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Common mission definition.
- UAV/task assignments.
- Role assignments.
- HUB task-state messages.
- Mission Planner task-state records.
- Concurrent execution logs.
- Mission completion/status records.

## Traceability
- [[SYS-REQ-078]]
- [[ARCH-027]]



