# BlueSky PRO — ATM / Regulatory Integration Framework

**Status:** WORKING BASELINE — REGULATORY INTEGRATION

## Objective

BlueSky shall integrate the operational flight lifecycle with applicable airspace, ATM and regulatory systems. External regulatory interaction is a controlled operational interface, not merely a map-data import.

## Lifecycle

```text
Mission
  ↓
Airspace / NOTAM / Weather / GIS assessment
  ↓
Operational constraints
  ↓
Flight Plan generation
  ↓
Regulatory / ATM submission
  ↓
Accepted / Rejected / Correction Required
  ↓
Correction loop
  ↓
Authorization received
  ↓
Authorization linked to mission/flight
  ↓
Pre-flight release
  ↓
Flight
  ↓
Status / occurrence reporting where required
  ↓
Flight closure / record
```

## Interface classes

BlueSky shall provide adapters for applicable external systems, including:

- airspace/restriction data;
- NOTAM/AIP and related aeronautical information sources;
- flight-plan submission/exchange systems;
- authorization/approval systems;
- traffic/ATM information where available;
- reporting/closure interfaces where required.

The exact external system and protocol shall be defined per operating jurisdiction and customer authorization environment.

## FPL / authorization object

The regulatory integration layer shall maintain a structured object containing, as applicable:

- flight identity;
- aircraft/UAV identity;
- operator identity;
- route/trajectory;
- altitude and time parameters;
- airspace constraints;
- operational purpose;
- applicable authorization;
- submission status;
- external reference/identifier;
- rejection/correction information;
- approval validity;
- version of the submitted mission/flight plan.

## Version integrity

The approved regulatory plan shall be cryptographically or otherwise unambiguously associated with the operational mission version. A material change to the route, timing, aircraft, operating area or other regulated element shall trigger the applicable revalidation/re-submission workflow.

## Multi-UAV operations

The regulatory layer shall support both:

- one authorization/plan covering a permitted coordinated group operation;
- separate plans/authorizations for aircraft operating under separately filed plans.

BlueSky shall not assume that a multi-UAV mission automatically corresponds to one regulatory flight plan. The regulatory adapter shall follow the applicable jurisdictional procedure and retain the relationship between the common operational mission and individual regulatory submissions.

## Failure handling

External ATM/regulatory failures shall have explicit states:

`UNAVAILABLE | SUBMISSION_PENDING | ACCEPTED | REJECTED | CORRECTION_REQUIRED | EXPIRED | CANCELLED`

No flight release shall be inferred from a successful network transaction alone; BlueSky shall require the applicable positive authorization state.

## Audit

All submissions, responses, corrections, approvals, cancellations and relevant external identifiers shall be retained in the Flight Record/audit trail.
