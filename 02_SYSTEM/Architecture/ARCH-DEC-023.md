---
id: ARCH-DEC-023
type: architecture_decision
title: Deployment Runtime Environment and Operational Modes Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-023 — Deployment / Runtime Environment / Operational Modes Architecture

## Decision

BlueSky shall support controlled deployment and runtime operation across defined operational environments and modes.

The deployment architecture shall preserve the separation between planning, simulation, sandbox, validation, approval and real execution.

## Runtime Environments

The architecture shall distinguish at minimum:

- Development;
- Test;
- Simulation;
- Sandbox;
- Operational.

Each environment shall have an explicit identity and configuration.

## Environment Isolation

Simulation and Sandbox environments shall remain isolated from real UAV execution.

Development and Test environments shall not automatically obtain operational execution authority.

Operational execution shall require the applicable authorization, validation, readiness and safety gates.

## Operational Modes

The system shall support explicit operational modes including:

- Planning;
- Validation;
- Optimization;
- Simulation;
- Readiness Review;
- Approval;
- Execution;
- Monitoring;
- Recovery;
- Emergency.

The active mode shall be visible to the Core and relevant UI services.

## Mode Authority

Operational mode shall be controlled by the C++ Core.

QML shall display and request mode changes through Core services.

QML shall not directly transition the system into safety-critical execution.

## Planning Mode

Planning mode shall support:

- mission creation;
- task configuration;
- UAV selection;
- capability selection;
- route planning;
- constraints;
- mission configuration.

Planning mode shall not directly execute UAV commands.

## Validation Mode

Validation mode shall execute the applicable validation processes.

Validation shall evaluate:

- individual constraints;
- spatial constraints;
- mission constraints;
- dynamic conditions.

A failed mandatory validation shall prevent progression to the applicable downstream gate.

## Optimization Mode

Optimization mode shall generate and evaluate candidate mission variants.

Supported profiles include:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

Optimization shall remain subordinate to mandatory safety and validation constraints.

## Simulation Mode

Simulation mode shall execute the mission through Simulation / Digital Twin.

Simulation shall use the Mission Architecture without creating a direct path to real UAV execution.

Simulation results shall be identified as SIMULATED.

## Readiness Mode

Readiness mode shall evaluate current mission readiness.

Readiness shall consider applicable:

- airspace;
- terrain;
- geofence;
- weather;
- C2 coverage;
- fleet coordination;
- energy;
- contingency.

A previous READY result shall not automatically make a changed mission READY.

## Approval Mode

Approval mode shall provide the authorized operator or authority with the information required for mission approval.

Approval shall reference the exact mission revision.

Approval shall not bypass Safety Engine or mandatory validation.

## Execution Mode

Execution mode shall be entered only after the applicable gates are satisfied.

The execution path shall remain:

QML
→ CORE
→ AUTHORIZATION
→ VALIDATION / SAFETY
→ ADAPTER
→ EXTERNAL SYSTEM.

QML shall not directly control UAV execution.

## Monitoring Mode

During execution the system shall continuously monitor relevant:

- UAV state;
- mission state;
- communication;
- energy;
- weather;
- spatial conditions;
- fleet state;
- safety conditions.

Material changes shall trigger the applicable revalidation or adaptation path.

## Recovery Mode

Recovery mode shall support:

- recovery;
- adaptation;
- replanning;
- resource reassignment;
- contingency activation.

Recovery shall remain subject to Safety Engine authority.

## Emergency Mode

Emergency mode shall provide access to applicable emergency behaviour.

Emergency operation shall not depend on QML availability.

Emergency functions shall remain implemented through Core and approved external adapters.

## State Persistence

The active operational state shall be recorded where required.

State records shall remain associated with:

- mission revision;
- configuration;
- environment;
- operator identity where applicable;
- timestamp.

## Configuration

Runtime environment and deployment configuration shall be versioned according to ARCH-DEC-019.

Material configuration changes shall be traceable.

## Deployment Profiles

The architecture shall support deployment profiles appropriate to:

- development;
- test;
- simulation;
- sandbox;
- operational deployment.

Profiles may differ in enabled services and external connections.

## External Connections

External system connections shall be explicitly configured for the active environment.

Simulation and Sandbox shall use isolated or simulated external interfaces where required.

Operational deployment shall use only authorized external systems.

## Feature Availability

Environment-specific features shall be explicitly identified.

A feature unavailable in an environment shall not appear to be operational.

Safety-critical functionality shall not be silently replaced by a non-equivalent development or simulation implementation.

## Startup Validation

At startup the Core shall verify applicable runtime conditions including:

- configuration validity;
- required services;
- adapter availability;
- security state;
- mission state where applicable;
- safety services;
- data integrity.

Operational execution shall be blocked if mandatory startup conditions fail.

## Runtime Health

The system shall expose runtime health for relevant services.

Health information may include:

- service state;
- adapter state;
- communication state;
- storage state;
- processing state;
- safety state.

## Failure Handling

Runtime failures shall integrate with ARCH-DEC-017.

Applicable responses may include:

- warning;
- degraded mode;
- recovery;
- revalidation;
- replanning;
- blocking;
- abort;
- emergency response.

## Performance

Runtime resource management shall follow ARCH-DEC-022.

Non-critical workloads shall not compromise safety-critical runtime processing.

## Security

Runtime environments shall follow ARCH-DEC-018.

Environment identity shall contribute to access and authorization decisions where applicable.

Operational execution authority shall not be inherited by Simulation or Sandbox.

## Data Lifecycle

Runtime data shall follow ARCH-DEC-021.

The system shall preserve distinction between:

- PLANNED;
- SIMULATED;
- ACTUAL.

## Reproducibility

Runtime environment identity and relevant configuration versions shall be recorded to support historical analysis and reproducibility.

## Upgrade and Deployment

Software and configuration updates shall be controlled.

Operational updates shall preserve:

- configuration traceability;
- version identity;
- rollback information where applicable;
- safety configuration;
- mission compatibility.

## Rollback

Where supported, deployment rollback shall restore a known valid software/configuration state.

Rollback shall not silently alter historical mission records.

## Operator Visibility

The operator shall be able to determine:

- active environment;
- active operational mode;
- software version;
- configuration version;
- connection state;
- service health;
- safety state.

## Audit

Material mode transitions and deployment changes shall be auditable.

Audit records shall include applicable:

- timestamp;
- identity;
- environment;
- previous mode;
- new mode;
- configuration;
- result.

## Traceability

Deployment and runtime traceability shall connect:

ENVIRONMENT
→ SOFTWARE VERSION
→ CONFIGURATION
→ OPERATIONAL MODE
→ MISSION REVISION
→ VALIDATION
→ READINESS
→ APPROVAL
→ EXECUTION
→ ACTUAL RESULT.

## Rationale

Explicit runtime environments and operational modes prevent simulation, development and sandbox activities from becoming accidental execution paths and provide controlled transitions from mission planning through validated operational execution.


