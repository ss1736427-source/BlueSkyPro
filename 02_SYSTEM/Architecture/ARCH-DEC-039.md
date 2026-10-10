---
id: ARCH-DEC-039
type: architecture_decision
title: Mission Runtime Adaptation and Dynamic Replanning Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-039 — Mission Runtime Adaptation / Dynamic Replanning Architecture

## Decision

BlueSky shall support controlled runtime adaptation and dynamic mission replanning when actual conditions differ materially from the approved mission basis.

Runtime adaptation shall preserve Safety Engine authority, mission traceability, operator authority and explicit validation of material changes.

## Authority

The C++ Core shall own authoritative runtime mission state.

Mission AI may detect conditions and propose adaptations.

The operator may select or approve adaptations where required.

Safety Engine shall have authority to block unsafe adaptations.

## Runtime Adaptation Triggers

Runtime adaptation may be triggered by:

- UAV failure;
- UAV degradation;
- fleet resource loss;
- battery or energy deviation;
- communication degradation;
- C2 loss;
- weather change;
- terrain information change;
- airspace change;
- geofence change;
- unexpected obstacle;
- route conflict;
- mission delay;
- resource exhaustion;
- external system failure;
- significant telemetry deviation.

## Adaptation Classification

Runtime changes shall be classified as:

- non-material;
- operational;
- mission-impacting;
- safety-impacting;
- approval-required.

Classification shall determine required validation and authority.

## No Silent Mission Change

Material mission changes shall not silently modify the approved mission.

Material changes shall create a traceable mission revision or runtime adaptation record.

## Mission Intent

Runtime adaptation shall preserve the approved mission intent unless an explicitly authorized change is made.

A change of fundamental mission intent shall require applicable operator decision and approval.

## Runtime State

The system shall distinguish:

- approved baseline;
- current execution state;
- proposed adaptation;
- validated adaptation;
- approved adaptation;
- active adaptation.

## Detection

Runtime monitoring shall detect deviations between expected and actual state.

Detection shall identify:

- source;
- time;
- affected mission;
- affected task;
- affected UAV;
- severity;
- confidence where applicable.

## Evaluation

Detected deviations shall be evaluated against mission, safety and operational constraints.

Evaluation shall determine whether:

- no action is required;
- adaptation is sufficient;
- replanning is required;
- execution must pause;
- execution must abort.

## Replanning Trigger

Replanning shall be initiated when the current mission can no longer satisfy applicable constraints or objectives without material change.

## Replanning Process

The standard process shall be:

DETECT
→ CLASSIFY
→ EVALUATE
→ GENERATE OPTIONS
→ SIMULATE / ANALYZE
→ VALIDATE
→ SELECT / APPROVE
→ EXECUTE.

## Option Generation

The system may generate multiple alternatives.

Options may differ by:

- route;
- altitude;
- timing;
- UAV assignment;
- task ordering;
- resource allocation;
- mission variant.

## Option Evaluation

Alternatives shall be evaluated against:

- safety;
- mission objectives;
- constraints;
- resources;
- energy;
- risk;
- quality;
- execution time.

## Optimization

Replanning may use optimization profiles:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

Safety constraints shall remain mandatory regardless of selected profile.

## Safety Priority

Safety shall take priority over:

- speed;
- economic efficiency;
- quality;
- optimization score.

## Safety Validation

Every material adaptation shall pass applicable Safety Engine checks.

Unsafe alternatives shall be rejected or blocked.

## Mission Validation

Material adaptations shall pass Mission Validation before becoming executable.

## Readiness

Readiness shall be reassessed after material runtime adaptation.

## Approval

Where approval is required, execution shall not continue with the new mission state until the required approval is obtained.

## Operator Selection

When multiple valid alternatives exist, the operator may select an authorized alternative.

Selection shall be recorded.

## Automatic Adaptation

Automatic adaptation may occur only for explicitly authorized classes of changes.

Automatic adaptation shall remain subject to predefined safety and validation rules.

## Automatic Replanning

Automatic replanning shall not grant unrestricted execution authority.

A generated plan shall remain a proposal until applicable validation and approval requirements are satisfied.

## Mission AI

Mission AI may:

- detect patterns;
- identify deviations;
- generate alternatives;
- estimate consequences;
- recommend replanning;
- explain alternatives.

Mission AI shall not independently authorize material execution changes.

## Explainability

Material AI-generated or automated adaptations shall provide an explanation where applicable.

The explanation shall identify:

- trigger;
- affected mission;
- constraints;
- proposed change;
- expected benefit;
- risk;
- rejected alternatives where available.

## Conflict Integration

Runtime conflicts shall integrate with Conflict Resolution.

The system shall preserve:

- conflict;
- affected entities;
- generated options;
- selected option;
- validation;
- result.

## Resource Integration

Replanning shall use current resource state.

Resource loss may cause:

- UAV reassignment;
- task redistribution;
- schedule change;
- mission degradation;
- replanning.

## Fleet Integration

Fleet Coordination shall provide current UAV and fleet state to replanning.

A failed UAV shall not remain treated as available.

## Communication Integration

Communication state shall be considered during replanning.

Loss of required communication capability shall trigger applicable handling.

## C2 Integration

C2 availability shall be included when the mission depends on command or communication links.

## Environmental Integration

Replanning shall consider current:

- weather;
- terrain;
- airspace;
- geofence;
- environmental constraints.

## Spatial Integration

Replanning shall evaluate spatial validity.

The mission shall be evaluated in four dimensions where applicable:

X
Y
Z
TIME.

## Route Adaptation

Route changes shall be validated against:

- terrain;
- airspace;
- geofence;
- obstacles;
- weather;
- UAV limits;
- mission constraints.

## Altitude Adaptation

Altitude changes shall remain subject to applicable:

- airspace;
- terrain;
- UAV;
- safety;
- mission

constraints.

## Task Adaptation

Tasks may be:

- reordered;
- deferred;
- reassigned;
- modified;
- cancelled

where permitted.

## Task Dependency

Replanning shall preserve mandatory task dependencies.

## Resource Reallocation

Resources may be dynamically reallocated after validation.

Reallocation shall remain traceable.

## Energy Adaptation

Battery and energy deviations may trigger replanning.

The system shall preserve required reserves and safety margins.

## Communication Degradation

Communication degradation may trigger:

- route adaptation;
- task adaptation;
- relay allocation;
- degraded operation;
- pause;
- abort.

## Weather Adaptation

Material weather changes shall trigger applicable reassessment.

Forecast information shall remain distinguishable from observed conditions.

## Airspace Adaptation

Airspace changes affecting the mission shall trigger revalidation and applicable replanning.

## Geofence Adaptation

Geofence changes shall trigger applicable spatial validation.

## UAV Loss

Loss of a UAV shall trigger impact analysis.

The system shall determine affected:

- tasks;
- resources;
- schedule;
- mission objectives.

## Fleet Reallocation

Available UAVs may be reassigned where capability, safety and mission constraints permit.

## Partial Mission

The system shall support partial mission completion.

Completed tasks shall remain completed and traceable.

## Mission Degradation

Where full mission objectives cannot be maintained, the system may enter an explicitly defined degraded mission state.

Degradation shall be visible to the operator.

## Pause Boundary

When safe continuation cannot be established, execution may enter a paused or blocked state pending reassessment.

## Abort Boundary

When continued execution is unsafe, the Safety Architecture shall control abort behaviour.

## Simulation Support

Candidate replans may be evaluated in Simulation or Digital Twin before execution where time and safety constraints permit.

Simulation shall remain isolated from actual execution.

## Fast Runtime Replanning

Time-critical replanning may use a FAST profile.

Fast replanning shall not bypass mandatory safety validation.

## Safe Runtime Replanning

SAFE profile shall prioritize safety margins and conservative alternatives.

## Economic Runtime Replanning

ECONOMIC profile may optimize resource or energy consumption subject to safety constraints.

## High Quality Runtime Replanning

HIGH QUALITY profile may prioritize mission quality and objective fulfilment subject to safety constraints.

## Revalidation

After material replanning:

VALIDATE
→ READINESS
→ APPROVAL where required
→ EXECUTE.

## Baseline Preservation

The previous approved mission baseline shall remain preserved.

The new runtime plan shall reference the baseline from which it originated.

## Revision Identity

Every material replan shall have an identifiable:

- adaptation ID;
- mission revision;
- parent revision;
- trigger;
- timestamp;
- author or source.

## Versioning

Runtime adaptations shall be versioned and traceable.

## Rollback

Where safe and technically possible, the system may return to a previously valid plan.

Rollback shall itself be validated.

## No Unsafe Rollback

Rollback shall not restore a plan that is incompatible with current conditions.

## State Reconciliation

After adaptation, the Core shall reconcile:

- mission state;
- task state;
- UAV state;
- fleet state;
- resource state;
- schedule;
- communication;
- environment.

## External State

External adapter state shall be reconciled before applying commands resulting from a replan.

## Command Safety

Commands resulting from a replan shall pass through normal execution boundaries.

## No Direct Replan Execution

A replanning engine shall not directly command UAVs.

## Persistence

Runtime adaptation state shall be persisted sufficiently for recovery and reconstruction.

## Recovery

After restart or service recovery, the system shall identify:

- active mission;
- active adaptation;
- previous baseline;
- pending approval;
- current validation;
- current readiness.

## Recovery Gate

Recovered adaptations shall not automatically resume execution without applicable safety and readiness checks.

## Observability

Adaptation events shall integrate with observability.

The system shall record:

- trigger;
- detection;
- evaluation;
- options;
- selection;
- validation;
- approval;
- execution;
- result.

## Audit

Material runtime adaptations shall be auditable.

Audit shall include:

- operator or automatic source;
- mission;
- previous state;
- new state;
- reason;
- authorization;
- validation;
- result.

## Operator Visibility

The UI shall clearly distinguish:

- current approved baseline;
- actual state;
- detected deviation;
- proposed replan;
- selected replan;
- validation state;
- approval state;
- execution state.

## No False Success

A proposed adaptation shall not be shown as active until authoritative execution state confirms activation.

## Planned / Simulated / Actual

Runtime adaptation shall preserve distinction between:

PLANNED;
SIMULATED;
ACTUAL.

## Traceability

Runtime adaptation shall support:

EVENT
→ DETECTION
→ CLASSIFICATION
→ PROPOSAL
→ EVALUATION
→ VALIDATION
→ APPROVAL
→ REVISION
→ EXECUTION
→ RESULT.

## Verification

The architecture shall be verified through:

- normal deviation;
- route conflict;
- UAV loss;
- resource loss;
- battery deviation;
- communication loss;
- C2 degradation;
- weather change;
- airspace change;
- geofence change;
- task failure;
- deadline failure;
- automatic adaptation;
- operator-selected adaptation;
- AI recommendation;
- simulation;
- approval;
- rollback;
- recovery.

## Negative Testing

Verification shall confirm that:

- Mission AI cannot directly execute a replan;
- QML cannot directly execute a replan;
- unsafe replans are blocked;
- unvalidated replans cannot execute;
- unapproved material replans cannot execute;
- stale state cannot authorize adaptation;
- simulation cannot create actual execution;
- rollback cannot restore an unsafe plan;
- mission intent cannot be silently changed;
- duplicate adaptation events cannot create duplicate execution.

## Performance

Runtime replanning shall meet applicable timing requirements.

Critical replanning paths shall receive sufficient resources.

## Scalability

The architecture shall support:

- multiple simultaneous missions;
- multiple UAVs;
- fleet-wide replanning;
- multiple concurrent conflicts;
- high telemetry rates.

## Security

Runtime adaptation shall respect authentication, authorization and least privilege.

## Safety Boundary

Safety Engine authority shall remain above runtime adaptation and replanning.

## Rationale

Controlled runtime adaptation allows BlueSky to respond to changing real-world conditions without abandoning the approved mission baseline or safety architecture.

The architecture provides a traceable path from detected deviation to validated, approved and executable adaptation while preserving operator authority and allowing Mission AI to assist without becoming execution authority.


