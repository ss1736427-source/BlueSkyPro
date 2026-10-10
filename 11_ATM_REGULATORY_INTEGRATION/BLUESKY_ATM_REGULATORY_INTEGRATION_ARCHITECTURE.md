# BlueSky PRO — ATM / Regulatory Integration Architecture

## 1. Purpose

BlueSky must not be limited to displaying airspace data. Regulatory/ATM integration is an external operational contour through which a planned mission is checked, prepared, submitted, coordinated and tracked according to the applicable aviation procedures.

## 2. Architectural position

```text
MISSION INTENT
      ↓
BLUE SKY PLANNING CORE
      ↓
ROUTE / FLEET / ENERGY OPTIMIZATION
      ↓
REGULATORY VALIDATION
      ↓
ATM / AIRSPACE SERVICES
      ↓
SUBMISSION / COORDINATION / APPROVAL
      ↓
AUTHORIZED MISSION
      ↓
FLIGHT EXECUTION
      ↓
STATUS / CHANGES / EVENTS
      ↓
BLUE SKY
```

The regulatory layer is separate from the planning and autopilot layers.

## 3. External interfaces

The architecture shall support adapters for the external systems and services required in the target operating jurisdiction, including where applicable:

- authoritative airspace and restriction data;
- NOTAM and other operational information sources;
- flight-plan submission and exchange services;
- ATM/ATC coordination interfaces;
- UTM/U-space services where applicable;
- authorization/permit workflows;
- operator/company regulatory systems;
- identity, registration and aircraft/operator data sources;
- weather and other aviation information services when required by the operational procedure.

No single external provider shall be hard-coded into the BlueSky planning core.

## 4. Regulatory data pipeline

```text
EXTERNAL SOURCE
      ↓
ADAPTER
      ↓
NORMALIZATION
      ↓
VALIDITY / FRESHNESS CHECK
      ↓
AIRSPACE & REGULATORY MODEL
      ↓
MISSION VALIDATION
      ↓
PLANNING / REPLANNING
```

Data must carry source, validity period, timestamp/version and provenance where available.

## 5. Mission submission

BlueSky shall distinguish between:

1. internal mission plan;
2. regulatory flight-plan package;
3. submitted package;
4. accepted/approved package;
5. executable mission package.

A mission shall not be represented as operationally authorized merely because it passed internal BlueSky validation.

## 6. Group UAV operations

For multi-UAV missions, BlueSky shall support both:

- one coordinated submission where the applicable procedure permits a group operation;
- separate flight plans/authorizations for individual UAVs when required.

The mission coordinator shall retain the relationship between all individual plans and the parent operational mission.

```text
GROUP MISSION
     ↓
REGULATORY DECOMPOSITION
     ↓
┌──────────┬──────────┬──────────┐
│ UAV-01   │ UAV-02   │ UAV-03   │
│ FPL/AUTH │ FPL/AUTH │ FPL/AUTH │
└──────────┴──────────┴──────────┘
     ↓
ALL REQUIRED STATUS RECEIVED
     ↓
MISSION RELEASE
```

## 7. Change management

Changes to route, timing, aircraft, operating area or other regulated parameters shall trigger a re-evaluation of the affected regulatory requirements.

BlueSky shall identify whether a change:

- remains within the existing authorization;
- requires an amended submission;
- requires a new authorization;
- makes the current mission non-executable.

## 8. Traceability

For each regulated mission BlueSky shall retain, subject to applicable retention and security requirements:

- submitted data;
- submission timestamp;
- source/version of regulatory data used;
- responses and status changes;
- authorization identifiers;
- relationship between approved and executable mission versions;
- relevant operator/system actions;
- final operational status.

## 9. Separation of responsibilities

**BlueSky Core:** plans and validates the mission.

**Regulatory Adapter:** translates BlueSky data to the external system and maps external responses back into the BlueSky model.

**External ATM/Regulatory System:** remains authoritative for the external regulatory decision.

**Autopilot:** executes the authorized executable mission and provides aircraft state/telemetry.

## 10. Safety principle

Regulatory constraints are hard constraints for mission planning. The Algorithm Orchestrator may optimize only within the set of solutions permitted by the applicable regulatory and safety rules.

```text
ALGORITHM ORCHESTRATOR
          ↓
 candidate plans
          ↓
 SAFETY / REGULATORY GATE
          ↓
 only admissible plans
          ↓
 energy / resource / mission-quality optimization
```

## 11. Universal integration principle

BlueSky shall use an adapter architecture so that adding a new national ATM/UTM/regulatory interface does not require modification of the core planning algorithms.

The same principle applies to autopilots, communication systems, weather providers and other external services.

## 12. Implementation status

This document defines the required architectural contour. Specific adapters, national interfaces, schemas, authentication mechanisms and operational procedures are implementation items and shall be specified against the target jurisdiction and authoritative interface documentation before certification and deployment.
