---
id: ARCH-DEC-013
type: architecture_decision
title: Communication and C2 Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-013 — Communication and C2 Architecture

## Decision

BlueSky shall treat Command and Control (C2) and communication as explicit architectural capabilities and interfaces.

C2 shall remain separated from the Mission Core while providing bidirectional communication between the Core and UAV / Fleet systems.

## Communication Architecture

The primary communication path shall be:

C++ CORE
→ C2 ADAPTER
→ COMMUNICATION LINK
→ UAV / FLEET

The reverse path shall be:

UAV / FLEET
→ COMMUNICATION LINK
→ C2 ADAPTER
→ C++ CORE

## C2 Adapter

The C2 Adapter shall provide an abstraction between the Mission Core and specific C2 implementations.

The adapter shall expose applicable:

- command state;
- telemetry state;
- connection state;
- communication quality;
- latency;
- link availability;
- link degradation;
- link loss;
- acknowledgement state;
- communication health.

The Core shall not depend directly on a specific communication implementation.

## Communication State

The system shall represent communication state explicitly.

Applicable states may include:

- CONNECTED;
- DEGRADED;
- LOST;
- RECOVERING;
- UNAVAILABLE.

Additional states may be introduced when required by the communication architecture.

## Communication Quality

The system shall be able to evaluate applicable communication characteristics including:

- availability;
- latency;
- bandwidth;
- reliability;
- signal quality;
- packet loss;
- connection stability.

The exact metrics may depend on the communication technology.

## Mission Validation Integration

Communication state shall be available to Mission Validation.

Validation shall consider whether communication conditions satisfy the requirements of the current mission.

A mission may require different communication characteristics depending on:

- mission type;
- UAV role;
- payload;
- required capabilities;
- operating area;
- autonomy level;
- fleet coordination requirements;
- contingency requirements.

## Safety Integration

Communication degradation or loss shall be available to the Safety Engine.

The Safety Engine shall determine applicable safety responses according to mission configuration and operational conditions.

Communication failure shall not be interpreted by the communication layer itself as authorization for an unsafe action.

## Mission AI Integration

Mission AI may use communication state as an input for:

- route planning;
- relay planning;
- UAV role assignment;
- resource allocation;
- conflict resolution;
- adaptation;
- contingency planning.

Mission AI shall not override Safety Engine decisions concerning communication-related safety conditions.

## Communication Relay

The architecture shall support Communication Relay as a mission capability.

A relay may be provided by:

- a UAV;
- multiple UAVs;
- dedicated communication equipment;
- ground infrastructure;
- other approved communication systems.

The selected relay configuration shall remain subject to mission validation and safety constraints.

## Fleet Communication

For multi-UAV missions, communication architecture shall support applicable:

- UAV-to-UAV communication;
- UAV-to-ground communication;
- C2-to-UAV communication;
- relay communication;
- shared mission state;
- coordination information;
- telemetry distribution.

The exact topology shall depend on the mission and available communication infrastructure.

## Loss of Communication

When communication is lost, the system shall detect the condition and expose it to the applicable mission and safety mechanisms.

The system may then require:

- recovery;
- relay reallocation;
- route adaptation;
- mission adaptation;
- replanning;
- return behaviour;
- emergency behaviour;
- mission termination.

The applicable response shall be determined by mission configuration and Safety Engine rules.

## Communication Degradation

Communication degradation shall be treated separately from complete communication loss.

The system shall be able to detect a degradation condition and evaluate whether the current mission remains feasible.

If degradation causes mission requirements or safety constraints to fail, the mission shall enter the applicable revalidation or replanning path.

## Runtime Communication Monitoring

During execution, communication state shall be continuously or periodically monitored according to the applicable system configuration.

A material communication change may trigger:

EXECUTING
→ REVALIDATE
→ ADAPT

or:

EXECUTING
→ REQUIRES_REPLAN

or:

EXECUTING
→ ABORTED

according to applicable safety decisions.

## Simulation Integration

Communication behaviour shall be representable in Simulation / Digital Twin.

Simulation may model:

- communication availability;
- degradation;
- loss;
- latency;
- relay behaviour;
- fleet communication;
- recovery.

Simulation results may be used by Mission Validation, Mission Readiness and Mission Optimization.

## Mission State Integration

Communication conditions shall integrate with the Mission State Machine.

A communication condition that materially changes mission feasibility shall be able to trigger:

- VALIDATION_FAILED;
- CONFLICT;
- BLOCKED;
- REQUIRES_REPLAN;
- ABORTED.

## Safety Boundary

The communication layer shall not create a direct path from the UI to UAV control.

The execution architecture remains:

QML
→ C++ CORE
→ VALIDATION / SAFETY
→ C2 ADAPTER
→ COMMUNICATION LINK
→ UAV / FLEET

## Adapter Independence

The architecture shall permit replacement or addition of:

- communication protocols;
- C2 systems;
- radio systems;
- network technologies;
- relay mechanisms;
- external communication services.

Such changes shall not require changes to the basic Mission Architecture.

## Traceability

Communication-related decisions shall remain traceable to:

- mission revision;
- communication requirements;
- selected C2 configuration;
- selected relay configuration;
- communication state;
- validation results;
- safety decisions;
- adaptation or replanning decisions;
- operator decision when applicable.

## Rationale

Treating C2 and communication as explicit interfaces and capabilities allows BlueSky to support different communication technologies while maintaining mission validation, fleet coordination, runtime adaptation and safety authority.


