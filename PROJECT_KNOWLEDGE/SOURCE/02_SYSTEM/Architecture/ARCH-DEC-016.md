---
id: ARCH-DEC-016
type: architecture_decision
title: Safety Architecture and Execution Gate
status: accepted
scope: BlueSky System
---

# ARCH-DEC-016 — Safety Architecture and Execution Gate

## Decision

BlueSky shall enforce a dedicated Safety Architecture separating mission planning, validation, readiness, approval and real execution.

No planning, optimization, simulation, AI or UI component shall bypass the Safety Gate.

## Execution Gate

The normal execution path shall be:

MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION

Each stage shall have an explicit state and result.

## Mission Validation

Mission Validation shall determine whether the mission satisfies applicable technical, spatial, operational and environmental requirements.

Validation shall include applicable:

- individual validation;
- spatial validation;
- mission validation;
- dynamic validation;
- energy;
- communication;
- payload;
- weather;
- resilience;
- timing;
- dependencies;
- coverage.

A failed critical validation shall prevent progression to execution.

## Mission Readiness

Mission Readiness shall consolidate the relevant validation and operational state.

Readiness shall evaluate applicable:

- airspace;
- terrain;
- geofence;
- weather;
- C2 coverage;
- fleet coordination;
- energy;
- contingency.

The readiness result shall identify:

- PASS;
- WARNING;
- BLOCKING condition.

WARNING shall not automatically imply execution approval.

A blocking condition shall prevent execution until resolved or otherwise handled by the applicable safety and operator process.

## Safety Gate

The Safety Gate shall be authoritative for safety-critical execution decisions.

The Safety Gate shall evaluate applicable:

- validation results;
- readiness;
- safety constraints;
- conflicts;
- communication state;
- energy state;
- environmental conditions;
- fleet state;
- emergency conditions;
- runtime state.

The Safety Gate shall be independent from Mission AI optimization preferences.

## Safety Authority

Safety constraints shall have priority over:

- user optimization preferences;
- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY;
- Mission AI recommendations;
- route optimization;
- resource optimization;
- simulation results.

No optimization objective shall override a mandatory safety constraint.

## Operator Approval

Where operator approval is required, the mission shall follow:

READY
→ APPROVAL_REQUIRED
→ APPROVED
→ EXECUTING

The system shall provide the operator with the relevant information required for approval.

Mission AI shall not convert APPROVAL_REQUIRED into APPROVED.

QML shall not independently authorize execution.

## Execution Boundary

The execution authority chain shall remain:

QML
→ C++ CORE
→ VALIDATION / SAFETY
→ ADAPTER
→ EXTERNAL SYSTEM

There shall be no direct execution path:

QML
→ UAV

or:

Mission AI
→ UAV

or:

Simulation
→ UAV

## Blocking Conditions

The following may create a blocking execution condition when applicable:

- failed critical validation;
- failed safety constraint;
- invalid geofence;
- unacceptable airspace condition;
- unacceptable terrain condition;
- unsafe weather;
- insufficient energy;
- unacceptable communication state;
- unresolved critical conflict;
- unavailable required capability;
- failed contingency;
- failed readiness;
- emergency condition.

The exact blocking decision shall remain subject to the applicable Safety Engine rules.

## Conflict Integration

Critical conflicts shall be resolved or explicitly handled before execution.

The normal path shall be:

CONFLICT
→ GENERATE OPTIONS
→ EVALUATE
→ SELECT
→ REVALIDATE
→ READINESS
→ SAFETY GATE

A new blocking condition shall prevent execution.

## Optimization Integration

Mission Optimization may generate multiple alternatives.

Each selected alternative shall pass through:

OPTIMIZATION
→ VALIDATION
→ READINESS
→ SAFETY GATE

Optimization shall never constitute execution authorization.

## Simulation Integration

Simulation / Digital Twin may provide additional evidence for readiness and planning.

Simulation shall not replace:

- Mission Validation;
- Mission Readiness;
- Safety Gate;
- required operator approval.

The normal relationship shall be:

PLAN
→ SIMULATE
→ EVALUATE
→ OPTIMIZE
→ VALIDATE
→ READINESS
→ SAFETY GATE
→ APPROVE
→ EXECUTE

## Mission AI Integration

Mission AI may:

- generate plans;
- generate alternatives;
- optimize;
- explain decisions;
- propose adaptations;
- propose conflict resolutions.

Mission AI shall not:

- bypass validation;
- bypass readiness;
- bypass Safety Gate;
- approve a mission;
- directly control UAV execution.

## Runtime Safety

Safety control shall continue during execution.

Runtime monitoring shall evaluate applicable:

- UAV state;
- energy;
- communication;
- position;
- altitude;
- weather;
- terrain;
- geofence;
- fleet state;
- mission state.

A material safety change may trigger:

EXECUTING
→ REVALIDATE
→ ADAPT

or:

EXECUTING
→ REQUIRES_REPLAN

or:

EXECUTING
→ ABORTED

according to Safety Engine rules.

## Runtime Revalidation

Any material mission change during execution shall be subject to applicable revalidation.

This includes changes caused by:

- Mission AI;
- operator actions;
- conflict resolution;
- resource reallocation;
- communication changes;
- weather changes;
- energy changes;
- fleet changes;
- spatial changes.

## Emergency Authority

Emergency behaviour shall remain under the defined safety architecture.

Emergency actions shall not depend on the availability of the QML interface.

The system shall support predefined emergency and contingency behaviours appropriate to the mission.

## Safety and Simulation Boundary

Simulation may test safety scenarios but shall remain isolated from real execution.

A simulation or Sandbox environment shall not issue real UAV execution commands.

## Traceability

Safety-critical decisions shall remain traceable to:

- mission revision;
- validation result;
- readiness result;
- safety evaluation;
- blocking conditions;
- conflicts;
- selected mission variant;
- optimization profile;
- simulation result;
- operator decision;
- runtime safety event;
- resulting action.

## Authority Hierarchy

The execution authority hierarchy shall be:

MANDATORY SAFETY CONSTRAINTS
>
SAFETY ENGINE
>
MANDATORY SYSTEM CONSTRAINTS
>
MISSION VALIDATION
>
MISSION READINESS
>
OPERATOR APPROVAL REQUIREMENTS
>
MISSION REQUIREMENTS
>
USER PREFERENCES
>
OPTIMIZATION PREFERENCES
>
MISSION AI RECOMMENDATIONS

## Rationale

A dedicated Safety Gate ensures that sophisticated planning, optimization, simulation and AI capabilities can improve mission quality without becoming an uncontrolled path to real-world execution.


