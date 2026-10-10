---
id: ARCH-DEC-012
type: architecture_decision
title: Fleet and Resource Coordination
status: accepted
scope: BlueSky System
---

# ARCH-DEC-012 — Fleet and Resource Coordination

## Decision

BlueSky shall provide a Fleet and Resource Coordination architecture for allocating available UAVs, payloads, communication resources and other mission resources according to the required mission capabilities.

Resource allocation shall be driven by the mission requirements and required capabilities rather than by a fixed platform assignment.

## Resource Allocation Flow

The primary flow shall be:

MISSION
→ REQUIRED CAPABILITIES
→ AVAILABLE RESOURCES
→ FLEET / UAV ROLES
→ RESOURCE ALLOCATION
→ MISSION PLAN
→ VALIDATION
→ EXECUTION

## Required Capabilities

Fleet coordination shall use the capabilities required by the current mission configuration.

The system shall determine which available resources can provide the required capabilities.

A capability may be provided by:

- one UAV;
- one payload;
- multiple UAVs;
- multiple payloads;
- communication infrastructure;
- computing resources;
- combinations of system components.

## Fleet Representation

The fleet model shall represent applicable:

- UAVs;
- UAV capabilities;
- payloads;
- payload capabilities;
- health;
- battery or energy state;
- position;
- availability;
- communication state;
- mission assignment;
- current role;
- operational constraints.

## UAV Roles

The system shall support assignment of UAV roles according to mission requirements.

Roles may include:

- primary mission UAV;
- support UAV;
- relay UAV;
- observation UAV;
- mapping UAV;
- search UAV;
- reserve UAV;
- recovery or contingency role.

Additional roles may be introduced without changing the basic fleet architecture.

## Resource Constraints

Allocation shall consider applicable:

- capability requirements;
- UAV availability;
- payload availability;
- energy;
- range;
- communication;
- terrain;
- weather;
- airspace;
- geofence;
- operational restrictions;
- safety requirements;
- mission timing;
- optimization profile.

## Optimization

Resource allocation may be influenced by the active optimization profile:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

FAST may prefer reduced execution time.

SAFE may prefer increased redundancy and safety margins.

ECONOMIC may prefer efficient resource and energy usage.

HIGH QUALITY may prefer additional resources or capabilities required to achieve the required result.

Optimization preferences shall not override mandatory safety constraints.

## Fleet Coordination

The system shall coordinate multiple UAVs when required by the mission.

Coordination may include:

- task allocation;
- role assignment;
- timing;
- spatial separation;
- communication;
- relay positioning;
- shared mission state;
- capability coordination;
- data fusion;
- contingency behaviour.

## Dynamic Resource State

Fleet resources shall be treated as dynamic.

The system shall account for changes in:

- UAV availability;
- battery;
- health;
- position;
- communication;
- payload state;
- weather;
- mission conditions;
- fleet composition.

## Dynamic Reallocation

When a material resource change occurs, the system may:

- reassign a role;
- reallocate a task;
- select a reserve UAV;
- change payload assignment;
- change relay allocation;
- modify resource distribution;
- request replanning.

The resulting mission change shall be subject to applicable validation and safety controls.

## Resource Loss

If an assigned UAV, payload or communication resource becomes unavailable, the system shall determine whether:

- the mission can continue;
- another resource can replace the unavailable resource;
- the mission must be adapted;
- the mission requires replanning;
- the mission must be blocked or aborted.

## Reserve Resources

The architecture shall support reserve resources when required by the mission.

Reserve resources may be maintained for:

- failure recovery;
- battery degradation;
- communication degradation;
- contingency;
- replacement of unavailable UAVs;
- mission continuity.

## Conflict Integration

Fleet and resource conflicts shall integrate with Conflict Resolution.

A resource conflict may produce:

CONFLICT
→ GENERATE OPTIONS
→ EVALUATE
→ SELECT
→ REVALIDATE
→ REPLAN

Resolution options may include:

- different UAV;
- different role;
- different timing;
- different route;
- different payload;
- different resource allocation.

## Mission AI Integration

Mission AI may propose fleet assignments and resource allocations.

Mission AI shall not bypass:

- Mission Validation;
- Mission Readiness;
- Safety Engine;
- operator approval when required.

## Simulation Integration

Fleet allocation shall be testable in Simulation / Digital Twin.

Simulation may evaluate:

- fleet interaction;
- role assignment;
- resource utilization;
- communication;
- energy;
- contingencies;
- dynamic reallocation.

Simulation results may be used to improve resource allocation before execution.

## Mission State Integration

Fleet coordination shall integrate with the Mission State Machine.

Material fleet changes may cause:

EXECUTING
→ REVALIDATE
→ ADAPT

or:

EXECUTING
→ REQUIRES_REPLAN

or:

EXECUTING
→ ABORTED

according to applicable safety and mission conditions.

## Safety Authority

Fleet and resource allocation shall remain subordinate to Safety Engine decisions.

The system shall not allocate or reallocate resources in a manner that violates mandatory safety constraints.

## Traceability

The system shall preserve traceability between:

MISSION
→ REQUIRED CAPABILITIES
→ RESOURCE SET
→ UAV ROLES
→ RESOURCE ALLOCATION
→ VALIDATION
→ SIMULATION
→ EXECUTION
→ ACTUAL RESULT

Material allocation changes shall identify:

- previous allocation;
- new allocation;
- triggering condition;
- affected mission elements;
- validation result;
- operator decision when applicable.

## Rationale

Capability-driven fleet coordination allows BlueSky to select and dynamically allocate resources according to the mission rather than binding the mission to a fixed UAV or fleet configuration.


