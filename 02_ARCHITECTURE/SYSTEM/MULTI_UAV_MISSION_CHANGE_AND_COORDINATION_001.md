# BLUESKY PRO — MULTI-UAV MISSION CHANGE AND COORDINATION 001

## Status
CANDIDATE_FOR_AGREEMENT

## Purpose
Define how BlueSky PRO shall handle a mission involving multiple UAVs when one or more assigned aircraft, equipment sets, routes, or task parameters are changed.

## Architectural Principle
A multi-UAV mission is one logical mission containing independently identifiable UAV task allocations. Changes to one UAV shall not require reconstruction of unaffected UAV allocations when the system can safely determine that they remain valid.

## Mission Structure

```text
GROUP MISSION
      │
      ├── UAV-01
      │    ├── task
      │    ├── route
      │    └── equipment
      │
      ├── UAV-02
      │    ├── task
      │    ├── route
      │    └── equipment
      │
      └── UAV-03
           ├── task
           ├── route
           └── equipment
```

Each UAV allocation shall retain its own identity and state while remaining linked to the common mission.

## Change Scenarios

### Change of one UAV
If UAV-02 becomes unavailable, the system shall identify the affected allocation and determine whether reassignment, task redistribution, or replanning is required.

### Change of equipment
If equipment assigned to one UAV changes, the system shall determine whether the affected task, performance model, route, payload or other dependent planning elements require recalculation.

### Change of route/task for one UAV
The system shall identify affected dependencies and avoid unnecessary reconstruction of unaffected UAV allocations where safe and technically valid.

### Change affecting the whole group
A change to a group-level parameter shall trigger assessment of all affected UAV allocations and applicable coordination constraints.

## Controlled Change Flow

```text
GROUP MISSION
      ↓
CHANGE DETECTED
      ↓
IDENTIFY AFFECTED UAV / COMPONENTS
      ↓
DEPENDENCY ANALYSIS
      ↓
REASSIGN / REPLAN WHERE REQUIRED
      ↓
GROUP COORDINATION CHECK
      ↓
SAFETY / READINESS REVALIDATION
      ↓
NEXT CONTROL GATE
```

## Functional Requirements

1. The system shall maintain a common mission identity for a multi-UAV mission.
2. Each assigned UAV shall have an independently identifiable task allocation within the mission.
3. Each UAV allocation shall retain its associated route, task parameters and assigned equipment as applicable.
4. The system shall identify the affected UAV allocation when a change is made to a member of the group.
5. The system shall assess dependencies between the changed allocation and other UAV allocations.
6. The system shall avoid unnecessary reconstruction of unaffected allocations when their validity can be established.
7. When reassignment or redistribution is required, the system shall create a traceable changed mission revision.
8. Changes affecting group coordination shall trigger the applicable multi-UAV coordination checks.
9. Changes affecting safety-significant parameters shall trigger applicable safety and readiness validation before execution.
10. A changed group mission shall not inherit an earlier release or authorization state when the change invalidates that state.
11. The system shall preserve the ability to inspect the state of each UAV independently within the group mission.
12. The system shall support comparison of relevant UAV allocations without creating duplicate mission data.
13. The system shall prevent a change to one UAV allocation from silently changing another allocation.
14. If a safe automatic redistribution or recalculation cannot be completed, the system shall identify the required operator/engineer action.

## Resource and Assignment Integrity

A group mission shall maintain explicit relationships between:

```text
MISSION
→ UAV
→ EQUIPMENT
→ TASK
→ ROUTE
→ PERSONNEL / AUTHORITY
```

Changing one relationship shall not silently alter another.

## Coordination Integrity

The system shall consider applicable inter-UAV constraints when a change affects:

- route geometry;
- timing;
- altitude;
- separation;
- task allocation;
- UAV availability;
- equipment/performance;
- launch or recovery arrangement.

The exact numerical separation criteria and operational rules shall be defined by the applicable system and safety requirements rather than by this architecture document.

## UX Principle
The primary working view shall show only the information needed for the current group-mission operation.

When one UAV is changed:

```text
PRIMARY VIEW
→ affected UAV / required action

DETAILS ON DEMAND
→ dependency impact
→ other affected allocations
→ revision history
→ coordination diagnostics
```

The interface shall not force the operator to inspect every UAV when only one allocation is affected, unless a group-level check requires it.

## Relationship to Mission Revision
This document extends `MISSION_REUSE_REVISION_REPLANNING_001`.

A group change creates or updates a revision of the common mission. Individual UAV allocations remain identifiable inside that revision.

## Relationship to Synchronization
The multi-UAV mission shall use the existing synchronization model between PC, HUB and tablet. Partial changes shall be represented as controlled revisions rather than uncontrolled file replacement.

## Relationship to Safety
The aviation system definition identifies multi-UAV separation/coordination as a potential safety-significant function. fileciteturn562file0L2-L2

Therefore this document does not assign a safety classification; it requires the appropriate safety analysis and validation to be applied when a change can affect safe coordination.

## Verification Considerations
Verify at minimum:
- creation of a multi-UAV mission;
- independent identification of each UAV allocation;
- change to one UAV without unintended modification of others;
- equipment replacement and dependency detection;
- UAV replacement/reassignment;
- partial replanning;
- group-level change propagation;
- coordination validation after changes;
- preservation of revision history;
- preservation of safety/readiness/release boundaries;
- synchronization of partial changes across authorized clients.

## Traceability
This document is a candidate architecture extension based on the existing multi-UAV scope, mission revision model and synchronization architecture. Before baseline approval, exact overlap shall be checked against existing `SYS-REQ-*`, `MUL-REQ-*`, `MIS-REQ-*`, safety and mission-planning records.
