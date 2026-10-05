---
id: ARCH-DEC-038
type: architecture_decision
title: Mission Execution Orchestration and Runtime State Coordination Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-038 — Mission Execution Orchestration / Runtime State Coordination Architecture

## Decision

BlueSky shall use a centralized Mission Execution Orchestrator within the C++ Core to coordinate execution of an approved mission.

The orchestrator shall coordinate mission tasks, UAVs, fleet resources, communication, timing, safety state, runtime adaptation and execution results.

The orchestrator shall not replace the authority of the Safety Engine.

## Execution Authority

The C++ Core shall own authoritative execution state.

The execution path shall remain:

QML
→ Core
→ Safety / Validation
→ Execution Orchestrator
→ Adapter
→ External System.

QML shall not directly execute UAV actions.

Mission AI shall not directly execute UAV actions.

## Execution Preconditions

Execution shall require applicable:

- valid mission revision;
- applicable configuration;
- successful validation;
- sufficient readiness;
- required approval;
- available resources;
- compatible capabilities;
- acceptable communication;
- acceptable safety state.

Failure of a mandatory precondition shall block execution.

## Execution State

Execution shall distinguish:

REQUESTED
→ AUTHORIZING
→ VALIDATING
→ READY
→ APPROVED
→ STARTING
→ EXECUTING
→ COMPLETED

and applicable failure states:

BLOCKED
VALIDATION_FAILED
CONFLICT
REQUIRES_REPLAN
ABORTED
FAILED
RECOVERING.

## State Authority

Runtime execution state shall be authoritative in the C++ Core.

UI state shall be derived from Core state.

Telemetry and external subsystem information shall update Core state through defined adapters.

## Mission Start

Mission start shall be an explicit state transition.

The system shall verify applicable execution conditions immediately before activation.

## Task Orchestration

The orchestrator shall coordinate mission tasks according to the Mission Graph.

Tasks shall respect:

- dependencies;
- ordering;
- timing;
- resource requirements;
- UAV capabilities;
- safety constraints.

## Task States

Tasks may transition through:

PENDING
→ READY
→ STARTING
→ RUNNING
→ COMPLETED

or:

PENDING
→ BLOCKED

and:

RUNNING
→ FAILED
→ RECOVERING.

## Task Dependency

A dependent task shall not execute before mandatory prerequisites are satisfied.

Failed prerequisites shall prevent dependent execution unless an explicitly defined recovery path permits continuation.

## Parallel Execution

Independent tasks may execute concurrently where resource, safety and mission constraints permit.

Parallel execution shall not create conflicting UAV or resource assignments.

## Sequential Execution

Tasks requiring ordered execution shall remain sequential.

## UAV Assignment

Each executable UAV task shall reference an authoritative UAV assignment.

The assignment shall be checked against current:

- availability;
- capability;
- health;
- battery;
- communication;
- mission state.

## Fleet Execution

Fleet missions shall coordinate UAV execution through the Fleet Coordination architecture.

The orchestrator shall preserve individual UAV execution state.

## Resource Coordination

Execution shall consume resources through the Resource Management architecture.

The orchestrator shall not assume that previously reserved resources remain available.

## Resource Loss

Loss of a required runtime resource shall produce an explicit execution condition.

The system shall determine whether to:

- continue;
- degrade;
- reallocate;
- replan;
- pause;
- abort.

## Communication Dependency

Tasks requiring communication shall consider current communication state.

Communication degradation or loss shall be visible to execution coordination.

## C2 Dependency

Tasks requiring C2 shall verify applicable C2 availability before and during execution.

## Telemetry Dependency

Safety-relevant execution shall consume current authoritative telemetry where required.

Stale or missing telemetry shall trigger applicable safety handling.

## Runtime Validation

Runtime conditions may require validation during execution.

Material changes shall trigger revalidation according to policy.

## Safety Engine

Safety Engine shall remain authoritative for safety-critical execution decisions.

The orchestrator shall not override mandatory safety blocks.

## Safety Gate

Before executing safety-critical actions, the orchestrator shall satisfy applicable safety gates.

## Execution Gate

Execution shall pass the applicable execution gate.

The execution gate shall verify the current mission, approval, readiness and safety state.

## Operator Approval

Actions requiring human approval shall not execute without valid approval.

Approval shall bind to the applicable mission revision and execution context.

## Mission AI

Mission AI may provide runtime recommendations and replanning proposals.

Mission AI shall not independently authorize or execute operational actions.

## Runtime Adaptation

Runtime adaptation may modify execution when explicitly permitted.

Adaptation shall preserve:

- mission intent;
- safety constraints;
- authority boundaries;
- traceability.

## Runtime Replanning

Runtime replanning may be triggered by:

- conflict;
- UAV loss;
- resource loss;
- communication degradation;
- weather;
- airspace change;
- terrain change;
- mission deviation;
- safety condition.

Replanning shall pass through Validation and applicable Approval requirements before execution of the resulting mission revision.

## Mission Intent Preservation

Runtime adaptation shall not silently change the fundamental mission intent.

Material intent changes shall create a new mission revision or require explicit operator action according to policy.

## Execution Context

Every execution shall maintain an execution context containing applicable:

- mission ID;
- mission revision;
- configuration;
- schedule;
- UAV assignments;
- resource allocations;
- environment;
- operational mode;
- approval;
- safety state.

## Execution Commands

Commands generated by the orchestrator shall be validated before being passed to external adapters.

## Adapter Boundary

External execution shall use defined adapters.

The orchestrator shall not contain manufacturer-specific UAV control logic where adapter abstraction is appropriate.

## Command Acknowledgement

Where supported, external commands shall produce an acknowledgement or observable execution result.

An unacknowledged command shall not automatically be considered successful.

## Idempotency

Execution operations shall use idempotency mechanisms where repeated requests could create unsafe duplicate actions.

## Ordering

Execution commands shall preserve required ordering.

Out-of-order execution shall be detected where operationally relevant.

## Duplicate Handling

Duplicate commands or events shall not create duplicate authoritative state transitions.

## Timeout

Execution operations shall have applicable timeout policies.

Timeouts shall produce explicit execution conditions.

## Retry

Retries shall be controlled.

Safety-critical commands shall not be blindly retried.

Retry policy shall consider command idempotency and current external state.

## Backpressure

Execution orchestration shall protect the Core and adapters from uncontrolled command or event rates.

## Monitoring

Runtime execution shall continuously monitor applicable:

- mission state;
- task state;
- UAV state;
- fleet state;
- communication;
- resources;
- environment;
- safety;
- validation;
- readiness.

## Health Integration

Execution health shall integrate with system and mission health.

Execution degradation shall become observable.

## Runtime Events

The system shall record important events including:

- execution requested;
- authorization result;
- validation result;
- execution started;
- task started;
- task completed;
- task failed;
- resource loss;
- communication loss;
- conflict;
- replanning;
- operator action;
- safety action;
- execution completed;
- abort.

## Event Correlation

Runtime events shall correlate with:

- mission;
- mission revision;
- task;
- UAV;
- fleet;
- configuration;
- schedule;
- operator;
- execution context.

## Planned / Simulated / Actual

Execution shall preserve distinction between:

PLANNED;
SIMULATED;
ACTUAL.

Actual execution state shall never be inferred from simulation state.

## Simulation

Simulation shall use the same mission architecture while remaining isolated from operational execution.

Simulation shall not create actual execution commands.

## Digital Twin

Digital Twin may reproduce execution logic and state transitions for evaluation.

Digital Twin shall remain outside operational execution authority.

## Sandbox

Sandbox execution shall remain isolated from operational resources and external execution authority.

## Environment

Execution shall verify that the current environment is appropriate.

An environment mismatch shall block or invalidate execution as required.

## Operational Mode

Execution shall respect the active operational mode.

Mode transitions during execution shall trigger applicable reassessment.

## Pause

Where supported, execution may enter:

PAUSE_REQUESTED
→ PAUSING
→ PAUSED.

Pause shall be subject to safety and mission constraints.

## Resume

Resume shall verify:

- mission state;
- UAV state;
- resources;
- communication;
- safety;
- validation;
- readiness.

No unsafe automatic resume shall occur.

## Abort

Abort shall follow the Safety Architecture.

The orchestrator shall execute the defined abort sequence and record its result.

## Emergency

Emergency execution shall follow dedicated emergency procedures.

Emergency behaviour shall remain under Safety Authority.

## Failure Handling

Execution failures shall be classified.

Categories may include:

- task failure;
- UAV failure;
- communication failure;
- resource failure;
- external adapter failure;
- validation failure;
- safety block;
- configuration failure.

## Recovery

Recoverable failures may enter a defined recovery state machine.

Recovery shall verify current state before resuming execution.

## No Unsafe Resume

The system shall not resume execution solely because a failed service or resource becomes available.

Applicable safety, validation and readiness checks shall be performed first.

## Partial Completion

The system shall represent partially completed missions and tasks explicitly.

Completed work shall remain traceable.

## Mission Completion

Mission completion shall require applicable task completion and mission completion criteria.

The system shall not report COMPLETED while mandatory tasks remain unresolved.

## Execution Result

The execution result shall distinguish:

- successful;
- partially successful;
- failed;
- aborted;
- incomplete.

## Deviation

Actual execution shall be compared with planned and simulated behaviour where applicable.

Material deviation shall become observable.

## Execution Metrics

Runtime metrics may include:

- task duration;
- execution latency;
- command latency;
- resource consumption;
- battery consumption;
- communication quality;
- deviation;
- failures;
- recovery time.

## Resource Release

Resources shall be released after task or mission completion according to policy.

## UAV Release

UAVs shall transition to the appropriate post-mission state after execution.

## Fleet Reconciliation

Fleet state shall be reconciled after mission completion, failure or abort.

## Communication Recovery

Communication recovery shall reconcile current external state before continuing execution.

## External State Reconciliation

After adapter reconnection, the Core shall reconcile authoritative state with external system state.

The system shall not blindly replay commands without state verification.

## Persistence

Execution state shall be persisted sufficiently to support recovery and reconstruction.

## Restart Recovery

After Core restart, the system shall recover applicable execution state and determine whether execution may safely continue.

## Recovery Gate

Recovered execution shall pass applicable:

- integrity;
- configuration;
- safety;
- validation;
- readiness;
- authority

checks before resumption.

## Audit

Material execution decisions shall be auditable.

Audit shall include where applicable:

- operator;
- mission;
- revision;
- action;
- authorization;
- validation;
- safety state;
- execution result;
- timestamp.

## Observability

Execution shall integrate with the observability and logging architecture.

The system shall distinguish:

REQUEST
→ DECISION
→ COMMAND
→ ACKNOWLEDGEMENT
→ RESULT.

## Operator Visibility

The UI shall expose authoritative execution state including:

- current mission;
- current task;
- UAV assignment;
- execution state;
- active constraints;
- warnings;
- conflicts;
- failures;
- recovery state.

## Human Boundary

The operator shall remain informed of material runtime changes.

Where operator approval is required, execution shall stop at the applicable approval boundary.

## No Direct UI Execution

QML shall not directly issue execution commands.

## No Direct AI Execution

Mission AI shall not directly issue execution commands.

## No Direct Simulation Execution

Simulation shall not issue operational execution commands.

## Safety Boundary

No execution orchestration mechanism shall bypass Safety Engine authority.

## Security

Execution requests shall follow authentication and authorization requirements.

Execution commands shall respect established trust boundaries.

## Performance

The orchestrator shall provide predictable runtime coordination without blocking safety-critical processing.

## Scalability

The architecture shall support increasing:

- missions;
- tasks;
- UAVs;
- fleet size;
- execution events;
- telemetry volume.

## Verification

Execution orchestration shall be verified through:

- normal execution;
- dependency ordering;
- parallel execution;
- resource loss;
- UAV loss;
- communication loss;
- C2 degradation;
- telemetry loss;
- timeout;
- retry;
- duplicate command;
- out-of-order event;
- runtime replanning;
- pause;
- resume;
- abort;
- emergency;
- restart recovery;
- partial completion;
- simulation isolation;
- sandbox isolation.

## Negative Testing

Verification shall confirm that:

- QML cannot directly execute UAV commands;
- Mission AI cannot directly execute UAV commands;
- simulation cannot create actual execution;
- an unapproved mission cannot execute;
- failed validation blocks execution;
- insufficient readiness blocks execution;
- stale state cannot authorize execution;
- duplicate commands cannot create unsafe duplicate actions;
- failed commands are not reported as successful;
- unsafe resume cannot occur;
- adapter reconnection cannot silently replay unsafe commands.

## Traceability

Execution shall support:

MISSION
→ REVISION
→ CONFIGURATION
→ SCHEDULE
→ APPROVAL
→ EXECUTION CONTEXT
→ TASK
→ COMMAND
→ ACKNOWLEDGEMENT
→ RESULT.

## Rationale

A dedicated Mission Execution Orchestrator provides deterministic coordination between approved mission intent and runtime execution while preserving Core authority, Safety Engine authority, resource coordination, adapter isolation, recovery and complete execution traceability.


