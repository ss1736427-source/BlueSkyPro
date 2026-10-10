---
id: ARCH-DEC-029
type: architecture_decision
title: Operational Logging Event Correlation and Audit Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-029 — Operational Logging / Event Correlation / Audit Architecture

## Decision

BlueSky shall provide centralized, structured and traceable operational logging, event correlation and audit recording.

Operational events shall be correlated across Mission, Core, UAV, Fleet, C2, Safety, Validation, Simulation and external integrations.

Logging shall support operational monitoring, troubleshooting, safety analysis, verification, incident investigation and audit.

## Logging Authority

The C++ Core shall provide the authoritative operational event stream for Core-controlled system behaviour.

Subsystems shall publish structured events through controlled interfaces.

QML shall not be the authoritative source for safety-critical or execution-critical events.

## Logging Domains

The architecture shall support logging for:

- system lifecycle;
- application lifecycle;
- mission lifecycle;
- mission configuration;
- mission validation;
- mission readiness;
- approval;
- execution;
- UAV;
- fleet;
- C2;
- communication;
- route planning;
- conflict resolution;
- optimization;
- simulation;
- Mission AI;
- Safety Engine;
- external integrations;
- security;
- configuration;
- recovery;
- deployment;
- operator actions.

## Event Model

Operational events shall have a structured representation.

An event shall contain applicable:

- event identifier;
- event type;
- timestamp;
- source;
- severity;
- mission identifier;
- mission revision;
- configuration version;
- actor;
- correlation identifier;
- causation identifier;
- state;
- result;
- relevant data;
- error information.

## Event Identity

Events shall have unique identifiers sufficient for reliable correlation and investigation.

Duplicate events shall be detectable where required.

## Timestamp

Events shall use a consistent time representation.

The architecture shall preserve sufficient timing information for:

- event ordering;
- mission reconstruction;
- telemetry correlation;
- incident investigation;
- planned versus actual comparison.

## Time Synchronization

Relevant system components shall maintain synchronized or otherwise reconcilable time.

Time uncertainty shall be detectable where it affects operational interpretation.

## Severity

Events shall support severity classification.

Applicable levels include:

- DEBUG;
- INFO;
- NOTICE;
- WARNING;
- ERROR;
- CRITICAL;
- EMERGENCY.

Safety-critical events shall use an appropriate severity.

## Event Lifecycle

Events may progress through applicable states such as:

DETECTED
→ RECORDED
→ CORRELATED
→ ANALYZED
→ ACKNOWLEDGED
→ RESOLVED
→ CLOSED.

Not every event requires every state.

## Correlation

Events shall support correlation across subsystem boundaries.

The architecture shall support:

MISSION
→ TASK
→ CORE OPERATION
→ ADAPTER
→ EXTERNAL EVENT
→ RESULT.

## Causation

Where applicable, events shall identify the event or operation that caused them.

This shall support determination of:

- trigger;
- response;
- consequence.

## Mission Correlation

Mission-related events shall remain associated with the applicable:

- mission;
- mission revision;
- task;
- route;
- UAV;
- fleet;
- configuration baseline.

## Configuration Correlation

Operational events shall identify material configuration state where applicable.

This shall allow historical events to be interpreted against the correct configuration.

## Version Correlation

Events shall preserve relevant:

- software version;
- Core version;
- adapter version;
- interface version;
- data version.

## Operator Correlation

Operator actions shall be associated with the applicable authenticated identity.

Where anonymous system activity is permitted, the event shall identify the system actor.

## Automatic Action Correlation

Automatic actions shall identify the responsible subsystem or authority.

Mission AI, Optimization, Safety Engine and automatic conflict resolution shall remain distinguishable.

## Audit Boundary

Audit records shall capture material actions affecting:

- security;
- authorization;
- mission approval;
- execution authority;
- safety configuration;
- mission configuration;
- system configuration;
- release;
- recovery;
- protected data.

## Audit Immutability

Audit records shall be protected against unauthorized modification or deletion.

Historical audit evidence shall not be silently rewritten.

## Audit Integrity

The architecture shall provide mechanisms to detect unauthorized alteration of audit records.

## Audit Retention

Audit retention shall follow applicable security, operational and regulatory requirements.

Retention shall remain consistent with ARCH-DEC-021.

## Access Control

Operational logs and audit records shall use controlled access.

Users shall receive only the information required for their role.

Sensitive security information shall not be unnecessarily exposed to ordinary operators.

## Security Logging

Security-relevant events shall include where applicable:

- authentication;
- authorization;
- failed authentication;
- privilege changes;
- session changes;
- configuration security changes;
- access to protected resources;
- emergency access.

## Mission Logging

Mission lifecycle events shall record applicable transitions:

DRAFT
→ CONFIGURING
→ PLANNED
→ VALIDATING
→ OPTIMIZING
→ SIMULATING
→ READY
→ APPROVAL
→ APPROVED
→ EXECUTING
→ COMPLETED.

Exceptional states shall also be recorded.

## Validation Logging

Validation shall record:

- validation start;
- validation completion;
- checks;
- warnings;
- blocking findings;
- result;
- relevant configuration;
- mission revision.

## Readiness Logging

Readiness shall record:

- readiness evaluation;
- individual checks;
- spatial checks;
- mission checks;
- dynamic checks;
- environmental checks;
- C2 checks;
- fleet checks;
- energy checks;
- contingency checks;
- final result.

## Approval Logging

Approval events shall record:

- approving authority;
- mission revision;
- configuration baseline;
- timestamp;
- result;
- applicable conditions.

## Execution Logging

Execution events shall record material:

- start;
- pause;
- resume;
- adaptation;
- replanning;
- conflict;
- emergency;
- abort;
- completion.

## UAV Logging

UAV events shall include applicable:

- state;
- health;
- position;
- altitude;
- velocity;
- battery;
- navigation;
- payload;
- communication;
- mission state.

High-rate telemetry shall be handled separately from event logs where appropriate.

## Fleet Logging

Fleet events shall record:

- UAV join;
- UAV loss;
- resource allocation;
- role changes;
- capability changes;
- dynamic reallocation;
- coordination events.

## Communication Logging

Communication events shall record applicable:

- connected;
- degraded;
- lost;
- recovering;
- unavailable;
- latency changes;
- bandwidth changes;
- packet loss;
- relay state.

## Conflict Logging

Conflict Resolution shall record:

- conflict detection;
- conflict classification;
- affected resources;
- generated options;
- evaluation;
- selected option;
- operator selection where applicable;
- revalidation;
- replanning;
- final result.

## Optimization Logging

Optimization shall record:

- optimization request;
- optimization goal;
- candidate variants;
- evaluation;
- selected variant;
- automatic or operator selection;
- reason;
- safety constraints;
- reoptimization.

## Simulation Logging

Simulation shall record:

- simulation start;
- scenario;
- mission revision;
- configuration;
- environment;
- simulation result;
- comparison result;
- resimulation;
- SIMULATE → EVALUATE → OPTIMIZE transitions.

## Planned / Simulated / Actual

Events shall preserve the applicable data state:

- PLANNED;
- SIMULATED;
- ACTUAL.

The architecture shall support correlation between these states.

## Mission AI Logging

Mission AI shall record material:

- input;
- inference;
- recommendation;
- confidence where applicable;
- decision support output;
- adaptation proposal.

AI logs shall distinguish recommendations from authorized execution.

## Safety Logging

Safety Engine events shall record material:

- hazard;
- rule activation;
- blocking condition;
- emergency condition;
- safety response;
- revalidation;
- authorization decision.

Safety records shall remain distinguishable from ordinary application logs.

## Error Logging

Errors shall include sufficient information for investigation without exposing unnecessary secrets.

Where applicable an error shall include:

- error code;
- subsystem;
- operation;
- cause;
- severity;
- recovery action;
- final result.

## Exception Handling

Unhandled exceptions or equivalent critical failures shall be recorded.

The system shall preserve enough evidence to investigate the failure.

## Recovery Logging

Recovery events shall record:

FAILURE
→ DETECTION
→ ISOLATION
→ ASSESSMENT
→ RESTORE
→ RECONCILIATION
→ VALIDATION
→ READINESS
→ RESUME / ABORT.

## Deployment Logging

Deployment events shall identify:

- release;
- baseline;
- environment;
- operator or automation actor;
- start;
- result;
- rollback where applicable.

## Configuration Change Logging

Material configuration changes shall record:

- previous version;
- new version;
- changed domain;
- reason;
- actor;
- authority;
- approval;
- verification;
- validation.

## External Integration Logging

External interactions shall preserve applicable:

- source;
- destination;
- adapter;
- interface;
- operation;
- request identity;
- response;
- error;
- correlation identifier.

Secrets shall not be recorded in plaintext.

## Data Logging

Data events shall preserve applicable lineage:

SOURCE
→ ADAPTER
→ NORMALIZATION
→ VALIDATION
→ CORE
→ DECISION.

## Structured Logging

Logs shall use structured records rather than relying exclusively on free-form text.

Structured fields shall support machine processing and automated analysis.

## Log Levels

Runtime logging levels shall be configurable within controlled boundaries.

Safety-critical logging shall not be disabled through ordinary operator configuration.

## No Loss of Critical Events

Critical safety, security, authorization and execution events shall use appropriate persistence mechanisms to minimize loss.

## Buffering

Temporary buffering may be used for high-rate or intermittent data.

Buffer overflow behaviour shall be defined.

Critical events shall receive appropriate priority.

## Persistence

Operational logs shall be persisted according to their criticality.

Temporary diagnostic logs may have shorter retention.

Audit and safety records shall use protected persistence.

## Storage Separation

Where appropriate, operational telemetry, simulation data, audit records and application diagnostics shall use separate logical storage domains.

## Data Integrity

Log integrity shall be monitored.

The system shall detect where applicable:

- corruption;
- missing sequence;
- invalid timestamps;
- duplicate identifiers;
- inconsistent references.

## Sequence Integrity

Ordered event streams shall support sequence or equivalent integrity checks where required.

Gaps shall be detectable.

## Event Replay

The architecture shall support replay of relevant event sequences for:

- investigation;
- debugging;
- simulation;
- verification;
- training.

Replay shall not create real execution authority.

## Mission Reconstruction

The event architecture shall support reconstruction of mission history sufficient to determine:

- what was planned;
- what was simulated;
- what was approved;
- what was executed;
- what changed;
- why it changed;
- what the system did.

## Incident Investigation

Incident investigation shall be able to correlate:

- operator actions;
- system events;
- UAV telemetry;
- communication;
- environment;
- Safety Engine;
- Mission AI;
- configuration;
- external systems.

## Observability Integration

Logging shall integrate with ARCH-DEC-024.

Health monitoring shall consume relevant operational events.

## Alerting Integration

Critical events shall be available to the alerting subsystem.

Alert lifecycle shall remain distinguishable from raw event lifecycle.

## Performance

Logging shall not compromise safety-critical runtime processing.

High-volume telemetry and diagnostic logging shall be isolated or throttled as required.

## Resource Protection

Logging shall have controlled limits for:

- CPU;
- memory;
- storage;
- network;
- queue depth.

Resource exhaustion caused by logging shall be detectable and handled.

## Security

Logging shall follow ARCH-DEC-018.

Logs shall not expose:

- credentials;
- private keys;
- authentication secrets;
- protected tokens;
- unnecessary sensitive data.

## Privacy

Where personal or operator-related information is logged, collection shall be limited to operationally justified information.

## Access Audit

Access to protected logs and audit records shall itself be auditable.

## Export

Authorized users shall be able to export relevant evidence for:

- verification;
- incident investigation;
- operational review;
- certification;
- audit.

Exported evidence shall retain provenance.

## Evidence Integrity

Exported evidence shall preserve:

- source;
- timestamp;
- event identity;
- correlation;
- version;
- integrity information.

## Verification

Logging and audit architecture shall be verified according to ARCH-DEC-025.

Verification shall include:

- event generation;
- event persistence;
- correlation;
- ordering;
- failure handling;
- audit integrity;
- access control;
- replay;
- reconstruction.

## Simulation

Logging behaviour shall be testable in Simulation and Sandbox.

Simulation events shall remain distinguishable from operational events.

## Recovery

Logging and audit recovery shall follow ARCH-DEC-027.

Recovery shall preserve historical event identity and ordering where possible.

## External Integration

Integration events shall follow ARCH-DEC-028.

External event identifiers shall remain distinguishable from BlueSky event identifiers.

## Change Control

Logging schema changes shall follow ARCH-DEC-026.

Material changes shall preserve backward interpretation of historical records where required.

## No Silent Event Modification

Historical events shall not be silently modified to reflect later system interpretation.

Corrections shall be represented as explicit correction or superseding events.

## Traceability

Operational traceability shall support:

REQUIREMENT
→ MISSION
→ CONFIGURATION
→ EVENT
→ DECISION
→ ACTION
→ RESULT
→ EVIDENCE.

## Rationale

Structured operational logging, event correlation and immutable audit evidence provide the evidence required to understand BlueSky behaviour across planning, simulation, execution, safety, external integration and recovery while preserving authority boundaries and historical integrity.


