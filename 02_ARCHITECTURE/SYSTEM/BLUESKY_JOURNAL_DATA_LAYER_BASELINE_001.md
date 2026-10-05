# BlueSky PRO — Journal Data Layer Baseline 001

Status: WORKING

## Purpose

Provide a controlled application-layer boundary between HMI events and authoritative Journal/Audit persistence.

## Event record

The baseline `JournalEvent` contains:

- event ID;
- event type;
- UTC timestamp;
- Mission ID and mission version;
- UAV ID;
- triggering condition;
- assessment factors;
- system recommendation;
- operator decision;
- resulting action;
- operator identity;
- source context.

## Boundary

`HMI → JournalEvent → JournalStore → authoritative persistence`

The HMI remains presentation/event-source only. The JournalStore interface is the application boundary. The in-memory implementation is a development/test adapter and is not claimed as certification-grade persistence.

## Integrity

Authoritative immutable storage, retention, access control, synchronization and evidence/export requirements remain to be allocated to the controlled persistence implementation.

## UAV decision service

`JournalService::recordUavDecision()` is the controlled application entry point for `UAV_DECISION` events. It validates the minimum event identity (`event_type`, Mission ID, UAV ID and operator decision) before forwarding the event to `JournalStore`.

The HMI exposes an append integration signal but does not instantiate or own the authoritative JournalStore.
