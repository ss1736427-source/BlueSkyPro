---
id: ARCH-DEC-008
type: architecture_decision
title: Conflict Resolution and Replanning
status: accepted
scope: BlueSky System
---

# ARCH-DEC-008 — Conflict Resolution and Replanning

## Decision

BlueSky shall provide an explicit conflict resolution and replanning mechanism.

A detected conflict shall result in one or more candidate resolution options rather than an immediate uncontrolled modification of the mission.

## Conflict Resolution Flow

The target flow is:

CONFLICT
→ DETECT
→ GENERATE OPTIONS
→ EVALUATE
→ SELECT
→ REVALIDATE
→ REPLAN
→ READY

The exact transition shall depend on the type and severity of the conflict.

## Conflict Detection

Conflicts may be detected by:

- Mission Validation;
- Mission Readiness;
- Safety Engine;
- Mission Graph;
- dynamic monitoring;
- communication state;
- fleet coordination;
- environmental changes;
- operator input.

## Resolution Options

The system shall be able to generate alternative resolutions involving:

- time;
- altitude;
- route;
- route segment;
- UAV role;
- resource allocation.

Additional resolution mechanisms may be added without changing the basic conflict model.

## Option Evaluation

Each candidate resolution shall be evaluated against applicable mission criteria.

At minimum, the system shall show the estimated impact on:

- time;
- energy;
- risk;
- mission quality.

Other applicable criteria may also be included.

## Optimization

Candidate conflict resolutions may be evaluated using the active optimization profile:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

The optimization profile shall influence preference between alternatives but shall not override mandatory safety constraints.

## Automatic Resolution

The system may automatically select a resolution when:

- the applicable rules permit automatic resolution;
- the resulting mission remains valid;
- mandatory safety constraints remain satisfied;
- no required operator decision is pending.

An automatically selected resolution shall remain traceable.

## Operator Selection

When operator review or approval is required, the system shall present the available resolution options and their expected impact.

The operator may select an applicable option.

The selected option shall become part of the mission revision and shall remain traceable to the original conflict.

## Revalidation

No conflict resolution shall be considered final solely because an option was selected.

After a material mission change, the system shall perform applicable:

VALIDATION
→ READINESS
→ SAFETY GATE

checks.

A resolution that produces a failed critical validation shall not proceed to execution.

## Replanning

When conflict resolution changes mission behaviour, the system shall regenerate or update the affected mission plan.

The system shall preserve the relationship between:

ORIGINAL PLAN
→ CONFLICT
→ RESOLUTION OPTION
→ SELECTED RESOLUTION
→ NEW PLAN

## Mission State Integration

Conflict resolution shall integrate with the Mission State Machine.

A conflict may cause a transition to:

- CONFLICT;
- REQUIRES_REPLAN;
- VALIDATION_FAILED;
- BLOCKED.

After successful resolution and revalidation, the mission may return to an applicable planning or readiness state.

## Runtime Conflicts

During execution, a newly detected conflict may trigger:

EXECUTING
→ REVALIDATE
→ ADAPT

or:

EXECUTING
→ REQUIRES_REPLAN

or, when required by safety conditions:

EXECUTING
→ ABORTED

The Safety Engine remains authoritative for safety-critical decisions.

## Traceability

The system shall preserve:

- conflict identification;
- conflict type;
- detection source;
- affected mission elements;
- generated options;
- option evaluation;
- selected option;
- operator decision when applicable;
- resulting mission revision;
- validation results after resolution;
- readiness result after resolution.

## Safety Priority

Conflict resolution shall not bypass safety controls.

The priority relationship is:

SAFETY
>
MANDATORY CONSTRAINTS
>
MISSION REQUIREMENTS
>
OPERATOR / USER PREFERENCES
>
OPTIMIZATION PREFERENCES

## Rationale

Separating conflict detection, option generation, evaluation, selection and revalidation prevents an optimization decision from silently introducing an invalid or unsafe mission configuration.


