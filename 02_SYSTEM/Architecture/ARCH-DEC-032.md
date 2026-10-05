---
id: ARCH-DEC-032
type: architecture_decision
title: System Health Diagnostics and Self-Monitoring Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-032 — System Health / Diagnostics / Self-Monitoring Architecture

## Decision

BlueSky shall provide a centralized health, diagnostics and self-monitoring architecture covering Core services, mission state, UAVs, fleet, C2, communication, data, external dependencies, Simulation and Mission AI.

Health information shall support operator visibility, safety decisions, validation, readiness, recovery and audit.

Health state shall not be inferred solely from process availability.

## Health Authority

The C++ Core shall own the authoritative operational health model.

QML shall display health information received from the Core.

QML shall not independently determine that the system is safe or operationally ready.

Mission AI may consume health information for planning and recommendation but shall not override authoritative health state.

## Health Domains

The architecture shall monitor:

- system services;
- Core;
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
- data stores;
- external systems;
- Simulation;
- Sandbox;
- performance;
- resources;
- security;
- configuration.

## Health Model

Each monitored component shall expose an identifiable health state.

The health model shall support states including:

- HEALTHY;
- DEGRADED;
- WARNING;
- CRITICAL;
- UNAVAILABLE;
- FAILED;
- UNKNOWN;
- RECOVERING.

The exact state vocabulary may be extended while preserving unambiguous semantics.

## Health Dimensions

Health evaluation may include:

- availability;
- responsiveness;
- freshness;
- correctness;
- consistency;
- capacity;
- latency;
- error rate;
- resource utilization;
- dependency state;
- security state;
- configuration state.

## Process Health

A running process shall not automatically be considered healthy.

Process health shall be evaluated together with service responsiveness and required functional checks.

## Service Health

Services shall expose health information appropriate to their function.

Health checks shall distinguish:

- process alive;
- service responsive;
- service functional;
- dependency-ready;
- operationally usable.

## Core Health

The Core shall monitor its critical subsystems including:

- Mission Engine;
- Route Planner;
- Safety Engine;
- Validation;
- Readiness;
- Conflict Resolution;
- Telemetry;
- Communications;
- Data Processing;
- Simulation interfaces.

## Mission Health

Mission health shall include the current state of:

- mission lifecycle;
- validation;
- readiness;
- approval;
- execution;
- conflicts;
- active constraints;
- required capabilities.

Mission health shall remain distinct from general system health.

## UAV Health

Each UAV shall have an identifiable health state.

Relevant information may include:

- connection;
- battery;
- position validity;
- navigation;
- payload;
- flight state;
- health;
- communication;
- mission state.

## Fleet Health

Fleet health shall aggregate individual UAV states while preserving individual UAV visibility.

A fleet shall not be reported fully healthy when a safety-critical condition exists in a required member.

## Communication Health

Communication health shall monitor:

- connected;
- degraded;
- lost;
- recovering;
- unavailable.

Where applicable it shall also include:

- latency;
- bandwidth;
- availability;
- packet quality;
- freshness.

## C2 Health

C2 health shall distinguish:

- C2 service availability;
- communication link state;
- UAV connectivity;
- command path availability;
- telemetry path availability.

## Telemetry Health

Telemetry monitoring shall include:

- freshness;
- sequence integrity;
- timestamps;
- source identity;
- expected update rate;
- missing data;
- invalid data.

Stale telemetry shall not be treated as current telemetry.

## Data Health

Data health shall monitor:

- availability;
- integrity;
- consistency;
- schema compatibility;
- synchronization;
- storage availability.

## External Dependency Health

External dependencies shall have explicit health states.

Applicable dependencies include:

- map;
- terrain;
- airspace;
- geofence;
- weather;
- C2;
- communication;
- UAV adapters;
- simulation services.

## Dependency Propagation

A critical dependency failure shall propagate to dependent services according to defined dependency rules.

The system shall not report a dependent service as fully healthy when a mandatory dependency is unavailable.

## Safety Health

Safety-critical health information shall be available to the Safety Engine.

A health failure affecting a safety-critical capability may result in:

- warning;
- blocking;
- degraded operation;
- revalidation;
- replanning;
- emergency handling.

## Validation Health

Mission Validation shall be able to consume relevant health information.

A material health change affecting mission validity shall trigger applicable revalidation.

## Readiness Health

Mission Readiness shall incorporate applicable health state.

System health shall not automatically grant Mission Readiness.

## Runtime Monitoring

Health shall be monitored continuously during operational execution.

Monitoring shall detect material runtime changes.

## Health Transitions

Health transitions shall be represented explicitly.

Examples include:

HEALTHY
→ WARNING
→ DEGRADED
→ CRITICAL
→ FAILED

and:

FAILED
→ RECOVERING
→ HEALTHY.

Transitions shall be observable and auditable.

## Alerting

Material health transitions shall generate appropriate alerts.

Alerts shall identify:

- component;
- previous state;
- new state;
- severity;
- timestamp;
- mission context;
- recommended action where applicable.

## Alert Priority

Alerts shall have priority appropriate to severity and safety impact.

Critical safety events shall take precedence over informational events.

## Alert Lifecycle

Alerts shall support:

- CREATED;
- ACKNOWLEDGED;
- RESOLVED;
- CLOSED;

where applicable.

Unresolved critical alerts shall remain visible.

## Operator Visibility

The operator shall be able to determine:

- overall system health;
- mission health;
- UAV health;
- fleet health;
- communication health;
- dependency health;
- critical alerts;
- degraded capabilities.

## No False Health

The system shall not report HEALTHY when required health evidence is unavailable or stale.

UNKNOWN shall not silently become HEALTHY.

## Health Evidence

Health state shall be based on identifiable evidence.

Evidence may include:

- heartbeat;
- functional health check;
- telemetry;
- service response;
- dependency status;
- resource measurements;
- error records.

## Health Freshness

Health information shall have a freshness indicator or timestamp.

Expired health information shall be marked stale or unknown according to defined policy.

## Diagnostics

The architecture shall provide diagnostics sufficient to investigate material health failures.

Diagnostics may include:

- service state;
- dependency state;
- recent errors;
- performance metrics;
- configuration;
- version;
- recent transitions;
- related events.

## Error Correlation

Diagnostics shall correlate health failures with relevant:

- mission;
- configuration;
- software version;
- deployment;
- operator;
- UAV;
- communication;
- external dependency.

## Event Correlation

Health events shall integrate with the event and logging architecture.

A material health transition shall produce a traceable event.

## Metrics

The monitoring system shall collect appropriate metrics for:

- CPU;
- GPU;
- memory;
- storage;
- network;
- latency;
- throughput;
- queue depth;
- service availability;
- processing time.

## Resource Health

Resource exhaustion shall produce an explicit health condition.

The system shall distinguish:

- normal;
- elevated;
- constrained;
- exhausted.

## Performance Health

Performance degradation shall be represented separately from complete service failure where possible.

## Storage Health

Storage monitoring shall include:

- capacity;
- availability;
- write failures;
- read failures;
- integrity;
- retention conditions.

## Security Health

Security monitoring shall include relevant:

- authentication failures;
- authorization failures;
- certificate state;
- credential state;
- suspicious access;
- configuration violations.

Security health shall remain separate from functional health while contributing to overall operational status.

## Configuration Health

The system shall detect material configuration mismatch or drift.

Configuration health shall identify:

- active version;
- expected version;
- mismatch;
- source;
- affected capability.

## Simulation Health

Simulation services shall have independent health state.

Simulation health shall not imply operational UAV health.

## Sandbox Health

Sandbox services shall be monitored independently.

Sandbox health shall not grant operational execution capability.

## Monitoring Failure

Failure of the monitoring subsystem shall itself produce a health condition.

Loss of observability shall not be interpreted as proof of system health.

## Monitoring Independence

Safety-critical monitoring shall remain sufficiently independent from the services it monitors to avoid common-mode false health.

## Degraded Observability

If complete monitoring is unavailable, the system shall expose the reduced observability state.

The system shall not conceal monitoring gaps.

## Recovery

Health recovery shall be monitored explicitly.

A component returning to service shall pass applicable health checks before being reported HEALTHY.

## Recovery Validation

Recovery of a safety-critical component may require:

- validation;
- readiness reassessment;
- revalidation;
- operator acknowledgement;

according to applicable rules.

## Runtime Adaptation

Material health changes may trigger:

- replanning;
- resource reallocation;
- communication fallback;
- mission adaptation;
- conflict resolution.

Such adaptation shall remain subject to Safety Engine authority.

## Mission AI

Mission AI may use health information to:

- identify degraded capabilities;
- propose replanning;
- recommend resource changes;
- evaluate mission alternatives.

Mission AI shall not suppress or falsify health state.

## Fleet Coordination

Fleet coordination shall consume current fleet health.

Loss or degradation of a required UAV shall be available to resource allocation and conflict resolution.

## Battery Health

Battery health shall be available to:

- Mission Validation;
- Mission Readiness;
- Route Planning;
- Optimization;
- Fleet Coordination;
- Safety Engine.

## Weather Health

Weather data shall have:

- source;
- timestamp;
- freshness;
- validity;
- availability.

Stale or unavailable weather shall be identified explicitly.

## Data Fusion

Health information from multiple sources shall be normalized and correlated without losing source identity.

Conflicting health information shall remain detectable.

## Health Authority Conflicts

Where health sources disagree, the system shall apply defined source authority and confidence rules.

The system shall not silently select an unsafe interpretation.

## State Persistence

Material health events shall be persisted according to the data lifecycle architecture.

Persistent records shall support later investigation and mission reconstruction.

## Audit

Material health state transitions and operator acknowledgements shall be auditable.

## Notifications

The system shall provide appropriate operator notifications for:

- critical health;
- communication loss;
- UAV failure;
- fleet degradation;
- safety service failure;
- monitoring failure;
- recovery;
- readiness impact.

## Performance

Monitoring shall not consume resources in a manner that compromises safety-critical execution.

Monitoring workloads shall have defined resource protections.

## Load Handling

The monitoring architecture shall support the expected number of:

- services;
- UAVs;
- telemetry streams;
- events;
- alerts;
- metrics.

## Failure Handling

Monitoring shall continue operating under partial system failure where possible.

Critical monitoring functions shall have defined fallback behaviour.

## Security Boundary

Health information shall follow access-control rules.

Sensitive diagnostic information shall not be exposed to unauthorized users.

## Deployment Integration

Health architecture shall integrate with ARCH-DEC-031 deployment readiness.

Startup shall verify required monitoring services.

## Logging Integration

Health events shall integrate with ARCH-DEC-029 logging.

## Data Architecture Integration

Health data shall follow the authoritative data and storage architecture.

## Verification

Health monitoring shall be verified through:

- normal operation;
- degraded operation;
- service failure;
- dependency failure;
- telemetry loss;
- communication loss;
- resource exhaustion;
- monitoring failure;
- recovery;
- stale data;
- conflicting data.

## Negative Testing

Verification shall include conditions where health evidence is intentionally absent, stale, invalid or contradictory.

The system shall not incorrectly report HEALTHY.

## Simulation Verification

Health scenarios shall be reproducible in Simulation.

Simulation health shall remain distinguishable from actual operational health.

## Traceability

Health traceability shall support:

HEALTH STATE
→ EVIDENCE
→ EVENT
→ MISSION
→ CONFIGURATION
→ ACTION
→ RESULT.

## Rationale

A centralized but evidence-driven health architecture provides reliable operator visibility, supports safety and readiness decisions, enables runtime adaptation and recovery, and prevents false assumptions that process availability or missing telemetry represent a healthy operational system.


