# BlueSky PRO — Project Capability Map

## Purpose

This document maps the major capabilities already represented in the BlueSky PRO knowledge base to the controlled technical-review surface.

It is an orientation and architecture map, not a copy of the knowledge base. Detailed requirements, algorithms, evidence and implementation records remain in the project knowledge base.

## Capability map

| Capability | What it covers | Knowledge-base area | Review entry point | Review status |
|---|---|---|---|---|
| Product lifecycle | Mission → aircraft → readiness → planning → authorization → execution → evidence → learning | `00_MASTER`, `00_PROJECT` | `00_PROJECT_OVERVIEW.md` | Defined |
| Requirements | Regulatory, system, software and interface requirements | `01_REQUIREMENTS` | `06_ASSURANCE_AND_TRACEABILITY.md` | Defined |
| Regulatory | Regulatory sources, applicability, clause mapping and external regulatory interfaces | `01_REQUIREMENTS/REGULATORY` | `12_CERTIFICATION_READINESS.md` | Defined |
| Safety | Hazard log, safety requirements, safety case and safety allocation | `01_REQUIREMENTS/SAFETY` | `12_CERTIFICATION_READINESS.md` | Defined |
| System architecture | Domain boundaries, system decomposition and interfaces | `02_ARCHITECTURE` | `02_SYSTEM_ARCHITECTURE.md` | Defined |
| Administrator | Roles, access, technician/engineer authority and configuration context | `02_ARCHITECTURE/ADMINISTRATOR` | `02_SYSTEM_ARCHITECTURE.md` | Defined |
| Flight | Mission lifecycle, planning and operational state | `02_ARCHITECTURE`, `03_SYSTEM` | `01_PRODUCT_CONCEPT.md` | Defined |
| Navigation | Navigation model, route/WP logic, verification and traceability | navigation-related requirements and verification records | `04_FLIGHT_PLANNING.md` | Defined |
| C2 / communications | C2 requirements, channel management, degradation and verification | C2 regulatory/system/safety/verification records | `02_SYSTEM_ARCHITECTURE.md` | Defined |
| HUB / telemetry | Data convergence, normalized telemetry and operational state | `03_SYSTEM`, telemetry/C2 records | `02_SYSTEM_ARCHITECTURE.md` | Defined |
| Integration | Aircraft, autopilot, payload, C2 and external data adapters | `01_REQUIREMENTS/INTEROPERABILITY`, `03_SYSTEM`, software adapter records | `10_TECHNICAL_REVIEW_GUIDE.md` | Defined |
| AI | Internal learning, corrections, prediction and external analysis | AI-related requirements, architecture and orchestration records | `03_AI_CONCEPT.md` | Defined / Experimental |
| Operational Orchestrator | Runtime context, readiness, decision pipeline, next action, mission adaptation and decision evidence | `00_PROJECT/EXTERNAL_ORCHESTRATOR`, CI/workflow records | `03_AI_CONCEPT.md` + technical review | Experimental / Defined |
| Flight planning | Wind/environment, energy, constraints, route generation and corrections | system/planning requirements and design records | `04_FLIGHT_PLANNING.md` | Defined |
| Multi-UAV | Task decomposition, independent aircraft state and coordinated mission | multi-UAV architecture/requirements/verification | `05_MULTI_UAV_AND_DATA.md` | Defined |
| HMI | Map-first Flight Chart, panels, operational views and role-oriented workspace | `08_HMI` | `00_PROJECT_OVERVIEW.md` | Defined |
| Verification | Test, analysis, inspection, simulation, demonstration and evidence | `05_VERIFICATION` | `06_ASSURANCE_AND_TRACEABILITY.md` | Defined |
| Traceability | Requirement → architecture → implementation → verification → evidence | `01_REQUIREMENTS/TRACEABILITY`, `10_TRACEABILITY` | `06_ASSURANCE_AND_TRACEABILITY.md` | Defined |
| Configuration management | Configuration items, baselines, applicability and controlled changes | `00_PROJECT/CONFIGURATION` | `12_CERTIFICATION_READINESS.md` | Defined |
| Certification | Scope, basis, strategy, requirements, compliance, verification and evidence | `06_CERTIFICATION` | `12_CERTIFICATION_READINESS.md` | Working draft |
| Aircraft-level risk / insurance | Aircraft identity, configuration, maintenance, history, readiness and insurance status | project risk/insurance records where applicable | `12_CERTIFICATION_READINESS.md` | Defined |
| Documentation / Journal | Flight records, audit trail and reusable operational knowledge | `00_PROJECT`, documentation/Journal records | `06_ASSURANCE_AND_TRACEABILITY.md` | Defined |
| Development / CI | Automated architecture, adapter, telemetry, orchestrator and planning validation | `.github/workflows` | `10_TECHNICAL_REVIEW_GUIDE.md` | Implemented / Experimental |

## Architecture relationship

The intended relationship is:

```
Regulation / operational context
          ↓
Requirements
          ↓
Safety / architecture
          ↓
System functions
          ↓
Software / interfaces
          ↓
Verification
          ↓
Evidence / traceability
          ↓
Configuration baseline
          ↓
Operational readiness
          ↓
Flight execution
          ↓
Flight record
          ↓
AI corrections / experience
          ↺
Future planning
```

AI recommendations remain subject to defined safety, compliance and operational authority boundaries. Learning must not silently modify a controlled certification baseline.

## What a technical team should inspect

### Frontend
- Flight Chart and map-first shell
- panel/window architecture
- aircraft and telemetry state
- route/profile editing
- readiness and compliance views
- configuration and evidence views
- role-based workspace behavior

### Backend
- Mission / Aircraft / Fleet domain boundaries
- readiness and authorization state
- C2 and telemetry normalization
- navigation and planning services
- integration adapters
- flight records
- Journal / audit trail
- requirements and evidence objects
- configuration baselines
- AI experience/correction storage

### Safety and assurance
- authoritative state versus AI recommendation
- Safety Gate and execution authority
- requirement allocation
- verification cases and evidence
- configuration references
- change-impact analysis
- regression strategy

### Integration
- autopilot
- C2
- telemetry
- weather/environment
- maps
- regulatory/authorization interfaces
- payload/data processing
- risk/insurance provider interfaces

## Disclosure boundary

This map intentionally exposes architecture and capability boundaries while avoiding:
- proprietary optimization formulas;
- calibration coefficients;
- model weights;
- private training datasets;
- credentials and keys;
- production infrastructure secrets;
- implementation details unnecessary for initial technical review.

## Status interpretation

- **Implemented** — working implementation is present.
- **Defined** — architecture/requirements are sufficiently specified, implementation may be incomplete.
- **Planned** — intentionally included but not yet sufficiently specified.
- **Experimental** — under investigation or validation.

A documented capability is not automatically production-ready or certification-approved.

## Primary references

- `00_PROJECT_OVERVIEW.md`
- `10_TECHNICAL_REVIEW_GUIDE.md`
- `12_CERTIFICATION_READINESS.md`

Detailed source records remain in `BlueSky-PRO-Knowledge` and should be consulted selectively during technical review.
