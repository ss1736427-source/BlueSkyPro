---
id: ARCH-DEC-002
type: architecture_decision
title: Core / UI Boundary
status: accepted
scope: BlueSky Pilot
---

# ARCH-DEC-002 — Core / UI Boundary

## Decision

BlueSky Pilot shall separate the user interface from the mission and system logic.

## Architecture

BLUE SKY PILOT
→ Presentation API
→ C++ Core
→ Domain / Mission Logic
→ Adapters / External Systems

## UI Layer

The QML / Qt Quick layer is responsible for:

- presentation;
- panels;
- maps;
- buttons;
- telemetry visualization;
- animations;
- interface states;
- operator interaction.

QML shall not own critical mission logic.

## Core Layer

The C++ Core is responsible for:

- Task Engine;
- Template Engine;
- Capability Engine;
- Mission Graph;
- Mission AI;
- Mission Validation;
- Mission Readiness;
- Conflict Resolution;
- Mission Optimization;
- Simulation / Digital Twin;
- system state;
- computational logic.

## External Interfaces

The Core communicates with external systems through defined interfaces and adapters.

The UI shall not communicate directly with UAVs or other critical external systems.

## Independence

The Core shall not depend on a specific UI implementation.

Replacement of the QML / Qt interface shall not require changes to the basic Mission Architecture.

The Core should be usable without the graphical interface for:

- Simulation;
- automated verification;
- service scenarios;
- future server-side or headless operation.

## Safety Boundary

Critical execution path:

QML
→ Presentation API
→ C++ Core
→ Safety / Validation
→ Adapter
→ External System

Direct QML → UAV control is prohibited by architecture.

## Rationale

Separation of presentation and system logic improves modularity, testability, safety isolation and future extensibility.


