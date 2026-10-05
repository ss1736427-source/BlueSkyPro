# BlueSky PRO — Repository SSOT and Consolidation Plan

**Status:** ACTIVE MIGRATION BASELINE  
**Date:** 2026-10-04  
**Authoritative repository:** `ss1736427-source/BlueSkyPro`

## 1. Decision

`BlueSkyPro` is the single authoritative repository (SSOT) for the BlueSky PRO product.

All product-critical code, architecture, requirements, algorithms, mission models, knowledge records, HMI implementation, verification contracts and engineering documentation shall converge into this repository.

No second repository may become an independent source of truth for BlueSky PRO product data.

## 2. Repository audit

### 2.1 BlueSkyPro

Role: **PRIMARY / SSOT**

Contains the current product implementation and the integrated architecture baseline:

- Planning Kernel and planning architecture;
- optimization layer;
- Knowledge & Learning Layer;
- mission-template catalog;
- task-module architecture;
- linked interface/data-flow architecture;
- Multi-UAV planning and validation;
- schemas and regression validators;
- HMI/Qt implementation where integrated;
- engineering documentation.

Current integration branch:
`integration/architecture-2026-10-04`

SSOT consolidation branch:
`integration/ssot-consolidation-2026-10-04`

### 2.2 BlueSky-PRO-Knowledge

Role: **SOURCE MATERIAL / MIGRATION SOURCE — NOT SSOT**

Audit shows that this repository contains substantial BlueSky PRO material, including:

- requirements;
- architecture decisions;
- system design;
- fleet/equipment data;
- regulatory material;
- verification/certification material;
- HMI design specifications;
- Qt Design Studio/QML working material;
- planning and optimization documents;
- traceability;
- legacy and source archives.

It also contains historical, backup, duplicate, generated, tool-specific and potentially conflicting material.

Therefore it MUST NOT be copied wholesale into the SSOT.

Migration rule:

`SOURCE → AUDIT → CLASSIFY → DEDUPLICATE → RECONCILE → VALIDATE → IMPORT`

Only approved records are migrated.

Until migration is completed, the repository is treated as a **read-only source for migration/audit purposes**. New authoritative BlueSky PRO decisions must be recorded in `BlueSkyPro`.

### 2.3 docs

Role: **UNRELATED EXTERNAL REPOSITORY**

The current `ss1736427-source/docs` repository is the Vue.js documentation project. Its README identifies it as a VitePress/Vue documentation site, and its recent commits are Vue documentation changes.

It is NOT BlueSky PRO documentation and is excluded from consolidation.

It must not be renamed, modified, merged or migrated as part of BlueSky PRO consolidation.

## 3. Knowledge migration classes

| Class | Examples | Action |
|---|---|---|
| A — Authoritative candidate | approved requirements, architecture decisions, safety rules, current mission definitions | migrate after validation |
| B — Useful engineering knowledge | algorithms, equipment, planning methods, HMI specifications | migrate after deduplication/reconciliation |
| C — Historical | superseded decisions, old designs | migrate to controlled archive only when provenance is needed |
| D — Backup/duplicate | `_BACKUP*`, exact duplicates, temporary copies | do not migrate as active records |
| E — Source archive | original imported materials | retain only under explicit archive policy |
| F — Generated/tool state | Obsidian state, transient tool files, generated artifacts | do not migrate as authoritative knowledge |
| G — Product implementation | QML/code/workflows | migrate only where it is the current implementation baseline; do not duplicate existing BlueSkyPro implementation |

## 4. Canonical ownership

After consolidation:

- Requirements → `BlueSkyPro`
- Architecture decisions → `BlueSkyPro`
- Planning algorithms → `BlueSkyPro`
- Mission templates → `BlueSkyPro`
- Task modules → `BlueSkyPro`
- UAV/payload capability records → `BlueSkyPro`
- Regulatory/safety records → `BlueSkyPro`
- Verification contracts/tests → `BlueSkyPro`
- HMI design baseline → `BlueSkyPro`
- Qt/QML implementation → `BlueSkyPro`
- Knowledge & Learning records → `BlueSkyPro`
- Historical provenance → `BlueSkyPro` controlled archive

## 5. No parallel authority

The following is prohibited after consolidation:

```text
BlueSkyPro/knowledge/A.md
        ↕
BlueSky-PRO-Knowledge/A.md
```

There must be one canonical record.

If an external source is retained for provenance, it is referenced as a source/archive and is not an editable competing authority.

## 6. Migration order

1. Freeze new authoritative decisions in `BlueSky-PRO-Knowledge`.
2. Inventory all 1,881 tracked files.
3. Classify records A–G.
4. Identify exact and semantic duplicates.
5. Identify conflicting requirements/architecture decisions.
6. Map canonical destination paths in `BlueSkyPro`.
7. Import approved records.
8. Reconcile terminology and identifiers.
9. Update cross-references.
10. Run architecture/traceability/validator checks.
11. Record the final migration manifest.
12. Only then archive/decommission the old knowledge repository.

## 7. Safety rule

Migration is a documentation/data-governance operation. It must not silently change:

- safety requirements;
- regulatory constraints;
- verification status;
- requirement identity;
- authoritative algorithm behavior;
- mission release criteria.

Every such change requires an explicit engineering decision.

## 8. Current migration boundary

This commit establishes the SSOT governance boundary only.

It intentionally does NOT perform a bulk copy of the Knowledge repository. The source contains 1,881 tracked files and includes backups, archives, generated/tool state and implementation material; blind copying would create the exact duplication problem this consolidation is intended to remove.

## 9. Target state

```text
                    BlueSky PRO
                         |
                         v
                  +--------------+
                  |  BlueSkyPro  |
                  |     SSOT     |
                  +------+-------+
                         |
          +--------------+--------------+
          |              |              |
          v              v              v
       Product        Knowledge      Engineering
        Code          & Learning     Documentation
          |              |              |
          +--------------+--------------+
                         |
                         v
                  Planning Kernel
                         |
                  Optimization
                         |
                  Safety Verification
                         |
                  Mission Release
```

**Rule:** one product, one repository, one authoritative version of each controlled record.
