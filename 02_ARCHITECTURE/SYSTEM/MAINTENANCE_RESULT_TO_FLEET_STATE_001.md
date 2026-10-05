# BLUESKY PRO — MAINTENANCE RESULT TO FLEET STATE 001

## Status
CANDIDATE_FOR_AGREEMENT

## Purpose
Define how a Technician's completed or incomplete maintenance result affects the operational state presented to the Engineer, without introducing a standalone maintenance-management subsystem.

## Core Principle
The Technician records the factual result of assigned work. BlueSky PRO uses that result to keep the relevant UAV/equipment state current and to inform the Engineer when attention is actually required.

## Normal Completion

When an assigned task is completed successfully:

```text
TECHNICIAN
   ↓
COMPLETED
   ↓
TASK RESULT RECORDED
   ↓
RELEVANT UAV / EQUIPMENT STATE UPDATED
   ↓
ENGINEER NOTIFIED
```

The Engineer does not perform a mandatory approval merely to acknowledge normal completion.

## Unfinished Work / Finding

When work is not completed or a finding is recorded:

```text
TECHNICIAN
   ↓
STATUS + COMMENT / FINDING
   ↓
RESULT RECORDED
   ↓
ENGINEER SEES STATUS + COMMENT
   ↓
ENGINEER DECISION ONLY WHERE REQUIRED
```

The system shall not infer successful readiness from an unfinished task.

## State Update Rules

1. A task result shall be associated with the UAV and/or equipment affected by the work.
2. A successful completion may remove or update the corresponding maintenance condition when the task is defined to do so.
3. An unfinished task or finding shall preserve an applicable warning, restriction, or attention state until appropriately resolved.
4. BlueSky PRO shall not silently convert a technical finding into `READY`.
5. Where external maintenance data is authoritative, the external system remains authoritative for the data it owns; BlueSky PRO consumes the normalized result through the integration boundary.
6. State changes shall retain source, author, timestamp and relevant task reference where required for traceability.

## Engineer View

The Engineer's fleet view should present the resulting state through compact indicators. Detailed task results remain available by selecting the affected UAV or attention condition.

The Engineer should therefore see, for example:

```text
UAV-001   READY        ✓
UAV-002   ATTENTION    ⚠
UAV-003   MAINTENANCE  !
```

The indicator is not a substitute for the underlying technical record.

## No Automatic Flight Release

A maintenance task completion or resulting fleet-state update shall not by itself constitute flight release. Readiness remains subject to the applicable technical, mission, airspace, weather, equipment, personnel and release controls.

## Integration

The model shall operate both with native BlueSky PRO maintenance task records and with future external maintenance/MRO integration. External data shall enter through the existing integration boundary and be normalized before presentation to the Engineer.

## UX Principle

Do not expose the full maintenance transaction on the Engineer's main screen. Show the operational consequence first; expose the underlying task, comment, finding and technical record on demand.

## Verification Considerations

Verify at minimum:

- completed work updates the relevant state where defined;
- unfinished work does not produce a false ready state;
- Engineer receives completion notification;
- Engineer sees status and comment for unfinished work;
- source, author and timestamp remain traceable;
- external authoritative data is not overwritten improperly;
- state changes do not bypass Safety Gate or Release Gate;
- detailed maintenance data remains available without permanently overloading the Engineer UI.
