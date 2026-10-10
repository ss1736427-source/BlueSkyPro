---
id: ARCH-DEC-034
type: architecture_decision
title: Human System Interaction Operator Control and UI Authority Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-034 — Human–System Interaction / Operator Control / UI Authority Architecture

## Decision

BlueSky shall separate operator presentation, operator intent, system decisions and execution authority.

The QML / Qt Quick interface shall provide the operator with situational awareness, mission interaction, telemetry, alerts, configuration views and controlled requests.

The C++ Core shall remain authoritative for mission state, validation, readiness, safety and execution boundaries.

QML shall not directly control UAV execution.

## UI Technology

The baseline operator interface shall use:

- Qt 6;
- QML;
- Qt Quick;
- Qt Quick 3D where applicable.

The UI shall communicate with the C++ Core through defined interfaces.

## UI Boundary

The architectural path shall be:

QML
→ C++ Core
→ Safety / Validation
→ Adapter
→ External System.

QML shall not bypass the Core.

## Core Authority

The C++ Core shall own authoritative:

- mission state;
- mission lifecycle;
- validation state;
- readiness state;
- approval state;
- execution state;
- safety state;
- UAV state;
- fleet state;
- communication state;
- configuration state.

## Operator Authority

The operator shall retain authority over actions explicitly assigned to the operator role.

Operator authority shall be governed by:

- authentication;
- authorization;
- role;
- mission state;
- safety state;
- approval state;
- applicable operational mode.

## Intent

The UI shall represent operator actions as explicit intent.

Examples include:

- create mission;
- modify mission;
- select profile;
- request validation;
- request optimization;
- request simulation;
- request approval;
- request execution;
- request abort;
- acknowledge alert;
- select conflict resolution option.

An operator request shall not automatically constitute execution authorization.

## Command Processing

Operator commands shall pass through the C++ Core.

The Core shall:

1. identify the request;
2. verify authority;
3. evaluate current state;
4. apply safety rules;
5. perform validation where required;
6. determine whether the action is permitted;
7. execute or reject the action;
8. record the result.

## UI State

The UI shall reflect authoritative Core state.

The UI shall not maintain an independent authoritative mission state.

Temporary UI state may exist for editing and presentation.

## Mission Builder

The Mission Builder shall support:

- task definition;
- route editing;
- capability selection;
- UAV selection;
- profile selection;
- constraints;
- mission configuration.

Changes shall produce an explicit mission revision.

## Draft Editing

Draft edits shall remain distinguishable from an approved or executing mission.

Editing an approved mission shall create a new revision where required.

## Mission State Visibility

The UI shall clearly display applicable mission states including:

- DRAFT;
- CONFIGURING;
- PLANNED;
- VALIDATING;
- OPTIMIZING;
- SIMULATING;
- READY;
- APPROVAL REQUIRED;
- APPROVED;
- EXECUTING;
- COMPLETED;
- VALIDATION FAILED;
- CONFLICT;
- BLOCKED;
- REQUIRES REPLAN;
- ABORTED.

## Readiness Visibility

Mission Readiness shall be displayed independently from system health and operational mode.

A system being operational shall not imply that the mission is Ready.

## Approval Visibility

Approval state shall be clearly visible.

The UI shall not represent a mission as approved when authoritative approval is absent.

## Execution Visibility

Execution state shall be clearly visible.

The UI shall distinguish:

- requested;
- authorized;
- starting;
- executing;
- paused where supported;
- completed;
- aborted;
- failed.

## Safety Visibility

Safety status shall remain visible during applicable operational workflows.

Safety warnings shall not be hidden behind non-critical UI elements.

## Blocking Conditions

When an action is blocked, the UI shall provide an understandable reason where security and safety policy permit.

Examples include:

- validation failure;
- readiness failure;
- missing approval;
- communication loss;
- UAV unavailable;
- conflict;
- unsafe configuration;
- stale data;
- environment mismatch.

## Operator Confirmation

High-impact actions shall require explicit confirmation where defined by the safety architecture.

Examples may include:

- mission approval;
- execution;
- abort;
- material mission changes;
- operational mode transitions.

Confirmation shall not replace backend authorization.

## No UI-Only Authorization

A button being enabled shall not be treated as proof that an action is authorized.

The C++ Core shall perform authoritative authorization and state checks.

## No Direct UAV Control

QML shall not directly issue UAV execution commands.

UAV commands shall pass through:

QML
→ Core
→ Safety / Validation
→ UAV Adapter
→ UAV.

## No Direct C2 Control

QML shall not directly bypass the Core to communicate with operational C2.

C2 interaction shall use the defined Core / C2 Adapter boundary.

## No Direct Simulation Authority

QML may request Simulation but shall not independently establish simulation or execution authority.

Simulation state shall be controlled by the Core.

## No Direct Safety Override

QML shall not disable or bypass Safety Engine rules.

Safety-critical blocks shall remain authoritative.

## Mission AI Boundary

Mission AI may provide:

- recommendations;
- mission variants;
- optimization results;
- conflict options;
- explanations;
- replanning proposals.

Mission AI shall not directly execute UAV commands.

## AI Recommendations

AI-generated recommendations shall be distinguishable from:

- operator decisions;
- validated mission state;
- approved mission state;
- actual execution state.

## Operator Selection

Where multiple AI or optimization alternatives are presented, the operator may select an authorized option.

Selection shall create an explicit traceable decision.

## Explainability

The UI shall provide an explanation for material automated recommendations where applicable.

The explanation shall identify relevant:

- objective;
- constraints;
- trade-offs;
- risk;
- quality;
- resource implications.

## Conflict Resolution UI

The UI shall display detected conflicts and available resolution options.

The UI shall distinguish:

- detected conflict;
- generated options;
- recommended option;
- operator-selected option;
- validated result.

## Optimization UI

The UI shall display applicable optimization profiles:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

Profile selection shall remain subject to Core validation and safety rules.

## Simulation UI

The UI shall provide access to Simulation results.

Simulation shall remain visibly distinct from ACTUAL execution.

The UI shall support comparison of:

- PLANNED;
- SIMULATED;
- ACTUAL.

## FPV Extension

The architecture shall permit a future optional FPV capability.

FPV shall be implemented through a controlled Media / Video Adapter boundary.

FPV shall not be required for baseline mission execution.

Future Camera, Video, Gimbal and FPV Control capabilities shall not require redesign of the Mission Core.

## FPV Authority

A future FPV interface shall not independently obtain execution authority.

If FPV control is introduced, its command path shall remain subject to:

Core
→ Safety
→ Validation / applicable control rules
→ Adapter
→ External System.

## Telemetry UI

Telemetry presentation shall consume authoritative telemetry and health information from the Core.

The UI shall indicate stale or unavailable telemetry where applicable.

## Map UI

The map shall present:

- route;
- mission graph;
- UAV positions;
- fleet state;
- geofences;
- terrain;
- airspace;
- weather;
- conflicts;
- relevant operational constraints.

Map visualization shall not itself establish safety validity.

## Notifications

The UI shall provide structured notifications for:

- safety;
- validation;
- readiness;
- communication;
- UAV;
- fleet;
- conflict;
- mission;
- system health;
- deployment;
- security.

## Alert Priority

Critical alerts shall have priority over informational notifications.

Safety-critical notifications shall remain visible until acknowledged or otherwise resolved according to policy.

## Acknowledgement

Acknowledgement shall record:

- operator;
- timestamp;
- alert;
- action;
- resulting state.

Acknowledgement shall not automatically resolve the underlying condition.

## Operator Roles

The UI shall adapt available actions according to authorized roles.

Examples may include:

- observer;
- operator;
- mission planner;
- safety authority;
- administrator.

Role separation shall not weaken backend authority checks.

## Least Privilege

The UI shall expose only actions appropriate to the authenticated user's role and current state.

Backend authorization remains authoritative.

## Multi-User Operation

Where multiple users are supported, the system shall maintain clear ownership and authority for material actions.

Conflicting edits shall be detectable.

## Session State

The UI session shall not become the authoritative source of mission state.

After reconnect or restart, the UI shall reconstruct its presentation from authoritative Core state.

## Reconnection

Following communication loss between UI and Core, the UI shall:

- detect the loss;
- indicate stale state;
- prevent unsafe assumptions;
- reconnect where possible;
- resynchronize authoritative state.

## Stale UI State

Stale UI information shall be identified.

The UI shall not present stale mission or execution information as current.

## Offline Behaviour

If offline operation is supported, its capabilities shall be explicitly restricted.

Offline UI state shall not silently become operational authority.

## Error Handling

UI errors shall provide useful operator information without exposing sensitive implementation details unnecessarily.

Backend failures shall remain distinguishable from UI failures.

## Safety-Critical Errors

Safety-critical errors shall be presented prominently.

The UI shall not hide safety errors because of unrelated interface failures.

## Accessibility

The UI shall support clear presentation of critical information, status and alerts.

Safety-critical information shall not rely solely on colour.

## Human Factors

The UI shall minimize ambiguity for high-impact actions.

Material actions shall have clear:

- action name;
- target;
- current state;
- expected effect;
- confirmation requirement.

## Preventing Accidental Actions

The interface shall reduce accidental activation of high-impact operations.

Where appropriate it shall use:

- confirmation;
- deliberate interaction;
- state-dependent enablement;
- contextual warnings.

## State-Dependent UI

UI actions shall be enabled or disabled according to authoritative state.

This is a usability mechanism and shall not replace backend authorization.

## Operator Override

Where operator override is architecturally permitted, it shall be:

- explicitly defined;
- role controlled;
- safety constrained;
- auditable;
- explainable.

No UI mechanism shall create an unrestricted safety override.

## Emergency Interaction

Emergency actions shall follow the Safety Architecture.

Emergency interaction shall prioritize rapid execution of approved emergency behaviour without bypassing safety authority.

## Audit

Material operator actions shall be audited.

Records shall include where applicable:

- operator;
- timestamp;
- action;
- mission;
- previous state;
- resulting state;
- authorization;
- reason.

## Observability

UI actions and important UI-to-Core requests shall integrate with the observability architecture.

The system shall distinguish:

- UI request;
- Core decision;
- execution result.

## Performance

The UI shall remain responsive during:

- telemetry processing;
- mission planning;
- optimization;
- simulation;
- large fleet updates.

Heavy computation shall remain in C++ or appropriate backend services.

## Threading

Long-running or computationally intensive operations shall not block the UI thread.

QML presentation shall remain responsive.

## Data Binding

QML shall consume structured Core data models.

Data binding shall not create a second authoritative source of mission state.

## C++ / QML Interface

The C++ / QML interface shall use controlled APIs and models.

Interfaces shall be versioned and documented where required.

## Interface Security

UI-to-Core interfaces shall respect authentication, authorization and session security.

## Configuration UI

Configuration changes shall use controlled configuration services.

QML shall not directly modify protected configuration files.

## Deployment UI

The UI may display deployment, release and environment state.

Deployment authority shall remain outside the presentation layer.

## Monitoring UI

The UI shall display health and observability information supplied by the Core.

Health indicators shall not be locally fabricated from incomplete UI state.

## Simulation / Sandbox UI

Simulation and Sandbox shall have explicit visual identity.

The UI shall make environment boundaries obvious to the operator.

## Environment Indicator

The current environment shall be continuously identifiable where operationally relevant.

Examples:

- DEVELOPMENT;
- TEST;
- SIMULATION;
- SANDBOX;
- OPERATIONAL.

## No Silent Environment Change

The UI shall not silently transition between environments.

Material environment changes shall be explicit and auditable.

## Release and Configuration Visibility

The UI shall expose applicable:

- release;
- configuration baseline;
- mission revision;
- active profile;
- environment;
- operational mode.

## Data State Visibility

The UI shall distinguish:

- PLANNED;
- SIMULATED;
- ACTUAL.

Historical and simulated information shall not be represented as current actual state.

## Human Approval Boundary

The architecture shall preserve explicit human approval where required.

Automation may prepare or recommend an action, but approval authority remains with the authorized actor.

## Human–AI Boundary

The system shall clearly distinguish:

AI recommendation
→ operator decision
→ Core validation
→ approval
→ execution.

## Human–Safety Boundary

Operator actions remain subject to Safety Engine authority.

The operator shall not receive an unrestricted mechanism for bypassing mandatory safety constraints.

## Human–System Feedback

The UI shall provide feedback for accepted, rejected, blocked and pending requests.

A request shall not be represented as successful before authoritative confirmation.

## Traceability

Human interaction shall support:

OPERATOR
→ INTENT
→ REQUEST
→ AUTHORIZATION
→ VALIDATION
→ DECISION
→ APPROVAL
→ EXECUTION
→ RESULT.

## Verification

UI and operator interaction shall be verified through:

- valid actions;
- invalid actions;
- unauthorized actions;
- stale state;
- reconnect;
- communication loss;
- safety blocks;
- validation failures;
- readiness failures;
- conflict handling;
- AI recommendations;
- simulation;
- sandbox;
- emergency interaction.

## Negative Testing

Verification shall confirm that:

- QML cannot directly command UAV execution;
- UI cannot bypass Safety Engine;
- UI cannot independently establish readiness;
- UI cannot independently establish approval;
- AI recommendations cannot become execution authority;
- stale UI state cannot authorize an action;
- Simulation cannot silently become ACTUAL;
- Sandbox cannot silently become OPERATIONAL.

## Rationale

Separating human interaction from authoritative system execution allows BlueSky to provide a responsive Qt/QML operator experience while preserving C++ Core authority, safety boundaries, explicit human approval and complete traceability.

The architecture also provides a controlled path for future optional capabilities such as FPV without coupling presentation technology to execution authority.


