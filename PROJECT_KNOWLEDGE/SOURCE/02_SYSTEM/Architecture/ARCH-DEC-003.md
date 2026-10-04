---
id: ARCH-DEC-003
type: architecture_decision
title: Core / UAV / C2 / Simulation Interfaces
status: accepted
scope: BlueSky System
---

# ARCH-DEC-003 — Core / UAV / C2 / Simulation Interfaces

## Decision

The C++ Core shall communicate with UAVs, C2 systems and Simulation / Digital Twin through defined interfaces and adapters.

## Architecture

C++ CORE
→ UAV Adapter
→ UAV / Fleet

C++ CORE
→ C2 Adapter
→ Communication Link
→ UAV / Fleet

C++ CORE
→ Simulation Adapter
→ Digital Twin

## UAV Adapter

The UAV Adapter shall provide a unified representation of:

- UAV state;
- position;
- altitude;
- velocity;
- attitude;
- battery;
- payload state;
- navigation state;
- health;
- connection state;
- mission state.

The adapter shall support the corresponding mission, navigation, payload, recovery and emergency functions required by the Core architecture.

## C2 Adapter

C2 shall be treated as a separate communication interface.

The interface shall support bidirectional information flow:

CORE
→ C2 Adapter
→ Communication Link
→ UAV / Fleet

UAV / Fleet
→ Communication Link
→ C2 Adapter
→ CORE

Changes in communication state, including loss or degradation of communication, shall be available to Mission Validation and Safety Engine.

## Simulation Adapter

Simulation / Digital Twin shall use the same mission architecture as real execution.

A mission shall be capable of being processed through:

Mission
→ Real Execution

or:

Mission
→ Simulation / Digital Twin

The architecture shall support:

PLAN
→ SIMULATE
→ EVALUATE
→ OPTIMIZE
→ SIMULATE
→ APPROVE
→ EXECUTE

## Adapter Independence

The Core shall use abstract interfaces rather than direct dependencies on specific UAV models, manufacturers, communication systems or simulation backends.

The architecture shall allow future replacement or addition of:

- UAV types;
- UAV manufacturers;
- communication systems;
- simulation backends;
- map backends;
- external system adapters.

without changing the basic Mission Architecture.

## Safety Boundary

The critical execution path shall be:

QML
→ Core
→ Safety / Validation
→ Adapter
→ External System

The UI shall not have a direct execution path to UAV control.

## Mission Data States

The architecture shall distinguish:

- PLANNED;
- SIMULATED;
- ACTUAL.

This allows Digital Twin and subsequent analysis to compare planned, simulated and actual mission behaviour.

## Rationale

Adapter-based interfaces isolate the Mission Core from specific external implementations and provide a common architecture for real execution, simulation, verification and future system integrations.


