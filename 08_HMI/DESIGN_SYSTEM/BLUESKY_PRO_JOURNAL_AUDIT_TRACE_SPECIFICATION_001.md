# BlueSky PRO — Journal / Audit Trace Specification 001

Status: WORKING

## Scope

The Journal / Audit trail records operational events and operator decisions that require traceability. The HMI emits event data; controlled system layers remain authoritative for persistence and immutable records.

## UAV decision event

For a UAV context decision, the event shall preserve, where applicable:

- event type;
- controlled time reference / timestamp;
- immutable Mission ID;
- mission version where available;
- affected UAV ID;
- triggering condition / event;
- assessment factors;
- system recommendation;
- operator decision;
- resulting action;
- operator identity;
- source HMI/application context.

## Current HMI integration

The current HMI emits:

- event type: `UAV_DECISION`;
- Mission ID;
- UAV index;
- operator decision: `RETURN` or `CONTINUE`.

The HMI does not persist the authoritative audit record and does not issue the flight command.

## Authority boundary

`HMI event → controlled Journal/Audit layer → traceable record`

Flight/safety authority remains outside the visual HMI layer.

## Alignment

This specification follows the existing HMI requirement that the event, assessment factors, recommendation, operator decision and resulting action are recorded in Journal/Audit, while preserving the existing mission identity and traceability model.
