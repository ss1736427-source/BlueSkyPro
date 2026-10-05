---
id: ARCH-DEC-036
type: architecture_decision
title: Resource Management Scheduling and Workload Coordination Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-036 — Resource Management / Scheduling / Workload Coordination Architecture

## Decision

BlueSky shall provide centralized resource management and workload coordination for mission execution, fleet operations, simulation, Mission AI and system services.

The architecture shall allocate resources according to mission requirements, capability constraints, safety constraints, priorities and current system state.

Resource management shall not override Safety Engine authority.

## Resource Authority

The C++ Core shall own authoritative runtime resource state and allocation decisions.

QML shall display resource state and may submit authorized requests.

Mission AI may propose allocations or alternatives but shall not independently commit operational resource allocation.

## Resource Domains

The architecture shall manage applicable:

- UAV resources;
- fleet resources;
- CPU;
- GPU;
- memory;
- storage;
- network;
- communication channels;
- telemetry capacity;
- payload resources;
- mission resources;
- simulation resources;
- external service capacity.

## Resource Identity

Resources shall have stable identities.

Resource records shall identify:

- resource ID;
- type;
- owner;
- capability;
- state;
- capacity;
- allocation;
- availability.

## Resource State

Resources shall support states including:

- AVAILABLE;
- RESERVED;
- ALLOCATED;
- ACTIVE;
- DEGRADED;
- CONSTRAINED;
- UNAVAILABLE;
- FAILED;
- RECOVERING.

## Capacity

Each managed resource shall expose applicable capacity information.

Capacity shall distinguish:

- total;
- available;
- reserved;
- allocated;
- consumed;
- remaining.

## Resource Constraints

Allocation shall respect:

- safety constraints;
- mission constraints;
- capability requirements;
- energy limits;
- communication limits;
- environmental constraints;
- hardware limits;
- configuration;
- operational mode.

## Mission Resource Requirements

A mission shall identify required resources where applicable.

Requirements may include:

- UAV count;
- UAV capabilities;
- payload;
- energy;
- communication;
- compute;
- storage;
- external services.

## Capability Matching

Resource allocation shall match mission requirements to actual capability availability.

Configured capability shall not be treated as available capability without verification.

## UAV Allocation

UAV allocation shall consider:

- capability;
- health;
- battery;
- position;
- mission state;
- communication;
- availability;
- current workload.

## Fleet Allocation

Fleet resource allocation shall preserve awareness of individual UAV state.

Fleet-level allocation shall not conceal a safety-critical condition affecting an individual UAV.

## Dynamic Allocation

Resources may be dynamically reallocated when mission or system conditions change.

Dynamic allocation shall remain subject to safety and validation rules.

## Resource Reservation

Resources may be reserved before execution.

Reservations shall have:

- owner;
- mission;
- resource;
- scope;
- validity;
- priority;
- expiration where applicable.

## Reservation Conflicts

Conflicting reservations shall be detected.

The system shall not silently overwrite an existing authoritative reservation.

Conflicts shall be passed to the appropriate Conflict Resolution mechanism.

## Scheduling

The system shall provide controlled scheduling of workloads and resource usage.

Scheduling shall consider:

- priority;
- deadlines;
- dependencies;
- resource availability;
- mission state;
- safety;
- operational mode.

## Priority

Workloads shall have explicit priority where required.

Safety-critical workloads shall take precedence over non-critical workloads.

## Priority Inversion

The scheduling architecture shall identify and mitigate priority inversion where it can affect safety-critical execution.

## Preemption

Preemption shall be controlled.

Safety-critical operations shall not be interrupted in a manner that creates unsafe execution.

## Cancellation

Authorized workloads shall support cancellation where technically safe.

Cancellation shall produce an observable state transition.

## Workload Identity

Each managed workload shall have an identifiable:

- workload ID;
- owner;
- mission;
- priority;
- resource requirements;
- state.

## Workload States

Workloads may transition through:

QUEUED
→ READY
→ RUNNING
→ COMPLETED

or:

QUEUED
→ CANCELLED

and:

RUNNING
→ FAILED
→ RECOVERING.

## Dependencies

Workloads shall declare required dependencies where applicable.

A workload shall not execute when mandatory dependencies are unavailable.

## Dependency Ordering

The scheduler shall respect workload dependency ordering.

Dependent work shall not start before required prerequisites are complete.

## Deadlines

Where applicable, workloads shall define timing requirements.

Missed or threatened deadlines shall become observable conditions.

## Real-Time Work

Real-time workloads shall have explicit timing requirements.

The scheduler shall protect timing-sensitive operations from non-critical workload interference.

## Core Protection

Resource scheduling shall protect the C++ Core from uncontrolled workload consumption.

## Safety Engine Protection

Safety Engine resources shall have sufficient protection to maintain safety-critical operation under load.

## Validation Protection

Mission Validation shall remain available for required mission state transitions.

## Readiness Protection

Mission Readiness processing shall not be starved by lower-priority workloads.

## Telemetry Processing

Telemetry processing shall receive resources appropriate to its operational priority.

Telemetry overload shall produce an observable condition.

## Communication Resources

Communication resource allocation shall consider:

- bandwidth;
- latency;
- availability;
- priority;
- fleet requirements.

Safety-relevant communication shall receive appropriate priority.

## Network Scheduling

Network workloads may be prioritized according to operational importance.

Resource exhaustion shall not silently degrade critical communication.

## Compute Resources

CPU and GPU resources shall be managed according to workload requirements.

Heavy workloads such as optimization and simulation shall not compromise safety-critical execution.

## GPU

GPU-intensive workloads shall have controlled resource allocation.

Mission AI and simulation shall not consume GPU resources required by higher-priority workloads without explicit policy.

## Memory

Memory consumption shall be monitored.

The system shall detect:

- elevated usage;
- constrained memory;
- exhaustion;
- allocation failure.

## Storage

Storage allocation shall account for:

- mission data;
- telemetry;
- logs;
- simulation;
- audit;
- temporary workloads.

Storage exhaustion shall be observable.

## Data Processing

Data processing workloads shall have controlled resource requirements.

Backpressure shall prevent uncontrolled resource consumption.

## Simulation Resources

Simulation workloads shall be isolated and scheduled according to available resources.

Simulation shall not compromise operational execution resources.

## Sandbox Resources

Sandbox workloads shall remain isolated from operational resources according to deployment policy.

## Mission AI Resources

Mission AI shall operate within defined resource limits.

AI resource usage shall not compromise Safety Engine, Validation or Core execution.

## Optimization Resources

Optimization workloads shall have defined resource budgets.

Optimization may be deferred, degraded or cancelled when required to preserve higher-priority execution.

## Route Planning Resources

Route planning shall receive sufficient resources for required mission planning and runtime replanning.

## Conflict Resolution Resources

Conflict Resolution shall remain available when required to resolve operational conflicts.

## Resource Loss

Loss of a required resource shall produce an explicit state change.

The system shall determine affected missions and workloads.

## Degraded Operation

When resources become constrained, the system may enter degraded operation according to defined policy.

Degradation shall remain visible to the operator.

## Resource Reallocation

Resource reallocation shall consider:

- mission priority;
- safety;
- deadlines;
- resource criticality;
- capability requirements;
- current state.

## Safety Priority

Safety shall have priority over:

- optimization;
- performance;
- economic efficiency;
- quality improvements;
- non-critical workloads.

## Optimization Priority

Optimization shall operate within resources remaining after safety-critical requirements are satisfied.

## Operational Profiles

Resource allocation may consider:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

The selected profile shall not override mandatory safety constraints.

## Profile Resource Effects

Profiles may influence:

- compute allocation;
- route calculation effort;
- simulation depth;
- optimization effort;
- quality targets;
- execution time.

Actual profile behaviour shall be defined by technical requirements and algorithms.

## Automatic Scheduling

Automatic scheduling shall remain within defined authority boundaries.

Automatic scheduling shall not authorize prohibited operational actions.

## Operator Control

Authorized operators may influence scheduling or resource allocation where permitted.

Operator requests shall pass through Core authorization and safety checks.

## Mission AI Scheduling

Mission AI may propose:

- workload ordering;
- resource reallocation;
- mission alternatives;
- scheduling changes.

The Core shall validate and authorize applicable changes.

## Fleet Coordination

Fleet Coordination shall consume current resource state.

Resource allocation shall remain synchronized with fleet state.

## Runtime Changes

Material runtime resource changes shall be observable.

Changes affecting mission validity shall trigger applicable revalidation.

## Mission Replanning

Resource loss or significant resource degradation may trigger replanning.

Replanning shall remain subject to:

- Safety Engine;
- Validation;
- Readiness;
- approval requirements.

## Conflict Integration

Resource conflicts shall integrate with Conflict Resolution.

The system shall preserve:

- conflict source;
- affected resources;
- affected missions;
- options;
- selected resolution;
- resulting state.

## Resource Accounting

The system shall maintain accounting of resource allocation and consumption where required.

Accounting shall distinguish:

- requested;
- reserved;
- allocated;
- consumed;
- released.

## Reservation Expiration

Expired reservations shall be released according to defined policy.

Material automatic releases shall be observable.

## Resource Leakage

The system shall detect resources that remain allocated after workload completion or cancellation.

## Recovery

Recovered resources shall not automatically become AVAILABLE until applicable health and integrity checks succeed.

## Failure Handling

Resource allocation failure shall produce an explicit result.

The system shall not report successful allocation when allocation was incomplete.

## Graceful Degradation

When capacity is insufficient, the system may:

- defer non-critical work;
- reduce workload;
- reduce optimization effort;
- reduce simulation fidelity;
- release non-essential reservations;
- enter degraded operation.

Safety-critical requirements shall remain protected.

## Overload

Overload conditions shall be detected before possible loss of safety-critical capacity where feasible.

## Admission Control

The system may reject or defer new workloads when required resources are unavailable.

Admission decisions shall be observable.

## Queue Management

Queues shall have defined:

- capacity;
- priority;
- ordering;
- timeout;
- overflow behaviour.

## Backpressure

Backpressure shall protect system resources from excessive incoming work.

## Monitoring

Resource state and scheduler behaviour shall integrate with the observability architecture.

Metrics shall include where applicable:

- utilization;
- queue depth;
- wait time;
- execution time;
- allocation failures;
- resource exhaustion.

## Health Integration

Resource health shall integrate with ARCH-DEC-032.

Resource degradation shall be reflected in applicable health state.

## Logging Integration

Material allocation, scheduling, failure and recovery events shall integrate with ARCH-DEC-033.

## Configuration Integration

Scheduling policies and resource limits shall be versioned configuration.

Material changes shall follow configuration control.

## Deployment Integration

Deployment profiles shall define applicable resource budgets and service requirements.

## Environment Separation

Resource pools for:

- DEVELOPMENT;
- TEST;
- SIMULATION;
- SANDBOX;
- OPERATIONAL

shall remain appropriately separated.

## Multi-Mission Operation

Where multiple missions are active, resource allocation shall preserve mission isolation and priority rules.

One mission shall not silently consume resources reserved for another mission.

## Multi-UAV Operation

Fleet workloads shall account for individual UAV and fleet-wide resource constraints.

## External Resources

External service capacity shall be treated as a managed dependency.

Unavailable external capacity shall not be silently assumed to exist.

## Resource Forecasting

The system may estimate future resource demand for:

- mission planning;
- simulation;
- optimization;
- fleet coordination;
- deployment.

Forecasts shall remain distinguishable from measured actual usage.

## Predictive Allocation

Predictive resource allocation may be used where supported.

Predictions shall not override authoritative runtime resource state.

## Data States

Resource planning shall distinguish:

- planned;
- simulated;
- actual.

## Simulation Resource Model

Simulation may model resource constraints to evaluate mission feasibility.

Simulated resource availability shall not imply actual resource availability.

## Actual Resource State

Actual resource state shall come from authoritative runtime measurements and subsystem state.

## Resource Comparison

The system shall support comparison between:

PLANNED
→ SIMULATED
→ ACTUAL

resource usage where applicable.

## Audit

Material resource allocation and scheduling decisions shall be auditable.

## Traceability

Resource traceability shall support:

RESOURCE
→ REQUIREMENT
→ RESERVATION
→ ALLOCATION
→ WORKLOAD
→ CONSUMPTION
→ RELEASE
→ RESULT.

## Verification

The architecture shall be verified through:

- normal load;
- high load;
- resource exhaustion;
- workload cancellation;
- workload failure;
- dependency loss;
- UAV loss;
- communication degradation;
- competing missions;
- priority conflicts;
- reservation conflicts;
- scheduler recovery;
- simulation load;
- Mission AI load.

## Negative Testing

Verification shall confirm that:

- safety-critical workloads cannot be starved by non-critical workloads;
- feature optimization cannot consume protected safety resources;
- failed allocations are not reported as successful;
- resources are not silently double-allocated;
- simulation cannot consume operational resources outside policy;
- Mission AI cannot independently authorize resource allocation;
- stale resource state cannot be treated as current;
- recovered resources are not marked available before validation.

## Performance

Scheduling overhead shall remain bounded and shall not compromise Core responsiveness.

## Scalability

The architecture shall scale with:

- UAV count;
- fleet size;
- mission count;
- workload count;
- telemetry volume;
- simulation workloads;
- operator count.

## Security

Resource management operations shall follow authentication, authorization and least-privilege requirements.

## Safety Boundary

Resource allocation shall remain subordinate to Safety Engine authority.

No scheduler action shall bypass mandatory safety constraints.

## Operator Visibility

The UI shall provide visibility into:

- resource state;
- allocation;
- reservations;
- workload state;
- constraints;
- overload;
- degradation;
- resource conflicts.

## Explainability

Material automatic resource allocation decisions should provide an explanation where applicable.

The explanation may include:

- requirement;
- priority;
- constraint;
- selected resource;
- rejected alternatives;
- resulting impact.

## Rationale

Centralized resource management prevents uncontrolled competition between mission, fleet, simulation, AI and system workloads.

It provides predictable scheduling, protects safety-critical services, supports dynamic adaptation and enables traceable resource allocation across planned, simulated and actual operation.


