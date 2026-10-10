---
id: ARCH-DEC-030
type: architecture_decision
title: Operational Profiles and Mode Management Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-030 — Operational Profiles / Mode Management Architecture

## Decision

BlueSky shall use controlled operational profiles and explicit system modes to determine which capabilities, policies and execution behaviours are available.

Mode changes shall be explicit, traceable and subject to applicable authority and safety constraints.

## Mode Authority

The C++ Core shall own authoritative system mode state.

QML shall display and request mode changes through the Core.

QML shall not independently establish an operational mode.

Mission AI shall not independently change an operational mode.

## Operational Profiles

The architecture shall support controlled profiles including:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

Profiles shall influence planning and optimization behaviour without bypassing Safety Engine requirements.

## Profile Identity

Each selected profile shall have an identifiable profile identity and version where applicable.

The active profile shall be associated with the applicable mission revision.

## Profile Selection

Profile selection may be:

- operator-selected;
- task-derived;
- automatically selected according to approved rules.

Automatic selection shall remain explainable and traceable.

## Profile Constraints

Profiles shall define applicable preferences for:

- execution time;
- resource consumption;
- risk;
- quality;
- optimization objectives.

Safety constraints shall have higher authority than profile preferences.

## FAST

FAST shall prioritize appropriate reduction of execution time while remaining within mandatory safety, validation and readiness constraints.

FAST shall not authorize unsafe shortcuts.

## SAFE

SAFE shall prioritize risk reduction within the applicable mission constraints.

Safety Engine rules remain authoritative.

## ECONOMIC

ECONOMIC shall prioritize resource efficiency while preserving required safety and mission objectives.

## HIGH QUALITY

HIGH QUALITY shall prioritize mission result quality and applicable sensing or processing objectives while preserving safety constraints.

## Profile Comparison

The system shall support comparison of profile outcomes where applicable.

Comparison may include:

- execution time;
- resources;
- risk;
- quality;
- route;
- UAV allocation.

## Profile and Mission Variants

Different profiles may generate different mission variants.

Each variant shall remain traceable to:

- mission;
- mission revision;
- profile;
- configuration;
- optimization result.

## Profile and Optimization

Optimization shall use the active profile as an optimization preference.

Optimization shall not override mandatory:

- Safety Engine rules;
- Validation;
- Readiness;
- approval;
- execution authority.

## Profile and Validation

Changing a material profile shall trigger applicable validation.

Validation shall evaluate the resulting mission rather than assuming that a profile change is harmless.

## Profile and Readiness

A material profile change shall trigger applicable readiness reassessment.

Previous readiness shall not automatically remain valid.

## Profile and Approval

Where profile selection materially changes an approved mission, applicable approval shall be reconsidered.

## Profile and Simulation

Simulation shall preserve the selected profile.

Simulation results shall identify:

- profile;
- profile version;
- mission revision;
- configuration;
- environment.

## Profile and Execution

The execution path shall use the approved profile associated with the approved mission revision.

A material mismatch shall be detected and shall prevent unsafe execution.

## Mode Model

The system shall distinguish operational modes such as:

- DEVELOPMENT;
- TEST;
- SIMULATION;
- SANDBOX;
- OPERATIONAL.

These modes shall remain explicitly identifiable.

## Development Mode

Development mode shall support engineering activities.

Development mode shall not silently receive operational execution authority.

## Test Mode

Test mode shall support verification and integration testing.

Test mode shall remain distinguishable from operational execution.

## Simulation Mode

Simulation mode shall support virtual mission execution.

Simulation shall not directly command real UAV execution.

## Sandbox Mode

Sandbox mode shall support isolated experimentation and training.

Sandbox shall not affect real operational missions.

## Operational Mode

Operational mode shall use an accepted release and configuration baseline.

Operational execution shall remain subject to validation, readiness, approval and Safety Engine controls.

## Mode Identity

The active mode shall be visible and programmatically identifiable.

The system shall expose sufficient information to determine:

- current mode;
- mode version where applicable;
- environment;
- release;
- configuration baseline.

## Mode Transition

Mode transitions shall use a controlled state machine.

Applicable transitions shall be explicitly defined.

## Mode Transition Validation

Before entering a higher-authority mode, the system shall perform applicable validation.

For operational entry, applicable:

- startup validation;
- configuration validation;
- security checks;
- safety checks;
- service health checks;

shall be satisfied.

## Mode Transition Authority

Mode transition authority shall be controlled.

Development or test users shall not automatically receive operational mode authority.

## Startup

At startup the system shall determine:

- deployment mode;
- configuration;
- release;
- service health;
- applicable dependencies.

The system shall not report operational readiness before required startup checks complete.

## Runtime Mode Change

Runtime mode changes shall be restricted.

A mode change affecting operational authority shall require controlled transition and applicable operator or system authorization.

## No Silent Mode Change

The system shall not silently change from:

- SIMULATION to OPERATIONAL;
- SANDBOX to OPERATIONAL;
- TEST to OPERATIONAL.

Any such transition shall be explicit and auditable.

## Simulation Isolation

Simulation mode shall isolate:

- mission commands;
- UAV execution;
- operational C2;
- operational payload control.

Simulation data shall remain identified as SIMULATED.

## Sandbox Isolation

Sandbox mode shall prevent direct operational execution.

Sandbox users shall operate on isolated or simulated resources.

## Environment Separation

Development, Test, Simulation, Sandbox and Operational environments shall use controlled environment boundaries.

Operational endpoints shall not be silently reused by non-operational modes.

## Configuration Binding

Each mode shall use a controlled configuration set.

Configuration shall be versioned according to ARCH-DEC-019 and ARCH-DEC-026.

## Feature Availability

Capabilities may vary by mode.

The system shall clearly identify unavailable capabilities.

Unavailable capabilities shall not be represented as operationally available.

## Safety Availability

Safety-critical controls shall remain available in all modes where applicable.

Simulation and Sandbox may use simulated safety services but shall preserve the architectural safety boundary.

## Validation Availability

Mission Validation shall remain available for applicable mission workflows.

Simulation shall not be used to bypass validation.

## Readiness Availability

Mission Readiness shall remain distinct from system mode.

Being in Operational mode shall not by itself mean that a mission is Ready.

## Approval Availability

Operational execution shall require the applicable mission approval independently of the current system mode.

## Execution Authority

Execution authority shall remain separate from:

- UI mode selection;
- profile selection;
- Mission AI recommendation;
- simulation state.

## Monitoring

Mode and profile state shall be visible to ARCH-DEC-024 monitoring.

The system shall detect unexpected or inconsistent mode state.

## Health

Mode availability shall depend on required system health.

Critical service failure may force a controlled degraded or blocked state.

## Failure Handling

Mode failures shall produce an explicit state.

The system shall avoid ambiguous combinations such as:

- Operational + unhealthy;
- Approved + wrong baseline;
- Simulation + operational endpoint;
- Sandbox + execution authority.

## Degraded Mode

Where safe, the system may enter a controlled degraded state.

Degraded operation shall identify unavailable capabilities and restrictions.

## Emergency Mode

Emergency handling shall remain governed by the Safety Architecture.

Emergency behaviour shall not be implemented merely as a UI mode.

## Recovery

After restart or recovery, the system shall reconstruct the authoritative mode.

Recovery shall not automatically restore operational execution authority.

## Persistence

Mode state shall be persisted where required for recovery and audit.

Transient runtime state shall remain distinguishable from persistent operational state.

## Audit

Material mode and profile changes shall be auditable.

Audit shall identify:

- previous state;
- new state;
- actor;
- timestamp;
- reason;
- authorization;
- result.

## Operator Visibility

The operator shall clearly see:

- active mode;
- active profile;
- mission revision;
- release;
- configuration baseline;
- readiness;
- approval;
- execution state.

## Explainability

Automatic profile or mode decisions shall provide an explanation sufficient for operator review.

## Mission AI Boundary

Mission AI may recommend a profile or mode change where authorized.

Mission AI shall not directly establish operational execution authority.

## Conflict Handling

A conflict between profile preference and safety constraints shall resolve in favour of safety.

A conflict between mode requirements and operational readiness shall block the unsafe transition.

## Revalidation

Material mode or profile changes shall trigger applicable revalidation.

## Replanning

If a profile change materially affects mission planning, the system shall support replanning.

## Simulation Loop

The architecture shall support:

PLAN
→ SIMULATE
→ EVALUATE
→ OPTIMIZE
→ SIMULATE
→ VALIDATE
→ READY
→ APPROVE
→ EXECUTE.

Profile identity shall remain preserved through the loop.

## Data States

Mode and profile information shall remain associated with:

- PLANNED;
- SIMULATED;
- ACTUAL.

This supports comparison and reproducibility.

## Security

Mode transitions shall follow ARCH-DEC-018.

Operational mode authority shall not be granted through ordinary UI permissions alone.

## Configuration Change

Changes to profile definitions or mode rules shall follow ARCH-DEC-026.

Material changes shall receive applicable verification and validation.

## Verification

Mode management shall be verified according to ARCH-DEC-025.

Verification shall include:

- valid transitions;
- invalid transitions;
- unauthorized transitions;
- profile selection;
- profile switching;
- simulation isolation;
- sandbox isolation;
- recovery;
- failure handling.

## Performance

Mode and profile management shall not compromise safety-critical runtime processing.

## Traceability

Mode/profile traceability shall support:

PROFILE / MODE
→ MISSION
→ CONFIGURATION
→ VALIDATION
→ READINESS
→ APPROVAL
→ EXECUTION
→ RESULT.

## Rationale

Explicit mode and profile management prevents accidental crossing between development, simulation, sandbox and operational environments while allowing optimization preferences to influence mission planning without bypassing safety, validation, readiness or execution authority.


