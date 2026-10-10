---
id: ARCH-DEC-040
type: architecture_decision
title: Mission State Persistence Recovery and Runtime Continuity Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-040 — Mission State Persistence / Recovery / Runtime Continuity Architecture

## Decision

BlueSky shall maintain authoritative persistent mission state sufficient to recover mission execution, configuration, scheduling, resources, UAV and fleet state after service interruption, restart or infrastructure failure.

Recovery shall never imply automatic permission to resume execution.

## Authority

The C++ Core shall own authoritative runtime mission state.

Persistent state shall be derived from authoritative Core state.

QML shall display recovered state but shall not determine recovery authority.

Mission AI may assist recovery analysis but shall not authorize execution recovery.

Safety Engine authority shall remain above recovery and resume decisions.

## Persistence Domains

Persistence shall cover applicable:

- mission state;
- mission revision;
- configuration;
- schedule;
- task state;
- UAV state;
- fleet state;
- resource reservations;
- resource allocation;
- communication state;
- safety state;
- validation state;
- readiness state;
- approval state;
- execution state;
- runtime adaptations;
- recovery state;
- audit records.

## State Identity

Persisted state shall identify:

- mission ID;
- mission revision;
- execution ID;
- task ID;
- UAV ID;
- fleet ID;
- configuration version;
- schedule version;
- state version;
- timestamp.

## State Versioning

Authoritative state shall be versioned.

State transitions shall preserve previous state where required for reconstruction and audit.

## Atomicity

Material state transitions shall be persisted atomically or through an equivalent consistency mechanism.

The system shall not expose partially committed authoritative state as valid state.

## Ordering

Persisted events and state transitions shall preserve sufficient ordering for deterministic reconstruction.

## Event Sequence

Events shall have sequence information sufficient to detect:

- missing events;
- duplicates;
- out-of-order events;
- conflicting state updates.

## Event Replay

The architecture shall support replay of persisted mission events where required.

Replay shall reconstruct state without creating actual external execution.

## Checkpoints

The system may create persistent checkpoints containing authoritative mission state.

Checkpoints shall identify:

- state version;
- mission revision;
- execution context;
- timestamp;
- integrity information.

## Snapshot and Event Model

The architecture may combine:

- authoritative snapshots;
- ordered events;
- incremental state updates.

The selected mechanism shall preserve reconstruction and recovery requirements.

## Storage Separation

Operational mission state, telemetry, simulation data and audit data shall remain logically separated according to the Data Architecture.

## Mission Store

Mission state shall be persisted in the authoritative Mission Store.

## Telemetry Store

Telemetry persistence shall remain distinguishable from authoritative mission state.

Telemetry shall not silently replace authoritative mission state.

## Simulation Store

Simulation state shall remain isolated from operational mission state.

## Audit Store

Audit records shall remain independently protected according to audit requirements.

## Integrity

Persisted state shall support integrity verification.

Corrupted or unverifiable state shall not be treated as authoritative.

## Persistence Failure

Persistence failure shall produce an explicit system condition.

The system shall not report successful persistence when the authoritative state was not safely stored.

## Write Failure

Write failures shall be classified and handled.

The system may:

- retry;
- fail over;
- enter degraded operation;
- block a state transition;
- require operator intervention.

## Read Failure

Read failures shall not silently produce default operational state.

Recovery shall distinguish:

- unavailable state;
- incomplete state;
- corrupt state;
- valid state.

## Storage Failure

Storage failure shall integrate with system health and recovery.

## Backup

Authoritative state shall be covered by applicable backup policy.

Backups shall preserve sufficient consistency for recovery.

## Backup Consistency

Mission state backups shall maintain a consistent recovery point.

Cross-domain state required for mission recovery shall be coordinated.

## Backup Verification

Backups shall be verified according to recovery policy.

An unverified backup shall not be represented as recovery-ready evidence.

## Restore Testing

Restore procedures shall be tested.

Testing shall verify:

- integrity;
- completeness;
- state reconstruction;
- dependency recovery;
- recovery timing.

## Recovery Point Objective

The architecture shall support a defined RPO for applicable mission state.

Actual achieved RPO shall be observable.

## Recovery Time Objective

The architecture shall support a defined RTO for applicable recovery domains.

Actual recovery duration shall be observable.

## Recovery Classification

Recovery shall distinguish:

- service restart;
- component failure;
- Core restart;
- storage failure;
- communication failure;
- fleet subsystem failure;
- infrastructure failure;
- disaster recovery.

## Recovery State Machine

Recovery shall use explicit states such as:

RECOVERY_REQUIRED
→ RECOVERING
→ STATE_LOADED
→ STATE_RECONCILING
→ VALIDATING
→ READY_FOR_DECISION

and:

RECOVERY_FAILED

where applicable.

## Recovery Detection

The system shall detect interruption or loss of authoritative runtime continuity.

## Recovery Initialization

Recovery shall first establish environment, configuration and persistence availability.

## State Loading

Persisted state shall be loaded with integrity verification.

## State Reconstruction

The system shall reconstruct applicable mission state from valid snapshots and events.

## State Reconciliation

Recovered state shall be reconciled against current:

- UAV state;
- fleet state;
- resources;
- communication;
- telemetry;
- environment;
- configuration;
- schedule;
- safety state.

## External Reconciliation

External adapter state shall be reconciled before any operational command is issued.

The system shall not blindly replay previously issued commands.

## UAV Reconciliation

Each UAV shall be reconciled against actual observed state.

A previously ACTIVE UAV shall not automatically be considered ACTIVE after recovery.

## Fleet Reconciliation

Fleet state shall be reconstructed from authoritative and current information.

## Resource Reconciliation

Reservations and allocations shall be compared with current resource availability.

Unavailable resources shall not remain falsely allocated.

## Communication Reconciliation

Communication state shall be re-established and validated.

## C2 Reconciliation

C2 state shall be verified before operations depending on C2 resume.

## Mission Reconciliation

Mission state shall be compared against actual execution evidence.

## Task Reconciliation

Task states shall be reconciled with actual completion evidence.

The system shall not mark an incomplete task as completed solely because a previous state indicated RUNNING or STARTING.

## Execution Reconciliation

Execution state shall distinguish:

- planned;
- previously active;
- externally confirmed;
- unknown;
- recovered;
- safe to continue;
- requires decision.

## Approval Reconciliation

Approval shall remain bound to the applicable mission revision and execution context.

A recovery event shall not automatically create a new approval.

## Validation Reconciliation

Recovered mission state shall undergo applicable validation.

## Readiness Reconciliation

Readiness shall be reassessed after recovery.

## Safety Reconciliation

Safety state shall be reassessed before any operational continuation.

## Recovery Gate

No operational resume shall occur until the recovery gate confirms applicable:

- state integrity;
- configuration integrity;
- external state;
- safety;
- validation;
- readiness;
- authority.

## No Unsafe Resume

Recovery shall never automatically resume operational execution solely because the software restarted successfully.

## Resume Decision

After recovery, the system may determine:

- resume permitted;
- resume requires approval;
- revalidation required;
- replan required;
- abort required;
- operator decision required.

## Operator Decision

Where recovery produces uncertainty, the operator shall receive explicit state and decision information.

## Mission AI Recovery

Mission AI may analyze recovered state and propose:

- recovery options;
- replanning;
- task continuation;
- task cancellation;
- resource reassignment.

Mission AI shall not authorize resume.

## Recovery Alternatives

Where multiple recovery paths exist, the system may generate alternatives.

Alternatives shall be evaluated for:

- safety;
- mission continuity;
- resource availability;
- execution time;
- risk;
- quality.

## Revalidation

Material differences between persisted and current state shall trigger revalidation.

## Replanning

Where the original plan is no longer valid, recovery shall transition to replanning.

## Rollback

Where appropriate, the system may restore a previous valid mission state.

Rollback shall be validated against current conditions.

## No Unsafe Rollback

A historical state shall not be restored merely because it was previously valid.

Current safety and environmental constraints remain authoritative.

## Configuration Recovery

Configuration shall be restored together with its applicable version identity.

## Configuration Compatibility

Recovered configuration shall be checked against deployed software and interfaces.

## Version Compatibility

State created by incompatible software or schema versions shall not be silently interpreted as current state.

## Migration

State migration shall be explicit, versioned and traceable.

## Failed Migration

Failed migration shall block use of the affected state until recovery or rollback is established.

## Schema Integrity

Persistent schemas shall be versioned.

Schema changes shall follow configuration and change-control architecture.

## Mission Memory

Mission Memory shall distinguish persistent knowledge from authoritative operational state.

Historical experience shall not silently modify recovered mission state.

## Data Lineage

Recovered values shall retain lineage where applicable:

SOURCE
→ STORED
→ RECOVERED
→ RECONCILED
→ ACCEPTED.

## Traceability

Recovery shall preserve the relationship between:

MISSION
→ STATE
→ EVENT
→ CHECKPOINT
→ RECOVERY
→ RECONCILIATION
→ VALIDATION
→ DECISION.

## Recovery Evidence

The system shall preserve evidence supporting recovery decisions.

Evidence may include:

- checkpoint;
- event sequence;
- telemetry;
- external acknowledgement;
- validation result;
- safety result;
- operator decision.

## Recovery Audit

Recovery operations shall be auditable.

The audit shall identify:

- failure;
- recovery start;
- recovered state;
- discrepancies;
- decisions;
- operator actions;
- final result.

## Observability

Recovery shall integrate with monitoring and observability.

Metrics shall include:

- recovery duration;
- recovery failures;
- reconciliation failures;
- state discrepancies;
- restore duration;
- RPO;
- RTO.

## Alerting

Critical recovery conditions shall generate appropriate alerts.

## Degraded Operation

If full recovery is unavailable, the system may enter degraded operation only according to defined safety policy.

## Isolation

Failed or untrusted state shall be isolated from authoritative runtime state until validated.

## Disaster Recovery

Disaster recovery shall preserve:

- mission data;
- configuration;
- audit;
- required execution history;
- recovery evidence.

## Failover

Failover shall preserve authoritative state consistency.

A standby component shall not become authoritative without defined authority transition.

## Authority Transfer

Authority transfer shall be explicit and traceable.

Multiple components shall not simultaneously claim authoritative runtime state.

## Split Brain

The architecture shall detect and prevent conflicting authoritative runtime state.

## Duplicate Recovery

Repeated recovery attempts shall not create duplicate authoritative state transitions.

## Recovery Idempotency

Recovery operations shall be idempotent where practical.

## External Commands

Recovery shall not blindly repeat commands whose external execution status is unknown.

Unknown command state shall require reconciliation.

## Communication Recovery

After communication restoration, the system shall obtain current external state before continuing applicable execution.

## Service Recovery

Recovered services shall pass health checks before becoming available to dependent components.

## Dependency Recovery

Dependencies shall recover in defined order where ordering is required.

## Core Recovery

Core recovery shall establish authoritative state before exposing operational readiness.

## Safety Engine Recovery

Safety Engine recovery shall be independently validated before operational execution resumes.

## Validation Recovery

Validation services shall be available and consistent before required validation is performed.

## Readiness Recovery

Readiness shall remain false or unresolved until applicable recovery checks complete.

## Scheduler Recovery

Scheduler state shall reconcile with mission and resource state.

## Resource Manager Recovery

Resource allocations shall reconcile with actual resource availability.

## Mission Execution Recovery

Execution orchestration shall reconcile active tasks and external execution state before resuming.

## Runtime Adaptation Recovery

Active runtime adaptations shall be restored with their parent mission revision and validation state.

## Simulation Recovery

Simulation recovery shall remain isolated from operational recovery.

## Sandbox Recovery

Sandbox recovery shall remain isolated from operational state.

## Planned / Simulated / Actual

Recovered state shall preserve:

PLANNED;
SIMULATED;
ACTUAL.

These states shall never be conflated.

## Restart

A normal service restart shall preserve required authoritative state.

## Graceful Shutdown

Graceful shutdown shall persist required state before termination where possible.

## Unclean Shutdown

Unclean shutdown shall be detected where possible and trigger recovery verification.

## Crash Recovery

Crash recovery shall reconstruct the last consistent state.

## Power Loss

Power-loss recovery shall use the most recent verified consistent persistence point.

## Recovery Testing

Recovery shall be tested against:

- clean restart;
- process crash;
- Core crash;
- storage failure;
- corrupted state;
- incomplete write;
- lost event;
- duplicate event;
- out-of-order event;
- communication loss;
- UAV loss;
- fleet subsystem loss;
- configuration mismatch;
- schema mismatch;
- external command uncertainty.

## Negative Testing

Verification shall confirm that:

- corrupt state cannot become authoritative;
- incomplete state cannot be reported as valid;
- stale state cannot authorize execution;
- recovery cannot bypass Safety Engine;
- recovery cannot bypass Validation;
- recovery cannot bypass Readiness;
- recovery cannot create approval;
- recovery cannot silently change mission intent;
- recovered resources are not assumed available without reconciliation;
- unknown external commands are not blindly replayed;
- simulation state cannot become operational state;
- QML cannot authorize recovery;
- Mission AI cannot authorize resume;
- split-brain authority cannot occur.

## Performance

Recovery shall meet defined RTO requirements for applicable operational domains.

## Scalability

Recovery architecture shall scale with:

- mission count;
- task count;
- UAV count;
- fleet size;
- event volume;
- telemetry volume;
- persistence size.

## Security

Persisted operational state and recovery data shall follow:

- authentication;
- authorization;
- least privilege;
- encryption;
- access control;
- audit.

## Privacy

Sensitive data shall be protected according to applicable data policy.

## Operator Visibility

The UI shall expose:

- recovery state;
- recovered mission;
- discrepancies;
- validation;
- readiness;
- approval requirement;
- resume status;
- blocked conditions.

## No False Recovery

The system shall not report recovery as complete until authoritative state has passed required recovery checks.

## Verification

Recovery readiness shall be verified before operational acceptance.

## Rationale

Persistent authoritative state and controlled recovery prevent service interruption from producing ambiguous or unsafe mission execution.

The architecture deliberately separates state recovery from execution authorization: recovery reconstructs reality, reconciliation establishes current truth, validation and safety establish whether continuation is permissible, and only then may the execution authority decide how to proceed.

## Traceability

Recovery shall support:

FAILURE
→ PERSISTED STATE
→ RECOVERY
→ RECONCILIATION
→ VALIDATION
→ READINESS
→ APPROVAL
→ DECISION
→ EXECUTION / REPLAN / ABORT
→ RESULT.


