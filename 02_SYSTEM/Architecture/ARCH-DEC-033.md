---
id: ARCH-DEC-033
type: architecture_decision
title: Observability Metrics Logging and Operational Diagnostics Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-033 — Observability / Metrics / Logging / Operational Diagnostics Architecture

## Decision

BlueSky shall provide an integrated observability architecture for operational visibility, diagnostics, performance analysis, safety investigation, mission reconstruction and verification.

Observability shall combine structured logs, metrics, events, traces, health information and diagnostic records while preserving authoritative source information.

## Observability Authority

The C++ Core shall own authoritative runtime observability state for system and mission execution.

QML shall consume observability information for operator presentation.

QML shall not modify authoritative operational records.

## Observability Domains

The architecture shall observe:

- Core;
- Mission Engine;
- Safety Engine;
- Mission Validation;
- Mission Readiness;
- Mission AI;
- Mission Memory;
- UAVs;
- fleet;
- C2;
- communication;
- telemetry;
- route planning;
- optimization;
- conflict resolution;
- simulation;
- sandbox;
- data;
- configuration;
- security;
- deployment;
- recovery;
- external integrations.

## Event Model

Operational events shall have a structured representation.

An event shall support identification of:

- event ID;
- timestamp;
- severity;
- source;
- mission;
- configuration;
- release;
- operator where applicable;
- correlation ID;
- causation information where available.

## Event Identity

Events shall have stable identities suitable for correlation, investigation and replay.

Duplicate event handling shall preserve event integrity.

## Timestamp

Events shall contain timestamps generated using an authoritative system time source.

Time synchronization status shall be observable.

## Severity

The event model shall support severity levels appropriate to:

- informational;
- notice;
- warning;
- error;
- critical;
- emergency.

Severity shall not be used to hide or suppress safety-relevant events.

## Correlation

Observability records shall support correlation across:

- mission;
- UAV;
- fleet;
- operator;
- configuration;
- release;
- deployment;
- request;
- transaction;
- event chain.

## Causation

Where technically available, events shall identify the event or action that caused a resulting state change.

## Mission Correlation

Mission-related records shall contain sufficient mission identity to reconstruct the relevant mission context.

## Configuration Correlation

Material events shall identify the applicable configuration baseline or revision.

## Version Correlation

Operational records shall identify relevant:

- software version;
- interface version;
- configuration version;
- mission revision;
- capability version.

## Operator Correlation

Operator-initiated actions shall be associated with the applicable authenticated operator identity.

## Structured Logging

Operational logs shall use structured representations.

Structured logs shall support machine processing without losing human-readable diagnostic value.

## Logging Domains

Logging shall cover applicable:

- mission;
- validation;
- readiness;
- approval;
- execution;
- UAV;
- fleet;
- communication;
- conflict;
- optimization;
- simulation;
- Mission AI;
- safety;
- errors;
- recovery;
- deployment;
- configuration changes;
- external integrations.

## Safety Logging

Safety-relevant decisions, blocks, warnings, overrides and state transitions shall be logged.

Safety logging shall remain available independently of ordinary informational logging where required.

## Validation Logging

Mission Validation results and material validation transitions shall be logged.

## Readiness Logging

Mission Readiness decisions and state transitions shall be logged.

## Approval Logging

Mission approval and rejection events shall be logged with applicable authority information.

## Execution Logging

Execution lifecycle events shall be recorded, including:

- execution start;
- execution state;
- material changes;
- execution completion;
- abort;
- failure;
- recovery.

## UAV Logging

Relevant UAV state and command events shall be associated with the UAV identity.

## Fleet Logging

Fleet allocation, coordination and material resource changes shall be observable.

## Communication Logging

Communication state transitions shall be recorded, including:

- connected;
- degraded;
- lost;
- recovering;
- unavailable.

## Conflict Logging

Conflict detection, evaluation, resolution and operator selection shall be recorded.

## Optimization Logging

Optimization requests and results shall identify:

- selected profile;
- mission revision;
- objective;
- constraints;
- result;
- applicable alternatives.

## Simulation Logging

Simulation shall record:

- mission revision;
- simulation configuration;
- environment;
- profile;
- simulation start;
- simulation result;
- material events;
- comparison results.

Simulation records shall remain distinguishable from ACTUAL execution records.

## Planned / Simulated / Actual

Observability shall preserve distinction between:

- PLANNED;
- SIMULATED;
- ACTUAL.

This distinction shall never be silently removed during aggregation.

## Mission AI Logging

Mission AI recommendations, decisions, confidence where applicable, inputs and resulting actions shall be traceable.

Mission AI shall not be represented as execution authority when it only generated a recommendation.

## Automatic Actions

Automatically triggered system actions shall be observable.

Records shall identify:

- triggering condition;
- action;
- result;
- authority;
- affected mission or subsystem.

## Audit Boundary

Security and authority-relevant records shall remain within the audit boundary defined by the security architecture.

## Audit Immutability

Audit records shall be protected against unauthorized modification.

## Audit Integrity

The system shall provide mechanisms to detect material alteration or corruption of audit records.

## Audit Retention

Retention shall follow the data lifecycle and security requirements.

## Access Control

Observability data shall follow role-based access and least-privilege principles.

Sensitive diagnostic data shall not be exposed to unauthorized users.

## Security Logging

Security events shall be logged, including applicable:

- authentication failures;
- authorization failures;
- privilege changes;
- configuration security changes;
- suspicious access;
- credential events.

## Error Logging

Errors shall contain sufficient information to identify:

- component;
- operation;
- context;
- failure type;
- severity;
- correlation;
- recovery state where applicable.

## Exception Handling

Unhandled or material exceptions shall be observable.

Exception records shall not expose sensitive information unnecessarily.

## Recovery Logging

Recovery attempts and outcomes shall be logged.

Recovery logging shall support reconstruction of:

- failure;
- detection;
- response;
- recovery;
- resulting state.

## Configuration Change Logging

Material configuration changes shall identify:

- previous value or revision;
- new value or revision;
- actor;
- authorization;
- timestamp;
- reason;
- result.

## External Integration Logging

External system interactions shall be traceable.

Where applicable records shall include:

- external system;
- adapter;
- interface version;
- request;
- result;
- error;
- timeout;
- retry.

Sensitive payloads shall be protected according to security policy.

## Data Logging

Important data processing operations shall be observable without unnecessarily duplicating large payloads.

Large data objects may be referenced by stable identifiers.

## Metrics

The system shall expose metrics appropriate for:

- health;
- performance;
- capacity;
- latency;
- throughput;
- resource usage;
- reliability;
- mission execution.

## Resource Metrics

Resource metrics shall include applicable:

- CPU;
- GPU;
- memory;
- storage;
- network;
- queue depth;
- thread or task utilization.

## Performance Metrics

Performance metrics shall support measurement of:

- command processing;
- route calculation;
- validation;
- optimization;
- simulation;
- telemetry processing;
- data fusion;
- UI-facing Core response.

## Real-Time Metrics

Safety-critical and runtime services shall expose metrics sufficient to identify timing degradation.

## Availability Metrics

Service availability and dependency availability shall be measurable.

## Freshness Metrics

Telemetry and external data freshness shall be measurable.

## Error Metrics

Error rates and failure frequency shall be measurable.

## Capacity Metrics

The architecture shall provide metrics for resource and workload capacity.

## Alerting

Observability shall integrate with the health and alerting architecture.

Alerts shall be generated from defined conditions rather than arbitrary UI state.

## Alert Correlation

Related alerts shall support correlation to reduce diagnostic ambiguity without hiding individual events.

## Traceability

Observability records shall support:

EVENT
→ MISSION
→ CONFIGURATION
→ VERSION
→ ACTION
→ RESULT.

## Distributed Correlation

Where multiple services participate in one operation, a shared correlation identity shall allow the operation to be reconstructed across service boundaries.

## Request Tracing

Important requests shall support tracing through:

QML
→ C++ Core
→ subsystem
→ adapter
→ external service

where applicable.

The trace shall preserve the safety boundary.

## No Direct QML Logging Authority

QML may emit UI telemetry and diagnostic information but shall not become the authoritative source for mission execution records.

## No Direct AI Logging Authority

Mission AI shall provide structured diagnostic information through the Core.

The AI layer shall not independently modify authoritative audit records.

## Sampling

High-volume telemetry or diagnostic data may use controlled sampling.

Safety-critical evidence shall not be sampled in a way that prevents required reconstruction.

## Buffering

Observability components may buffer records during temporary storage or network interruption.

Buffers shall have defined capacity and overflow behaviour.

## Persistence

Required operational, audit and safety records shall be persisted according to their retention class.

## Storage Separation

Where required, operational logs, audit records, telemetry, simulation data and other stores shall remain logically separated.

## Storage Failure

Storage failure shall produce an explicit health condition.

The system shall not silently claim successful persistence when persistence failed.

## Backpressure

The observability architecture shall protect Core and safety-critical workloads from excessive logging or telemetry load.

## Resource Protection

Observability workloads shall have resource limits and priorities.

Logging shall not consume resources required for safety-critical execution.

## Privacy

Sensitive personal or operational information shall be minimized and protected according to applicable policy.

## Export

Authorized users shall be able to export appropriate diagnostic and audit records.

Exports shall preserve:

- source;
- timestamps;
- correlation;
- integrity information;
- record identity.

## Evidence Integrity

Records used as verification or incident evidence shall preserve sufficient integrity metadata to demonstrate provenance.

## Event Replay

Where supported, event streams shall permit controlled replay for investigation and testing.

Replay shall not create operational execution authority.

## Mission Reconstruction

The observability architecture shall support reconstruction of mission history from:

- planned state;
- simulation state;
- actual state;
- telemetry;
- events;
- decisions;
- configuration;
- operator actions.

## Incident Investigation

The system shall support investigation of:

- failures;
- conflicts;
- safety events;
- communication loss;
- degraded operation;
- unexpected behaviour;
- deployment problems.

## Observability Failure

Failure of observability shall itself be observable.

Loss of monitoring, logging or telemetry collection shall not be silently ignored.

## Degraded Observability

When observability is degraded, the system shall expose the affected monitoring capability and its impact.

## Recovery

After observability recovery, buffered records shall be reconciled where possible.

Missing records shall remain identifiable rather than being fabricated.

## Monitoring Integration

Observability shall integrate with ARCH-DEC-032 health monitoring.

Health transitions shall generate appropriate observable records.

## Deployment Integration

Deployment and release state shall integrate with ARCH-DEC-031 observability.

Startup, upgrade, rollback and deployment failures shall be observable.

## Configuration Integration

Configuration and change records shall integrate with ARCH-DEC-026.

## Security Integration

Security events shall integrate with ARCH-DEC-018.

## Data Architecture Integration

Observability data shall follow the authoritative data architecture and lifecycle controls.

## Verification

Observability shall be verified through:

- normal operation;
- high event volume;
- resource pressure;
- storage failure;
- network interruption;
- service failure;
- recovery;
- clock or time synchronization issues;
- duplicate events;
- out-of-order events;
- stale data;
- unauthorized access;
- export;
- replay;
- mission reconstruction.

## Negative Testing

Verification shall confirm that the system does not:

- silently lose safety events;
- report failed persistence as successful;
- merge SIMULATED with ACTUAL;
- hide monitoring failure;
- permit unauthorized audit modification;
- attribute AI recommendations as operator decisions;
- attribute UI actions as execution authority.

## Performance

Observability shall satisfy performance requirements without compromising Core execution or safety-critical processing.

## Scalability

The architecture shall scale with:

- mission count;
- UAV count;
- fleet size;
- telemetry volume;
- event volume;
- operator count;
- simulation workloads.

## Auditability

All material observability configuration changes shall be auditable.

## Change Control

Changes to logging schemas, metrics, event models or retention policies shall follow controlled change management.

Material schema changes shall preserve backward interpretation or provide migration.

## Reproducibility

Observability records shall support reproducible analysis where the required source data remains available.

## Operational Diagnostics

Operators and authorized engineers shall have diagnostic views appropriate to their role.

Diagnostic views shall distinguish:

- current state;
- historical state;
- predicted state;
- simulated state;
- actual state.

## Operator Visibility

The operational UI shall provide concise visibility into important:

- health;
- alerts;
- mission events;
- communication state;
- UAV/fleet state;
- performance;
- safety conditions.

Detailed diagnostics may be available through dedicated views.

## Traceability

Observability shall maintain:

SOURCE
→ EVENT
→ CORRELATION
→ MISSION
→ DECISION
→ ACTION
→ RESULT
→ EVIDENCE.

## Rationale

A unified observability architecture provides trustworthy operational visibility and diagnostic evidence while protecting Core performance, preserving safety boundaries and maintaining traceability across mission, configuration, execution, simulation and recovery.


