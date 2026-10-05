# BlueSky PRO — Unified Telemetry and Event Model Baseline

**Status:** ARCHITECTURE BASELINE — P0 integration contract

## 1. Purpose

Provide one normalized information model connecting UAV/autopilot telemetry, C2, mission execution, planning, replanning and post-flight replay.

## 2. Principle

```text
AUTOPILOT / UAV ─┐
PAYLOAD ─────────┤
C2 ──────────────┤→ NORMALIZATION → RUNTIME → DECISION / UI / LOG / REPLAY
ATM / REGULATORY ┤
PLANNING ────────┘
```

Source-specific protocols remain at their adapters. The core consumes normalized data with explicit provenance and quality.

## 3. Event envelope

Every operational event shall contain, as applicable:

- event ID;
- event type;
- timestamp;
- vehicle ID;
- mission ID/version;
- mission package ID/version;
- source subsystem;
- source protocol/message identity where relevant;
- sequence/order information;
- data quality/state;
- payload/data;
- correlation ID for commands and transactions.

## 4. Telemetry data classes

The normalized model shall support at minimum:

- position;
- altitude and reference;
- velocity/ground speed/airspeed where available;
- heading/course;
- attitude;
- navigation state and quality;
- battery state and energy estimates;
- propulsion state and resource data where available;
- C2 link/channel state;
- autopilot/vehicle state;
- mission progress;
- active waypoint/segment;
- payload/camera/gimbal state;
- environmental inputs;
- regulatory/airspace state relevant to execution;
- faults, warnings and failsafe state.

## 5. Data quality

Telemetry shall not be treated as equally reliable merely because it is present.

The model shall distinguish at least:

```text
CURRENT
DELAYED
STALE
MISSING
INVALID
ESTIMATED
DERIVED
```

Where useful, source timestamp and receipt timestamp shall both be retained.

## 6. Command lifecycle

Commands shall correlate with telemetry/events through a transaction identity:

```text
REQUESTED
 ↓
SENT
 ↓
ACKNOWLEDGED
 ↓
EXECUTED / REJECTED / TIMEOUT / UNKNOWN
```

`UNKNOWN` is retained whenever execution cannot be established. It shall not be converted to `EXECUTED` by inference.

## 7. Runtime state

Normalized telemetry feeds the Runtime State Machine. State transitions shall be event-driven and traceable.

```text
TELEMETRY + EVENTS
       ↓
STATE EVALUATION
       ↓
NORMAL / DEGRADED / CRITICAL
       ↓
CONTINUE / LOCAL REPLAN / FULL REPLAN / RECOVERY / ABORT
```

## 7.1 Operator-facing INFORMATION projection

The normalized event model is authoritative. The Right Panel **INFORMATION** view is a presentation/projection of relevant system events; it is not a separate source of operational truth and shall not own fault detection, severity classification, mission-state transitions or safety decisions.

For operator-facing events, the projection shall expose, where applicable:

- stable event ID and event type;
- occurrence/source timestamp and receipt timestamp where available;
- source subsystem and affected UAV, mission, mission element or resource;
- severity/classification;
- concise summary and detailed explanation;
- current operational/system response;
- whether pilot intervention is required;
- the applicable action/context reference;
- acknowledgement state and event-resolution state as separate fields.

The operator-message lifecycle shall be distinct from the underlying event lifecycle:

```text
EVENT DETECTED / RECEIVED
        ↓
NORMALIZE + CORRELATE + CLASSIFY
        ↓
PUBLISH TO INFORMATION PROJECTION
        ↓
UNACKNOWLEDGED → READ → ACKNOWLEDGED
        ↓
REMOVE FROM INFORMATION OVERVIEW
        │
        └── underlying event remains in event store / Journal / Audit
```

Acknowledgement means only that the operator has acknowledged reading the message. It shall not imply that a fault is cleared, a warning condition is resolved, an operational action is completed, or a safety/mission state has changed.

Where intervention is required, the event shall carry a typed reference to the owning operational workflow/context. The HMI may navigate to that context, but the action itself must pass through the owning service/Core and its authorization, validation and Safety Engine constraints. QML shall not execute a safety-critical action merely because a message was selected or acknowledged.

The INFORMATION overview shall contain only messages that remain unacknowledged under the configured presentation policy. If there are no such messages, the message list is empty; absence of messages shall not be interpreted as an independent “all clear” assertion.

Critical/safety-relevant events shall remain discoverable and shall not be made inaccessible by ordinary HMI display filters. Their presentation may be additionally surfaced through the header, map alert or applicable local context. Hiding or acknowledging the INFORMATION card changes presentation only.

The event store / Journal / Audit shall preserve the event and its provenance independently of the INFORMATION overview. Acknowledgement records shall include, where available, event ID, operator identity, acknowledgement timestamp and UI/client context. The implementation shall support durable acknowledgement state across the applicable session/device boundary and prevent duplicate event delivery from creating duplicate operator messages.

## 8. Planning feedback

Operational data shall be available to planning decisions without coupling planning to a particular autopilot protocol.

Relevant inputs include:

- actual wind;
- actual energy consumption;
- actual route/ground track;
- actual vehicle performance;
- payload operating state;
- C2 conditions;
- deviations from predicted values.

## 9. Replanning trigger data

Events capable of triggering replanning shall be explicitly represented, including:

- significant wind change;
- energy margin deterioration;
- route/airspace constraint change;
- vehicle capability degradation;
- payload state change;
- C2 degradation/recovery;
- vehicle failure in a multi-UAV mission;
- deviation from planned trajectory or timing.

## 10. Multi-UAV model

Telemetry and events are independently associated with each UAV while retaining a common parent mission ID.

```text
MISSION M-001
 ├── UAV-01 telemetry/events
 ├── UAV-02 telemetry/events
 └── UAV-03 telemetry/events
```

This permits individual vehicle failures or link losses to be evaluated without corrupting the state of the remaining fleet.

## 11. Logging and evidence

Normalized events shall be persisted with sufficient provenance to reconstruct the operational sequence.

The evidence chain shall allow correlation between:

- approved mission;
- mission package;
- commands;
- acknowledgements;
- vehicle state;
- C2 state;
- environmental conditions;
- replanning decisions;
- operator actions;
- final outcome.

## 12. Replay

Replay shall consume the same normalized event representation where possible, rather than requiring separate interpretation logic for every source protocol.

Replay shall support reconstruction of the mission timeline and comparison of planned versus actual execution.

## 13. Source adapters

```text
ArduPilot ─┐
PX4 ───────┤
OEM ───────┤
C2 ────────┤→ NORMALIZED EVENT MODEL
Payload ───┘
```

Adapters shall preserve source provenance and shall not silently discard safety-relevant information that has no current normalized field; unsupported data must remain identifiable for extension.

## 14. Clock and ordering

The system shall distinguish source time from receipt time where possible. Clock quality and ordering uncertainty shall be represented where it can affect operational interpretation.

## 15. Acceptance criteria

The model is contract-complete when:

1. ArduPilot, PX4 and OEM data can map to common telemetry/event concepts;
2. C2 state is represented independently from vehicle state;
3. command lifecycle can be correlated with results;
4. stale/delayed/missing data is distinguishable;
5. mission/package/version provenance is retained;
6. multi-UAV events remain independently attributable;
7. runtime decisions can be traced to input events;
8. replay can reconstruct the operational timeline;
9. planning can consume actual execution/environment data;
10. extensions do not require changing the core protocol semantics.

## 16. Implementation boundary

Concrete protocol mappings, message decoders, storage format and transport mechanisms are implementation artifacts under the relevant adapter/integration modules. This document defines the common semantic contract only.
