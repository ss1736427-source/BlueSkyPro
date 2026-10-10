# BlueSky PRO — ATM / Regulatory Integration Baseline

**Status:** ARCHITECTURE BASELINE — P0 integration contract

## 1. Purpose

BlueSky shall treat ATM/regulatory integration as an operational lifecycle, not as a map layer. The system must connect mission planning with applicable airspace constraints, planning procedures, submissions, responses, permissions/conditions, execution and post-flight evidence.

For the Russian operating environment, the implementation shall support the applicable procedures and interfaces of the Unified Air Traffic Management System (ЕС ОрВД), including flight-plan submission workflows where required. Applicable requirements must be version-controlled against current regulations and official procedures.

## 2. Operational chain

```text
MISSION
  ↓
AIRSPACE / REGULATORY DATA
  ↓
ROUTE CONSTRAINT CHECK
  ↓
REGULATORY FEASIBILITY
  ↓
FPL / REQUIRED SUBMISSION
  ↓
SUBMISSION / EXCHANGE
  ↓
RESPONSE / CONDITIONS / AUTHORIZATION
  ↓
MISSION RELEASE
  ↓
EXECUTION
  ↓
CHANGES / EVENTS
  ↓
AMENDMENT / REPLAN / ABORT
  ↓
POST-FLIGHT RECORD
```

Russian rules provide for submission of UAV flight-plan information for obtaining permission to use airspace in the applicable cases, and define transmission through prescribed channels and message formats. citeturn0search1turn0search0

## 3. Separation of layers

BlueSky shall distinguish:

1. **Airspace data** — geometry, restrictions, status, validity and source.
2. **Regulatory rules** — rules determining applicability and required actions.
3. **Operational procedure** — what must be submitted, when, to whom and through which channel.
4. **External system adapter** — technical connection to an external service/system.
5. **Authorization/condition state** — the current operational permission and its conditions.

A map displaying a restriction is therefore not considered regulatory integration by itself.

## 4. Airspace and constraint model

The common model shall represent, as applicable:

- prohibited/restricted/danger areas;
- temporary/local regimes;
- temporary restrictions;
- controlled airspace;
- aerodrome-related constraints;
- route/altitude restrictions;
- time windows;
- geographic boundaries;
- applicable operational conditions;
- source;
- publication/version;
- effective start/end;
- retrieval/verification timestamp.

The planner shall use the authoritative validity interval rather than treating all restrictions as permanent.

## 5. Regulatory rule engine

The Regulatory Rule Engine determines the procedural consequences of the planned operation.

```text
MISSION + AIRSPACE + UAV + OPERATIONAL CONDITIONS
                    ↓
             RULE EVALUATION
                    ↓
      ┌─────────────┴─────────────┐
      ↓                           ↓
NO EXTERNAL ACTION          SUBMISSION REQUIRED
                                  ↓
                         FPL / APPLICATION / OTHER
```

Rules shall be configurable/versioned so regulatory changes do not require rewriting the mission planner.

## 6. Flight-plan generation

When a flight plan is required, BlueSky shall generate it from the approved mission representation rather than requiring duplicate manual data entry.

The generated message shall be validated against the applicable message specification before submission.

For multi-UAV operations, the system shall determine whether the applicable procedure permits a common/group representation or requires separate plans/submissions. Each vehicle-specific operational authorization shall remain traceable to the corresponding UAV mission package.

## 7. Submission adapter

External regulatory/ATM communication shall use adapters.

```text
BLUE SKY REGULATORY API
          ↓
   PROCEDURE ADAPTER
          ↓
┌─────────┼─────────┐
↓         ↓         ↓
ЕС ОрВД  Other ATM  OEM/Authority
```

The core planner must not contain transport-specific logic for web, telephone, telegraphic, API or other external channels.

## 8. Submission state machine

```text
DRAFT
 ↓
READY_FOR_SUBMISSION
 ↓
SUBMITTED
 ↓
RECEIVED / ACKNOWLEDGED
 ↓
UNDER_REVIEW
 ↓
APPROVED / AUTHORIZED
        or
REJECTED / RETURNED / CONDITIONAL
```

The exact states shall be mapped to the terminology and responses of each authority/system.

## 9. Authorization and conditions

An approval/permission shall be represented as structured operational data, including where available:

- authorization identity;
- associated flight plan/mission;
- vehicle(s);
- geographic/altitude scope;
- effective period;
- conditions/restrictions;
- issuing authority/system;
- document/message reference;
- status;
- timestamp/version.

A textual document alone is insufficient for automated release if its safety-relevant conditions cannot be represented or verified.

## 10. Mission release gate

The mission cannot be released solely because the route is technically feasible.

```text
TECHNICALLY FEASIBLE
        +
REGULATORYALLY FEASIBLE
        +
REQUIRED SUBMISSIONS COMPLETE
        +
AUTHORIZATION / CONDITIONS SATISFIED
        ↓
   RELEASE ELIGIBLE
```

Regulatory failure is a hard gate where authorization is mandatory.

## 11. Changes after approval

Any change affecting the authorized operation shall be evaluated against the applicable regulatory procedure.

Examples:

- route/waypoint change;
- altitude change;
- operating time change;
- UAV substitution;
- group composition change;
- launch/recovery point change;
- significant mission extension;
- change caused by weather or C2 recovery.

BlueSky shall determine whether the change requires amendment, resubmission, new authorization, or can remain within existing conditions.

## 12. Runtime interaction

ATM/regulatory status remains connected to runtime execution.

```text
AUTHORIZATION
     ↓
EXECUTION
     ↓
EVENT / DEVIATION
     ↓
REGULATORY IMPACT CHECK
     ↓
CONTINUE / AMEND / REPLAN / ABORT
```

The system shall not automatically assume that a technically safe replanned route is legally executable.

## 13. External-system independence

The architecture shall support multiple regulatory environments without changing the mission core.

```text
COMMON BLUE SKY MISSION / REGULATORY MODEL
                    ↑
             ADAPTER LAYER
        ┌───────────┼───────────┐
        ↓           ↓           ↓
     ЕС ОрВД      Other ATM   Other states
```

Country/operator-specific procedures shall be isolated in configuration and adapters.

## 14. Data integrity and provenance

Every regulatory datum used in a decision shall retain:

- source;
- version/publication identifier where available;
- retrieval time;
- effective validity;
- transformation/version;
- decision usage.

Stale or unverifiable safety-critical regulatory data shall be treated according to the configured operational policy and shall not silently be treated as current.

## 15. Acceptance criteria

The ATM/Regulatory contract is complete when:

1. airspace data and regulatory procedures are separated;
2. applicable rules can be evaluated automatically;
3. required flight-plan/application messages can be generated from the mission;
4. submission/response states are represented;
5. authorization and conditions are linked to the mission;
6. regulatory status participates in the release gate;
7. changes trigger regulatory impact evaluation;
8. runtime deviations can trigger the required regulatory workflow;
9. country/system-specific integration is isolated behind adapters;
10. regulatory evidence is versioned and traceable.

## 16. Current implementation priority

P0 implementation shall establish:

- authoritative regulatory/airspace data ingestion;
- rule-engine interface;
- FPL/message generation and validation;
- ЕС ОрВД submission workflow adapter where technically and legally available;
- response/authorization state model;
- multi-UAV authorization linkage;
- change/amendment workflow;
- audit/evidence trail.

## 17. Important boundary

This baseline defines the product architecture and integration contract. It does not assert that a particular external authority currently exposes a public API or that any automated submission is legally permitted in every operational case. Such capabilities shall be verified against the current official interface and procedure before implementation and certification.
