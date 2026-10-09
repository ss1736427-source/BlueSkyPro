---
id: MT01-MT02-TEST-DATASETS-001
type: verification_dataset_definition
status: draft_for_agreement
authority: BLUESKY_ALGORITHM_ORCHESTRATION_AND_OPTIMIZATION
---

# MT-01 / MT-02 Controlled Test Dataset Definitions

## 1. Purpose

Defines controlled dataset identities referenced by the MT-01 / MT-02 verification cases.

These records define dataset structure and scenario intent. They are **not yet execution data** and do not constitute evidence.

## 2. Common dataset contract

Every dataset version shall identify:

- dataset_id;
- dataset_version;
- CRS / reference frame;
- AOI or reconstruction target geometry;
- terrain/elevation source and version;
- obstacle geometry/source;
- airspace/restriction state;
- weather/wind snapshot where applicable;
- UAV configuration;
- equipment/payload configuration;
- battery/SOC/SOH/degradation state where applicable;
- C2 state;
- mission requirements / quality profile;
- algorithm/planner configuration;
- expected outcome;
- invalidation trigger where relevant;
- deterministic replay identity.

No universal numerical threshold is introduced by this dataset definition.

## 3. MT-01 datasets

| ID | Scenario | Expected use |
|---|---|---|
| MT01-T01 | Valid baseline AOI, valid terrain/environment/UAV/equipment | Positive baseline and replay |
| MT01-T02 | Convex AOI with regular acquisition geometry | Coverage geometry |
| MT01-T03 | Concave AOI with boundary complexity | Decomposition and edge coverage |
| MT01-T04 | AOI containing prohibited/restricted geometry | Spatial constraint filtering |
| MT01-T05 | Variable terrain and elevation profile | Terrain following and acquisition geometry |
| MT01-T06 | Fixed mission with changed wind snapshot only | Incremental wind recalculation |
| MT01-T07 | Mission whose energy budget cannot protect required reserve | Energy hard gate |
| MT01-T08 | Required acquisition capability absent/incompatible | Capability hard gate |
| MT01-T09 | Mandatory subarea / edge acquisition requirement | Mandatory coverage quality gate |

## 4. MT-02 datasets

| ID | Scenario | Expected use |
|---|---|---|
| MT02-T21 | Valid target surface / reconstruction baseline | Positive baseline and replay |
| MT02-T22 | Relief / variable surface normals and distance | Surface-aware viewpoint generation |
| MT02-T23 | Vertical structures / facades | Oblique and side-looking viewpoints |
| MT02-T24 | Occluded or weakly visible target regions | Weak-region detection/refinement |
| MT02-T25 | Deliberately weak observation network / overlap | Quality and connectivity gate |
| MT02-T26 | Candidate viewpoints blocked by obstacles | Viewpoint feasibility filtering |
| MT02-T27 | Changed camera/equipment configuration | Dependency invalidation |
| MT02-T28 | Changed wind snapshot | Incremental performance/energy recalculation |
| MT02-T29 | Insufficient protected energy reserve | Energy hard gate |
| MT02-T30 | Multi-UAV target decomposition | Allocation and 4D conflict verification |
| MT02-T31 | Fixed target/environment/configuration replay snapshot | Deterministic replay |

## 5. Dataset state

All dataset identities are currently:

`DEFINED / NOT EXECUTED`.

Actual source files, captured environment snapshots and numeric parameter values shall be bound before execution.

## 6. Controlled-source rule

A dataset must not silently inherit changing external data. Safety-significant or reproducibility-relevant inputs require explicit source/version/timestamp/validity/provenance binding.

## 7. Completion state

```
Dataset identity: CONTROLLED
Scenario definition: DEFINED
Actual data package: OPEN
Configuration binding: OPEN
Execution: NOT EXECUTED
Evidence: OPEN
```
