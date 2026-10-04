---
id: TEST-041
type: test_case
title: Common Fleet Mission State Test
status: draft
verifies:
  - "[[SYS-REQ-079]]"
verification_method: test
result: not_run
---

# TEST-041 — Common Fleet Mission State Test

## Objective

Verify [[SYS-REQ-079]] by testing maintenance and propagation of a common current mission state for a heterogeneous UAV fleet.

## Preconditions

- HUB Core is available.
- Multiple heterogeneous UAV participants are available or represented by test stubs.
- A common mission is available.
- Mission Planner can access fleet state through HUB Core.
- Required state variables can be observed.

## Test Procedure

1. Configure multiple UAV participants with different platform characteristics.
2. Create a common mission for the fleet.
3. Register the UAVs with HUB Core.
4. Start the mission.
5. Verify that HUB Core maintains the mission_state.
6. Verify that HUB Core maintains the fleet_state.
7. Verify that HUB Core maintains the UAV_state for each participant.
8. Verify that HUB Core maintains the task_state.
9. Verify that HUB Core maintains the capability_state.
10. Verify that HUB Core maintains the communication_state.
11. Verify that HUB Core maintains the health_state.
12. Verify that HUB Core maintains the risk_state.
13. Change representative mission, task, communication, health or risk conditions.
14. Verify that the corresponding state is updated.
15. Verify that the resulting state remains consistent across the heterogeneous fleet.
16. Verify that Mission Planner can retrieve the current common fleet mission state through HUB Core.

## Expected Result

HUB Core maintains a single current representation of the heterogeneous fleet mission state.

The representation contains mission_state, fleet_state, UAV_state, task_state, capability_state, communication_state, health_state and risk_state.

Changes in relevant operational conditions are reflected in the corresponding state.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- HUB state records.
- Fleet state snapshot.
- UAV state records.
- Task state records.
- Capability state records.
- Communication state records.
- Health state records.
- Risk state records.
- Mission Planner state view or API output.

## Traceability
- [[SYS-REQ-079]]
- [[ARCH-027]]



