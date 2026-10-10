# BLUESKY PRO — TECHNICIAN SHIFT WORK ASSIGNMENT AND COMPLETION 001

## Status
CANDIDATE_FOR_AGREEMENT

## Purpose
Define the minimal Technician workflow for receiving work assigned by the Engineer for the current shift, performing the authorized work, and recording its completion in BlueSky PRO.

## Role Boundary
The Administrator assigns users to their roles. The Engineer determines which work a Technician is authorized and assigned to perform within the Engineer's authority and the Technician's permissions.

The Technician shall see and act only on work assigned to the Technician and within the Technician's granted access.

## Primary Technician Workflow

```text
ENGINEER
   ↓
ASSIGN WORK
   ↓
TECHNICIAN — CURRENT SHIFT
   ↓
VIEW ASSIGNED WORK
   ↓
OPEN TASK
   ↓
PERFORM WORK
   ↓
ENTER RESULT
   ↓
COMPLETED → NOTIFY ENGINEER
   │
   └── NOT COMPLETED / FINDING → STATUS + COMMENT → ENGINEER SEES
```

## Technician Main View

The Technician's primary work screen shall be task-oriented and shall show the work relevant to the current shift rather than the full technical structure of the fleet.

Minimum immediately available information:

- task status;
- assigned work;
- priority where applicable;
- UAV identifier;
- equipment/component concerned;
- required action;
- relevant due time/date where applicable;
- indication when Engineer attention is required.

Detailed technical information shall be available from the selected task when needed and shall not permanently occupy the main screen.

## Work Assignment

An Engineer shall be able to assign a defined maintenance/technical task to a Technician.

The assignment should identify, as applicable:

- Technician;
- UAV;
- equipment/component;
- task description;
- applicable procedure or reference;
- priority;
- planned completion period;
- restrictions or special instructions.

The assignment shall be traceable to its assigning Engineer.

## Task States

The implementation should support a controlled task lifecycle. At minimum:

```text
ASSIGNED
   ↓
IN PROGRESS
   ↓
COMPLETED
```

For an unfinished task, the Technician shall select an appropriate non-completed status and provide a comment describing the reason, current condition, or other information needed by the Engineer.

## Completion Entry

After performing the assigned work, the Technician shall be able to record the result directly against the task.

The completion record shall support, as applicable:

- completion status;
- date/time;
- Technician identification;
- work/result note;
- findings or defects discovered;
- indication that additional work is required;
- relevant measured values or evidence where required by the applicable procedure;
- reference to attached technical evidence where supported.

When all assigned actions have been completed successfully and no finding requiring Engineer intervention has been recorded, the Technician shall be able to close the assigned task as `COMPLETED`.

## Completion Notification

For a normally completed task, no Engineer action or approval is required solely because the Technician has completed the assigned work.

Upon completion, BlueSky PRO shall provide the Engineer with a notification that the assigned task has been completed. The notification is informational and provides traceability; it shall not create an additional approval step when no Engineer decision is required.

The completion record shall remain available to the Engineer for information and operational history.

```text
TECHNICIAN
   ↓
COMPLETED
   ↓
NOTIFICATION TO ENGINEER
   ↓
NO ACTION REQUIRED
```

## Unfinished Work / Findings

If the Technician discovers a defect, abnormal condition, or cannot complete the assigned work, the Technician shall not mark the task as `COMPLETED`.

Instead, the Technician shall record:

- the current task status;
- a comment explaining the reason for non-completion and/or the finding;
- additional information or evidence where required by the applicable procedure.

The Engineer shall see the status and Technician comment in the Engineer's work/fleet view.

Only in such cases, or where the assigned procedure explicitly requires Engineer involvement, shall the result require Engineer attention.

```text
TASK
 ↓
NOT COMPLETED / FINDING
 ↓
STATUS + TECHNICIAN COMMENT
 ↓
ENGINEER SEES
 ↓
ENGINEER REVIEW WHERE REQUIRED
```

## Engineer Control

The Engineer shall retain visibility of assigned work and its current state.

For a normally completed task, Engineer visibility is informational and does not constitute a mandatory approval workflow.

For unfinished work or findings, the Engineer shall see the recorded status and Technician comment and may take the appropriate action where required.

## Access Control

Technician access is constrained by the permissions and technical scope determined by the Engineer. The Technician must not be able to use completion entry to grant themselves access, modify their authority, release a UAV, or alter Engineer-controlled permissions.

## UX Principle

The Technician interface shall answer one primary question immediately:

**"What work do I need to do on this shift?"**

The interface should therefore prioritize the current assigned work list and its status. Fleet-wide technical information, administrative functions and unrelated configuration data shall not be presented as persistent information.

Completion of a normal task shall require only the minimum information necessary to record the result. Engineer notification should be automatic rather than requiring the Technician to perform a separate reporting action.

For unfinished work, the interface shall make the status and comment entry simple and direct. The Technician shall not be forced through an unnecessary approval workflow.

## Integration

The workflow shall be compatible with the external maintenance integration architecture. BlueSky PRO may receive assignments or maintenance context from an external system and may return completion/finding records through the integration boundary where such integration is implemented.

The Technician workflow must remain usable without requiring BlueSky PRO to become a standalone MRO system.

## Traceability / Verification Considerations

Verify at minimum:

- only assigned tasks are visible to the Technician;
- task assignment is traceable to the Engineer;
- Technician access respects Engineer-defined scope;
- task state transitions are controlled;
- completion records retain author and timestamp;
- a normally completed task does not require unnecessary Engineer approval;
- Engineer receives notification when the task is completed;
- an unfinished task cannot be falsely represented as completed;
- unfinished-task status is recorded;
- Technician comment is recorded and visible to the Engineer;
- findings can be escalated to Engineer review;
- task completion does not independently produce flight release;
- completion data is available to the Engineer where required;
- the workflow remains usable with the intended minimal-information UX.
