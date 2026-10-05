---
id: ARCH-DEC-037
type: architecture_decision
title: Resource Reservation Mission Scheduling and Temporal Coordination Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-037 — Resource Reservation / Mission Scheduling / Temporal Coordination Architecture

## Decision

BlueSky shall provide a controlled temporal coordination architecture for mission scheduling, resource reservation, execution windows, dependencies and time-based constraints.

Scheduling shall operate through the C++ Core and shall remain subordinate to Safety Engine, Validation, Readiness, Approval and Execution Authority.

## Scheduling Authority

The C++ Core shall own authoritative scheduling state.

QML may create or modify scheduling requests through controlled interfaces.

Mission AI may propose schedules and temporal alternatives but shall not independently authorize execution.

## Mission Schedule

A mission schedule shall identify applicable:

- mission;
- revision;
- execution window;
- tasks;
- dependencies;
- resources;
- UAV assignments;
- constraints;
- environment;
- configuration;
- approval state.

## Schedule Identity

Each authoritative schedule shall have:

- schedule ID;
- mission ID;
- mission revision;
- version;
- creation timestamp;
- effective period;
- author or source;
- status.

## Schedule States

Schedules may transition through:

DRAFT
→ VALIDATING
→ READY
→ APPROVAL REQUIRED
→ APPROVED
→ ACTIVE
→ COMPLETED

or:

DRAFT
→ VALIDATION FAILED

and:

ACTIVE
→ INTERRUPTED
→ REPLANNING
→ APPROVED
→ ACTIVE

where permitted.

## Temporal Constraints

Scheduling shall support:

- start time;
- end time;
- duration;
- deadline;
- earliest start;
- latest start;
- execution window;
- hold time;
- timeout;
- synchronization point.

## Time Authority

Authoritative system time shall be used for operational scheduling.

Time synchronization state shall be observable.

Unsynchronized or unreliable time shall be treated as an operational constraint where required.

## Time Uncertainty

Where time accuracy is insufficient for a mission requirement, the affected schedule shall be identified as invalid, degraded or requiring reassessment according to policy.

## Mission Dependencies

Tasks may depend on completion or state of other tasks.

The scheduler shall enforce dependency ordering.

## Temporal Dependency

A task shall not start before all mandatory temporal and logical prerequisites are satisfied.

## Resource Dependency

A task shall not become executable when required resources are unavailable.

## UAV Availability

Scheduling shall consider actual UAV:

- availability;
- health;
- battery;
- location;
- mission state;
- communication;
- capability.

## Fleet Scheduling

Fleet schedules shall coordinate individual UAV schedules while preserving individual safety constraints.

## Reservation

Resources may be reserved for future mission execution.

Reservations shall identify:

- resource;
- mission;
- schedule;
- reservation owner;
- validity period;
- priority;
- status.

## Reservation Lifecycle

Reservations may transition through:

REQUESTED
→ VALIDATING
→ RESERVED
→ ALLOCATED
→ RELEASED.

Failed reservations shall not become authoritative.

## Reservation Conflict

Overlapping incompatible reservations shall be detected.

The scheduler shall not silently replace or overwrite an authoritative reservation.

## Conflict Resolution

Reservation conflicts shall integrate with Conflict Resolution.

The system shall preserve:

- conflicting resources;
- missions;
- schedules;
- time windows;
- alternatives;
- selected resolution;
- resulting state.

## Priority

Scheduling priority shall consider:

- safety;
- emergency;
- mission criticality;
- deadline;
- operator-defined priority;
- resource constraints.

Safety requirements shall have highest authority.

## Emergency Scheduling

Emergency schedules shall follow the Safety Architecture.

Emergency handling shall not create unrestricted scheduling authority.

## Deadline Management

The system shall monitor deadlines.

A threatened or missed deadline shall produce an observable condition.

## Deadline Failure

A deadline failure shall not be silently ignored.

The system shall determine whether to:

- continue;
- replan;
- defer;
- abort;
- require operator intervention.

## Execution Window

Execution shall occur only within an applicable authorized execution window.

The scheduler shall not convert a planned window into execution authorization.

## Approval Binding

Approval shall bind to the specific mission revision and applicable schedule where required.

Material schedule changes after approval shall trigger applicable revalidation and approval.

## Schedule Change

Changes may affect:

- route;
- task ordering;
- UAV assignment;
- resource allocation;
- execution window;
- safety;
- communication;
- environment.

Material changes shall trigger impact analysis.

## Revalidation

A material scheduling change shall trigger revalidation.

Readiness shall be reassessed where applicable.

## Replanning

Scheduling conditions may trigger replanning due to:

- resource loss;
- weather;
- airspace;
- communication degradation;
- UAV failure;
- conflict;
- deadline changes;
- environmental changes.

## Runtime Scheduling

Runtime schedule adaptation may occur where explicitly permitted.

Runtime adaptation shall remain subject to Safety Engine and applicable validation rules.

## No Silent Schedule Change

Material schedule changes shall not occur silently.

The system shall record:

- previous schedule;
- new schedule;
- trigger;
- actor or automatic source;
- validation;
- approval;
- result.

## Operator Scheduling

Authorized operators may:

- create schedules;
- modify draft schedules;
- request replanning;
- select authorized alternatives;
- cancel or defer work where permitted.

Operator requests shall pass through Core authority.

## Mission AI Scheduling

Mission AI may propose:

- task ordering;
- execution windows;
- resource reservations;
- schedule alternatives;
- replanning options.

Mission AI shall not independently activate operational schedules.

## Explainability

Automatic scheduling decisions shall provide an explanation where applicable.

The explanation may include:

- constraints;
- priorities;
- resource availability;
- deadlines;
- rejected alternatives;
- selected schedule.

## Optimization

Scheduling may use optimization according to configured profiles:

- FAST;
- SAFE;
- ECONOMIC;
- HIGH QUALITY.

Optimization shall remain subordinate to safety constraints.

## Temporal Optimization

Optimization may consider:

- total execution time;
- resource utilization;
- energy;
- risk;
- quality;
- deadlines.

Optimization shall not violate mandatory temporal or safety constraints.

## Simulation Scheduling

Schedules may be evaluated in Simulation.

Simulation shall preserve the distinction between:

- planned schedule;
- simulated schedule;
- actual schedule.

## Simulation Time

Simulation may use simulated time.

Simulated time shall never be interpreted as actual operational time.

## Digital Twin

Digital Twin scheduling shall use the same mission architecture as operational execution while remaining isolated from actual execution authority.

## Planned / Simulated / Actual

The architecture shall preserve:

PLANNED
→ SIMULATED
→ ACTUAL

as distinct states.

## Schedule Comparison

The system shall support comparison between planned, simulated and actual timing.

Differences may include:

- start time;
- task duration;
- delay;
- resource usage;
- completion;
- deviation.

## Temporal Traceability

Each material schedule event shall be traceable to its applicable mission revision and configuration.

## Scheduling Events

The system shall record:

- schedule created;
- schedule changed;
- reservation created;
- reservation conflict;
- schedule approved;
- schedule activated;
- task started;
- task delayed;
- task completed;
- schedule interrupted;
- schedule replanned;
- schedule cancelled;
- schedule completed.

## Event Ordering

Scheduling events shall preserve ordering sufficient for mission reconstruction.

Out-of-order events shall be detectable.

## Duplicate Events

Duplicate scheduling events shall not create duplicate authoritative state transitions.

## Scheduler Failure

Scheduler failure shall produce an explicit health condition.

The system shall not silently assume that scheduled work remains valid.

## Scheduling Recovery

After scheduler recovery, the system shall reconcile:

- active schedules;
- reservations;
- task state;
- resource state;
- mission state.

Recovered state shall be revalidated where required.

## Resource Release

Completed, cancelled or expired schedules shall release reservations according to policy.

## Reservation Expiration

Expired reservations shall become identifiable and shall not remain silently authoritative.

## Resource Protection

The scheduler shall protect resources reserved for safety-critical operations.

## Multi-Mission Scheduling

Multiple missions shall be scheduled with explicit resource and priority isolation.

One mission shall not silently consume resources reserved for another.

## Fleet Coordination

Fleet schedules shall integrate with Fleet Coordination and Resource Allocation.

Changes to one UAV may affect the schedule of other UAVs.

## Communication Constraints

Scheduling shall consider communication availability and quality where communication is required for the task.

## C2 Constraints

C2 state shall be considered for tasks requiring operational command or telemetry links.

## Environmental Constraints

Scheduling shall consider applicable:

- weather;
- terrain;
- airspace;
- geofence;
- environmental windows.

Changes in environmental conditions shall trigger applicable reassessment.

## Energy Constraints

Scheduling shall consider:

- battery;
- energy reserve;
- charging;
- payload consumption;
- mission duration.

Energy constraints shall remain subordinate to safety.

## Payload Constraints

Tasks requiring payload capabilities shall only be scheduled to compatible UAVs.

## Map and Spatial Constraints

Scheduling shall integrate with spatial validation.

Temporal and spatial constraints shall be evaluated together where required.

## Four-Dimensional Mission Model

Where applicable, mission scheduling shall operate on:

X
Y
Z
TIME

constraints.

Spatial validity alone shall not establish temporal validity.

## Airspace Windows

Time-dependent airspace restrictions shall be represented as scheduling constraints.

## Geofence Windows

Time-dependent geofence restrictions shall be represented where applicable.

## Weather Windows

Weather forecasts and observations may define temporal mission constraints.

Forecast uncertainty shall remain distinguishable from actual conditions.

## Data Freshness

Time-sensitive scheduling data shall have observable freshness.

Stale external data shall not silently be treated as current.

## External Scheduling Dependencies

External systems may provide:

- time windows;
- resource availability;
- airspace restrictions;
- weather;
- operational constraints.

External information shall pass through defined adapters and validation.

## No Silent External Override

External systems shall not silently modify an authoritative schedule.

Changes shall pass through defined integration boundaries.

## Configuration Binding

Scheduling policies shall be versioned configuration.

Material policy changes shall follow configuration control.

## Deployment Binding

Scheduling capabilities shall depend on deployment profile and environment.

## Environment Separation

Schedules in:

- DEVELOPMENT;
- TEST;
- SIMULATION;
- SANDBOX;
- OPERATIONAL

shall remain appropriately isolated.

## Simulation Isolation

Simulation schedules shall not create actual reservations or execution.

## Sandbox Isolation

Sandbox schedules shall not create operational resource allocations.

## Operational Mode

Scheduling behaviour shall respect the active operational mode.

Mode changes shall trigger applicable schedule reassessment.

## Pause

Where mission architecture permits pausing, the scheduler shall distinguish:

- requested pause;
- authorized pause;
- paused;
- resume pending;
- resumed.

## Resume

Resume shall require verification of applicable:

- mission state;
- resource state;
- safety;
- validation;
- readiness;
- environmental conditions.

No unsafe automatic resume shall occur.

## Cancellation

Authorized cancellation shall produce an explicit state transition.

Cancellation shall release resources according to policy.

## Abort

Abort shall follow the Safety Architecture and shall not be treated as ordinary cancellation.

## Failure Recovery

A failed task may enter recovery according to its defined recovery policy.

Recovery shall not silently continue unsafe execution.

## Workload Integration

Scheduling shall integrate with ARCH-DEC-036 resource and workload management.

The temporal scheduler determines when work may proceed; resource management determines whether required resources are available.

## Health Integration

Scheduling health shall integrate with ARCH-DEC-032.

Scheduler degradation shall be visible.

## Observability Integration

Scheduling events, decisions and timing metrics shall integrate with ARCH-DEC-033.

## Configuration Integration

Schedule configuration shall integrate with ARCH-DEC-035.

## UI Integration

QML shall display schedule state, timing, reservations, conflicts and changes through Core-provided state.

QML shall not become scheduling authority.

## Audit

Material schedule and reservation decisions shall be auditable.

Audit shall include applicable:

- operator;
- source;
- mission;
- schedule;
- previous state;
- resulting state;
- timestamp;
- authorization.

## Performance

Scheduling shall support required mission timing without unacceptable processing overhead.

## Scalability

The architecture shall scale with:

- mission count;
- UAV count;
- fleet size;
- task count;
- reservation count;
- external constraints.

## Verification

Scheduling shall be verified through:

- normal scheduling;
- overlapping schedules;
- reservation conflicts;
- resource loss;
- deadline pressure;
- time synchronization issues;
- stale data;
- weather changes;
- communication loss;
- UAV failure;
- fleet changes;
- replanning;
- cancellation;
- abort;
- recovery;
- simulation;
- sandbox isolation.

## Negative Testing

Verification shall confirm that:

- schedule does not equal execution authority;
- approval cannot silently apply to a different mission revision;
- conflicting reservations cannot silently overwrite one another;
- simulated schedules cannot create actual reservations;
- stale time-sensitive data cannot silently be treated as current;
- Mission AI cannot independently activate a schedule;
- QML cannot bypass Core scheduling authority;
- unsafe resume cannot occur automatically;
- failed scheduling is not reported as successful.

## Traceability

Scheduling shall support:

MISSION
→ REVISION
→ SCHEDULE
→ TIME WINDOW
→ RESERVATION
→ RESOURCE
→ TASK
→ VALIDATION
→ APPROVAL
→ EXECUTION
→ RESULT.

## Rationale

A dedicated temporal coordination architecture prevents mission timing, resource reservation and execution authority from becoming conflated.

It allows BlueSky to coordinate complex multi-UAV missions, deadlines, resources and environmental windows while preserving safety authority, explicit approval, simulation isolation and complete traceability.


