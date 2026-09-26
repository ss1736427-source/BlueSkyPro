# BlueSky PRO — Controlled Project Status & System Overview

## 1. Purpose

This document is the controlled external-facing summary of the BlueSky PRO project.

It is intended for:

- initial technical familiarization;
- architecture review;
- estimation and planning;
- specialist onboarding;
- discussion with potential engineering partners.

It is not a substitute for the protected project knowledge base and is not intended to provide a reproducible specification.

## 2. Product definition

BlueSky PRO is a map-first UAS flight-planning and mission-management platform intended to unify mission planning, aircraft context, environmental information, communications, operational readiness, execution records, assurance and controlled learning within one lifecycle.

## 3. Core operating idea

The user should work primarily through the Flight Chart / map-centric workspace while the surrounding system coordinates the supporting technical information.

This includes, at review level:

- mission preparation;
- aircraft and fleet context;
- route/planning support;
- telemetry and communications;
- readiness and compliance;
- operational records;
- verification and evidence;
- controlled post-flight learning.

## 4. System organization

The system is described through cooperating areas rather than one monolithic application:

- FLIGHT;
- FLIGHT CHART;
- PILOT;
- HUB;
- ADMINISTRATOR;
- INTEGRATION;
- AI / ANALYTICS;
- ASSURANCE;
- DOCUMENTATION / JOURNAL.

The detailed service decomposition, interfaces and implementation responsibilities are intentionally omitted.

## 5. Development stages

### Stage 1 — Concept and product definition
**State: substantially defined**

The product purpose, operational philosophy, core workspaces and major capability groups are established.

### Stage 2 — System and domain architecture
**State: defined / being formalized**

The repository contains an architectural foundation covering mission, aircraft, integrations, planning, assurance, regulatory context and operational lifecycle.

### Stage 3 — Planning architecture
**State: defined / implementation development**

Mission planning, objective profiles, environmental/energy considerations and multi-UAV planning direction are represented in the project material.

Detailed planning mechanisms remain protected.

### Stage 4 — Verification foundation
**State: partially implemented / under development**

The repository contains machine-readable schemas and validator/test material for substantial parts of the planning and verification model. The complete executable product integration is not yet complete.

### Stage 5 — HMI / Flight Chart
**State: development / refinement**

The map-first desktop HMI direction and workspace model are being developed, including configurable panels and operational views.

### Stage 6 — Integration
**State: defined / implementation development**

C2, telemetry, aircraft, payload, environmental and external integration boundaries are documented. Full production integrations remain a development activity.

### Stage 7 — Assurance / certification readiness
**State: working engineering baseline**

Requirements, traceability, verification, configuration and certification-readiness structures are documented. This does not constitute certification or regulatory approval.

### Stage 8 — End-to-end operational product
**State: not yet complete**

The complete chain from user mission creation through executable planning, operational integration, release and flight-record lifecycle remains under development.

## 6. Multi-UAV capability — review level

The system includes a defined workstream for coordinated multi-UAV missions.

At a high level it addresses:

- dividing a mission into manageable operational areas;
- associating suitable aircraft with those areas;
- generating and checking routes;
- considering environmental and aircraft-performance effects;
- checking fleet-level temporal/spatial consistency;
- handling limited planning conflicts;
- performing a final technical release check.

The protected project material contains the actual algorithmic specification, data contracts, validation cases, parameters and implementation components. Those details are not reproduced here.

## 7. AI capability — review level

AI is intended to support analysis, prediction and controlled learning from operational experience.

Important boundary:

**AI is not the uncontrolled source of operational authority.**

The system is designed so that recommendations or learned corrections remain subject to validation and defined operational/control rules.

## 8. Assurance and traceability

The architecture includes a traceability direction connecting:

```
Requirement
   ↓
Architecture / function
   ↓
Implementation
   ↓
Verification
   ↓
Evidence
   ↓
Controlled baseline
```

This is intended to support future engineering assurance, auditability and certification preparation.

## 9. Certification readiness

Certification-readiness is treated as an architectural concern affecting:

- requirements;
- safety/assurance;
- configuration management;
- verification evidence;
- change impact;
- operational readiness;
- relevant aircraft/configuration records.

The current material is a working engineering basis. It must not be presented as an issued certification basis, certificate or authority acceptance.

## 10. What the technical reviewer can determine

The package should allow a specialist to understand:

- the product's operational purpose;
- the major system boundaries;
- the main engineering workstreams;
- which areas are defined versus implemented;
- where implementation risk remains;
- what requires further engineering clarification.

It intentionally does not provide enough detail to reconstruct the protected system independently.

## 11. What requires the project owner

Reproduction, extension of protected subsystems, interpretation of unresolved design decisions and access to proprietary implementation knowledge require direct involvement of the project owner and the controlled engineering material.

## 12. Disclosure rule

This document must be treated as a **controlled overview**.

It should not be expanded by copying protected algorithms, formulas, internal contracts, implementation code or sensitive configuration into the review package.

