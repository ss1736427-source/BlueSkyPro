---
id: ARCH-DEC-028
type: architecture_decision
title: External Systems API and Integration Contract Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-028 — External Systems / API / Integration Contract Architecture

## Decision

BlueSky shall integrate external systems through explicit, versioned and controlled interfaces.

External integrations shall not bypass the C++ Core, Safety Engine, Mission Validation, authorization or applicable execution boundaries.

## Integration Authority

The C++ Core shall own the authoritative integration state.

External systems shall communicate through approved adapters, services or interfaces.

QML shall not directly control external operational systems.

Mission AI shall not directly control external operational systems.

## Integration Domains

The architecture shall support controlled integration with:

- UAV systems;
- UAV fleets;
- C2 systems;
- communication systems;
- payload systems;
- navigation systems;
- positioning systems;
- map services;
- terrain services;
- airspace services;
- geofence services;
- weather services;
- simulation systems;
- external data sources;
- identity and authorization services;
- storage systems.

## Adapter Boundary

Each external integration shall be isolated behind an adapter or equivalent interface boundary.

The Core shall depend on abstract contracts rather than vendor-specific implementations.

Adapters shall translate between:

- Core representation;
- external representation.

## Interface Contract

Each integration contract shall define where applicable:

- interface identity;
- version;
- supported operations;
- data model;
- message format;
- state model;
- error model;
- timeout behaviour;
- availability;
- security requirements;
- compatibility requirements.

## Interface Versioning

Interfaces shall be explicitly versioned.

Breaking interface changes shall require a new compatible contract or coordinated migration.

Historical mission records shall retain the interface version relevant to their execution or data acquisition.

## Compatibility

The system shall identify compatibility between:

- Core version;
- adapter version;
- external system version;
- interface version;
- data schema version.

Unsupported combinations shall be rejected or clearly marked unavailable.

## Data Normalization

External data shall be normalized before becoming authoritative Core state where applicable.

Normalization shall address:

- units;
- identifiers;
- timestamps;
- coordinate systems;
- state representations;
- source identity.

## Source Authority

External data shall retain source identity.

The system shall distinguish:

- authoritative Core state;
- external source state;
- derived state;
- historical state.

External information shall not silently overwrite authoritative Core state.

## UAV Integration

UAV integration shall use the UAV Adapter defined by ARCH-DEC-003.

The adapter shall provide a unified representation of applicable:

- position;
- altitude;
- velocity;
- attitude;
- battery;
- payload;
- navigation;
- health;
- connection;
- mission state.

## Fleet Integration

Fleet integrations shall support:

- UAV discovery;
- capability state;
- resource state;
- assignment state;
- communication state;
- health state;
- mission state.

Fleet changes shall remain visible to Fleet Coordination and applicable validation mechanisms.

## C2 Integration

C2 integration shall support bidirectional communication where applicable.

The interface shall expose communication state including:

- CONNECTED;
- DEGRADED;
- LOST;
- RECOVERING;
- UNAVAILABLE.

C2 state shall be available to Mission Validation and Safety Engine.

## Communication Integration

Communication adapters shall expose relevant:

- availability;
- latency;
- bandwidth;
- quality;
- loss;
- degradation.

Communication changes shall trigger the applicable monitoring and safety paths.

## Payload Integration

Payload integrations shall expose applicable:

- payload identity;
- capability;
- state;
- health;
- availability;
- command status;
- data status.

Payload control shall remain subject to mission authority and safety boundaries.

## Navigation Integration

Navigation integrations shall provide applicable:

- position;
- altitude;
- velocity;
- attitude;
- navigation state;
- positioning quality.

Invalid or unavailable navigation information shall be surfaced to applicable validation and safety mechanisms.

## Map Integration

Map integrations shall support controlled access to applicable:

- map layers;
- coordinates;
- terrain;
- airspace;
- geofence;
- spatial restrictions.

Map data shall preserve source and version identity where material.

## Terrain Integration

Terrain data shall preserve applicable:

- source;
- version;
- coordinate reference;
- timestamp;
- resolution.

Terrain changes shall be available to spatial validation.

## Airspace Integration

Airspace data shall preserve applicable:

- source;
- validity interval;
- geographic scope;
- restrictions;
- version.

Material airspace changes shall trigger applicable revalidation.

## Geofence Integration

Geofence information shall preserve:

- source;
- version;
- geometry;
- validity;
- applicable authority.

Geofence changes shall be integrated with spatial validation and safety.

## Weather Integration

Weather data shall preserve:

- source;
- timestamp;
- validity;
- location;
- forecast or observation identity.

Material weather changes shall be visible to Mission Validation and Readiness.

## Simulation Integration

Simulation systems shall use the Simulation Adapter defined by ARCH-DEC-003.

Simulation shall remain isolated from real execution.

Simulation data shall be identified as SIMULATED.

## External Data Integration

External data sources shall use explicit contracts.

The system shall validate external data before using it in mission-critical decisions.

Invalid, stale or unavailable external data shall be detected.

## Data Freshness

Integration services shall expose data freshness where operationally relevant.

The system shall distinguish:

- current;
- delayed;
- stale;
- unavailable.

Stale external data shall not silently become current operational truth.

## Error Model

Integration errors shall be classified.

Applicable classes include:

- timeout;
- unavailable;
- malformed data;
- authentication failure;
- authorization failure;
- protocol error;
- incompatible version;
- stale data;
- rate limit;
- dependency failure.

## Failure Handling

Integration failures shall enter the applicable error-handling architecture:

DETECT
→ CLASSIFY
→ WARN / DEGRADE / CRITICAL
→ RESPOND
→ RECOVER / REPLAN / ABORT.

## Timeout

External operations shall use controlled timeout behaviour.

A timeout shall not leave the Core in an ambiguous state.

## Retry

Retries may be used where safe and technically appropriate.

Retries shall be bounded and shall not create uncontrolled command duplication.

Safety-critical operations shall use explicit retry semantics.

## Idempotency

Operations that may be retried shall define idempotency where applicable.

Repeated requests shall not unintentionally produce repeated mission-critical actions.

## Ordering

Where message order matters, the interface shall preserve or explicitly reconstruct required ordering.

Out-of-order data shall be detected where operationally significant.

## Duplicate Handling

The system shall detect or safely handle duplicate messages where applicable.

Duplicate operational commands shall not silently cause repeated unsafe actions.

## Rate Limiting

External integrations shall support controlled rate limiting where required.

External traffic shall not starve safety-critical communication or Core processing.

## Backpressure

High-volume integrations shall support controlled backpressure or buffering.

Resource exhaustion shall remain observable.

## Security Boundary

External integrations shall follow ARCH-DEC-018.

Authentication and authorization shall be applied to protected interfaces.

External systems shall receive only the access required for their approved function.

## Trust Boundary

External systems shall be treated as separate trust domains.

External data shall not automatically be considered trusted authoritative state.

## Credential Handling

Credentials and secrets shall not be embedded in mission data or QML.

Credential management shall follow the system security architecture.

## Audit

Material external interactions shall be auditable where required.

Audit information may include:

- source;
- destination;
- operation;
- identity;
- timestamp;
- result;
- error;
- correlation identifier.

## Observability

Integration health shall be exposed to monitoring.

Applicable information includes:

- connection;
- availability;
- latency;
- throughput;
- errors;
- retries;
- queue depth;
- data freshness;
- version.

## Correlation

External operations shall support correlation with the originating mission operation where applicable.

The system should be able to trace:

MISSION
→ CORE OPERATION
→ ADAPTER
→ EXTERNAL REQUEST
→ EXTERNAL RESPONSE
→ CORE RESULT.

## State Synchronization

External state shall be reconciled with Core state where required.

State discrepancies shall be detected.

The system shall not silently choose an external state over authoritative Core state without an approved reconciliation rule.

## Runtime Changes

Changes in external system state shall be monitored.

Material changes may trigger:

- revalidation;
- readiness reassessment;
- adaptation;
- replanning;
- blocking;
- abort.

## External System Recovery

After external system recovery, state shall be reconciled before normal operation resumes.

Recovered connectivity shall not automatically imply recovered operational readiness.

## Sandbox

Sandbox integrations shall use isolated or simulated external systems.

Sandbox shall not expose direct operational execution authority.

## Simulation

Simulation integrations shall remain isolated from real external execution systems unless an explicitly controlled test interface is being used.

## Test Integration

External interfaces shall support test environments where applicable.

Test environments shall remain distinguishable from operational environments.

## Contract Testing

Integration contracts shall be verified through applicable:

- schema tests;
- interface tests;
- compatibility tests;
- negative tests;
- failure tests;
- security tests;
- performance tests.

## Certification and Acceptance

External integrations required for operational acceptance shall have documented verification evidence.

Unverified operational interfaces shall not be treated as accepted execution dependencies.

## Configuration

Integration configuration shall be versioned according to ARCH-DEC-019 and ARCH-DEC-026.

Configuration shall identify applicable:

- endpoint;
- interface version;
- adapter version;
- credentials reference;
- timeout;
- retry policy;
- data mapping;
- environment.

## Deployment

Integration endpoints shall be environment-specific.

Development, Test, Simulation, Sandbox and Operational environments shall not silently share operational endpoints.

## Change Control

Changes to integration contracts, adapters or external dependencies shall undergo applicable impact analysis.

Material changes shall trigger regression verification and revalidation where required.

## Dependency Management

The architecture shall maintain visibility of external dependencies.

Dependencies shall identify:

- system;
- service;
- interface;
- version;
- owner where applicable;
- operational criticality.

## Availability Classification

External dependencies shall be classified according to operational importance.

Loss of a non-critical dependency shall not automatically stop all mission functions.

Loss of a safety-critical dependency shall enter the applicable safety response.

## Fallback

Where an approved fallback exists, the system may transition to the fallback path.

Fallback shall preserve safety and validation requirements.

The system shall not invent an unverified fallback.

## No Silent Fallback

Fallback transitions shall be observable and traceable.

A fallback shall not silently change mission semantics.

## External Command Boundary

External commands affecting UAVs, payloads or mission execution shall originate from authorized Core execution paths.

The UI shall not directly issue protected external commands.

## Safety Boundary

The integration path for execution-critical operations shall remain:

QML
→ CORE
→ AUTHORIZATION
→ SAFETY / VALIDATION
→ ADAPTER
→ EXTERNAL SYSTEM.

## Mission AI Boundary

Mission AI may consume approved external data through controlled interfaces.

Mission AI shall not bypass:

- authorization;
- validation;
- Safety Engine;
- execution authority.

## Data Ownership

The architecture shall identify ownership of material data.

External systems shall remain owners of their source data where applicable.

BlueSky shall own its authoritative mission representation.

## Data Lineage

External data used in mission decisions shall preserve lineage:

EXTERNAL SOURCE
→ ADAPTER
→ NORMALIZATION
→ VALIDATION
→ CORE STATE
→ DECISION.

## Performance

Integration performance shall follow ARCH-DEC-022.

Critical communication shall receive appropriate priority.

## Monitoring

Integration monitoring shall follow ARCH-DEC-024.

Integration degradation shall be visible to the operator and applicable safety services.

## Verification

Integration verification shall follow ARCH-DEC-025.

Material interfaces shall have appropriate:

- functional verification;
- failure testing;
- security testing;
- performance testing;
- compatibility testing.

## Recovery

Integration recovery shall follow ARCH-DEC-027.

Recovered external systems shall undergo state reconciliation before being treated as operational.

## Traceability

Integration traceability shall connect:

EXTERNAL SYSTEM
→ INTERFACE
→ ADAPTER
→ CONFIGURATION
→ REQUEST / DATA
→ RESPONSE
→ CORE STATE
→ VALIDATION
→ MISSION RESULT.

## Rationale

Explicit integration contracts prevent external systems from becoming uncontrolled dependencies and provide stable boundaries for UAVs, C2, maps, weather, simulation and future integrations while preserving security, safety, validation, traceability and replaceability.


