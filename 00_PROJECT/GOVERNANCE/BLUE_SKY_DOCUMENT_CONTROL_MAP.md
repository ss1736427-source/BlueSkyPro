# BLUE SKY — DOCUMENT CONTROL MAP

## Certification chain

`CERTIFICATION OBJECT / SCOPE`
↓
`CERTIFICATION BASIS`
↓
`REGULATORY SOURCE REGISTER`
↓
`COMPLIANCE MATRIX`
↓
`CERTIFICATION REQUIREMENTS BASELINE`
↓
`MASTER REQUIREMENTS REGISTER`
↓
`SYSTEM REQUIREMENTS SPECIFICATION`
↓
`AVIATION SYSTEM DEFINITION`
↓
`INTERFACE CONTROL DOCUMENTS`
↓
`SAFETY REQUIREMENTS BASELINE`
↓
`HAZARD LOG`
↓
`SAFETY CASE`
↓
`VERIFICATION PLAN`
↓
`REQUIREMENTS TRACEABILITY MATRIX`
↓
`VERIFICATION EVIDENCE INDEX`
↓
`CERTIFICATION EVIDENCE`

## Configuration

`CONFIGURATION OBJECT / SCOPE`
→ `CONFIGURATION BASELINE`
→ `CONFIGURATION-A APPLICABILITY MATRIX`
→ change control

## C2

`AIR CODE 78.1`
→ `C2_PROVIDER_REGULATORY_ASSESSMENT`
→ `C2 requirements`
→ `C2 ICD`
→ `SRS`
→ `Safety`
→ `Verification`
→ `Evidence`

C2 records are allocated by engineering role in the corresponding requirement, architecture/interface, safety, verification and HMI layers.

## Operational / external interface

`№140`
→ landing-site / infrastructure interface requirements

`№312`
→ operational / infrastructure interface requirements

`№367`
→ maintenance interface, conditional on applicability

`№354`
→ ground-equipment check interface, conditional on applicability

These references describe external constraints/interfaces. They do not by themselves create internal BlueSky PRO functions.

## Data

`Mission Data Retention Matrix`
→ retention requirements
→ archive controls
→ evidence / records

## Control files

`BLUE_SKY_PROJECT_STATE.md`
= current project state / handover baseline

`BLUE_SKY_DECISIONS_LOG.md`
= approved decisions; changes require documented basis

`Master Document Index`
= complete document inventory

## Status convention

- `DRAFT`
- `IN WORK`
- `BASELINED`
- `APPROVED`
- `SUPERSEDED`
- `REG-GAP / REGULATORY WATCH`

## Current controlled paths

```text
Project governance
→ 00_PROJECT/GOVERNANCE/

Project configuration
→ 00_PROJECT/CONFIGURATION/

Project knowledge
→ 00_PROJECT/KNOWLEDGE/

System requirements
→ 01_REQUIREMENTS/SYSTEM/

Regulatory basis
→ 01_REQUIREMENTS/REGULATORY/

Safety
→ 01_REQUIREMENTS/SAFETY/

Traceability
→ 01_REQUIREMENTS/TRACEABILITY/

Architecture
→ 02_ARCHITECTURE/

System modules
→ 03_SYSTEM/

Software
→ 04_SOFTWARE/

Verification
→ 05_VERIFICATION/

Certification
→ 06_CERTIFICATION/

External interfaces/data
→ 04_INTEGRATION/ + 07_DATA/ + applicable external-interface records

Note: `07_EXTERNAL/` is not present in current main.

HMI
→ 08_HMI/

Archive
→ 99_ARCHIVE/
```

The Master Document Index is the authoritative inventory of document identity and current path. The control map defines relationships and control responsibilities; it does not replace primary engineering records.
