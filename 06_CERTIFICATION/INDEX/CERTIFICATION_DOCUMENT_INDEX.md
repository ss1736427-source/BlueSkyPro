---
id: CERTIFICATION-DOCUMENT-INDEX-001
type: certification_document_index
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Certification Document Index

## 1. Назначение

Определить контролируемый перечень документов, формирующих certification documentation BlueSky, и связать их с certification basis, requirements, compliance, safety, verification, evidence и configuration.

## 2. Основной принцип

Certification documentation должна обеспечивать контролируемую цепочку:

```text
Certification Basis
→ Certification Requirement
→ Compliance Record / MoC
→ Design / Safety / Requirements
→ Verification
→ Evidence
→ Configuration / Baseline
```

## 3. Document status

```text
PLANNED
DRAFT
UNDER_REVIEW
ACCEPTED
BASELINED
SUPERSEDED
CANCELLED
```

Статус документа не означает автоматически его acceptance authority.

## 4. Certification document inventory

| ID | Document | Primary purpose | Status |
|---|---|---|---|
| CERT-DOC-001 | CERTIFICATION_BASIS.md | Certification basis | EXISTING |
| CERT-DOC-002 | CERTIFICATION_STRATEGY.md | Certification strategy | EXISTING |
| CERT-DOC-003 | CERTIFICATION_OBJECT_AND_SCOPE.md | Certification object and scope | EXISTING |
| CERT-DOC-004 | CERTIFICATION_REQUIREMENTS_BASELINE.md | Certification requirements | EXISTING |
| CERT-DOC-005 | CERTIFICATION_DOCUMENTATION_MASTER_PLAN.md | Documentation planning | EXISTING |
| CERT-DOC-006 | COMPLIANCE_MATRIX.md | Compliance records and MoC | EXISTING |
| CERT-DOC-007 | SAFETY_CASE.md | Safety case | EXISTING |
| CERT-DOC-008 | SAFETY_REQUIREMENTS_BASELINE.md | Safety requirements | EXISTING |
| CERT-DOC-009 | SYSTEM_REQUIREMENTS_SPECIFICATION.md | System requirements | EXISTING |
| CERT-DOC-010 | SYSTEM_ARCHITECTURE_BASELINE.md | System architecture | EXISTING |
| CERT-DOC-011 | SYSTEM_DESIGN_DESCRIPTION.md | Detailed design | EXISTING |
| CERT-DOC-012 | INTERFACE_CONTROL_DOCUMENT.md | Controlled interfaces | EXISTING |
| CERT-DOC-013 | VERIFICATION_PLAN.md | Verification planning | EXISTING |
| CERT-DOC-014 | VERIFICATION_EVIDENCE_INDEX.md | Evidence index | EXISTING |
| CERT-DOC-015 | CHANGE_AND_PROBLEM_PROCESS.md | Change/problem control | EXISTING |
| CERT-DOC-016 | CONFIGURATION_BASELINE.md | Configuration baseline | EXISTING |

## 5. Required relationships

```text
Certification Basis
→ Certification Requirements
→ Compliance Matrix
→ Verification Plan
→ Verification Evidence
→ Configuration Baseline

Safety Requirements
→ Safety Case
→ Verification
→ Evidence

System Requirements
→ Architecture
→ Design
→ Interfaces
→ Verification
→ Evidence

Change / Problem
→ Impact Assessment
→ Affected Requirements / Design / Evidence
→ Re-verification
→ Baseline Update
```

## 6. Document control

Each certification document shall have controlled:

```text
Document ID
Version
Status
Owner
Configuration
Change Record
Approval / Acceptance
Effective Baseline
Superseded Version where applicable
```

## 7. Evidence linkage

Certification evidence shall not be considered controlled solely because a document references it. Evidence must be identifiable and linked to the applicable requirement, verification result and configuration.

```text
Requirement
→ Compliance Record
→ Verification Case
→ Result
→ Evidence ID
→ Configuration
```

## 8. Change impact

Changes affecting certification basis, certification requirements, compliance records, MoC, safety requirements, verification method, accepted evidence or configuration baseline require review through the Change and Problem Process.

If previously accepted evidence becomes incompatible with the changed configuration, it shall be marked INVALIDATED or SUPERSEDED and replaced or re-established as required.

## 9. Authority / acceptance

Authority acceptance requirements remain dependent on the applicable certification basis and established certification process. This index does not itself constitute authority approval.

## 10. Current gaps

```text
CERT-GAP-001  Complete certification deliverable inventory pending
CERT-GAP-002  Final document ownership pending
CERT-GAP-003  Final approval / acceptance matrix pending
CERT-GAP-004  Full certification-basis-to-document mapping pending
CERT-GAP-005  Full compliance-record-to-evidence mapping pending
CERT-GAP-006  Evidence completeness pending
CERT-GAP-007  Final baseline relationships pending
```

## 11. Next document

`SYSTEM_DESIGN_DESCRIPTION.md`

Он переводит architecture baseline в детальное design description с allocation компонентов, data structures, state machines и implementation boundaries.

## 12. Status

**DRAFT_FOR_AGREEMENT**
