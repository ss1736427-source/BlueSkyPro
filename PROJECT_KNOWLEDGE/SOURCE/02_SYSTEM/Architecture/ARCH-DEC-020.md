---
id: ARCH-DEC-020
type: architecture_decision
title: External Systems and Integration Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-020 — External Systems and Integration Architecture

## Decision

BlueSky shall integrate with external systems through defined interfaces and adapters.

The C++ Core shall not contain direct dependencies on specific external implementations where an abstraction is applicable.

The integration architecture shall isolate external systems from the Mission Architecture, Safety Engine, Mission Validation, Mission AI and user interface.

## Integration Boundary

The primary integration path shall be:

QML
→ C++ CORE
→ DOMAIN SERVICE
→ ADAPTER / INTERFACE
→ EXTERNAL SYSTEM

External systems shall not obtain unrestricted direct access to internal Mission Core state.

## External System Categories

The architecture shall support integration with applicable:

- UAV systems;
- fleet systems;
- C2 systems;
- communication systems;
- payload systems;
- navigation systems;
- positioning systems;
- map providers;
- terrain providers;
- airspace data providers;
- geofence sources;
- weather providers;
- simulation / Digital Twin systems;
- external data services;
- future mission-related systems.

## UAV Integration

UAV integration shall use a UAV Adapter.

The adapter shall provide a normalized representation of applicable:

- UAV state;
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

The Core shall operate against the abstract UAV interface rather than against a specific UAV implementation.

## Fleet Integration

Fleet integration shall expose normalized fleet state to the Core.

Applicable information shall include:

- UAV availability;
- assigned role;
- current mission;
- capability state;
- resource state;
- communication state;
- health state.

Fleet changes shall be available to Mission Validation, Mission AI, Fleet Coordination and Safety Engine where applicable.

## C2 Integration

C2 shall be integrated through a dedicated adapter.

The interface shall support bidirectional communication:

CORE
→ C2 ADAPTER
→ COMMUNICATION SYSTEM
→ UAV / FLEET

UAV / FLEET
→ COMMUNICATION SYSTEM
→ C2 ADAPTER
→ CORE

The system shall expose communication state including:

- CONNECTED;
- DEGRADED;
- LOST;
- RECOVERING;
- UNAVAILABLE.

## Communication Integration

Communication systems shall be abstracted from mission logic.

The Core shall not assume a specific communication technology.

Applicable communication properties may include:

- availability;
- latency;
- bandwidth;
- link quality;
- connection state;
- loss;
- degradation.

Communication state shall be available to validation and safety mechanisms.

## Payload Integration

Payloads shall be integrated through defined payload interfaces or adapters.

The architecture shall support different payload types without changing the basic Mission Architecture.

Applicable payload state may include:

- availability;
- configuration;
- operational state;
- health;
- power state;
- capability state.

## Navigation and Positioning

Navigation and positioning systems shall be accessed through defined interfaces.

The architecture shall support integration of applicable:

- GNSS;
- inertial systems;
- external positioning;
- navigation sensors;
- localization services.

The Core shall use normalized navigation information.

## Map Integration

Map systems shall be isolated behind a map interface.

The map integration shall support applicable:

- base maps;
- spatial layers;
- coordinate systems;
- map metadata;
- version information.

The QML layer may visualize map data but shall not become the authoritative owner of spatial mission data.

## Terrain Integration

Terrain data shall be accessed through a defined terrain interface.

Terrain information shall support:

- elevation;
- terrain model;
- terrain restrictions;
- terrain version;
- spatial reference.

Terrain data shall be available to Route Planner, Spatial Validation, Simulation and Safety mechanisms where applicable.

## Airspace Integration

Airspace information shall be integrated through a defined interface.

Applicable information may include:

- restricted areas;
- controlled airspace;
- temporary restrictions;
- altitude restrictions;
- operational constraints;
- validity intervals.

Airspace information shall remain traceable to its source and version where applicable.

## Geofence Integration

Geofences shall be represented through normalized spatial interfaces.

The architecture shall support:

- static geofences;
- dynamic geofences;
- mission-specific geofences;
- temporary restrictions.

Geofence information shall be available to Mission Validation and Safety Engine.

## Weather Integration

Weather providers shall be accessed through a weather interface.

Applicable information may include:

- current conditions;
- forecast;
- wind;
- precipitation;
- visibility;
- temperature;
- operational restrictions;
- data timestamp;
- source information.

Material weather changes shall be available to Dynamic Validation and Runtime Safety.

## Simulation Integration

Simulation / Digital Twin shall use a Simulation Adapter.

The adapter shall allow the same Mission Architecture to be processed through:

Mission
→ Real Execution

or:

Mission
→ Simulation / Digital Twin

Simulation shall remain isolated from real execution.

## External Data Integration

External data shall enter the system through controlled interfaces.

Imported information shall be:

- identified;
- validated where applicable;
- associated with a source;
- timestamped where applicable;
- versioned where applicable.

Untrusted external data shall not automatically become authoritative mission state.

## Data Normalization

External systems may use different formats, units, coordinate systems and state representations.

Adapters shall normalize external information before it enters the relevant Core service.

Normalization may include:

- units;
- coordinate systems;
- timestamps;
- identifiers;
- state representations;
- capability representations.

## Data Validation

External data shall be checked for applicable:

- format;
- completeness;
- consistency;
- validity;
- freshness;
- source identity;
- expected ranges.

Invalid or stale data shall not silently replace valid authoritative state.

## Source Authority

The architecture shall define which system is authoritative for each category of information.

The Core shall maintain authoritative mission state.

External systems shall provide data or services through defined interfaces.

Conflicting external information shall be identified and handled according to applicable validation and safety rules.

## External System Failure

The system shall detect relevant external integration failures.

Applicable conditions include:

- unavailable service;
- timeout;
- communication loss;
- degraded quality;
- stale data;
- invalid data;
- incompatible version;
- adapter failure.

External failure shall be integrated with the Error Handling and Contingency Architecture.

## Failure Response

An external integration failure may cause:

- warning;
- degraded state;
- revalidation;
- replanning;
- resource reassignment;
- fallback source;
- contingency activation;
- mission blocking;
- emergency response.

The applicable response shall be determined by mission and safety constraints.

## Fallback Sources

Where appropriate, the architecture shall support alternate sources.

Fallback selection shall not silently violate:

- safety constraints;
- data validity requirements;
- mission requirements.

A fallback source shall be identified and traceable.

## Adapter Independence

Adapters shall isolate the Core from:

- UAV manufacturers;
- communication technologies;
- map providers;
- terrain providers;
- weather providers;
- simulation engines;
- payload manufacturers;
- external service implementations.

A replacement of an external provider should not require modification of the basic Mission Architecture.

## Interface Versioning

External interfaces shall support version identification where applicable.

Interface incompatibility shall be detected rather than silently ignored.

Changes that materially affect mission behaviour shall trigger the applicable validation and revalidation process.

## Security Boundary

External interfaces shall be subject to the applicable authentication and authorization mechanisms.

External connectivity shall not grant direct execution authority.

The security boundary shall remain:

EXTERNAL SYSTEM
→ ADAPTER
→ AUTHORIZATION / VALIDATION
→ CORE

where applicable.

## Safety Boundary

External systems shall not bypass the Safety Engine.

External data may influence planning, validation or runtime decisions, but safety-critical execution shall remain subject to the BlueSky Safety Architecture.

## Mission AI Integration

Mission AI may consume normalized external information for:

- planning;
- analysis;
- optimization;
- simulation;
- recommendation;
- adaptation.

Mission AI shall not directly invoke unrestricted external execution interfaces.

## Mission Validation Integration

External information relevant to mission validity shall be available to Mission Validation.

Examples include:

- airspace;
- terrain;
- geofence;
- weather;
- communication;
- fleet state;
- UAV capabilities.

## Mission Readiness Integration

External state relevant to readiness shall contribute to Mission Readiness.

A mission shall not remain READY solely because external conditions were previously valid.

## Runtime Integration

External state may change during execution.

The system shall support:

MONITOR
→ DETECT CHANGE
→ REVALIDATE
→ ADAPT / REPLAN / ABORT

where applicable.

## Simulation and Sandbox

Simulation and Sandbox shall provide isolated external-system representations where required.

A simulated external system shall remain distinguishable from a real external system.

Sandbox integrations shall not create a direct path to real UAV execution.

## Observability

External integrations shall expose sufficient operational information for diagnosis.

Applicable information includes:

- connection state;
- request state;
- response state;
- latency;
- errors;
- source;
- version;
- timestamp.

## Audit

Material external-system interactions shall be auditable where required.

The audit record shall identify:

- external system;
- adapter;
- operation;
- timestamp;
- mission revision where applicable;
- result;
- error or rejection where applicable.

## Traceability

External integration traceability shall connect:

MISSION
→ CORE SERVICE
→ ADAPTER
→ EXTERNAL SYSTEM
→ RESULT
→ VALIDATION / SAFETY EFFECT

The architecture shall allow determination of which external source contributed to a mission decision.

## Rationale

An adapter-based integration architecture allows BlueSky to evolve independently of specific UAVs, communication systems, data providers, payloads and simulation technologies while preserving mission, validation, safety and execution boundaries.


