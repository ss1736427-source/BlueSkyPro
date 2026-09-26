# BlueSky PRO — Controlled Technical Overview

## Purpose

This package is a controlled orientation document for a professional technical review of BlueSky PRO.

It is intended to answer three questions:

1. What is the product?
2. What major system capabilities and engineering workstreams exist?
3. What is the current maturity of each area?

It is **not** a technical handover and is deliberately insufficient to reproduce the product without the project owner and the protected engineering material.

## Product in one sentence

**BlueSky PRO is a desktop-oriented UAS flight-planning and mission-management system built around a map-first operational workspace, integrating planning, aircraft state, communications, compliance, operational records and controlled decision support.**

## What the review package exposes

The package exposes only review-level information:

- product purpose and operational lifecycle;
- major system blocks and responsibilities;
- principal engineering workstreams;
- high-level planning and multi-UAV concepts;
- UI/HMI direction;
- integration boundaries;
- assurance and traceability philosophy;
- certification-readiness direction;
- development maturity and remaining work.

It intentionally does **not** expose the implementation recipe.

## Main system areas

| Area | Review-level purpose |
|---|---|
| FLIGHT | Mission definition, planning and operational workflow |
| FLIGHT CHART | Primary map-centric planning and operational workspace |
| PILOT | Flight-oriented operational information and controls |
| HUB | Communications, telemetry and data convergence |
| ADMINISTRATOR | Configuration, roles, technical and operational context |
| INTEGRATION | Aircraft, payload, C2 and external interfaces |
| AI / ANALYTICS | Prediction, analysis and learning from approved operational data |
| ASSURANCE | Requirements, verification, traceability and evidence |
| DOCUMENTATION / JOURNAL | Controlled records, audit trail and reusable operational knowledge |

## Operational lifecycle

At review level the intended lifecycle is:

```
Mission
  ↓
Aircraft / configuration
  ↓
Readiness
  ↓
Environment and constraints
  ↓
Planning
  ↓
Verification
  ↓
Authorization / operational workflow
  ↓
Execution / tracking
  ↓
Flight record
  ↓
Post-flight analysis
  ↓
Controlled learning / improvement
```

The detailed sequencing, dependency rules and implementation mechanisms remain protected engineering material.

## Principal capability groups

### Flight planning

The system is intended to combine mission objectives, aircraft capability, environmental conditions, operational constraints and energy considerations when preparing a mission.

The project also includes a multi-aircraft planning workstream. The review package describes this only at the conceptual level; exact planning heuristics, optimization logic, correction strategy, thresholds, internal data structures and implementation details are intentionally withheld.

### Multi-UAV operations

The architecture supports coordinated missions involving multiple aircraft, with individual aircraft state and a combined mission context.

The project has a defined direction for:

- spatial task decomposition;
- aircraft-to-task allocation;
- route generation;
- temporal/spatial consistency checking;
- controlled conflict handling;
- final technical verification.

The exact algorithms, ordering criteria, numerical bounds and executable implementation are not part of this review package.

### C2, telemetry and integrations

The system is designed around adapter-style boundaries for:

- aircraft/autopilot communication;
- C2 and telemetry;
- environmental information;
- map services;
- regulatory/authorization services;
- payload and data interfaces;
- future provider integrations.

The review objective is to assess the boundaries and responsibilities, not to receive provider-specific implementation secrets.

### AI

AI is treated as a system capability rather than as the operational authority.

At review level the intended separation is:

```
Operational data
      ↓
Analysis / prediction
      ↓
Recommendation or correction candidate
      ↓
Validation / operational authority
      ↓
Approved operational result
```

The package intentionally excludes model architecture, proprietary scoring, training data, weights, calibration and protected learning mechanisms.

### Assurance and certification readiness

The project includes dedicated workstreams for requirements, verification, traceability, configuration control, safety-related assurance and certification preparation.

This is **readiness architecture and working engineering material**, not a claim of certification, regulatory approval or authority acceptance.

## Current maturity model

To avoid confusing documentation maturity with implementation maturity, each area is classified separately:

- **Implemented** — working implementation is evidenced in the current repository state.
- **Partially implemented** — some executable or demonstrable parts exist, but the capability is incomplete.
- **Defined** — architecture, requirements or contracts are sufficiently defined, with implementation still incomplete.
- **Planned** — intentionally on the roadmap but not yet defined to implementation level.
- **Experimental** — under technical investigation or validation.

A document, schema, test, prototype or architecture description does **not** by itself mean that a capability is production-ready.

## Current repository picture

The current repository contains a substantial architectural and verification foundation, including:

- product and system lifecycle documentation;
- a common mission model and mission-objective definitions;
- planning/orchestration architecture;
- C2/connectivity architecture;
- vehicle/payload integration architecture;
- ATM/regulatory integration architecture;
- operational-validation architecture;
- controlled-review documentation;
- machine-readable planning/verification schemas and validator material on the review branch.

At the same time, the repository documentation explicitly distinguishes the architecture baseline from the not-yet-complete executable product implementation. In particular, the controlled-review material states that the repository does not yet contain the executable Qt/C++/QML planning core.

Therefore the current project should be understood as an **engineering architecture and implementation-development stage**, not as a completed operational product.

## Review focus

A technical team should evaluate:

1. frontend/HMI architecture and implementation readiness;
2. backend/domain boundaries;
3. data and persistence model;
4. C2/telemetry and integration boundaries;
5. offline/local-operation strategy;
6. planning-engine implementation path;
7. AI integration boundaries;
8. verification and traceability;
9. security and access control;
10. implementation risks and sequencing.

## Controlled disclosure principle

The purpose of this package is to make the project understandable without making it reconstructable.

The following are intentionally excluded:

- source-code implementation of protected subsystems;
- proprietary algorithms and optimization recipes;
- exact formulas, thresholds, coefficients and calibration values;
- internal schemas whose combination would enable reproduction;
- model weights or private training data;
- production infrastructure details;
- credentials, keys or security bypasses;
- unreleased product decisions and implementation details.

The project owner remains the source of the missing engineering context required to reproduce, extend or validate the protected system.

## Review branch

The controlled technical review surface is maintained separately from the main project baseline.

**Important:** a branch name or branch URL is not itself an access-control boundary. Repository permissions must be controlled at the repository/account level.

## Project status

**Current overall state: ARCHITECTURE / ENGINEERING DEVELOPMENT**

The project has moved beyond a pure concept: core architecture, domain direction, assurance structure and substantial verification foundations are documented and partially formalized. The executable product, including the full planning core and complete end-to-end operational implementation, remains under development.

