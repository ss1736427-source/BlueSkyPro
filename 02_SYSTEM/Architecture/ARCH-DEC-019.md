---
id: ARCH-DEC-019
type: architecture_decision
title: Configuration Versioning and Reproducibility Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-019 — Configuration / Versioning / Reproducibility Architecture

## Decision

BlueSky shall provide explicit configuration management, versioning and reproducibility mechanisms for missions, system configuration, capabilities, spatial data, models, simulation environments and other inputs that materially affect mission planning or execution.

A mission result shall remain associated with the configuration and data versions used to produce that result.

## Configuration Identity

Each mission shall reference an explicit configuration identity.

The configuration shall identify applicable:

- mission revision;
- system configuration;
- capability configuration;
- UAV configuration;
- payload configuration;
- communication configuration;
- optimization profile;
- map data;
- terrain data;
- airspace data;
- geofence data;
- weather data;
- Mission AI configuration;
- simulation configuration.

## Mission Revision

A material mission change shall create a new mission revision.

The system shall preserve relationships between revisions.

The architecture shall support:

MISSION
→ REVISION
→ CONFIGURATION
→ VALIDATION
→ SIMULATION
→ APPROVAL
→ EXECUTION
→ ACTUAL RESULT

Previous revisions shall remain available for traceability and analysis.

## Configuration Versioning

Material system configuration shall be versioned.

Applicable versioned configuration may include:

- safety configuration;
- validation rules;
- route planning configuration;
- optimization configuration;
- fleet configuration;
- communication configuration;
- UI configuration where relevant;
- simulation configuration;
- external adapter configuration.

## Capability Versioning

Mission capabilities shall be associated with identifiable versions or configurations where their behaviour may affect mission planning or execution.

Changes to capability definitions that materially affect mission behaviour shall be traceable.

## UAV and Payload Configuration

UAV and payload characteristics used by a mission shall be recorded.

Applicable information may include:

- platform type;
- platform configuration;
- payload type;
- payload configuration;
- performance parameters;
- energy characteristics;
- communication characteristics;
- capability set.

The mission shall not depend on an unrecorded implicit platform configuration.

## Spatial Data Versioning

Spatial inputs shall be versioned or otherwise identifiable where applicable.

This includes:

- maps;
- terrain;
- airspace;
- geofences;
- spatial restrictions.

A mission result shall identify the spatial data used during planning or validation.

## Environmental Data

Material environmental inputs shall be associated with the applicable mission or simulation state.

Where applicable, the system shall preserve:

- weather data;
- forecast version or source;
- environmental conditions;
- relevant timestamps.

## Mission AI Configuration

Mission AI results shall remain associated with the applicable AI configuration.

Where technically applicable, this may include:

- model identity;
- model version;
- configuration;
- prompts or structured inputs;
- policy configuration;
- decision parameters.

AI configuration changes that materially affect planning results shall be traceable.

## Optimization Profile

The active optimization profile shall be explicitly recorded.

Supported profiles include:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

The system shall preserve the selected profile and applicable optimization parameters.

## Simulation Environment

Simulation results shall identify the simulation environment used.

Applicable information may include:

- simulation version;
- scenario;
- environment configuration;
- terrain dataset;
- weather dataset;
- vehicle configuration;
- payload configuration;
- communication configuration;
- simulation parameters.

## Reproducibility

The architecture shall support reproduction of a historical planning or simulation result when the required source data and configurations remain available.

Reproduction shall use the recorded mission revision and applicable configuration versions.

## Planned / Simulated / Actual

The system shall preserve the relationship between:

- PLANNED;
- SIMULATED;
- ACTUAL.

A comparison shall remain associated with the corresponding mission revision and configuration.

The architecture shall support:

PLANNED
vs
SIMULATED
vs
ACTUAL

analysis.

## Validation Reproducibility

Validation results shall identify the configuration and rule versions used for the validation.

A historical validation result shall not be represented as though it were generated using a newer validation configuration unless it is explicitly revalidated.

## Safety Configuration

Safety-related configuration shall be versioned.

Changes to safety configuration shall be traceable.

A mission shall identify the safety configuration applicable to its validation and execution.

## Readiness Reproducibility

Mission Readiness results shall remain associated with:

- mission revision;
- validation results;
- readiness configuration;
- applicable environmental inputs;
- applicable fleet state.

A historical READY result shall not automatically imply that the current mission is READY.

## Approval Reproducibility

Approval shall be associated with the exact mission revision approved.

A new mission revision shall not inherit approval automatically unless the applicable architecture explicitly permits it.

## Execution Reproducibility

Execution records shall identify the mission revision and configuration used for execution.

Actual results shall remain associated with the corresponding execution instance.

## Configuration Immutability

Once a configuration is referenced by a completed validation, approval or execution record, the referenced configuration shall not be silently modified.

Material changes shall create a new configuration version.

## Dependency Graph

The architecture shall support a dependency relationship between relevant versions:

MISSION REVISION
→ SYSTEM CONFIGURATION
→ CAPABILITIES
→ UAV / PAYLOAD
→ SPATIAL DATA
→ ENVIRONMENT
→ AI CONFIGURATION
→ OPTIMIZATION
→ SIMULATION
→ VALIDATION
→ READINESS
→ APPROVAL
→ EXECUTION

## Revalidation After Change

A material configuration change shall invalidate or require reassessment of affected results where applicable.

The system may require:

CONFIGURATION CHANGE
→ REVALIDATE
→ REASSESS READINESS
→ SAFETY GATE
→ APPROVAL WHEN REQUIRED

## Configuration Comparison

The system shall support comparison between configuration versions where required.

Comparison may identify:

- changed parameters;
- added elements;
- removed elements;
- changed capabilities;
- changed constraints;
- changed optimization settings;
- changed spatial data;
- changed simulation configuration.

## Audit Integration

Configuration changes shall be audited.

The audit record shall identify:

- previous version;
- new version;
- author;
- timestamp;
- reason where applicable;
- affected mission or system component.

## Mission Memory Integration

Mission Memory shall preserve references to the configuration used by historical missions.

Historical knowledge shall remain associated with its original context.

Mission Memory shall not silently apply a historical configuration to a new mission.

## Security Integration

Configuration changes shall require appropriate authorization.

Safety-critical configuration shall require the applicable elevated permissions.

Unauthorized configuration changes shall be rejected and audited.

## Operator Visibility

The operator shall be able to inspect the configuration identity relevant to a mission.

The system shall provide sufficient information to understand which configuration and data versions contributed to:

- mission planning;
- validation;
- simulation;
- readiness;
- approval;
- execution.

## Traceability

Configuration traceability shall connect:

MISSION
→ REVISION
→ CONFIGURATION
→ DATA VERSIONS
→ VALIDATION
→ SIMULATION
→ READINESS
→ APPROVAL
→ EXECUTION
→ ACTUAL RESULT

## Rationale

Explicit versioning and reproducibility prevent historical mission results from becoming ambiguous and allow BlueSky to determine which mission, configuration, data, model and validation context produced a particular decision or operational result.


