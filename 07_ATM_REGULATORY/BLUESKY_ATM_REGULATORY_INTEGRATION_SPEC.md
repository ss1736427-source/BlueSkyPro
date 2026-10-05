# BlueSky PRO — ATM / Regulatory Integration Specification

**Status:** WORKING BASELINE
**Position:** PHASE 7 — ATM / Regulatory Integration

## Purpose

Define the external aviation and regulatory interfaces required to move from a planned mission to a legally and operationally releasable flight.

## Architectural boundary

```text
             BLUESKY PRO
                  │
        Regulatory / ATM Service
                  │
        External-system adapters
                  │
      ┌───────────┼────────────┐
      ▼           ▼            ▼
 Airspace/AI   FPL/ATM     Authorization/
 NOTAM/AIP     exchange     reporting
```

The core mission model remains independent of a particular national system or submission protocol.

## External information

The regulatory layer shall be able to consume, subject to availability and authorization:

- controlled/restricted/prohibited airspace;
- temporary restrictions;
- NOTAM and applicable aeronautical information;
- relevant AIP/aeronautical publications;
- terrain/obstacle constraints;
- applicable traffic information;
- jurisdiction-specific operational constraints.

External data shall carry source, timestamp, validity/freshness and version metadata where available.

## Normalized regulatory objects

- Operator;
- Flight / Operation;
- Aircraft / UAV;
- Mission;
- Flight Plan;
- Airspace Constraint;
- Authorization / Permit;
- Submission;
- External Reference;
- Regulatory Status;
- Change / Correction;
- Flight Closure / Report where required.

## Submission lifecycle

```text
MISSION CREATED
      ↓
REGULATORY ANALYSIS
      ↓
FLIGHT PLAN GENERATED
      ↓
VALIDATION
      ↓
SUBMISSION
      ↓
PENDING
   ┌──┴──────────────┐
   ▼                 ▼
ACCEPTED          REJECTED
   │                 │
   │          CORRECTION REQUIRED
   │                 │
   │          ←──────┘
   ↓
AUTHORIZATION / APPROVAL
      ↓
LINK TO MISSION VERSION
      ↓
FLIGHT RELEASE
```

## State model

`DRAFT → VALIDATED → SUBMITTED → PENDING → ACCEPTED / REJECTED / CORRECTION_REQUIRED → AUTHORIZED → EXPIRED / CANCELLED / CLOSED`

Jurisdiction-specific states may be added by an adapter but must map to the normalized BlueSky model.

## Change control

A material change to an approved operation shall trigger re-evaluation. Examples include route/trajectory, operating area, altitude/time, aircraft, relevant operational parameters, authorization expiry, or newly applicable restriction.

BlueSky shall preserve the relationship between the approved regulatory version and executable mission version.

## Multi-UAV

A coordinated operational mission may require one or several regulatory flight plans/authorizations. BlueSky shall support both patterns and retain individual regulatory identity/status for every aircraft.

## Flight release gate

Submission success alone shall never mean flight authorization. At minimum:

```text
Mission validated
+ Airspace constraints valid
+ Required authorization valid
+ Authorization linked to current mission version
+ Aircraft identity matches
+ Time validity active
+ Required external systems available
+ No unresolved regulatory blocker
        ↓
READY FOR FLIGHT
```

## External-system failure

Explicit states:

`UNAVAILABLE | SUBMISSION_PENDING | ACCEPTED | REJECTED | CORRECTION_REQUIRED | EXPIRED | CANCELLED`

Loss of an external connection shall not silently change authorization state.

## Reporting / closure

Where required by the operating jurisdiction, BlueSky shall support applicable post-flight status, occurrence, closure or reporting workflows and associate them with the Flight Record.

## Audit

Retain, as applicable:

- submitted content/version;
- timestamp;
- submitting identity;
- external system identity;
- external reference;
- response;
- rejection/correction;
- authorization;
- cancellation/expiry;
- final closure/reporting result.

## Connector contract

Each jurisdiction-specific connector shall define:

`Transport → Authentication → API/Protocol → Schema → Validation → Submission → Response → Status mapping → Error handling → Audit → Verification`.

No jurisdiction-specific implementation shall leak into the core mission model.

## Verification

Test schema/unit behavior, connector integration, available sandbox environments, recorded response/replay, rejection/correction, expiry/cancellation, multi-UAV and authorization-to-mission version integrity scenarios.

## Gate

PHASE 7 is complete only when a representative mission can pass regulatory analysis, plan generation, validation, submission, response/correction, authorization linkage and flight-release decision through the applicable external system interface with a complete auditable record.
