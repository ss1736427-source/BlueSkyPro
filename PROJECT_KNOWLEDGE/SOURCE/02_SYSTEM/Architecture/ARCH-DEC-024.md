---
id: ARCH-DEC-024
type: architecture_decision
title: Monitoring Observability and Health Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-024 — Monitoring / Observability / Health Architecture

## Decision

BlueSky shall provide a unified monitoring and observability architecture for operational state, service health, communication state, mission execution, safety state, performance, data integrity and external integrations.

Monitoring shall provide the information required for safe operation, diagnosis, validation, recovery and operator awareness.

## Monitoring Authority

The C++ Core shall be the authoritative source for operational mission state.

Monitoring services shall consume authoritative Core state and relevant subsystem health information.

QML shall present monitoring information but shall not become the authoritative monitoring source.

## Monitoring Domains

The architecture shall monitor applicable:

- Mission state;
- Mission tasks;
- UAV state;
- Fleet state;
- C2 state;
- communication;
- battery and energy;
- navigation;
- payload;
- safety;
- validation;
- readiness;
- conflicts;
- weather;
- airspace;
- terrain;
- geofence;
- external integrations;
- storage;
- telemetry;
- performance;
- Mission AI;
- simulation;
- system services.

## Health Model

The system shall support explicit health states.

Applicable states include:

- HEALTHY;
- DEGRADED;
- WARNING;
- FAILED;
- UNAVAILABLE;
- RECOVERING;
- UNKNOWN.

Health state shall be distinguishable from mission state.

## Service Health

Core services shall expose health information where applicable.

Health information may include:

- service availability;
- initialization state;
- processing state;
- dependency state;
- error state;
- resource state.

## Mission Health

Mission health shall represent the current operational condition of the mission.

It may consider:

- validation;
- readiness;
- safety;
- communication;
- fleet;
- energy;
- environment;
- mission progress;
- active conflicts;
- contingency state.

Mission health shall not replace Mission Validation or Safety Engine authority.

## UAV Health

The system shall monitor relevant UAV health information including:

- connection;
- battery;
- navigation;
- positioning;
- payload;
- health state;
- mission state;
- capability state.

Material UAV degradation shall be available to Mission Validation and Safety Engine.

## Fleet Health

Fleet monitoring shall provide an aggregate and per-UAV view of fleet condition.

The system shall identify:

- available UAVs;
- assigned UAVs;
- unavailable UAVs;
- degraded UAVs;
- communication state;
- resource state;
- active failures.

Fleet health changes shall be available to Fleet Coordination.

## Communication Monitoring

Communication monitoring shall distinguish:

- CONNECTED;
- DEGRADED;
- LOST;
- RECOVERING;
- UNAVAILABLE.

Applicable communication metrics may include:

- latency;
- bandwidth;
- packet loss;
- link quality;
- availability.

Communication degradation shall be integrated with runtime validation and safety mechanisms.

## C2 Monitoring

C2 monitoring shall expose the current communication relationship between Core and applicable external systems.

C2 state shall remain distinguishable from UAV state.

## Telemetry Monitoring

The system shall monitor telemetry ingestion.

Applicable indicators include:

- telemetry rate;
- timestamp freshness;
- ingestion status;
- processing latency;
- backlog;
- invalid data;
- missing data.

## Data Freshness

The system shall identify stale or delayed information where freshness is operationally relevant.

Stale data shall not silently appear as current authoritative state.

## Safety Monitoring

Safety monitoring shall expose the current Safety Engine state.

Applicable states may include:

- SAFE;
- WARNING;
- BLOCKED;
- EMERGENCY.

Safety monitoring shall remain separate from ordinary service health.

## Validation Monitoring

The system shall expose Mission Validation status.

Applicable states may include:

- NOT_VALIDATED;
- VALIDATING;
- VALID;
- WARNING;
- FAILED;
- REVALIDATION_REQUIRED.

## Readiness Monitoring

The system shall expose Mission Readiness status.

Applicable states may include:

- NOT_READY;
- CHECKING;
- READY;
- WARNING;
- BLOCKED.

Readiness shall reflect current mission and environment state.

## Conflict Monitoring

Active conflicts shall be visible to the monitoring system.

Conflict information shall include where applicable:

- conflict type;
- affected mission element;
- affected UAV;
- severity;
- current resolution state;
- selected response;
- revalidation state.

## Environment Monitoring

The system shall monitor material environmental conditions including:

- weather;
- terrain;
- airspace;
- geofence;
- spatial restrictions.

Material environmental changes shall trigger the applicable runtime validation path.

## Performance Monitoring

The system shall monitor relevant performance indicators including:

- CPU;
- GPU where applicable;
- memory;
- storage;
- queue depth;
- processing latency;
- network utilization;
- telemetry backlog.

Performance degradation shall be distinguishable from mission degradation.

## Storage Monitoring

Storage services shall expose:

- capacity;
- utilization;
- recording state;
- synchronization state;
- errors;
- backlog.

Critical storage conditions shall be surfaced to the appropriate operational mechanisms.

## External Integration Monitoring

External adapters shall expose relevant health information.

Applicable information includes:

- connection;
- availability;
- latency;
- data freshness;
- version;
- errors;
- recovery state.

## Simulation Monitoring

Simulation / Digital Twin shall expose:

- simulation state;
- scenario;
- progress;
- processing state;
- errors;
- resource consumption;
- result availability.

Simulation monitoring shall remain distinguishable from real execution monitoring.

## Mission AI Monitoring

Mission AI services shall expose applicable:

- model state;
- processing state;
- inference status;
- workload;
- errors;
- configuration identity.

AI health shall not imply AI authority.

## Monitoring Events

Material monitoring changes shall generate events where applicable.

Events may include:

- health state transition;
- service failure;
- communication degradation;
- UAV failure;
- battery warning;
- safety state change;
- validation failure;
- readiness change;
- conflict detection;
- environmental change;
- resource exhaustion.

## Alerting

The system shall support operator-visible alerts for significant conditions.

Alerts shall communicate sufficient information to understand:

- condition;
- severity;
- affected component;
- affected mission;
- current response;
- required operator action.

## Alert Priority

Alerts shall be prioritized according to operational significance.

Applicable levels include:

- INFORMATION;
- WARNING;
- CRITICAL;
- EMERGENCY.

Safety-critical alerts shall not be hidden by lower-priority information.

## Alert Lifecycle

Alerts shall support a lifecycle such as:

GENERATED
→ PRESENTED
→ ACKNOWLEDGED
→ RESOLVED

where applicable.

Acknowledgement shall not imply that the underlying condition is resolved.

## Operator Acknowledgement

Where acknowledgement is required, the system shall record:

- operator identity;
- timestamp;
- alert;
- acknowledgement state.

Acknowledgement shall not bypass Safety Engine or mandatory system response.

## Runtime Change Detection

The monitoring architecture shall support:

MONITOR
→ DETECT CHANGE
→ ASSESS
→ REVALIDATE
→ ADAPT / REPLAN / BLOCK / ABORT

where applicable.

## Monitoring and Mission State

Monitoring shall remain integrated with the Mission State Machine.

Material health or operational changes may result in:

EXECUTING
→ REVALIDATE

EXECUTING
→ REQUIRES_REPLAN

EXECUTING
→ BLOCKED

EXECUTING
→ ABORTED

## Monitoring and Error Handling

Monitoring shall integrate with ARCH-DEC-017.

Detected abnormal conditions shall enter the applicable:

DETECT
→ CLASSIFY
→ RESPOND
→ REVALIDATE
→ RECOVER / ADAPT / REPLAN / ABORT

flow.

## Observability

The system shall provide sufficient observability to determine:

- what happened;
- when it happened;
- where it happened;
- which component was affected;
- which mission was affected;
- which configuration was active;
- what response occurred;
- what the resulting state was.

## Metrics

The architecture shall support metrics for relevant services and operational functions.

Metrics may include:

- latency;
- throughput;
- error rate;
- availability;
- resource utilization;
- queue depth;
- telemetry rate;
- communication quality;
- validation duration;
- optimization duration;
- simulation duration.

## Logs

Logs shall support diagnosis without becoming the sole source of authoritative mission state.

Logs shall include appropriate context such as:

- timestamp;
- component;
- severity;
- event;
- mission revision where applicable;
- correlation identifier where applicable.

## Correlation

Related operations shall be correlatable across services.

A mission operation should be traceable through:

UI
→ Core
→ Service
→ Adapter
→ External System
→ Result

where applicable.

## Recording

Material health and monitoring events shall be recorded according to ARCH-DEC-021.

Historical monitoring information shall remain associated with the applicable mission and configuration context.

## Audit Integration

Security- and authority-relevant monitoring actions shall integrate with ARCH-DEC-018 audit mechanisms.

## Configuration Integration

Monitoring thresholds and operational parameters shall be versioned according to ARCH-DEC-019 where material.

## Performance Integration

Monitoring shall integrate with ARCH-DEC-022 resource and performance management.

Monitoring itself shall not consume uncontrolled resources.

## Security

Monitoring data shall be subject to appropriate access control.

Sensitive operational information shall not be exposed to unauthorized users.

## Simulation and Sandbox

Monitoring shall support simulation and Sandbox environments.

The system shall clearly distinguish:

- simulated health;
- sandbox health;
- operational health.

Sandbox monitoring shall not create a direct execution path to real UAVs.

## Failure of Monitoring

The system shall detect loss or degradation of monitoring services where operationally significant.

Failure of a non-critical visualization component shall not automatically imply failure of the authoritative Core state.

Failure of monitoring required for safe execution shall be evaluated by the Safety Architecture.

## Operator Visibility

The UI shall provide an operator-oriented representation of relevant system state.

The presentation should make visible:

- mission state;
- safety state;
- readiness;
- communication;
- fleet;
- UAV health;
- active alerts;
- conflicts;
- performance degradation.

## No False Health

A subsystem shall not be reported as HEALTHY solely because its process is running.

Health shall consider applicable dependencies and operational state.

## Traceability

Monitoring information shall remain traceable to:

- mission revision;
- configuration;
- component;
- source;
- event;
- health state;
- response;
- resulting state.

## Rationale

A unified monitoring and observability architecture gives BlueSky a consistent operational picture while preserving the distinction between authoritative mission state, service health, safety state, diagnostic information and operator presentation.


