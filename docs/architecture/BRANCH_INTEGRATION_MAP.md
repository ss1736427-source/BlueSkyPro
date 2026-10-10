# BlueSky PRO — Branch Integration Map

**Status:** controlled integration baseline  
**Date:** 2026-10-04

## Branches

- `main` — authoritative stable baseline.
- `docs/mission-template-catalog-2026-10-01` — approved 13-template operator taxonomy.
- `docs/optimization-layer-ivanov-2026-10-04` — Planning Kernel / Optimization / Task Modules / linked interfaces / Knowledge & Learning architecture.
- `controlled-review-2026-09-22` — controlled HMI / planning implementation review branch; it diverges substantially from `main` and requires a separate controlled integration audit before merge.

## Integration rule

The documentation architecture branches are connected through this integration branch. The controlled-review branch is intentionally not force-merged until its 96-commit divergence and one-commit lag from `main` are reviewed for deletions, conflicts, and implementation dependencies.

## Dependency direction

`main → mission taxonomy → task interpretation → task modules → Planning Kernel → optimization → knowledge & learning → quality/safety verification → HMI/implementation`

The branches must not introduce competing sources of truth. The approved 13-template catalog defines the operator-facing taxonomy; the Task Module architecture defines reusable functional modules; the Planning Kernel remains authoritative for planning state; Knowledge & Learning supplies validated knowledge and recommendations; Safety Verification remains authoritative for release.

## Required next integration step

Perform a controlled merge/audit of `controlled-review-2026-09-22` against this integration baseline. Do not overwrite or delete existing authoritative documentation merely to resolve branch divergence.
