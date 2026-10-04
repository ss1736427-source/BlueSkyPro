---
id: ARCH-DEC-005
type: architecture_decision
title: Mission State Machine
status: accepted
scope: BlueSky System
---

# ARCH-DEC-005 — Mission State Machine

## Decision

BlueSky shall represent the Mission lifecycle as an explicit state machine.

Mission state transitions shall be controlled by defined system conditions, validation results, operator decisions and execution events.

The existence of a Mission object shall not by itself authorize execution.

## Primary Mission States

The primary lifecycle shall support:

DRAFT
→ CONFIGURING
→ PLANNED
→ VALIDATING
→ OPTIMIZING
→ SIMULATING
→ READY
→ APPROVAL_REQUIRED
→ APPROVED
→ EXECUTING
→ COMPLETED

## State Definitions

### DRAFT

The mission object exists but is not yet sufficiently configured for planning.

### CONFIGURING

The system is building or modifying the mission configuration, including templates, capabilities and mission parameters.

### PLANNED

A mission plan has been generated and is available for validation.

### VALIDATING

The Mission Validation Engine is evaluating the mission.

### OPTIMIZING

The system is generating or evaluating alternative mission variants according to the applicable optimization profile.

Supported profiles include:

- FAST
- SAFE
- ECONOMIC
- HIGH QUALITY

### SIMULATING

The mission is being evaluated in Simulation / Digital Twin.

### READY

Mission Validation and Mission Readiness indicate that the mission satisfies the conditions required for operator review.

### APPROVAL_REQUIRED

The mission requires an explicit approval decision before execution.

### APPROVED

The mission has received the required approval and may proceed to execution, subject to applicable runtime safety conditions.

### EXECUTING

The mission is being executed in the operational environment.

### COMPLETED

The mission execution has ended and actual execution data is available for analysis.

## Exception States

The state machine shall also support:

- VALIDATION_FAILED
- CONFLICT
- BLOCKED
- REQUIRES_REPLAN
- ABORTED

## VALIDATION_FAILED

The mission does not satisfy one or more required validation conditions.

The mission shall not proceed directly to execution.

The mission may return to CONFIGURING, OPTIMIZING or another applicable planning state after correction.

## CONFLICT

A mission conflict has been detected and requires conflict resolution.

The system shall provide or request conflict resolution before continuing.

## BLOCKED

Execution or progression is prohibited because a critical condition has not been satisfied.

A blocked mission shall not transition to EXECUTING until the blocking condition is resolved and applicable validation is repeated.

## REQUIRES_REPLAN

A change in requirements, environment, mission state or validation result requires the mission plan to be regenerated or substantially modified.

The mission shall return to an applicable planning state.

## ABORTED

Mission execution has been terminated or cancelled.

The mission shall preserve its previous state and relevant execution information for traceability.

## Transition Rules

A state transition shall occur only when the conditions required for that transition are satisfied.

Examples:

DRAFT
→ CONFIGURING

when mission configuration begins.

CONFIGURING
→ PLANNED

when a mission plan has been generated.

PLANNED
→ VALIDATING

when validation is requested.

VALIDATING
→ OPTIMIZING

when optimization is required.

VALIDATING
→ SIMULATING

when simulation is required.

VALIDATING
→ READY

when applicable validation and readiness conditions are satisfied.

OPTIMIZING
→ SIMULATING

when a candidate mission variant is ready for simulation.

SIMULATING
→ VALIDATING

when simulation results require validation.

READY
→ APPROVAL_REQUIRED

when the mission is ready for operator approval.

APPROVAL_REQUIRED
→ APPROVED

when the required approval is granted.

APPROVED
→ EXECUTING

only when all applicable execution conditions are satisfied.

EXECUTING
→ COMPLETED

when execution has ended successfully.

## Safety Rules

The following transitions shall not bypass required safety controls:

- READY → EXECUTING
- APPROVAL_REQUIRED → EXECUTING
- APPROVED → EXECUTING

Execution shall remain subject to Safety Engine decisions, Mission Validation, Mission Readiness and required operator approval.

A failed critical validation shall prevent execution.

## Revalidation

Changes to a mission after validation, optimization, simulation, readiness or approval may invalidate previous results.

When a change materially affects mission behaviour, the system shall return the mission to an applicable validation or planning state.

Previously obtained validation, simulation and readiness results shall not automatically be treated as valid for a materially changed mission.

## Runtime Adaptation

During EXECUTING, significant changes in:

- environment;
- weather;
- communication;
- battery or energy;
- fleet state;
- mission conditions;
- safety conditions

may trigger:

REVALIDATE
→ ADAPT
→ CONTINUE

or:

REVALIDATE
→ REQUIRES_REPLAN

or:

REVALIDATE
→ ABORTED

according to applicable system and safety decisions.

## Traceability

Every significant state transition shall be traceable to:

- the previous state;
- the new state;
- the triggering event or condition;
- the responsible system component;
- the operator decision, when applicable;
- relevant validation or simulation results.

## Rationale

An explicit Mission State Machine provides deterministic lifecycle control and prevents uncontrolled transitions between planning, validation, simulation, approval and execution.


