---
id: ARCH-DEC-041
type: architecture_decision
title: Mission Execution Evidence and Outcome Reconciliation Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-041 — Mission Execution Evidence / Outcome Reconciliation Architecture

## Decision

BlueSky shall maintain a dedicated architecture for reconciling planned, simulated and actual mission outcomes.

The architecture shall establish authoritative execution evidence, distinguish intended actions from observed results, identify deviations and preserve complete traceability from mission planning through final outcome.

## Authority

The C++ Core shall own authoritative mission outcome state.

Telemetry, external systems, adapters, simulation and Mission AI may provide evidence or analysis but shall not independently establish authoritative operational completion.

## Outcome States

Mission outcomes shall distinguish:

- PLANNED;
- SIMULATED;
- EXECUTED;
- PARTIALLY_COMPLETED;
- FAILED;
- ABORTED;
- UNKNOWN;
- RECONCILING;
- VERIFIED.

## Evidence

Execution evidence may include:

- command acknowledgement;
- telemetry;
- UAV state;
- task completion;
- resource state;
- communication state;
- external acknowledgement;
- operator confirmation;
- safety events;
- mission events.

## Evidence Identity

Evidence shall identify applicable:

- mission;
- revision;
- execution;
- task;
- UAV;
- fleet;
- event;
- timestamp;
- source;
- sequence.

## Source Authority

Evidence sources shall have defined authority.

Telemetry shall not automatically override authoritative Core state.

External acknowledgements shall be interpreted through adapter contracts.

Operator input shall remain subject to authorization.

## Evidence Integrity

Evidence shall support integrity verification.

Corrupt, incomplete or unverifiable evidence shall not be silently accepted as authoritative.

## Timestamp

Evidence shall contain authoritative or source timestamps where available.

Time synchronization quality shall remain observable.

## Event Ordering

Evidence shall preserve sufficient ordering for reconstruction.

Missing or conflicting sequence information shall be detectable.

## Correlation

Evidence shall correlate with:

- mission;
- mission revision;
- schedule;
- configuration;
- task;
- UAV;
- fleet;
- resource;
- operator;
- runtime adaptation.

## Planned Outcome

The planned outcome shall represent the approved mission expectation.

## Simulated Outcome

The simulated outcome shall represent evaluated behaviour in Simulation or Digital Twin.

Simulated success shall never imply actual execution success.

## Actual Outcome

The actual outcome shall be based on operational execution evidence.

Actual outcome shall not be inferred solely from planned or simulated state.

## Outcome Reconciliation

The system shall reconcile:

PLANNED
→ SIMULATED
→ ACTUAL
→ VERIFIED.

Differences shall remain visible.

## Deviation Detection

The architecture shall detect deviations in:

- route;
- altitude;
- timing;
- task completion;
- UAV assignment;
- resource consumption;
- battery;
- communication;
- environment;
- mission objectives.

## Deviation Classification

Deviations may be classified as:

- expected;
- accepted;
- non-material;
- material;
- safety-relevant;
- unresolved.

## Material Deviation

Material deviation shall trigger applicable:

- analysis;
- validation;
- replanning;
- operator notification;
- audit.

## Task Outcome

Each task shall have an independently traceable outcome.

A mission shall not be considered fully complete solely because the mission process terminated.

## Partial Completion

The system shall support partial completion.

Completed tasks shall remain verified even if subsequent tasks fail, unless later evidence invalidates that conclusion.

## Failure

Failed tasks and missions shall preserve:

- failure reason;
- evidence;
- state;
- affected resources;
- recovery;
- final result.

## Abort

Aborted missions shall distinguish:

- abort requested;
- abort authorized;
- abort executed;
- abort confirmed.

## Unknown Outcome

When execution status cannot be established, the system shall represent the outcome as UNKNOWN rather than assume success or failure.

## Reconciliation State

Unknown or conflicting outcomes shall enter RECONCILING.

Reconciliation shall determine the most authoritative available state.

## External Reconciliation

After communication recovery or adapter reconnection, external execution state shall be reconciled before final outcome is established.

## UAV Reconciliation

Actual UAV state shall be compared with expected mission state.

## Fleet Reconciliation

Fleet state shall be reconciled after mission completion, failure or abort.

## Resource Reconciliation

Actual resource consumption shall be compared with planned and simulated expectations.

## Communication Reconciliation

Communication events shall be considered when interpreting missing or delayed execution evidence.

## C2 Reconciliation

C2 state shall be considered when execution evidence depends on command connectivity.

## Telemetry Reconciliation

Telemetry shall be correlated with task and command events.

Stale telemetry shall not be treated as current evidence.

## Command Reconciliation

The system shall distinguish:

REQUESTED
→ SENT
→ ACKNOWLEDGED
→ OBSERVED
→ COMPLETED.

An acknowledgement shall not automatically prove physical completion.

## Duplicate Evidence

Duplicate evidence shall not create duplicate state transitions.

## Conflicting Evidence

Conflicting evidence shall enter an explicit conflict state.

The system shall preserve all relevant evidence rather than silently discard conflicting sources.

## Evidence Priority

Evidence interpretation shall follow configured source authority and safety rules.

Safety-critical discrepancies shall receive highest priority.

## Operator Confirmation

Where defined by mission policy, operator confirmation may be required to establish final outcome.

Operator confirmation shall be recorded.

## Mission AI

Mission AI may:

- analyze outcomes;
- identify deviations;
- classify anomalies;
- compare expected and actual results;
- recommend follow-up actions.

Mission AI shall not independently establish authoritative operational outcome.

## AI Explainability

AI-generated outcome analysis shall identify:

- evidence used;
- detected deviation;
- confidence where applicable;
- recommendation.

## Simulation Comparison

The system shall compare actual execution against simulation where meaningful.

Simulation assumptions shall remain distinguishable from measured operational evidence.

## Digital Twin Comparison

Digital Twin state may be compared with actual mission state.

Differences shall be observable.

## Performance Outcome

The system shall compare applicable:

- execution time;
- latency;
- resource use;
- energy;
- quality;
- mission completion.

## Energy Outcome

Actual energy or battery consumption shall be compared with planned and simulated estimates.

## Route Outcome

Actual route shall be compared with planned and simulated routes where data is available.

## Spatial Outcome

Actual position and altitude shall be compared with mission constraints.

Spatial verification shall integrate with four-dimensional mission state where applicable.

## Temporal Outcome

Actual timing shall be compared with scheduled execution windows.

## Environmental Outcome

Actual environmental conditions shall be retained where relevant to outcome analysis.

## Safety Outcome

Safety events and safety interventions shall be included in final mission outcome analysis.

## Validation Outcome

Validation results shall remain linked to the mission revision and execution result.

## Readiness Outcome

Readiness state shall remain linked to execution initiation and relevant runtime transitions.

## Approval Outcome

Approval state shall remain linked to the executed mission revision.

## Configuration Outcome

Actual execution configuration shall remain identifiable.

Configuration differences shall be visible during outcome reconciliation.

## Runtime Adaptation

Runtime adaptations shall be included in outcome reconstruction.

The final outcome shall identify applicable deviations from the approved baseline.

## Replanning Outcome

Replanned mission segments shall remain linked to the original mission baseline.

## Recovery Outcome

Recovery actions shall be included in the mission outcome record.

## Resource Outcome

The system shall record applicable:

- allocated resources;
- consumed resources;
- released resources;
- resource failures;
- resource reallocations.

## Fleet Outcome

Fleet-level outcome shall identify:

- participating UAVs;
- UAV failures;
- UAV substitutions;
- completed tasks;
- unresolved tasks.

## Communication Outcome

Communication interruptions and recovery shall be included where they materially affected execution.

## Audit

Final outcome records shall be auditable.

Audit shall preserve:

- mission;
- revision;
- execution;
- evidence;
- decisions;
- deviations;
- operator actions;
- final status.

## Evidence Retention

Evidence shall be retained according to data lifecycle and audit requirements.

## Data Lineage

Outcome information shall preserve lineage:

SOURCE
→ EVIDENCE
→ CORRELATION
→ RECONCILIATION
→ RESULT.

## Persistence

Outcome state and evidence shall be persisted sufficiently for later reconstruction.

## Recovery

Outcome reconciliation shall survive service restart and infrastructure recovery.

## No False Completion

The system shall not report mission completion until applicable completion evidence is established.

## No False Success

The system shall not report successful execution solely because:

- commands were issued;
- commands were acknowledged;
- tasks entered RUNNING;
- simulation succeeded;
- Mission AI predicted success.

## Verification

Final outcome shall be marked VERIFIED only after applicable evidence and validation requirements are satisfied.

## Unverified Outcome

An outcome lacking sufficient evidence shall remain explicitly unverified.

## Operator Visibility

The UI shall display:

- planned outcome;
- simulated outcome;
- actual outcome;
- deviations;
- unresolved evidence;
- verification status;
- final mission status.

## Explainability

Outcome reconciliation shall explain material differences between expected and actual execution.

## Observability

Outcome events shall integrate with system observability and logging.

## Incident Investigation

Evidence shall support post-mission incident investigation.

## Mission Reconstruction

The system shall support reconstruction of:

MISSION
→ PLAN
→ APPROVAL
→ EXECUTION
→ EVENTS
→ ADAPTATION
→ RECOVERY
→ OUTCOME.

## Reproducibility

Where possible, outcome analysis shall be reproducible from retained:

- configuration;
- mission revision;
- evidence;
- simulation version;
- algorithm version;
- environment data.

## Security

Outcome evidence shall follow authentication, authorization, least privilege and integrity requirements.

## Privacy

Sensitive operational or operator information shall be protected according to applicable policy.

## Negative Testing

Verification shall confirm that:

- simulation cannot establish actual success;
- acknowledgement cannot alone establish completion;
- stale telemetry cannot establish current outcome;
- missing evidence cannot silently become success;
- conflicting evidence cannot be silently discarded;
- Mission AI cannot establish authoritative outcome;
- QML cannot establish authoritative outcome;
- partial completion cannot be reported as full completion;
- unknown outcome cannot be reported as successful;
- corrupted evidence cannot become authoritative.

## Performance

Outcome reconciliation shall not compromise active mission execution.

Heavy post-mission analysis may execute asynchronously where safe.

## Scalability

The architecture shall support:

- multiple missions;
- large fleets;
- high telemetry volume;
- large event histories;
- concurrent reconciliation.

## Traceability

Outcome traceability shall support:

MISSION
→ REVISION
→ CONFIGURATION
→ SCHEDULE
→ APPROVAL
→ EXECUTION
→ COMMAND
→ ACK
→ TELEMETRY
→ TASK
→ ADAPTATION
→ RECOVERY
→ OUTCOME
→ VERIFICATION.

## Rationale

A dedicated outcome reconciliation architecture prevents planned intent, simulated expectations and actual operational results from being conflated.

It provides a defensible authoritative record of what was intended, what was simulated, what actually occurred, what evidence supports the conclusion and whether the final result has been verified.


