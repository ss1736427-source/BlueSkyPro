---
id: EXTERNAL-REGULATORY-INTERFACE-ALLOCATION-001
type: external_regulatory_interface_allocation
status: controlled_working_draft
system: BlueSky PRO
basis: REGULATORY-SOURCE-REGISTER-001; C2-TRACEABILITY-BASELINE-001; C2-ICD-BASELINE-001
---

# BLUE SKY PRO — EXTERNAL REGULATORY INTERFACE ALLOCATION 001

## 1. Purpose

Define the boundary between external regulatory obligations and BlueSky PRO functions for the currently identified external regulatory sources. The record prevents automatic conversion of external obligations into BlueSky system requirements while identifying interface consequences that can be implemented or prepared without test data.

## 2. Allocation rule

```text
SOURCE / CLAUSE
      ↓
OBJECT OF REGULATION
      ↓
BLUE SKY RESPONSIBILITY?
      ↓
DIRECT / INTERFACE / CONDITIONAL / EXTERNAL
      ↓
IF INTERFACE:
    own input/output/status/event function
      ↓
REQUIREMENT / ICD / OPERATIONAL CONSTRAINT
```

A requirement remains external when the regulated object, responsibility or equipment is outside the approved BlueSky system boundary.

## 3. Controlled source identification

| Register ID | Source | Current applicability |
|---|---|---|
| REG-017 | Order of the Ministry of Transport of Russia dated 21.04.2025 No. 140 — requirements for landing sites | EXTERNAL / CONDITIONAL |
| REG-018 | Order of the Ministry of Transport of Russia dated 01.10.2025 No. 312 — operation of aerodromes, heliports and landing sites | EXTERNAL / INTERFACE / CONDITIONAL |
| REG-019 | Order of the Ministry of Transport of Russia dated 18.10.2024 No. 367 — maintenance, Part 145 | EXTERNAL / INTERFACE / CONDITIONAL |
| REG-020 | Order of the Ministry of Transport of Russia dated 07.10.2024 No. 354 — ground/flight checks, including C2-provider ground equipment | EXTERNAL / INTERFACE / CONDITIONAL |

## 4. REG-017 — Order No. 140, landing sites

The source regulates landing sites intended for take-off, landing, taxiing and parking of civil aircraft. The source entered into force on 01.09.2025 and applies to owners of such landing sites.

### BlueSky allocation

**Primary classification: EXTERNAL.**

The following remain outside BlueSky unless the certification object explicitly includes them:

- construction and physical condition of a landing site;
- ownership and organizational operation of the site;
- marking and physical equipment of the site;
- compliance of the site itself with the aviation rules.

### Possible BlueSky interface

If the operational concept requires BlueSky to use landing-site information, BlueSky may implement a controlled input/validation function for externally supplied landing-site data. The function shall concern only:

- receipt of landing-site identity and location;
- availability/status data where such data is externally supplied;
- use of approved landing-site constraints in planning;
- consistency checking of the data used by the planner.

No requirement to certify, maintain or operate the physical landing site is allocated to BlueSky.

**Current status:** no new SYS-REQ created. Interface allocation is conditional on the operational concept and approved system boundary.

## 5. REG-018 — Order No. 312, aerodrome / heliport / landing-site operation

The source regulates operation of aerodromes, heliports and landing sites. The rules entered into force on 01.03.2026.

### BlueSky allocation

**Primary classification: EXTERNAL / CONDITIONAL.**

Operator obligations concerning physical infrastructure, aerodrome operation and organizational activities remain external.

### Possible BlueSky interface

Where an approved operational interface exists, BlueSky may consume external operational data relevant to flight planning and execution, including availability or operational restrictions supplied by the responsible external party.

The BlueSky-side responsibility is limited to:

```text
receive external data
→ validate format / applicability
→ apply as planning constraint
→ expose relevant restriction to operator
→ record the applied source/version where required
```

BlueSky does not certify the aerodrome/heliport/landing site and does not assume the external operator's operational responsibility.

**Current status:** no new SYS-REQ created. Interface allocation remains conditional until a concrete external data/interface is defined.

## 6. REG-019 — Order No. 367, maintenance Part 145

The source establishes rules for maintenance of subject-to-mandatory-certification unmanned aviation systems and/or their elements and associated aviation equipment, including requirements applicable to maintenance organizations.

### BlueSky allocation

**Primary classification: EXTERNAL / CONDITIONAL.**

Maintenance organization responsibilities, maintenance personnel, maintenance facilities and physical maintenance activities are external when BlueSky is not the maintenance organization and are not part of the BlueSky certification object.

### Possible BlueSky interface

The previously established external-maintenance architecture permits a narrow BlueSky-side interface for technical readiness information when such information is actually supplied by an authorized maintenance system/organization:

- maintenance status;
- release/return-to-service status where applicable;
- restrictions affecting operation;
- maintenance-related events required by the operational concept;
- configuration/version identity relevant to safe operation.

The BlueSky function is to consume and apply controlled external status; it does not create or replace the maintenance organization's certification record.

**Current status:** interface principle/design already exists; no new maintenance SYS-REQ is created from the external rule alone.

## 7. REG-020 — Order No. 354, C2-provider ground equipment

The source establishes the procedure for ground and flight checks, including checks of ground equipment required by providers of services for servicing C2/control lines. It entered into force on 01.03.2025.

### BlueSky allocation

**Primary classification: EXTERNAL / INTERFACE.**

Provider-side equipment ownership, inspection, calibration/measurement activities, maintenance and formal check records remain provider responsibilities unless that equipment is explicitly included in the BlueSky certification object.

### Concrete interface consequences

The source and the current C2 rules establish a usable interface boundary:

```text
C2 provider
    ↓
service availability / coverage / quality information
    ↓
BlueSky C2 interface
    ↓
planning + monitoring + alerting + event recording
```

The BlueSky-side functions that can be specified without test data are:

1. accept provider service/coverage information;
2. associate coverage information with route/flight context;
3. expose service state to the operator;
4. detect loss/degradation indications received from the C2 service;
5. record relevant C2 state transitions and events;
6. prevent provider-side inspection obligations from being represented as BlueSky verification evidence.

The formal provider inspection/check record remains external evidence.

**Current status:** interface consequence identified; no provider-side requirement transferred to BlueSky.

## 8. Cross-source boundary

| External domain | External responsibility | BlueSky responsibility | New SYS-REQ now? |
|---|---|---|---|
| Landing-site physical compliance | Site owner/operator | Consume approved data if required | NO |
| Aerodrome/heliport operation | Aerodrome/heliport operator | Consume operational constraints if required | NO |
| Maintenance Part 145 | Maintenance organization | Consume authorized readiness/status data if required | NO |
| C2-provider ground equipment checks | C2 provider/equipment owner | Consume service state and relevant events | NO |

## 9. Implementation consequence

The current architecture shall prefer stable adapters/contracts for all four external domains:

```text
External source/provider
        ↓
External Data Adapter
        ↓
Normalized BlueSky External Status Model
        ↓
Validation / applicability
        ↓
Planning / Flight / Monitoring
```

This prevents future provider, infrastructure or maintenance-system changes from propagating into the core planning and flight-control logic.

## 10. Verification preparation

The following verification classes can be prepared before real operational tests:

- source/interface inspection;
- schema validation;
- invalid/missing-data handling;
- applicability filtering;
- stale-data detection;
- state-transition recording;
- interface failure/degradation handling;
- audit-trail completeness.

No numerical acceptance threshold is invented here. Thresholds remain controlled by the applicable certification basis, approved requirements and verification configuration.

## 11. Deferred items

```text
REAL PROVIDER INTERFACE DATA       → test stage
ACTUAL LANDING-SITE DATA           → operational integration stage
ACTUAL MAINTENANCE SYSTEM DATA     → integration stage
QUANTITATIVE C2 SERVICE THRESHOLDS → approved verification basis
PROVIDER-SIDE CHECK EVIDENCE       → external evidence package
```

## 12. Decision status

No user decision is required to create this allocation record.

The record intentionally does not create new system requirements where the current operational concept and system boundary do not establish a BlueSky-owned function.

**Status: CONTROLLED WORKING DRAFT — EXTERNAL RESPONSIBILITIES SEPARATED; INTERFACE CONSEQUENCES IDENTIFIED; NO NEW UNSUPPORTED SYS-REQ CREATED.**
