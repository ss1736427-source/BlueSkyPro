---
id: ARCH-DEC-001
type: architecture_decision
title: BlueSky Pilot Technology Stack
status: accepted
scope: BlueSky Pilot
---

# ARCH-DEC-001 — BlueSky Pilot Technology Stack

## Decision

The BlueSky Pilot technology stack is based on Qt/QML for the frontend and C++ for the system core.

## Frontend

- Qt 6
- QML / Qt Quick

QML / Qt Quick shall be used for the BlueSky Pilot user interface, including:

- Header
- Flight Chart
- Mission Builder
- Fleet
- Telemetry
- Notifications
- panels;
- maps;
- buttons;
- animations;
- interface states.

## Core

The system core shall use C++.

The current architectural target is:

- C++20/23

The exact C++ standard shall be determined later.

C++ shall be responsible for heavy, system-level and critical logic, including:

- Mission Engine
- Flight Control
- Telemetry
- Communications
- Safety Engine
- Route Planner
- Data Processing

## 3D / Map

The target 3D technology is:

- Qt Quick 3D

A specialized mapping engine may be integrated when required by the system architecture.

## Build

- CMake

## IDE

Supported development environments:

- Qt Creator
- VS Code

## Architecture Principle

The presentation layer and system logic shall remain separated.

QML / Qt Quick:
UI and presentation.

C++:
system, computational and critical logic.

The exact C++ standard and specific supporting libraries are intentionally not fixed by this decision.

## Rationale

Qt/QML + C++ provides a native architecture for a mission-control application where the UI requires rich maps, telemetry, animations and dynamic states while the core requires deterministic, computational and safety-critical system logic.


