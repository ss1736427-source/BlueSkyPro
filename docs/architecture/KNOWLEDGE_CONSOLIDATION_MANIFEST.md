# BlueSky PRO — Knowledge Consolidation Manifest

Migration branch: integration/knowledge-consolidation-2026-10-04
Source repository: ss1736427-source/BlueSky-PRO-Knowledge
Target repository: ss1736427-source/BlueSkyPro
Source snapshot: c0c107479e89443bd0b0a60e91b447ba4206d22c

## Purpose

This file records the controlled consolidation of BlueSky-PRO-Knowledge into BlueSkyPro.

The target repository remains the product SSOT. The source repository is treated as migration material, not as a competing authority.

## Source inventory

At the source snapshot:
- 1,680 tracked blobs.
- 1,027 documentation/data files (md, txt, csv, yaml, yml, json).
- 452 implementation/reference files (cpp, hpp, py, qml, qmlproject, qtds, qrc, js, css, svg, bsrec).
- 6 other files.
- 195 files classified for exclusion from the first controlled knowledge import because they are backups, legacy/archive material, generated/tool state, repository automation, or screen/source-import material.

## Import policy

Imported knowledge is placed under PROJECT_KNOWLEDGE/SOURCE/<original source path>.

This preserves provenance and prevents an unreviewed source document from silently replacing an existing product SSOT document.

The following classes are deliberately not bulk-imported:
- .obsidian/ and Obsidian plugin/tool state;
- .github/ workflows from the Knowledge repository;
- _LEGACY/;
- _SOURCE_ARCHIVE/;
- backup trees and backup copies;
- generated/import staging material;
- Knowledge-repository tools;
- product implementation duplicates until code-level reconciliation.

## Imported in this consolidation pass

The branch currently contains controlled imports covering:
- core lifecycle and architecture records;
- normative references and requirements baselines;
- regulatory and safety material;
- system requirements and traceability records;
- planning/optimization source material;
- fleet/multi-UAV capability records;
- Mission Memory / Knowledge Engine architecture;
- HMI task-first/template-selection material;
- HMI design-system, layout, toolbar, panel, typography and traceability documentation;
- HMI/Qt Design Studio README and safety boundary;
- source repository README.

## Reconciliation rule

No source document becomes authoritative merely because it was imported.

SOURCE → COMPARE → RECONCILE → VALIDATE → CANONICALIZE

The existing BlueSkyPro authoritative artifact remains authoritative until the reconciliation decision is recorded.

## Implementation code

The Knowledge repository contains C++/HPP/Python/QML and related implementation material. These are not copied wholesale into the product implementation tree. They require code-level comparison against BlueSkyPro before any adoption.

This avoids creating two competing implementations of safety-critical or planning functions.

## Excluded repository

ss1736427-source/docs is not part of this consolidation. It is a separate Vue.js documentation repository.

## Completion criterion

Consolidation is complete only when:
1. all relevant Knowledge material is inventoried;
2. duplicates/backups/archive/generated material are classified;
3. authoritative requirements/architecture/algorithm/HMI/verification records are reconciled;
4. unique implementation material is reviewed against BlueSkyPro;
5. canonical documents are established in BlueSkyPro;
6. provenance and migration status are retained;
7. the Knowledge repository can be retired or frozen without losing authoritative project information.
