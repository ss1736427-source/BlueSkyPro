---
id: REQUIREMENTS-TRACEABILITY-MATRIX-001
type: requirements_traceability_matrix
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
authority: MASTER-REQUIREMENTS-REGISTER-001
---

# Requirements Traceability Matrix

## 1. Назначение

Единая controlled matrix связей между источниками, требованиями, safety, architecture, design, interfaces, verification, evidence и configuration.

Идентичность requirement определяется только:

`MASTER-REQUIREMENTS-REGISTER.md`

Эта матрица не создаёт вторую систему IDs.

## 2. Target traceability chain

```text
SOURCE
  ↓
REQUIREMENT
  ↓
SAFETY / HAZARD
  ↓
ARCHITECTURE DECISION
  ↓
DESIGN
  ↓
INTERFACE
  ↓
VERIFICATION
  ↓
EVIDENCE
  ↓
CONFIGURATION
```

Для certification:

```text
OFFICIAL SOURCE
→ CLAUSE
→ APPLICABILITY
→ CERTIFICATION REQUIREMENT
→ SYSTEM REQUIREMENT
→ COMPLIANCE METHOD
→ EVIDENCE
```

## 3. Traceability statuses

```text
COVERED
PARTIAL
NOT_COVERED
DEFERRED
TBD
NOT_APPLICABLE
CONFLICT
```

`COVERED` означает наличие подтверждённой связи, а не автоматически `VERIFIED`.

## 4. Authoritative sources

| Record | Role |
|---|---|
| MASTER_REQUIREMENTS_REGISTER.md | Requirement identity |
| CERTIFICATION_BASIS.md | Certification basis |
| REGULATORY_SOURCE_REGISTER.md | External/regulatory source register |
| SAFETY_REQUIREMENTS_BASELINE.md | Safety requirement presentation |
| HAZARD_LOG.md | Hazard identity |
| SYSTEM_ARCHITECTURE_BASELINE.md | Architecture baseline |
| SYSTEM_DESIGN_DESCRIPTION.md | Design description |
| INTERFACE_CONTROL_DOCUMENT.md | Interface control |
| VERIFICATION_PLAN.md | Verification planning |
| VERIFICATION_EVIDENCE_INDEX.md | Evidence index |
| CONFIGURATION_BASELINE.md | Configuration |
| CHANGE_AND_PROBLEM_PROCESS.md | Change/problem control |

## 5. Existing requirement identity

Existing IDs are preserved.

Known system requirement groups:

```text
SYS-REQ-001..003
SYS-REQ-009..023
SYS-REQ-025..030
SYS-REQ-032..033
SYS-REQ-035
SYS-REQ-067..078
SYS-REQ-080..104
SYS-REQ-107..108
SYS-REQ-110..112
```

This is an inventory of known records, not a claim that every number in each range exists.

## 6. Existing high-value traceability

| Requirement | Architecture | Status | Verification |
|---|---|---|---|
| SYS-REQ-080 | ARCH-DEC-008, 012, 039 | EXISTING LINK | Existing TEST-* linkage to be consolidated |
| SYS-REQ-081 | ARCH-DEC-017, 027, 040 | EXISTING LINK | Existing TEST-* linkage to be consolidated |
| SYS-REQ-082 | ARCH-DEC-016, 017, 038 | EXISTING LINK | Existing TEST-* linkage to be consolidated |
| SYS-REQ-083 | ARCH-DEC-003, 012 | PARTIAL | Existing TEST-* linkage to be checked |
| SYS-REQ-084 | ARCH-DEC-036, 037 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-085 | ARCH-DEC-007, 016, 036 | PARTIAL | Existing TEST-* linkage to be checked |
| SYS-REQ-086 | ARCH-DEC-017, 023, 039 | PARTIAL | Existing TEST-* linkage to be checked |
| SYS-REQ-087 | ARCH-DEC-022, 023, 035 | PARTIAL | Existing TEST-* linkage to be checked |
| SYS-REQ-088 | ARCH-DEC-011, 025, 026 | PARTIAL | Existing TEST-* linkage to be checked |
| SYS-REQ-089 | ARCH-DEC-022, 024, 032 | PARTIAL | Existing TEST-* linkage to be checked |
| SYS-REQ-090 | ARCH-DEC-022, 036 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-091 | ARCH-DEC-022 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-092 | ARCH-DEC-027, 040 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-093 | ARCH-DEC-017, 036 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-094 | ARCH-DEC-029, 041 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-095 | ARCH-DEC-029, 032 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-096 | ARCH-DEC-046, 039 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-097 | ARCH-DEC-046, 029 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-098 | ARCH-DEC-007, 025 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-099 | ARCH-DEC-026, 031 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-100 | ARCH-DEC-041, 024 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-101 | ARCH-DEC-017, 029 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-102 | ARCH-DEC-029, 041 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-104 | ARCH-DEC-029, 041 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-107 | ARCH-DEC-026, 027 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-108 | ARCH-DEC-017, 029 | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-110 | Existing capability traceability | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-111 | Existing capability traceability | EXISTING LINK | Existing TEST-* linkage to be checked |
| SYS-REQ-112 | Existing capability traceability | EXISTING LINK | Existing TEST-* linkage to be checked |

## 7. Safety Gate traceability

Authoritative existing chain:

```text
SYS-REQ-082
SYS-REQ-085
     ↓
ARCH-DEC-007
ARCH-DEC-016
ARCH-DEC-017
     ↓
Safety Gate
     ↓
Operator Approval
     ↓
Execution
```

Candidate records:

```text
SAF-REQ-019
SAF-REQ-020
AUTH-REQ-001
AUTH-REQ-002
AI-REQ-002
```

Status:

```text
TBD / DERIVED CANDIDATE
```

They must not be presented as independent baseline requirements until overlap analysis is completed.

## 8. AI / learning traceability

```text
SYS-REQ-087
SYS-REQ-088
SYS-REQ-094..104
SYS-REQ-107
SYS-REQ-108
        ↓
ARCH-DEC-011
ARCH-DEC-022
ARCH-DEC-025
ARCH-DEC-026
ARCH-DEC-029
ARCH-DEC-032
ARCH-DEC-035
ARCH-DEC-041
ARCH-DEC-046
```

Candidate:

```text
AI-REQ-001
AI-REQ-002
AI-REQ-003
```

Status:

```text
PARTIAL / OVERLAP REVIEW
```

## 9. Resource / performance traceability

```text
SYS-REQ-084
SYS-REQ-089
SYS-REQ-090
SYS-REQ-091
SYS-REQ-092
SYS-REQ-093
        ↓
ARCH-DEC-022
ARCH-DEC-024
ARCH-DEC-036
ARCH-DEC-037
ARCH-DEC-040
ARCH-DEC-044
```

New generic resource requirements must not duplicate this coverage.

## 10. Degradation / recovery traceability

```text
SYS-REQ-081
SYS-REQ-086
SYS-REQ-093
        ↓
ARCH-DEC-017
ARCH-DEC-023
ARCH-DEC-027
ARCH-DEC-039
ARCH-DEC-040
```

Candidates:

```text
C2-REQ-002
C2-REQ-003
RET-REQ-004
```

Status:

```text
PARTIAL / WORDING AND SCOPE REVIEW
```

## 11. Navigation traceability

Candidate:

```text
NAV-REQ-001..009
```

Target:

```text
Original Source
→ Specialized Knowledge Base
→ Navigation Knowledge Map
→ Engineering Rule
→ NAV Requirement
→ Design
→ Verification
```

Regulatory chain remains separate:

```text
Official Russian Source
→ Clause
→ Applicability
→ Requirement
```

The 1973 navigation source is not automatically a regulatory source.

## 12. Route / waypoint / mission traceability

Candidates:

```text
RTE-REQ-001..004
WP-REQ-001..003
MIS-REQ-001..003
```

Status:

```text
TBD
```

Required links:

```text
Requirement
→ Existing project records
→ Architecture
→ Design
→ TEST-*
→ Evidence
```

## 13. Dynamic Return traceability

Candidates:

```text
RET-REQ-001..004
```

Target chain:

```text
Validated State
→ Return Feasibility
→ Return Route
→ Safety Validation
→ Authorization / Operator Approval
→ Execution
→ Verification
```

Quantitative thresholds remain `TBD` until established and approved.

## 14. Collision avoidance traceability

Candidates:

```text
COL-REQ-001..004
```

Target chain:

```text
Track
→ Prediction
→ Conflict Detection
→ Separation Assessment
→ Avoidance Proposal
→ Authority
→ Execution
→ Verification
```

Exact separation criteria remain `TBD`.

## 15. C2 / Multi-UAV traceability

Candidates:

```text
C2-REQ-001..003
MUL-REQ-001..003
```

Required links:

```text
Requirement
→ Interface
→ Architecture
→ Failure Behaviour
→ Verification
→ Evidence
```

Status:

```text
TBD
```

## 16. Readiness / HMI / Data / Configuration

Candidates:

```text
RDY-REQ-001..002
HMI-REQ-001..002
DATA-REQ-001..002
CFG-REQ-001..003
```

Status:

```text
TBD / CONSOLIDATION PENDING
```

Readiness must remain distinct from Safety Gate and operator approval.

## 17. Software / hardware

Candidates:

```text
SW-REQ-001..003
HW-REQ-001..002
```

Status:

```text
TBD
```

Final assurance obligations depend on approved certification basis and allocation.

## 18. Verification traceability

Existing:

```text
TEST-*
```

are preserved.

Rule:

```text
Requirement
→ Verification Method
→ Existing TEST-*
→ Result
→ Evidence
→ Configuration
```

If an equivalent TEST already exists:

```text
LINK
```

If no coverage exists:

```text
NEW TEST CASE
```

A requirement is not `VERIFIED` merely because an architecture decision exists.

## 19. Evidence traceability

Target:

```text
Requirement
→ Verification Case
→ Result
→ Evidence ID
→ Configuration
```

Evidence must identify the configuration under which it was produced.

## 20. Configuration traceability

Target:

```text
Requirement
→ Design
→ Build / Configuration
→ Verification
→ Evidence
```

Material changes require:

```text
CHG
→ Impact Assessment
→ Affected Requirements
→ Affected Verification
→ Baseline Update
```

## 21. Certification traceability

No certification claim is considered complete without:

```text
Official Source
→ Clause
→ Applicability
→ Requirement
→ Compliance Method
→ Verification
→ Evidence
```

Current state:

```text
CERTIFICATION CLAUSE MAPPING = PENDING
```

Therefore certification-related rows remain `TBD`/`PARTIAL` until verified.

## 22. Traceability quality gates

Before a requirement is baselined:

```text
[ ] Requirement ID exists in Master Register
[ ] Source/basis identified
[ ] Duplicate check completed
[ ] Safety impact assessed
[ ] Architecture allocation identified
[ ] Design allocation identified where applicable
[ ] Interface identified where applicable
[ ] Verification method identified
[ ] Evidence path defined
[ ] Configuration path defined
```

Before `VERIFIED`:

```text
[ ] Verification executed
[ ] Result recorded
[ ] Evidence controlled
[ ] Configuration identified
[ ] Review completed
```

## 23. Orphan detection

The following are traceability defects:

```text
Requirement without source/basis
Requirement without architecture allocation
Requirement without verification method
Verification without requirement/claim
Evidence without verification case
Safety requirement without hazard/objective
Certification requirement without source clause
Design element without requirement allocation where required
```

## 24. Conflict handling

If two records conflict:

```text
Preserve both
→ identify authority
→ compare scope
→ create CHG / problem record
→ agree resolution
→ update authoritative record
→ update affected derived records
→ repeat traceability check
```

No silent overwrite.

## 25. Current reconciliation state

```text
Requirement identity: MASTER REGISTER
SRS role: DERIVED SPECIFICATION
Architecture decisions: EXISTING RECORDS PRESERVED
Verification TEST-* : EXISTING RECORDS PRESERVED

Existing requirement ↔ architecture:
PARTIALLY CONSOLIDATED

Requirement ↔ verification:
PENDING FULL EXTRACTION

Requirement ↔ evidence:
PENDING FULL EXTRACTION

Requirement ↔ official regulatory clauses:
PENDING

Navigation knowledge ↔ requirements:
PENDING DOMAIN RECONCILIATION
```

## 26. Immediate next operation

The next operation is factual repository reconciliation:

```text
1. Extract exact SYS-REQ text.
2. Extract exact SAF-REQ text.
3. Extract exact ARCH-DEC relationships.
4. Extract exact TEST-* relationships.
5. Compare candidate SRS requirements.
6. Mark KEEP / DERIVED / MERGE / GAP / CONFLICT.
7. Update Master Requirements Register.
8. Update this matrix.
9. Update SRS only where affected.
10. Baseline after review.
```

## 27. Status

**DRAFT_FOR_AGREEMENT**



