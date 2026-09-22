# BlueSky PRO — Technical Review Overview

## Purpose of this package

This branch is prepared for a professional technical review of the BlueSky PRO project by a software team working across frontend, backend, integrations and system architecture.

The objective is to expose enough of the existing product concept, architecture, requirements and implementation direction for specialists to understand the system as a whole, identify technical risks, and determine what is already defined versus what remains to be implemented.

This is **not** a source-code handoff and is not intended to expose credentials, production access, private keys, proprietary model assets, or implementation details that are unnecessary for the review.

## 1. Product in one sentence

**BlueSky PRO is a desktop-oriented UAS flight-planning and mission-management system in which the Flight Chart is the primary operational workspace and planning, aircraft state, communications, compliance, AI assistance, documentation and post-flight learning form one coordinated lifecycle.**

## 2. What the system is intended to cover

- mission definition and planning;
- aircraft and fleet representation;
- pilot and technician readiness;
- Flight Chart / route planning;
- environmental and wind-aware planning;
- energy-aware planning;
- aircraft-specific operational history;
- C2, telemetry and external integrations;
- multi-UAV mission coordination;
- operational tracking;
- post-flight records and evidence;
- AI-assisted prediction, correction and learning;
- regulatory and assurance workflows;
- aircraft-level risk and insurance data.

## 3. Operational lifecycle

```
Mission
  ↓
Aircraft selection
  ↓
Readiness
  ↓
Environment / constraints
  ↓
Route generation
  ↓
Optimization
  ↓
Human correction
  ↓
Authorization / compliance
  ↓
Execution / Tracking
  ↓
Flight record
  ↓
Post-flight analysis
  ↓
Corrections / Experience
  ↺
Future planning
```

The important architectural principle is that the system does not treat planning, execution and post-flight learning as isolated applications.

## 4. Main system blocks

| Block | Purpose |
|---|---|
| FLIGHT | Mission definition, planning and flight lifecycle |
| HUB | Communications, telemetry and data convergence |
| PILOT | Operational flight workspace |
| ADMINISTRATOR | Configuration, roles, technical conditions and maintenance context |
| INTEGRATION | Aircraft, payload, C2 and external data interfaces |
| AI | Prediction, correction learning, optimization support and external analysis |
| ASSURANCE | Requirements, verification, traceability and operational evidence |
| DOCUMENTATION | Flight records, audit trail and reusable operational knowledge |

## 5. Frontend concept

The Flight Chart is the dominant workspace.

The interface is designed around:

- persistent compact top bar;
- map-first operational view;
- compact contextual side panels;
- collapsible panels;
- telemetry and aircraft-state views;
- direct route/profile interaction;
- configurable tools;
- support for separate technical/monitoring/planning windows where required.

The intended implementation direction is a desktop application with a modern declarative UI approach; detailed implementation choices are subject to technical review.

## 6. Backend concept

The backend/domain model is expected to preserve separation between:

- mission;
- aircraft;
- fleet;
- operator/personnel;
- authorization;
- environmental observations;
- telemetry;
- external integrations;
- flight record;
- AI experience/corrections;
- assurance evidence;
- insurance/risk records.

External systems should be connected through adapters/interfaces so that the core operational model is not coupled to one aircraft, provider or communication technology.

## 7. AI concept

AI is a system capability, not merely a conversational interface.

### Internal learning layer

The system can compare:

```
forecast wind       ↔ actual wind
planned energy      ↔ actual energy
planned route       ↔ operator correction
expected behaviour  ↔ observed behaviour
```

These differences can produce structured corrections and operational experience for later planning.

### External analysis layer

A separate analytical function can evaluate external technology, regulatory changes and the surrounding technical environment.

Operational authority remains within the defined human/operator control process.

## 8. Flight planning concept

The planner is intended to evaluate:

- aircraft capability;
- installed equipment;
- mission objective;
- route geometry;
- altitude;
- restricted areas;
- environmental conditions;
- wind;
- mandatory waypoints;
- vertical/lateral bypass constraints;
- energy reserve;
- aircraft degradation history;
- multi-UAV task decomposition.

The goal is not simply the geometrically shortest route. The route is evaluated against mission objective, constraints, environmental conditions and energy preservation.

## 9. Multi-UAV concept

A mission may be decomposed between several aircraft.

Each UAV remains an independent operational entity with:

- its own identity;
- capability;
- readiness;
- authorization;
- mission state;
- flight record.

The mission layer coordinates the aircraft while preserving individual accountability.

## 10. Assurance and traceability

The project is designed around the chain:

```
Requirement
    ↓
Architecture
    ↓
Function
    ↓
Implementation
    ↓
Verification
    ↓
Operational evidence
```

This is intended to make later engineering, verification and certification work traceable.

## 11. Aircraft-level risk and insurance

The insurance/risk concept is associated with the **specific aircraft**, not merely with the mission or operator.

The architecture can retain structured information about the aircraft, its operational history, readiness, relevant assessments, insurance status and pre-flight evidence.

The detailed legal and actuarial rules remain subject to jurisdiction-specific validation and provider requirements.

## 12. Current status model

For review purposes, every major capability should be understood using four states:

- **Implemented** — working implementation exists.
- **Defined** — architecture/requirements are sufficiently specified but implementation is incomplete.
- **Planned** — intentionally included in the roadmap but not yet sufficiently specified.
- **Experimental** — under investigation or validation.

This distinction is important. The existence of a detailed architectural document does not by itself mean that a feature is production-ready.

## 13. What specialists are expected to evaluate

The technical review should concentrate on:

1. frontend architecture and UI implementation strategy;
2. backend/domain architecture;
3. API and integration boundaries;
4. persistence/data model;
5. real-time telemetry/C2 handling;
6. offline/local operation;
7. AI integration boundaries;
8. testing and verification strategy;
9. security and access boundaries;
10. implementation sequencing and technical risks.

## 14. Review entry points

Start here, then follow only the areas relevant to the specialist's role:

1. [01_PRODUCT_CONCEPT.md](01_PRODUCT_CONCEPT.md)
2. [02_SYSTEM_ARCHITECTURE.md](02_SYSTEM_ARCHITECTURE.md)
3. [03_AI_CONCEPT.md](03_AI_CONCEPT.md)
4. [04_FLIGHT_PLANNING.md](04_FLIGHT_PLANNING.md)
5. [05_MULTI_UAV_AND_DATA.md](05_MULTI_UAV_AND_DATA.md)
6. [06_ASSURANCE_AND_TRACEABILITY.md](06_ASSURANCE_AND_TRACEABILITY.md)
7. [07_DEVELOPMENT_ROADMAP.md](07_DEVELOPMENT_ROADMAP.md)
8. [08_DISCLOSURE_BOUNDARY.md](08_DISCLOSURE_BOUNDARY.md)
9. [09_PROJECT_IP_RECORD.md](09_PROJECT_IP_RECORD.md)
10. [10_TECHNICAL_REVIEW_GUIDE.md](10_TECHNICAL_REVIEW_GUIDE.md)

The detailed project knowledge base remains a separate public reference repository and should be treated as background material, not as a substitute for the controlled technical review surface.
