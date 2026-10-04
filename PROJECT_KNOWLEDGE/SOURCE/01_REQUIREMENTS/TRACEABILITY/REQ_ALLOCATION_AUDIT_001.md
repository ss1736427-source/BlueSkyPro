---
id: REQ-ALLOCATION-AUDIT-001
type: direct_requirement_allocation_audit
status: WORKING
authority: MASTER-REQUIREMENTS-REGISTER-001
date: 2026-09-20
---

# BlueSky PRO — Direct Requirement Allocation Audit 001

## 1. Purpose

Factual repository audit of the existing authoritative requirement population against:

`SOURCE/BASIS → REQUIREMENT → SAFETY/HAZARD → ARCHITECTURE → DESIGN/INTERFACE → VERIFICATION → RESULT → EVIDENCE → CONFIGURATION`

This record does not create, renumber, merge, baseline, or verify requirements.

## 2. Authority

Requirement identity remains controlled by:

`01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md`

The SRS and traceability matrices are derived/control records.

## 3. Audit result

The repository confirms that the high-value system requirement set `SYS-REQ-080…104, 107, 108, 110…112` has established title/identity records and direct architecture-decision allocations.

The repository also confirms `SAF-REQ-001…018` as controlled safety requirement wording with preliminary hazard and verification allocations.

The following distinction is mandatory:

- **Identity/title present** does not mean exact source wording is fully reconciled.
- **Architecture allocation present** does not mean design/interface allocation is closed.
- **Verification method/case present** does not mean verification result exists.
- **Evidence path present** does not mean evidence exists.
- **Configuration field/path present** does not mean result/evidence is configuration-bound.

## 4. SYS-REQ architecture allocation audit

| Population | Repository finding | Status |
|---|---|---|
| SYS-REQ-080 | ARCH-DEC-008, 012, 039 | ARCH ALLOCATED |
| SYS-REQ-081 | ARCH-DEC-017, 027, 040 | ARCH ALLOCATED |
| SYS-REQ-082 | ARCH-DEC-016, 017, 038 | ARCH ALLOCATED |
| SYS-REQ-083 | ARCH-DEC-003, 012 | ARCH ALLOCATED |
| SYS-REQ-084 | ARCH-DEC-036, 037 | ARCH ALLOCATED |
| SYS-REQ-085 | ARCH-DEC-007, 016, 036 | ARCH ALLOCATED |
| SYS-REQ-086 | ARCH-DEC-017, 023, 039 | ARCH ALLOCATED |
| SYS-REQ-087 | ARCH-DEC-022, 023, 035 | ARCH ALLOCATED |
| SYS-REQ-088 | ARCH-DEC-011, 025, 026 | ARCH ALLOCATED |
| SYS-REQ-089 | ARCH-DEC-022, 024, 032 | ARCH ALLOCATED |
| SYS-REQ-090 | ARCH-DEC-022, 036 | ARCH ALLOCATED |
| SYS-REQ-091 | ARCH-DEC-022 | ARCH ALLOCATED |
| SYS-REQ-092 | ARCH-DEC-027, 040 | ARCH ALLOCATED |
| SYS-REQ-093 | ARCH-DEC-017, 036 | ARCH ALLOCATED |
| SYS-REQ-094 | ARCH-DEC-029, 041 | ARCH ALLOCATED |
| SYS-REQ-095 | ARCH-DEC-029, 032 | ARCH ALLOCATED |
| SYS-REQ-096 | ARCH-DEC-046, 039 | ARCH ALLOCATED |
| SYS-REQ-097 | ARCH-DEC-046, 029 | ARCH ALLOCATED |
| SYS-REQ-098 | ARCH-DEC-007, 025 | ARCH ALLOCATED |
| SYS-REQ-099 | ARCH-DEC-026, 031 | ARCH ALLOCATED |
| SYS-REQ-100 | ARCH-DEC-041, 024 | ARCH ALLOCATED |
| SYS-REQ-101 | ARCH-DEC-017, 029 | ARCH ALLOCATED |
| SYS-REQ-102 | ARCH-DEC-029, 041 | ARCH ALLOCATED |
| SYS-REQ-104 | ARCH-DEC-029, 041 | ARCH ALLOCATED |
| SYS-REQ-107 | ARCH-DEC-026, 027 | ARCH ALLOCATED |
| SYS-REQ-108 | ARCH-DEC-017, 029 | ARCH ALLOCATED |
| SYS-REQ-110 | Existing capability traceability | ARCH ALLOCATION REQUIRES DIRECT RECORD CHECK |
| SYS-REQ-111 | Existing capability traceability | ARCH ALLOCATION REQUIRES DIRECT RECORD CHECK |
| SYS-REQ-112 | Existing capability traceability | ARCH ALLOCATION REQUIRES DIRECT RECORD CHECK |

## 5. Safety requirement allocation audit

The repository contains controlled wording for `SAF-REQ-001…018`.

Current direct safety/hazard allocations established by the safety traceability record include:

`001→H-NAV-001`  
`003→H-DATA-001 / stale-data`  
`005→Safety Gate`  
`006→H-AUTH-001`  
`007→H-NAV-001/H-NAV-002`  
`008→H-RTE-001`  
`009→H-RET-001`  
`011→H-COL-001`  
`012→H-MUL-001`  
`013→H-C2-001`  
`014→H-DATA-001`  
`015→material state change / revalidation`  
`016→traceability control`  
`017→configuration control`  
`018→H-AI-001`

`SAF-REQ-002, 004, 010` remain explicitly present in the safety baseline but require the same direct hazard/objective allocation pass before closure.

**Important:** these are allocation links, not completed verification evidence.

## 6. Design / interface closure

Current repository state does not establish a complete one-to-one controlled Design/Interface allocation for every populated requirement.

Therefore:

`DESIGN/INTERFACE = PARTIAL / OPEN`

No requirement is marked closed merely from an architecture decision.

The next pass must extract existing interface/design records and bind only demonstrable allocations.

## 7. Verification closure

The repository establishes verification identities and methods, including existing `TEST-*` records and verification registers.

Current state:

`REQUIREMENT → VERIFICATION METHOD/CASE = SUBSTANTIAL`

`VERIFICATION CASE → RESULT = OPEN`

`RESULT → EVIDENCE = OPEN`

`EVIDENCE → CONFIGURATION = OPEN`

No `VERIFIED` status is assigned by this audit.

## 8. Source / wording closure

The Master Register explicitly requires exact controlled wording and source/basis information.

The current high-value register provides titles and architecture linkage, but the repository still requires a controlled line-by-line recovery of exact source wording for the authoritative requirement population before full source closure.

Status:

`SOURCE/BASIS = PARTIAL`

`EXACT REQUIREMENT WORDING RECONCILIATION = OPEN`

No wording is reconstructed from summaries.

## 9. Duplicate / candidate control

Candidate families such as:

`NAV-REQ-*`, `RTE-REQ-*`, `WP-REQ-*`, `MIS-REQ-*`, `RET-REQ-*`, `COL-REQ-*`, `C2-REQ-*`, `MUL-REQ-*`, `RDY-REQ-*`, `HMI-REQ-*`, `AI-REQ-*`

remain candidate/derived until exact overlap and source analysis is completed.

No candidate is promoted by this audit.

## 10. Immediate controlled gaps

1. Recover exact authoritative wording for populated SYS/SAF requirements where not already directly recorded.
2. Bind each populated requirement to its direct design/interface element where applicable.
3. Resolve the remaining direct hazard allocation gaps for SAF-REQ-002/004/010.
4. Extract exact existing TEST-* / verification-case relationships without creating duplicates.
5. Keep result/evidence/configuration open until actual controlled execution records exist.
6. Continue orphan/duplicate/contradiction detection.

## 11. Closure rule

A requirement may only progress toward `VERIFIED` after:

`SOURCE → REQUIREMENT → SAFETY → ARCHITECTURE → DESIGN/INTERFACE → VERIFICATION → RESULT → EVIDENCE → CONFIGURATION`

is demonstrably populated for the applicable scope.

## 12. Status

**WORKING — DIRECT ALLOCATION AUDIT COMPLETED FOR CURRENT EVIDENCE; TRACEABILITY CLOSURE REMAINS OPEN.**
