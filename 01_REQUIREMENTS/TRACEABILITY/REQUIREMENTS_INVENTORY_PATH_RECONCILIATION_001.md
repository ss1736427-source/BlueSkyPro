---
id: REQUIREMENTS-INVENTORY-PATH-RECON-001
type: requirements_inventory_path_reconciliation
status: WORKING
parent: REQUIREMENTS-INVENTORY-001
authority: MASTER-REQUIREMENTS-REGISTER-001
---

# Requirements Inventory — Path Reconciliation 001

## 1. Purpose

This record corrects the path interpretation of the existing requirements inventory without changing requirement identity, wording, status, or history.

## 2. Verified current repository paths

The current repository structure places the authoritative requirements system documents under:

```text
01_REQUIREMENTS/SYSTEM/
```

Verified files:

```text
01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md
01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md
01_REQUIREMENTS/SYSTEM/REQUIREMENTS_INVENTORY.md
01_REQUIREMENTS/SYSTEM/REQUIREMENTS_INVENTORY.csv
```

The current traceability documents are under:

```text
01_REQUIREMENTS/TRACEABILITY/
```

Verified traceability file:

```text
01_REQUIREMENTS/TRACEABILITY/REQUIREMENTS_TRACEABILITY_MATRIX.md
```

## 3. Correction rule

References in older inventory output such as:

```text
00_PROJECT/MASTER_REQUIREMENTS_REGISTER.md
00_PROJECT/SYSTEM_REQUIREMENTS_SPECIFICATION.md
00_PROJECT/REQUIREMENTS_INVENTORY.md
00_PROJECT/REQUIREMENTS_TRACEABILITY_MATRIX.md
```

are treated as **legacy inventory paths**, not as current authoritative locations.

They must not be used for new traceability links.

## 4. Preservation rule

No requirement IDs are renumbered or removed by this reconciliation.

```text
Requirement identity  = preserved
Controlled wording     = preserved
Historical records     = preserved
Current path reference = corrected
```

## 5. Required canonical paths for new work

| Function | Canonical path |
|---|---|
| Master requirement identity | `01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md` |
| System requirements specification | `01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md` |
| Requirements inventory | `01_REQUIREMENTS/SYSTEM/REQUIREMENTS_INVENTORY.md` |
| Machine-readable requirements inventory | `01_REQUIREMENTS/SYSTEM/REQUIREMENTS_INVENTORY.csv` |
| Requirements traceability matrix | `01_REQUIREMENTS/TRACEABILITY/REQUIREMENTS_TRACEABILITY_MATRIX.md` |
| Navigation reconciliation | `01_REQUIREMENTS/TRACEABILITY/NAVIGATION_REQUIREMENT_RECONCILIATION_PASS_001.md` |
| Navigation closure status | `01_REQUIREMENTS/TRACEABILITY/NAVIGATION_TRACEABILITY_CLOSURE_STATUS_001.md` |

## 6. Control status

```text
PATH RECONCILIATION: COMPLETED FOR VERIFIED REQUIREMENTS DOCUMENTS
REQUIREMENT IDENTITY: UNCHANGED
CONTROLLED WORDING: UNCHANGED
TRACEABILITY MODEL: UNCHANGED
LEGACY REFERENCES: FLAGGED
```

## 7. Next operation

The next audit must use the canonical paths above when resolving `NAV-REQ-*`, `SYS-REQ-*`, and `SAF-REQ-*` records.

No new duplicate requirements database is to be created.
