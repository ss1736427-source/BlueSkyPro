---
id: MASTER-DOCUMENT-INDEX-001
type: master_document_index
status: CONTROLLED_WORKING_INDEX
jurisdiction: Russian Federation
system: aviation_system
---

# Master Document Index

## 1. Назначение

Единый реестр controlled documentation проекта BlueSky PRO.

Индекс отвечает на вопросы:

```text
Какой документ существует?
Где он находится?
Каков его тип и статус?
На какой authoritative source / record он опирается?
Какие документы с ним связаны?
```

Индекс не заменяет содержание первичных документов.

## 2. Repository structure

### 2.1 Фактические верхнеуровневые каталоги, присутствующие в main

Проверено по Git tree на 2026-09-20:

```text
00_PROJECT/
01_ARCHITECTURE/
01_NORMATIVE/
01_REQUIREMENTS/
02_ARCHITECTURE/
02_SYSTEM/
02_SYSTEM_DESIGN/
03_FLEET/
03_SYSTEM/
04_INTEGRATION/
04_SOFTWARE/
05_C2/
05_VERIFICATION/
06_CERTIFICATION/
06_EQUIPMENT/
07_ATM_REGULATORY/
07_DATA/
08_HMI/
08_PLANNING/
09_VERIFICATION/
10_TRACEABILITY/
15_CERTIFICATION/
90_TEMPLATES/
91_REGISTRIES/
BlueSky_Project/
PROJECT_KNOWLEDGE_IMPORT/
TRACEABILITY/
_LEGACY/
_SOURCE_ARCHIVE/
tools/
Скрины/
```

### 2.2 Назначение уровней

Каталоги выше являются фактическими элементами текущего `main` tree. Они не все имеют одинаковый authority level.

- `00_PROJECT` — управление проектом, решения, configuration/control, master indexes и project rules.
- `01_REQUIREMENTS` — requirements, normative, safety, compliance и traceability.
- `01_ARCHITECTURE` / `02_ARCHITECTURE` — архитектурные и интерфейсные материалы разных исторических/организационных слоёв; authority определяется конкретным controlled record.
- `02_SYSTEM` / `02_SYSTEM_DESIGN` / `03_SYSTEM` — system architecture, detailed design и functional modules.
- `03_FLEET` — fleet/adapter/vehicle configuration records.
- `04_INTEGRATION` — integration material.
- `04_SOFTWARE` — software implementation, tests и software engineering records.
- `05_C2` — C2 working material.
- `05_VERIFICATION` / `09_VERIFICATION` — verification material; authority определяется controlled verification record.
- `06_CERTIFICATION` / `15_CERTIFICATION` — certification material разных рабочих слоёв; final authority определяется certification control records.
- `06_EQUIPMENT` — equipment material.
- `07_ATM_REGULATORY` — ATM/regulatory working material.
- `07_DATA` — data, algorithms and capability records.
- `08_HMI` — HMI, UX, design system and HMI allocation.
- `08_PLANNING` — planning/optimization records.
- `10_TRACEABILITY` / `TRACEABILITY` — traceability and reconciliation records.
- `90_TEMPLATES` — templates.
- `91_REGISTRIES` — registries.
- `PROJECT_KNOWLEDGE_IMPORT`, `BlueSky_Project`, `_LEGACY`, `_SOURCE_ARCHIVE`, `Скрины` — imported, historical, source/archive or supporting material unless a specific controlled record explicitly promotes an item.
- `tools` — repository tooling.

`07_EXTERNAL` is **not present** in the current `main` tree and must not be referenced as an existing directory without a future creation/change record.

## 3. Source hierarchy

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

## 4. Current primary records

| Doc ID | Document | Class | Current path |
|---|---|---|---|
| DOC-CERT-001 | CERTIFICATION_BASIS.md | CERT | 06_CERTIFICATION/BASIS/ |
| DOC-CERT-002 | CERTIFICATION_BASIS_WORKING_REGISTER.md | CERT | 06_CERTIFICATION/BASIS/ |
| DOC-CERT-003 | CERTIFICATION_DOCUMENTATION_MASTER_PLAN.md | CERT | 06_CERTIFICATION/MASTER_PLAN/ |
| DOC-CERT-004 | CERTIFICATION_DOCUMENT_INDEX.md | CERT | 06_CERTIFICATION/INDEX/ |
| DOC-CERT-005 | CERTIFICATION_OBJECT_AND_SCOPE.md | CERT | 06_CERTIFICATION/SCOPE/ |
| DOC-CERT-006 | CERTIFICATION_REQUIREMENTS_BASELINE.md | CERT | 06_CERTIFICATION/REQUIREMENTS/ |
| DOC-CERT-007 | CERTIFICATION_STRATEGY.md | CERT | 06_CERTIFICATION/STRATEGY/ |
| DOC-REQ-001 | MASTER_REQUIREMENTS_REGISTER.md | REQ | 01_REQUIREMENTS/SYSTEM/ |
| DOC-REQ-002 | SYSTEM_REQUIREMENTS_SPECIFICATION.md | REQ | 01_REQUIREMENTS/SYSTEM/ |
| DOC-REQ-003 | REQUIREMENTS_INVENTORY.md | REQ | 01_REQUIREMENTS/SYSTEM/ |
| DOC-REQ-004 | REQUIREMENTS_INVENTORY.csv | REQ | 01_REQUIREMENTS/SYSTEM/ |
| DOC-REQ-005 | REQUIREMENTS_TRACEABILITY_MATRIX.md | REQ | 01_REQUIREMENTS/TRACEABILITY/ |
| DOC-REQ-006 | REQUIREMENTS_RECONCILIATION.md | REQ | 01_REQUIREMENTS/TRACEABILITY/ |
| DOC-REQ-007 | REQUIREMENTS_RECONCILIATION.csv | REQ | 01_REQUIREMENTS/TRACEABILITY/ |
| DOC-REQ-008 | REQUIREMENTS_RECONCILIATION_DECISION.md | REQ | 01_REQUIREMENTS/TRACEABILITY/ |
| DOC-REQ-009 | REQUIREMENTS_CONSOLIDATION_REPORT.md | REQ | 01_REQUIREMENTS/TRACEABILITY/ |
| DOC-REG-001 | REGULATORY_KNOWLEDGE_BASE.md | REG | 01_REQUIREMENTS/REGULATORY/ |
| DOC-REG-002 | REGULATORY_SOURCE_REGISTER.md | REG | 01_REQUIREMENTS/REGULATORY/ |
| DOC-REG-003 | REGULATORY_CLAUSE_MAPPING_001.md | REG | 01_REQUIREMENTS/REGULATORY/ |
| DOC-SAF-001 | SAFETY_REQUIREMENTS_BASELINE.md | SAF | 01_REQUIREMENTS/SAFETY/ |
| DOC-SAF-002 | SAFETY_CASE.md | SAF | 01_REQUIREMENTS/SAFETY/ |
| DOC-SAF-003 | HAZARD_LOG.md | SAF | 01_REQUIREMENTS/SAFETY/ |
| DOC-ARCH-001 | AVIATION_SYSTEM_DEFINITION.md | ARCH | 02_ARCHITECTURE/SYSTEM/ |
| DOC-ARCH-002 | EXTERNAL_PROCESSING_INTEGRATION_ARCHITECTURE_001.md | ARCH | 02_ARCHITECTURE/SYSTEM/ |
| DOC-ARCH-003 | EXTERNAL_MAINTENANCE_SYSTEM_INTEGRATION_ARCHITECTURE_001.md | ARCH | 02_ARCHITECTURE/SYSTEM/ |
| DOC-ARCH-004 | EXTERNAL_MAINTENANCE_INTEGRATION_PRINCIPLE_001.md | ARCH | 02_ARCHITECTURE/SYSTEM/ |
| DOC-ARCH-005 | EXTERNAL_MAINTENANCE_INTEGRATION_DESIGN_001.md | ARCH | 02_ARCHITECTURE/SYSTEM/ |
| DOC-ARCH-006 | EXTERNAL_MAINTENANCE_DATA_EXCHANGE_CONTRACT_001.md | ARCH | 02_ARCHITECTURE/SYSTEM/ |
| DOC-ARCH-007 | BLUESKY_PRO_UX_MINIMAL_INFORMATION_PRINCIPLE_001.md | UX | 02_ARCHITECTURE/UX/ |
| DOC-ARCH-008 | ADMINISTRATOR_DATA_INTEGRATION_ACCESS_MODEL_001.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-009 | ADMINISTRATOR_ENGINEER_TECHNICIAN_AUTHORITY_MODEL_001.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-010 | ADMINISTRATOR_ENGINEER_TECHNICIAN_ACCESS_RULE_002.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-011 | ADMINISTRATOR_ENGINEER_TECHNICIAN_ACCESS_RULE_003.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-012 | ADMINISTRATOR_ROLE_CATALOG_001.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-013 | ADMINISTRATOR_USERS_ROLES_CAPABILITIES_001.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-014 | ADMINISTRATOR_MENU_AND_ROLE_MODEL_001.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-015 | ADMINISTRATOR_MENU_STRUCTURE_001.md | ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-ARCH-016 | ADMINISTRATOR_USER_ROLE_ASSIGNMENT_UI_001.md | HMI/ARCH | 02_ARCHITECTURE/ADMINISTRATOR/ |
| DOC-IF-001 | C2_ICD_BASELINE_001.md | IF | 02_ARCHITECTURE/INTERFACES/C2/ |
| DOC-VER-001 | VERIFICATION_PLAN.md | VER | 05_VERIFICATION/PLAN/ |
| DOC-VER-002 | VERIFICATION_EVIDENCE_INDEX.md | EVD | 05_VERIFICATION/EVIDENCE/ |
| DOC-VER-003 | C2_VERIFICATION_CASES_001.md | VER/C2 | 05_VERIFICATION/C2/ |
| DOC-VER-004 | C2_VERIFICATION_CONFIGURATION_001.md | CFG/VER/C2 | 05_VERIFICATION/C2/ |
| DOC-VER-005 | C2_VERIFICATION_CONTROL_CLOSURE_001.md | CTRL/VER/C2 | 05_VERIFICATION/C2/ |
| DOC-VER-006 | C2_EXECUTION_PREREQUISITE_CHECK_001.md | CTRL/VER/C2 | 05_VERIFICATION/C2/ |
| DOC-VER-007 | C2_VERIFICATION_EXECUTION_RESULT_001.md | VER/RESULT/C2 | 05_VERIFICATION/C2/ |
| DOC-VER-008 | C2_EVIDENCE_INDEX_001.md | EVD/C2 | 05_VERIFICATION/C2/ |
| DOC-VER-009 | C2_QUANTITATIVE_PARAMETERS_BASIS_001.md | BASIS/VER/C2 | 05_VERIFICATION/C2/ |
| DOC-KB-001 | Самолетовождение_Черный_Кораблин_1973_KNOWLEDGE_BASE.md | KB | 00_PROJECT/ |
| DOC-KB-002 | Navigation_Knowledge_Map.md | KB | 03_SYSTEM/NAVIGATION/ |
| DOC-NAV-001 | NAVIGATION_DOCUMENT_MAP.md | MAP | 03_SYSTEM/NAVIGATION/ |
| DOC-NAV-002 | NAVIGATION_BASELINE_RECONCILIATION.md | RECON | 03_SYSTEM/NAVIGATION/ |
| DOC-NAV-003 | NAVIGATION_POSITION_MODULE_SPECIFICATION.md | MODULE | 03_SYSTEM/NAVIGATION/ |
| DOC-NAV-004 | NAVIGATION_VELOCITY_SPEED_MODULE_SPECIFICATION.md | MODULE | 03_SYSTEM/NAVIGATION/ |
| DOC-NAV-005 | NAVIGATION_HEADING_MODULE_SPECIFICATION.md | MODULE | 03_SYSTEM/NAVIGATION/ |
| DOC-NAV-006 | NAVIGATION_TRACK_COURSE_MODULE_SPECIFICATION.md | MODULE | 03_SYSTEM/NAVIGATION/ |
| DOC-NAV-007 | NAVIGATION_WIND_MODULE_SPECIFICATION.md | MODULE | 03_SYSTEM/NAVIGATION/ |

## 5. Project governance set

```text
00_PROJECT/GOVERNANCE/
├── BLUE_SKY_DECISIONS_LOG.md
├── BLUE_SKY_DOCUMENT_CONTROL_MAP.md
├── BLUE_SKY_PROJECT_STATE.md
├── BLUE_SKY_WORK_PLAN.md
├── CHANGELOG.md
├── CHANGE_AND_PROBLEM_PROCESS.md
├── DECISIONS.md
├── DOCUMENT_CONTROL_AND_BASELINE_PROCESS.md
├── MASTER_DOCUMENT_INDEX.md
├── PROJECT_RULES.md
├── PROJECT_WORK_ORDER.md
├── SOURCE_INDEX.md
└── TERMINOLOGY.md
```

## 6. Project configuration and knowledge

```text
00_PROJECT/CONFIGURATION/
├── CONFIGURATION_A_APPLICABILITY_MATRIX.md
├── CONFIGURATION_BASELINE.md
└── CONFIGURATION_ITEM_REGISTER.md

00_PROJECT/KNOWLEDGE/
├── PROJECT_KNOWLEDGE_BASE.md
└── NAVIGATION_KNOWLEDGE_REVIEW_001.md
```

## 7. Navigation working set

```text
03_SYSTEM/NAVIGATION/
├── NAVIGATION_DOCUMENT_MAP.md
├── NAVIGATION_BASELINE_RECONCILIATION.md
├── NAVIGATION_ALGORITHM.md
├── NAVIGATION_MATHEMATICAL_SPECIFICATION_001.md
├── NAVIGATION_POSITION_MODULE_SPECIFICATION.md
├── NAVIGATION_RULES.md
├── NAVIGATION_STATE_MODEL.md
├── NAVIGATION_TRACK_COURSE_MODULE_SPECIFICATION.md
├── NAVIGATION_VELOCITY_SPEED_MODULE_SPECIFICATION.md
├── NAVIGATION_WIND_MODULE_SPECIFICATION.md
├── NAVIGATION_HEADING_MODULE_SPECIFICATION.md
└── Navigation_Knowledge_Map.md

05_VERIFICATION/NAVIGATION/
├── NAVIGATION_TRACEABILITY_MATRIX_001.md
├── NAVIGATION_VERIFICATION_MODEL.md
└── TEST_VECTORS/
    └── NAVIGATION_TEST_VECTORS_001.md
```

Navigation materials are kept separated by role: system/module design, project knowledge/review, and verification. The Document Map and Baseline Reconciliation documents provide the cross-document linkage; they do not replace authoritative requirements, architecture or verification records.

## 8. C2 working set

C2 materials are separated by engineering role:

```text
01_REQUIREMENTS/REGULATORY/C2/
01_REQUIREMENTS/SYSTEM/C2/
01_REQUIREMENTS/SAFETY/C2/
01_REQUIREMENTS/TRACEABILITY/C2/
02_ARCHITECTURE/INTERFACES/C2/
05_VERIFICATION/
└── C2/
    ├── C2_VERIFICATION_CASES_001.md
    ├── C2_VERIFICATION_CONFIGURATION_001.md
    ├── C2_VERIFICATION_CONTROL_CLOSURE_001.md
    ├── C2_CONTROLLED_BASELINE_STATUS_001.md
    ├── C2_GAP_CLOSURE_PASS_001.md
    ├── C2_RECONCILIATION_PASS_002.md
    ├── C2_RECONCILIATION_PASS_003.md
    ├── C2_EXECUTION_PREREQUISITE_CHECK_001.md
    ├── C2_VERIFICATION_EXECUTION_RESULT_001.md
    ├── C2_EVIDENCE_INDEX_001.md
    └── C2_QUANTITATIVE_PARAMETERS_BASIS_001.md
08_HMI/C2/
```

The original controlled file contents are preserved; structural cleanup changes their repository location only.

## 9. HMI working set

```text
08_HMI/
└── C2/
    └── C2_HMI_ALLOCATION_001.md
```

Broken/empty AI interface artifact removed during cleanup. No valid source content was discarded.

## 10. Authoritative record rule

For each topic, exactly one record is designated authoritative at a given baseline.

Examples:

```text
Requirement wording
→ Master Requirements Register / controlled requirement record

Architecture decision
→ architecture controlled record

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

## 11. Duplicate control

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

## 12. Archive rule

Only valid historical/repair materials are retained under:

```text
99_ARCHIVE/REPAIR_BACKUPS/
```

They are not working documents and must not be used as authoritative sources. Broken empty artifacts are removed rather than retained.

## 13. Obsidian

Obsidian remains the navigation/knowledge presentation layer over the same repository mirror.

There is no second independent documentation source. GitHub/main remains the controlled repository; Obsidian presents the synchronized working tree.

## 14. Cleanup status

Current structural cleanup pass:

```text
[✓] Requirements separated
[✓] Regulatory material separated
[✓] Safety material separated
[✓] Architecture separated
[✓] Interface control separated
[✓] Navigation system materials separated
[✓] Navigation verification materials separated
[✓] C2 materials separated by role
[✓] Certification materials separated
[✓] Project governance grouped
[✓] Project configuration grouped
[✓] Project knowledge grouped
[✓] Broken empty AI artifact removed
[✓] Obsolete .bak artifact removed from working tree
[✓] Master index paths updated
```

Structural changes preserve Git history. No valid project source was intentionally deleted in this pass.

## 15. Mandatory document integration rule

Every new document must be integrated into this structure before it is considered part of the project:

```text
CHECK EXISTING BASE
→ DETERMINE NEED
→ ASSIGN ID / NUMBER
→ ASSIGN TYPE / STATUS / VERSION
→ PLACE IN CORRECT LAYER
→ LINK SOURCES / REQUIREMENTS / ARCHITECTURE
→ LINK HMI / INTERFACES / VERIFICATION / CERTIFICATION AS APPLICABLE
→ UPDATE INDEX / TRACEABILITY
→ VERIFY INTEGRATION
→ COMMIT
```

## 16. Next rule

Do not create another parallel document when an existing controlled record can be updated.

For the current development sequence, work proceeds from the existing architecture into one block and one module at a time, with the corresponding requirements, design, HMI, verification and technical documentation maintained in their designated layers.

**Status: CONTROLLED WORKING INDEX — structure verified 2026-09-20; content coverage remains subject to ongoing reconciliation.**


## 17. Latest repository-wide documentation audit

`TRACEABILITY/REPOSITORY_DOCUMENTATION_AUDIT_REPORT_004.md` is the current repository-wide reconciliation update. Audit 003 remains preserved as historical evidence; Audit 004 is the current interpretation of repository state at HEAD `d020995c25838f25cac767d1fff481cb80ca8e6a`.
