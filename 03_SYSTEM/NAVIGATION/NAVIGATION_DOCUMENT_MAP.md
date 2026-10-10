---
id: NAVIGATION-DOCUMENT-MAP-001
type: controlled_document_map
status: working_baseline
version: 0.4
parent: NAVIGATION
---

# BlueSky PRO — Navigation Document Map

## 1. Назначение

Единая карта существующей документации блока Navigation. Не заменяет первичные документы и не создаёт второй authority требований.

## 2. Authority chain

```text
01_REQUIREMENTS/REGULATORY/
        ↓
06_CERTIFICATION/
        ↓
01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md
        ↓
01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md
        ↓
02_ARCHITECTURE/SYSTEM/AVIATION_SYSTEM_DEFINITION.md
        ↓
03_SYSTEM/NAVIGATION/
        ↓
08_HMI/ + 02_ARCHITECTURE/INTERFACES/
        ↓
05_VERIFICATION/NAVIGATION/
        ↓
05_VERIFICATION/EVIDENCE/
```

Нижний уровень не становится authoritative только потому, что создан позднее.

## 3. Navigation records

### Core engineering

| ID | Record | Role |
|---|---|---|
| NAV-DOC-001 | `NAVIGATION_STATE_MODEL.md` | common state semantics |
| NAV-DOC-002 | `NAVIGATION_RULES.md` | engineering rules |
| NAV-DOC-003 | `NAVIGATION_ALGORITHM.md` | algorithmic logic |
| NAV-DOC-004 | `NAVIGATION_MATHEMATICAL_SPECIFICATION_001.md` | mathematical basis |
| NAV-DOC-005 | `Navigation_Knowledge_Map.md` | knowledge relationships |
| NAV-DOC-006 | `NAVIGATION_BASELINE_RECONCILIATION.md` | consolidation/reconciliation |
| NAV-DOC-007 | `NAVIGATION_DOCUMENT_MAP.md` | controlled document map |

### Module specifications

| ID | Module | Record | State |
|---|---|---|---|
| NAV-MOD-001 | Position | `NAVIGATION_POSITION_MODULE_SPECIFICATION.md` | RECONCILIATION |
| NAV-MOD-002 | Velocity / Speed | `NAVIGATION_VELOCITY_SPEED_MODULE_SPECIFICATION.md` | RECONCILIATION |
| NAV-MOD-003 | Heading | `NAVIGATION_HEADING_MODULE_SPECIFICATION.md` | RECONCILIATION |
| NAV-MOD-004 | Track / Course | `NAVIGATION_TRACK_COURSE_MODULE_SPECIFICATION.md` | RECONCILIATION |
| NAV-MOD-005 | Wind | `NAVIGATION_WIND_MODULE_SPECIFICATION.md` | RECONCILIATION |

Module specifications are derived design records. They must reference the existing authoritative records and must not silently create new requirements or architecture.

## 4. Requirements linkage

Authoritative system requirement record:

`01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md`

System requirements specification:

`01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md`

Safety requirements baseline:

`01_REQUIREMENTS/SAFETY/SAFETY_REQUIREMENTS_BASELINE.md`

Navigation allocation record:

`05_VERIFICATION/NAVIGATION/NAVIGATION_REQUIREMENT_ALLOCATION_001.md`

Navigation candidate requirement family `NAV-REQ-*` is provisional until reconciliation against existing system and safety requirements is completed.

```text
Navigation statement
→ existing requirement search
→ exact wording / basis check
→ regulatory applicability
→ safety allocation
→ architecture allocation
→ module allocation
```

## 5. Regulatory linkage

Regulatory authority remains outside the module specification:

`01_REQUIREMENTS/REGULATORY/REGULATORY_SOURCE_REGISTER.md`

→ `01_REQUIREMENTS/REGULATORY/REGULATORY_CLAUSE_MAPPING_001.md`

→ applicable `06_CERTIFICATION/` records

→ requirement / interface constraint

→ Navigation allocation

External requirements are not silently converted into internal Navigation functions.

## 6. Architecture linkage

Current system architecture authority available in the repository:

`02_ARCHITECTURE/SYSTEM/AVIATION_SYSTEM_DEFINITION.md`

It defines Navigation as a system-level functional area and states that final allocation remains subject to further decomposition.

Current controlled interface record available:

`02_ARCHITECTURE/INTERFACES/C2/C2_ICD_BASELINE_001.md`

No nonexistent generic `SYSTEM_ARCHITECTURE_BASELINE.md` or `INTERFACE_CONTROL_DOCUMENT.md` is referenced as an existing file.

## 7. Verification linkage

Verification identity is controlled through:

`05_VERIFICATION/VERIFICATION_REGISTER.md`

Navigation-specific verification definition and allocation:

```text
05_VERIFICATION/VERIFICATION_REGISTER.md
                    ↓
05_VERIFICATION/NAVIGATION/NAVIGATION_TRACEABILITY_MATRIX_001.md
                    ↓
05_VERIFICATION/NAVIGATION/NAVIGATION_VERIFICATION_MODEL.md
                    ↓
05_VERIFICATION/NAVIGATION/TEST_VECTORS/NAVIGATION_TEST_VECTORS_001.md
                    ↓
05_VERIFICATION/EVIDENCE/VERIFICATION_EVIDENCE_INDEX.md
```

Verification allocation is not evidence. `VERIFIED` requires controlled execution evidence.

## 8. HMI linkage

HMI is a separate controlled layer:

`08_HMI/`

A Navigation module specification allocates HMI behaviour; final visual implementation belongs to the HMI layer.

## 9. Cross-module semantic baseline

The common Navigation State Model governs these distinctions:

```text
PLANNED ≠ ACTUAL ≠ SIMULATED
Course ≠ Heading ≠ Track ≠ Bearing
Airspeed ≠ Groundspeed
Measured ≠ Estimated ≠ Derived ≠ Predicted
Navigation State ≠ Execution Authority
```

Module documents must reuse these semantics rather than define competing ones.

## 10. Current integration state

```text
Architecture → existing basis
Core Navigation records → existing
Module specifications → derived records
Requirements allocation → controlled working record
Regulatory applicability → clause-level reconciliation required
HMI linkage → allocation required
Verification identity → controlled register
Verification execution → not implied
Evidence → only after execution
```

## 11. Mandatory module completion gate

A module is complete only when:

```text
[ ] existing records checked
[ ] duplicate check completed
[ ] module ID assigned
[ ] parent block assigned
[ ] requirements linked
[ ] regulatory applicability checked
[ ] safety implications checked
[ ] architecture linked
[ ] interfaces linked where applicable
[ ] HMI allocated
[ ] verification allocated
[ ] evidence status controlled
[ ] open items resolved or formally accepted
[ ] Master Document Index updated
```

## 12. No-duplication rule

```text
SEARCH EXISTING RECORDS
        ↓
IDENTIFY AUTHORITATIVE RECORD
        ↓
CHECK OVERLAP
        ↓
UPDATE / LINK EXISTING RECORD
        ↓
ONLY IF ABSENT → CREATE NEW RECORD
```

## 13. Document integrity rule

Every new or materially revised document shall be integrated into the project structure before it is considered a project record:

```text
existence check
→ duplicate/overlap check
→ correct folder
→ unique ID / document number
→ document type
→ status
→ version / baseline information
→ parent object
→ source / authority
→ internal and external references
→ requirement / architecture allocation
→ HMI / interface / verification links where applicable
→ Master Document Index update
→ integrity check
```

A document without the required attributes and links is not considered integrated documentation.

**Status: WORKING_BASELINE**
