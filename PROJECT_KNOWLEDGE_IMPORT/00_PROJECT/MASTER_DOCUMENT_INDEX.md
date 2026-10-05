---
id: MASTER-DOCUMENT-INDEX-001
type: master_document_index
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Master Document Index

## 1. Назначение

Единый реестр controlled documentation проекта BlueSky.

Этот индекс отвечает на вопрос:

```text
Какой документ существует?
Где он находится?
Каков его тип и статус?
На какой authoritative source / record он опирается?
Какие документы с ним связаны?
```

Индекс не заменяет содержание первичных документов.

## 2. Source hierarchy

```text
REGULATORY / OFFICIAL SOURCES
        ↓
CERTIFICATION BASIS
        ↓
PROJECT REQUIREMENTS
        ↓
SAFETY / HAZARD
        ↓
ARCHITECTURE
        ↓
DESIGN
        ↓
INTERFACES
        ↓
VERIFICATION
        ↓
EVIDENCE
        ↓
CONFIGURATION / CHANGE HISTORY
```

Для технических знаний:

```text
ORIGINAL SOURCE
→ SPECIALIZED KNOWLEDGE BASE
→ KNOWLEDGE MAP
→ ENGINEERING RULE
→ REQUIREMENT / DESIGN
```

## 3. Controlled document classes

| Class | Назначение |
|---|---|
| REG | Regulatory / official source |
| CERT | Certification |
| REQ | Requirements |
| SAF | Safety |
| ARCH | Architecture |
| DES | Design |
| IF | Interface |
| VER | Verification |
| EVD | Evidence |
| CFG | Configuration |
| CHG | Change / Problem |
| KB | Knowledge |
| OPS | Operational |

## 4. Master document register

| Doc ID | Document | Class | Status | Primary role | Notes |
|---|---|---|---|---|---|
| DOC-CERT-001 | CERTIFICATION_BASIS.md | CERT | DRAFT_FOR_AGREEMENT | Certification basis | Must be mapped to verified external sources |
| DOC-REG-001 | REGULATORY_SOURCE_REGISTER.md | REG | DRAFT_FOR_AGREEMENT | Source register | Official-source verification required |
| DOC-CERT-002 | COMPLIANCE_MATRIX.md | CERT | DRAFT_FOR_AGREEMENT | Compliance mapping | Clause-level mapping pending |
| DOC-CERT-003 | CERTIFICATION_REQUIREMENTS_BASELINE.md | CERT | DRAFT_FOR_AGREEMENT | Certification requirements | Depends on verified basis |
| DOC-SAF-001 | SAFETY_REQUIREMENTS_BASELINE.md | SAF | DRAFT_FOR_AGREEMENT | Safety requirements | Must link to hazard analysis |
| DOC-SAF-002 | HAZARD_LOG.md | SAF | DRAFT_FOR_AGREEMENT | Hazard register | Risk methodology pending |
| DOC-SAF-003 | SAFETY_CASE.md | SAF | DRAFT_FOR_AGREEMENT | Safety argument | Evidence/acceptance pending |
| DOC-REQ-001 | REQUIREMENTS_TRACEABILITY_MATRIX.md | REQ | DRAFT_FOR_AGREEMENT | Requirements traceability | Consolidation with existing SYS-REQ required |
| DOC-VER-001 | VERIFICATION_EVIDENCE_INDEX.md | VER | DRAFT_FOR_AGREEMENT | Evidence index | Existing TEST records must be linked |
| DOC-CFG-001 | CONFIGURATION_BASELINE.md | CFG | DRAFT_FOR_AGREEMENT | Configuration | Baseline process pending |
| DOC-CHG-001 | CHANGE_AND_PROBLEM_PROCESS.md | CHG | DRAFT_FOR_AGREEMENT | Change control | Process definition |
| DOC-CHG-002 | DOCUMENT_CONTROL_AND_BASELINE_PROCESS.md | CHG | DRAFT_FOR_AGREEMENT | Document control | Master process |
| DOC-IF-001 | INTERFACE_CONTROL_DOCUMENT.md | IF | DRAFT_FOR_AGREEMENT | Interface control | Allocation pending |
| DOC-REQ-002 | SYSTEM_REQUIREMENTS_SPECIFICATION.md | REQ | DRAFT_FOR_AGREEMENT | System requirements | Must merge with existing requirements |
| DOC-ARCH-001 | SYSTEM_ARCHITECTURE_BASELINE.md | ARCH | DRAFT_FOR_AGREEMENT | Architecture baseline | Must link existing ARCH-DEC records |
| DOC-DES-001 | SYSTEM_DESIGN_DESCRIPTION.md | DES | DRAFT_FOR_AGREEMENT | Detailed design | Allocation pending |
| DOC-VER-002 | VERIFICATION_PLAN.md | VER | DRAFT_FOR_AGREEMENT | Verification planning | Must link existing TEST records |
| DOC-KB-001 | Самолетовождение_Черный_Кораблин_1973_KNOWLEDGE_BASE.md | KB | CONTROLLED_SOURCE_RECORD | Specialized source knowledge | Derived from original source |
| DOC-KB-002 | Navigation Knowledge Map | KB | EXISTING_PROJECT_RECORD | Navigation knowledge navigation | Existing project artefact |

## 5. Existing project records

The following classes must be incorporated rather than duplicated:

```text
ARCH-DEC-*
SYS-REQ-*
TEST-*
Navigation Knowledge Map
Navigation Knowledge Review
Navigation State Model
Navigation Rules
Navigation Algorithm
Navigation Verification Model
Existing System Traceability
Existing design / safety records
```

Their exact inventory must be established from the project repository before final baseline.

## 6. Authoritative record rule

For each topic, exactly one record is designated authoritative at a given baseline.

Examples:

```text
Requirement wording
→ Master Requirements Register / controlled requirement record

Architecture decision
→ ARCH-DEC record

Hazard
→ HAZARD_LOG

Verification result
→ Verification Evidence Index + controlled evidence

Configuration
→ CONFIGURATION_BASELINE

Change
→ CHANGE REGISTER
```

Derived documents must link to the authoritative record.

## 7. Status model

```text
DRAFT
UNDER REVIEW
AGREED
BASELINED
SUPERSEDED
ARCHIVED
```

Regulatory/authority acceptance is recorded separately.

## 8. Duplicate control

Before adding a document:

```text
Exact filename check
→ Title check
→ Type check
→ ID check
→ Topic check
→ Existing record check
```

If equivalent:

```text
UPDATE / LINK EXISTING RECORD
```

not:

```text
CREATE DUPLICATE
```

## 9. Document change control

Any material change requires:

```text
Document
→ Change ID
→ Impact Assessment
→ Review
→ Verification if required
→ Baseline revision
```

## 10. Obsidian

Obsidian is the navigation/knowledge presentation layer.

Recommended:

```text
00_PROJECT
├── Master Document Index
├── Master Requirements Register
├── Source Register
├── Hazard Log
├── Traceability
├── Verification
├── Configuration
└── Change Control
```

Obsidian notes should link to controlled records rather than become a second independent source of truth.

## 11. Current gaps

```text
MDI-GAP-001 Exact repository inventory not imported into master index
MDI-GAP-002 Existing ARCH-DEC inventory requires linkage
MDI-GAP-003 Existing SYS-REQ inventory requires consolidation
MDI-GAP-004 Existing TEST inventory requires linkage
MDI-GAP-005 Final authority/status assignments pending
MDI-GAP-006 Official regulatory source verification pending
```

## 12. Baseline rule

This index becomes the entry point for certification documentation once reviewed.

**Status: DRAFT_FOR_AGREEMENT**



