---
id: ARCH-DEC-014
type: architecture_decision
title: Data Architecture and Telemetry
status: accepted
scope: BlueSky System
---

# ARCH-DEC-014 — Data Architecture and Telemetry

## Decision

BlueSky shall use a centralized Core data architecture for mission state, telemetry, communication state, vehicle state, payload state and simulation data.

The C++ Core shall act as the authoritative runtime integration layer for operational mission data.

QML shall consume approved data exposed by the Core and shall not become the authoritative owner of mission or safety-critical state.

## Data Flow

The primary data flow shall be:

UAV / PAYLOAD / C2 / SIMULATION
→ DATA ADAPTERS
→ C++ CORE
→ SYSTEM MODULES
→ QML / UI

The Core shall distribute applicable data to:

- Mission Engine;
- Mission Graph;
- Mission Validation;
- Safety Engine;
- Mission AI;
- Fleet Coordination;
- Mission Readiness;
- Conflict Resolution;
- Simulation;
- telemetry and monitoring components.

## Authoritative Mission State

The C++ Core shall maintain the authoritative runtime representation of the Mission.

QML shall display and interact with mission data through defined Core interfaces.

The UI shall not independently maintain a conflicting authoritative mission state.

## Telemetry

The telemetry architecture shall support applicable:

- position;
- altitude;
- velocity;
- attitude;
- battery;
- energy;
- health;
- payload state;
- navigation state;
- communication state;
- mission state;
- UAV role;
- fleet state;
- environmental information.

Additional telemetry fields may be introduced without changing the basic data architecture.

## Telemetry Sources

Telemetry may originate from:

- UAVs;
- payloads;
- C2 systems;
- communication systems;
- ground systems;
- Simulation / Digital Twin;
- other approved system adapters.

All external telemetry shall enter the Core through the appropriate adapter boundary.

## Telemetry Normalization

The Core shall provide a normalized internal representation of applicable telemetry.

Different UAVs, payloads, C2 systems or simulation backends shall not require the QML layer to understand their proprietary data formats.

Adapters shall translate external representations into the internal BlueSky data model.

## Real and Simulated Data

The architecture shall distinguish:

- PLANNED;
- SIMULATED;
- ACTUAL.

Actual operational telemetry shall be associated with ACTUAL mission data.

Simulation telemetry shall be associated with SIMULATED mission data.

Mission planning data shall be associated with PLANNED mission data.

These data states shall remain distinguishable during analysis.

## Telemetry Time

Telemetry and mission events shall include applicable temporal information.

The system shall support reconstruction of mission behaviour over time.

Time information shall allow comparison of:

- planned timing;
- simulated timing;
- actual timing;
- detected events;
- state transitions;
- validation events;
- communication changes.

## Event Model

The Core shall support event-based representation of significant mission changes.

Applicable events may include:

- mission state transition;
- waypoint or graph-node completion;
- capability state change;
- battery threshold;
- communication degradation;
- communication loss;
- conflict detection;
- validation result;
- readiness result;
- safety decision;
- operator decision;
- mission adaptation;
- replanning;
- emergency event.

## Data Ownership

The architecture shall define clear ownership of critical data.

The C++ Core shall own the operational integration state.

Safety-critical decisions shall remain owned by the applicable Safety Engine and validation mechanisms.

Mission AI shall provide recommendations and planning results rather than becoming the authoritative owner of safety decisions.

QML shall provide presentation and user interaction.

## QML Boundary

QML shall be responsible for presentation functions including:

- panels;
- maps;
- charts;
- telemetry displays;
- notifications;
- mission visualization;
- state visualization;
- user controls.

QML shall not directly access UAV control interfaces.

The UI path shall remain:

QML
→ C++ CORE
→ VALIDATION / SAFETY
→ ADAPTER
→ EXTERNAL SYSTEM

## Telemetry and Mission Graph

Telemetry shall be usable by the Mission Graph to evaluate runtime conditions and events.

Telemetry changes may trigger applicable:

- conditions;
- events;
- branching;
- confirmations;
- adaptation;
- emergency behaviour.

## Validation Integration

Telemetry shall be available to Mission Validation when required.

Validation may use telemetry to evaluate:

- energy;
- communication;
- position;
- altitude;
- timing;
- mission progress;
- environmental conditions;
- fleet state;
- other applicable constraints.

## Safety Integration

Safety-relevant telemetry shall be available to the Safety Engine.

The Safety Engine shall be able to evaluate applicable changes in:

- position;
- altitude;
- energy;
- communication;
- health;
- environment;
- fleet state;
- mission state.

Telemetry itself shall not authorize an action. Safety decisions shall remain under the defined Safety Engine authority.

## Mission AI Integration

Mission AI may consume normalized telemetry and mission data to:

- detect changes;
- propose adaptations;
- propose replanning;
- generate alternatives;
- support resource allocation;
- support conflict resolution;
- explain mission behaviour.

Mission AI shall not bypass the Core, Validation or Safety boundaries.

## Simulation Integration

Simulation shall provide telemetry through the same logical Core data architecture.

The system shall support processing of simulated data without requiring a separate UI data model.

This allows QML and applicable Core modules to work with real and simulated mission states through common interfaces.

## Data Recording

The architecture shall support recording of relevant mission data for:

- mission reconstruction;
- validation;
- simulation comparison;
- troubleshooting;
- analysis;
- Mission Memory;
- knowledge generation.

Recorded data shall preserve mission revision and applicable temporal context.

## Data Integrity

Critical mission and telemetry data shall preserve sufficient integrity and provenance to support operational analysis.

The system shall distinguish:

- source;
- timestamp;
- mission revision;
- data state;
- applicable adapter;
- processing stage when required.

## Data and State Synchronization

The Core shall provide consistent state updates to connected modules.

Changes to authoritative mission state shall be propagated through defined interfaces.

Modules shall not silently maintain incompatible mission state representations.

## Traceability

Relevant data shall remain traceable to:

- source;
- mission revision;
- mission state;
- timestamp;
- originating adapter;
- processing module when applicable;
- resulting decision or event.

## Rationale

A centralized Core data architecture provides a common data model for real execution, simulation, telemetry, validation, safety, Mission AI and QML while preserving clear ownership and preventing the UI from becoming an operational authority.


