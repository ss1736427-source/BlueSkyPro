---
id: ARCH-DEC-009
type: architecture_decision
title: Simulation Digital Twin and Sandbox Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-009 — Simulation / Digital Twin / Sandbox Architecture

## Decision

BlueSky shall provide a Simulation / Digital Twin environment that uses the same Mission Architecture as real mission execution.

Simulation shall evaluate mission behaviour before real execution and shall support repeated simulation, evaluation and optimization.

## Mission Execution Abstraction

The same Mission object and Mission Graph shall be usable for:

MISSION
→ REAL EXECUTION

or:

MISSION
→ SIMULATION / DIGITAL TWIN

The simulation environment shall not require a separate mission definition that is incompatible with the operational mission model.

## Simulation Flow

The supported optimization loop shall be:

SIMULATE
→ EVALUATE
→ OPTIMIZE
→ SIMULATE

The loop may be repeated until the applicable mission objectives and constraints are satisfied.

## Digital Twin

The Digital Twin shall represent the relevant operational environment and system state required for mission simulation.

Depending on mission requirements, the simulation shall model:

- route and trajectories;
- UAV state;
- battery and energy;
- C2;
- communication;
- terrain;
- weather;
- DAA;
- AI detection;
- fleet interaction;
- applicable mission capabilities;
- emergency scenarios.

## Mission Graph

Simulation shall execute the Mission Graph rather than treating the mission as a simple sequence of waypoints.

Simulation shall therefore support applicable:

- conditions;
- events;
- branching;
- decisions;
- confirmations;
- adaptive behaviour;
- emergency behaviour.

## Capability Simulation

Simulation shall support the capabilities required by the mission.

Applicable capabilities may include:

- Navigation;
- Perception;
- Detection;
- Tracking;
- Mapping;
- Fleet Coordination;
- Communication Relay;
- Data Fusion;
- Payload Control;
- DAA;
- other mission-specific capabilities.

## Optimization Profiles

Simulation shall support evaluation of mission variants generated under:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

Simulation results may be used by Mission Optimization to generate improved variants.

Optimization shall remain subject to Validation and Safety constraints.

## Validation Integration

Simulation shall integrate with Mission Validation.

The simulation result may be used as an input to:

- Mission Validation;
- Mission Readiness;
- Conflict Resolution;
- Mission Optimization.

Simulation shall not replace these modules.

## Readiness Integration

Simulation results may contribute to Mission Readiness.

A successful simulation shall not by itself authorize real execution.

The execution path remains:

SIMULATION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ APPROVAL
→ EXECUTION

## Conflict Resolution

Simulation shall support evaluation of alternative conflict resolutions.

A candidate resolution may be:

CONFLICT
→ RESOLUTION OPTION
→ SIMULATE
→ EVALUATE
→ REVALIDATE

Simulation may therefore be used to compare the consequences of different resolution options before applying them to the operational mission.

## Emergency Scenarios

The simulation environment shall support controlled modelling of emergency scenarios, including applicable:

- communication degradation or loss;
- battery or energy degradation;
- weather changes;
- environmental changes;
- mission condition changes;
- recovery behaviour;
- emergency termination.

The simulation shall allow system reaction to these conditions to be evaluated without affecting real operations.

## Sandbox

BlueSky shall provide a Sandbox environment for safe experimentation and training.

Sandbox execution shall be isolated from real mission execution.

A Sandbox mission shall not:

- command a real UAV;
- modify a live operational mission;
- affect real flight control;
- bypass Safety Engine;
- bypass Mission Validation;
- bypass Mission Readiness;
- create an unintended execution path to external systems.

## Sandbox User Training

Sandbox shall support execution of virtual missions by users who are learning or testing the system.

Training activities shall remain isolated from operational mission state.

Sandbox results may be stored for analysis and learning without becoming real execution data.

## Mission Data States

The architecture shall distinguish:

- PLANNED;
- SIMULATED;
- ACTUAL.

PLANNED represents the intended mission plan.

SIMULATED represents results produced by Simulation / Digital Twin.

ACTUAL represents observed data from real execution.

These states shall remain distinguishable for analysis and traceability.

## Planned vs Simulated

The system shall support comparison of:

PLANNED
vs
SIMULATED

Comparison may include applicable:

- trajectory;
- timing;
- energy;
- communication;
- mission quality;
- safety conditions;
- other mission metrics.

## Planned vs Simulated vs Actual

When actual execution data is available, the system shall support:

PLANNED
vs
SIMULATED
vs
ACTUAL

This comparison shall support analysis of differences between intended, predicted and observed mission behaviour.

## Re-Simulation

After a material mission change, the system shall support another simulation run.

Previous simulation results shall not automatically be considered valid for a materially changed mission.

The applicable mission revision shall remain traceable to its simulation results.

## Simulation and State Machine

Simulation shall integrate with the Mission State Machine.

Applicable transitions include:

PLANNED
→ SIMULATING

SIMULATING
→ VALIDATING

SIMULATING
→ OPTIMIZING

SIMULATING
→ REQUIRES_REPLAN

SIMULATING
→ READY

The exact transition shall depend on simulation results and applicable validation requirements.

## Simulation Isolation

Simulation shall use adapters and interfaces defined by the Core architecture.

Simulation shall not create a direct execution path from the simulation environment to real UAV control.

The operational boundary remains:

QML
→ C++ CORE
→ VALIDATION / SAFETY
→ ADAPTER
→ EXTERNAL SYSTEM

Simulation remains a separate execution environment behind the appropriate adapter boundary.

## Traceability

Simulation shall preserve traceability between:

MISSION REVISION
→ SIMULATION RUN
→ SIMULATION INPUTS
→ SIMULATION CONDITIONS
→ SIMULATION RESULTS
→ EVALUATION
→ OPTIMIZATION
→ RESULTING MISSION REVISION

## Rationale

Using the same Mission Architecture for simulation and real execution reduces divergence between planned and simulated behaviour while preserving a strict separation between virtual and real execution.

Sandbox isolation additionally provides a controlled environment for training, experimentation and validation without creating an unintended path to real flight operations.


