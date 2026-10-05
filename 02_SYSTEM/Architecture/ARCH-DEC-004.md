---
id: ARCH-DEC-004
type: architecture_decision
title: Mission Data Model / Mission State
status: accepted
scope: BlueSky System
---

# ARCH-DEC-004 — Mission Data Model / Mission State

## Decision

BlueSky shall use a unified Mission object as the primary system object connecting task understanding, mission configuration, capabilities, planning, validation, optimization, simulation, approval and execution.

The Mission object shall preserve traceability to the original user task throughout its lifecycle.

## Mission Lifecycle

The target mission lifecycle is:

CLIENT INTENT
→ TASK
→ MISSION CONFIGURATION
→ REQUIRED CAPABILITIES
→ MISSION GRAPH
→ MISSION PLAN
→ VALIDATION
→ OPTIMIZATION
→ SIMULATION
→ READINESS
→ APPROVAL
→ EXECUTION
→ ACTUAL

## Mission Object

The Mission object shall contain or reference the information required to represent:

- original user intent;
- structured task;
- selected task template;
- modular mission configuration;
- required capabilities;
- Mission Graph;
- mission plan;
- optimization profile;
- validation results;
- conflict resolution results;
- simulation results;
- readiness assessment;
- approval state;
- execution state;
- actual execution data;
- traceability information.

## Traceability

The Mission object shall maintain a traceable relationship between:

CLIENT INTENT
→ TASK
→ CONFIGURATION
→ CAPABILITIES
→ MISSION GRAPH
→ PLAN
→ VALIDATION
→ SIMULATION
→ APPROVAL
→ EXECUTION
→ ACTUAL

Changes to the mission shall remain traceable to the originating task or subsequent operator/system decision.

## Mission States

The architecture shall distinguish the following data states:

### PLANNED

Represents the currently approved or proposed mission plan before execution.

### SIMULATED

Represents the results and state produced by virtual execution in Simulation / Digital Twin.

### ACTUAL

Represents data produced by real mission execution.

PLANNED, SIMULATED and ACTUAL data shall not be silently merged or treated as identical.

## State Relationships

The system shall support comparison between:

PLANNED ↔ SIMULATED

and, when actual execution data exists:

PLANNED ↔ SIMULATED ↔ ACTUAL

Differences between these states shall be available for analysis, validation and future optimization.

## Mission Evolution

The Mission object shall support iterative refinement.

A mission may be modified as a result of:

- user requirements;
- template adaptation;
- capability changes;
- validation findings;
- conflict resolution;
- optimization;
- simulation results;
- changing operational conditions;
- operator decisions.

Each significant modification shall preserve the relationship with the previous mission state or version.

## Versioning

Mission changes shall be represented as distinct revisions or otherwise remain recoverable.

The system shall be able to determine:

- what changed;
- when it changed;
- why it changed;
- which component or operator initiated the change;
- which validation or simulation state applies to the revision.

## Separation of Concerns

The Mission object is a system-level domain object.

UI representations shall not become the authoritative source of mission state.

The authoritative mission state shall be maintained by the C++ Core.

Simulation and external adapters shall operate on defined Mission representations and shall not create an independent incompatible mission model.

## Safety

A mission state shall not be considered executable solely because a Mission object exists.

Execution shall require the applicable:

- Mission Validation;
- Mission Readiness;
- Safety Engine decisions;
- Approval.

## Rationale

A unified Mission data model prevents individual modules from maintaining incompatible representations of the same mission and provides a common object for planning, validation, simulation, approval and execution.


