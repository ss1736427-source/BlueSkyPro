# BLUESKY PRO — MISSION REUSE, REVISION AND REPLANNING 001

## Status
CANDIDATE_FOR_AGREEMENT

## Purpose
Define the operational principle for reusing an existing mission, creating a new revision, and recalculating the affected flight plan after mission changes without creating a separate mission-management subsystem.

## Architectural Principle
An existing mission may be used as the starting point for a new mission or revision. The original mission state shall remain preserved. Changes shall create a traceable new revision rather than silently altering the approved or previously executed state.

## Operational Scenarios

### Reuse
A saved mission may be opened as the basis for a new mission. The new mission receives its own identity and does not modify the source mission.

### Revision
A mission under preparation may be edited. The system shall record the resulting revision and retain the previous revision for traceability.

### Replanning
When a change affects route-dependent parameters, BlueSky PRO shall recalculate the affected planning elements rather than requiring the user to reconstruct the complete route manually.

## Controlled Flow

```text
EXISTING MISSION
      ↓
OPEN / REUSE
      ↓
NEW MISSION OR REVISION
      ↓
CHANGE PARAMETERS
      ↓
DEPENDENCY ANALYSIS
      ↓
RECALCULATION
      ↓
REVALIDATION
      ↓
READY FOR NEXT CONTROL GATE
```

## Functional Requirements

1. The system shall allow an existing mission to be used as the basis for a new mission.
2. Reuse shall not modify the source mission.
3. The system shall maintain a distinct identity for the new mission or revision.
4. Mission changes shall be traceable to the resulting revision.
5. The system shall preserve previous mission revisions required for operational traceability.
6. The system shall identify planning elements affected by a mission change.
7. The system shall recalculate affected dependent elements when automatic recalculation is applicable.
8. The system shall indicate when recalculation cannot be completed automatically and requires user action.
9. A changed mission shall undergo all applicable validation and safety checks again before proceeding to a subsequent operational gate.
10. Reuse or revision shall not bypass aircraft, equipment, personnel, airspace, weather, safety, authorization, or release controls that are applicable to the resulting mission.
11. The interface shall expose revision and replanning controls contextually and shall not introduce a separate high-level subsystem solely for these functions.

## Revision Integrity

The system should retain, as applicable:
- mission identifier;
- revision identifier;
- source revision;
- author/user responsible for the change;
- change time;
- changed parameters;
- recalculation state;
- validation state;
- subsequent control-gate state.

## Safety Boundary

```text
REUSE / REVISION
       ≠
AUTOMATIC APPROVAL
       ≠
FLIGHT AUTHORIZATION
```

A previously validated or executed mission shall not be assumed to remain valid after changes that affect its operational conditions or planning data.

## UX Principle
The user should see the current mission state and the actions required for the current task. Detailed revision history and change information should remain available on demand rather than occupying the primary working area.

## Relation to Multi-UAV Operations
For a mission involving multiple UAVs, a revision shall preserve the association between the mission and its assigned aircraft, equipment and personnel. If a change affects only part of a multi-UAV mission, the system should identify the affected component(s) and avoid unnecessary reconstruction of unaffected elements where safe and technically valid.

## Verification Considerations
Verify at minimum:
- reuse of an existing mission without modification of the source;
- creation and identification of a new revision;
- traceability of changes;
- detection of affected planning elements;
- automatic recalculation where applicable;
- controlled handling when recalculation is not possible;
- revalidation after changes;
- preservation of mandatory safety and authorization gates;
- correct handling of multi-UAV mission revisions.

## Traceability
This document records a practical operational capability identified during the external-practice review. Before baseline approval, duplicate and overlap checks shall be completed against the Master Requirements Register and existing mission-planning requirements.
