---
id: ARCH-DEC-021
type: architecture_decision
title: Data Lifecycle Storage and Recording Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-021 — Data Lifecycle / Storage / Recording Architecture

## Decision

BlueSky shall provide a controlled architecture for creation, storage, synchronization, recording, retrieval and lifecycle management of mission and operational data.

The architecture shall preserve the relationship between planned, simulated and actual mission states.

## Data Authority

The BlueSky Core shall maintain the authoritative mission state.

The architecture shall distinguish:

- authoritative operational state;
- derived state;
- historical records;
- telemetry;
- events;
- simulation data;
- audit data;
- external source data.

QML shall consume Core data and shall not become the authoritative owner of mission state.

## Mission Data Lifecycle

Mission data shall support the lifecycle:

DRAFT
→ CONFIGURED
→ PLANNED
→ VALIDATED
→ OPTIMIZED
→ SIMULATED
→ READY
→ APPROVED
→ EXECUTING
→ COMPLETED

Exceptional states shall remain traceable.

## Planned Data

PLANNED data shall represent the mission configuration intended for execution.

It shall preserve applicable:

- mission revision;
- route;
- task configuration;
- UAV allocation;
- payload configuration;
- timing;
- constraints;
- optimization profile;
- validation state;
- readiness state;
- approval state.

## Simulated Data

SIMULATED data shall represent the result of virtual mission execution.

It shall remain associated with:

- mission revision;
- simulation scenario;
- simulation configuration;
- simulation environment;
- simulation timestamp;
- relevant input versions.

## Actual Data

ACTUAL data shall represent observed execution results.

It may include:

- actual trajectory;
- actual position;
- actual altitude;
- actual velocity;
- actual battery;
- actual payload state;
- actual communication state;
- actual mission state;
- actual events;
- actual task results.

## Telemetry

Telemetry shall be ingested through the appropriate Core interfaces.

The architecture shall support telemetry from:

- UAV;
- payload;
- C2;
- communication systems;
- positioning systems;
- other approved operational sources.

Telemetry shall be timestamped and associated with the applicable source.

## Telemetry Normalization

Telemetry shall be normalized before being exposed as authoritative Core state where applicable.

Normalization may include:

- units;
- timestamps;
- coordinate systems;
- identifiers;
- state representations;
- source identifiers.

## Event Recording

The system shall record material mission events.

Events may include:

- mission state changes;
- task transitions;
- validation results;
- readiness changes;
- approvals;
- conflicts;
- replanning;
- adaptation;
- communication changes;
- battery events;
- safety events;
- emergency events;
- operator actions.

## Event Ordering

Events shall preserve sufficient temporal information to reconstruct the relevant mission sequence.

Where distributed sources are involved, the architecture shall preserve source timestamps and system timestamps where required.

## Mission History

The system shall maintain mission history across revisions.

Mission history shall preserve:

- mission revisions;
- configuration versions;
- validation results;
- simulation results;
- readiness results;
- approval records;
- execution records;
- actual results.

## Data Recording

Material mission and safety-related state changes shall be recorded.

Recording shall support later:

- analysis;
- verification;
- incident review;
- replay;
- comparison;
- traceability;
- mission learning.

## Planned vs Simulated vs Actual

The architecture shall support comparison of:

PLANNED
vs
SIMULATED
vs
ACTUAL

Comparison shall preserve the mission revision and relevant configuration context.

## Data Synchronization

Data received from multiple sources shall be synchronized according to applicable source timestamps and system timing.

The architecture shall distinguish:

- current state;
- delayed state;
- stale state;
- historical state.

Stale or invalid data shall not silently replace current authoritative state.

## Data Integrity

Stored mission and operational data shall preserve integrity.

The architecture shall support detection of:

- incomplete records;
- corrupted records;
- inconsistent state;
- invalid references;
- missing dependencies.

## Storage Separation

The architecture shall separate storage concerns where appropriate.

Logical storage domains may include:

- mission store;
- configuration store;
- telemetry store;
- event store;
- simulation store;
- audit store;
- mission history store.

The exact physical storage technology shall remain an implementation decision unless otherwise specified.

## Mission Store

The Mission Store shall preserve authoritative mission definitions and revisions.

It shall support:

- creation;
- modification;
- versioning;
- retrieval;
- comparison;
- archival.

## Telemetry Store

The Telemetry Store shall preserve operational telemetry required for:

- monitoring;
- analysis;
- replay;
- verification;
- incident investigation.

## Simulation Store

The Simulation Store shall preserve simulation scenarios and results.

It shall allow association between:

- mission revision;
- simulation configuration;
- simulation result.

## Audit Store

Audit information shall remain logically distinct from ordinary mission data.

Audit records shall preserve security- and authority-relevant actions.

## Retention

Data retention shall be governed by applicable system and operational requirements.

Retention policies shall distinguish data categories where required.

Critical mission, safety, approval and audit records shall not be silently discarded as part of ordinary operational cleanup.

## Archival

Completed mission data may be archived.

Archived data shall remain identifiable and retrievable according to applicable retention requirements.

Archival shall preserve the references required to reconstruct historical mission context.

## Deletion

Deletion of data shall be controlled.

Safety-critical, approval, audit and traceability records shall not be deleted through ordinary user actions where retention requirements prohibit deletion.

Deletion actions shall be authorized and auditable where applicable.

## Access

Stored data shall be accessed through controlled Core services.

Direct unrestricted database access shall not be part of the normal QML execution path.

Authorization shall be applied according to ARCH-DEC-018.

## Mission AI

Mission AI may consume approved historical and operational data for:

- analysis;
- learning;
- optimization;
- recommendation;
- pattern detection.

Mission AI shall not silently modify authoritative historical records.

AI-generated derived knowledge shall remain distinguishable from confirmed operational records.

## Mission Memory

Mission Memory shall reference historical mission data and associated context.

Memory shall preserve the relationship between:

- user requirement;
- mission;
- configuration;
- planned result;
- simulated result;
- actual result;
- observed outcome.

## Data Lineage

Material derived data shall preserve lineage to its source.

The architecture shall support:

SOURCE
→ INGESTION
→ NORMALIZATION
→ CORE STATE
→ DERIVED RESULT
→ DECISION

where applicable.

## External Data

External data shall preserve source identity and applicable timestamps.

External information shall remain distinguishable from authoritative Core state until accepted through the appropriate integration and validation path.

## Security

Stored data shall be protected according to applicable security and authorization policies.

Access to mission, telemetry, audit and configuration data shall be controlled.

## Safety Data

Safety-relevant data shall remain available to the Safety Engine and applicable validation services.

The storage architecture shall not introduce a dependency that prevents safety-critical processing.

## Runtime Operation

Runtime data shall support the loop:

MONITOR
→ UPDATE STATE
→ VALIDATE
→ ADAPT / REPLAN
→ RECORD

where applicable.

## Replay

The architecture shall support replay or reconstruction of historical mission behaviour when sufficient data is available.

Replay shall distinguish recorded actual behaviour from newly simulated behaviour.

## Reproducibility

Historical results shall remain associated with the versions defined by ARCH-DEC-019.

Data storage shall preserve the references required for reproducibility.

## Failure Handling

Storage or recording failures shall be detectable.

A failure to persist non-critical derived data shall not silently alter authoritative mission state.

A failure affecting safety-critical recording or state integrity shall be surfaced to the appropriate safety and operational mechanisms.

## Observability

Data services shall expose sufficient operational information for diagnosis, including where applicable:

- storage status;
- synchronization status;
- ingestion status;
- recording status;
- data integrity status;
- backlog;
- errors.

## Traceability

Data traceability shall connect:

MISSION
→ REVISION
→ CONFIGURATION
→ SOURCE DATA
→ TELEMETRY / EVENTS
→ VALIDATION
→ SIMULATION
→ READINESS
→ APPROVAL
→ EXECUTION
→ ACTUAL RESULT

## Rationale

A controlled data lifecycle prevents mission state, telemetry, simulation results, audit information and historical records from becoming disconnected. It provides the foundation for operational monitoring, verification, replay, analysis, reproducibility and traceability.


