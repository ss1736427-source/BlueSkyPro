---
id: MT01-MT02-VERIFICATION-CASES-001
type: verification_case_definition
status: draft_for_agreement
authority: VERIFICATION-REGISTER-001; BLUESKY_ALGORITHM_ORCHESTRATION_AND_OPTIMIZATION
---

# MT-01 / MT-02 Verification Cases — Controlled Definition

## 1. Scope

This document defines the first controlled verification cases for the approved algorithm specifications of:

- MT-01 — Картографирование территории
- MT-02 — 3D-картография / реконструкция

These are **case definitions only**. No case is declared executed, passed or evidenced by this document.

Verification identity was checked against the current Verification Register and repository tree before creation. No existing `V-M01-*` or `V-M02-*` identities were found.

## 2. Controlled verification chain

```
Requirement / Design Basis
        ↓
MT verification case
        ↓
Controlled dataset
        ↓
Execution configuration
        ↓
Execution result
        ↓
Evidence
```

## 3. Status model

All cases below start as `DEFINED`.

`DEFINED` ≠ `EXECUTED` ≠ `PASSED`.

No numerical acceptance threshold is invented here where the algorithm specification requires a controlled mission, payload, safety or engineering parameter.

## 4. MT-01 cases

| ID | Objective | Dataset | Method | Acceptance basis |
|---|---|---|---|---|
| V-M01-01 | Positive baseline mapping of a valid AOI | MT01-T01 | TEST / SIMULATION | Valid candidate reaches final integrity validation and required mapping quality |
| V-M01-02 | Convex AOI coverage | MT01-T02 | TEST / SIMULATION | Required coverage is achieved without hard-constraint violation |
| V-M01-03 | Concave AOI and boundary coverage | MT01-T03 | TEST / SIMULATION | Concave geometry is covered; boundary gaps are correctly classified |
| V-M01-04 | Exclusion / restricted geometry | MT01-T04 | TEST / SIMULATION | Restricted geometry is excluded and remaining candidate remains physically/regulatorily valid |
| V-M01-05 | Terrain variation / terrain following | MT01-T05 | TEST / SIMULATION | Camera-ground geometry and trajectory remain within configured constraints; quality gate reflects deficiencies |
| V-M01-06 | Wind-only incremental recalculation | MT01-T06 | TEST / SIMULATION | Unaffected stages are reused; affected performance/energy/trajectory results are recalculated |
| V-M01-07 | Insufficient energy / protected reserve | MT01-T07 | TEST / SIMULATION | Candidate violating protected reserve is rejected; no unsafe energy trade is accepted |
| V-M01-08 | Payload / sensor infeasibility | MT01-T08 | TEST / SIMULATION | Missing/incompatible mandatory capability blocks candidate before route optimization |
| V-M01-09 | Edge / mandatory-subarea coverage | MT01-T09 | TEST / SIMULATION | Mandatory coverage gates are independently satisfied; aggregate score cannot hide failure |
| V-M01-10 | Deterministic replay | MT01-T01 + fixed snapshot | TEST / SIMULATION | Same controlled inputs, versions and ordering produce the same selected result and provenance |

## 5. MT-02 cases

| ID | Objective | Dataset | Method | Acceptance basis |
|---|---|---|---|---|
| V-M02-01 | Positive baseline 3D reconstruction | MT02-T21 | TEST / SIMULATION | Required target observations and reconstruction-quality gates are satisfied |
| V-M02-02 | Terrain relief / variable surface | MT02-T22 | TEST / SIMULATION | Viewpoint generation follows target geometry while respecting terrain and UAV constraints |
| V-M02-03 | Vertical structures / facades | MT02-T23 | TEST / SIMULATION | Required oblique/side-looking observations are generated and feasible |
| V-M02-04 | Occlusion / hidden surfaces | MT02-T24 | TEST / SIMULATION | Weak/occluded regions are detected and refined; unresolved required regions fail quality |
| V-M02-05 | Insufficient overlap / weak image network | MT02-T25 | TEST / SIMULATION | Weak connectivity/observation conditions are detected and rejected or repaired |
| V-M02-06 | Viewpoint obstacle conflict | MT02-T26 | TEST / SIMULATION | Infeasible viewpoints are removed; replacement candidates are evaluated |
| V-M02-07 | Camera / payload configuration change | MT02-T27 | TEST / SIMULATION | Affected viewpoint, performance, energy, trajectory and quality stages are invalidated/recalculated |
| V-M02-08 | Wind change | MT02-T28 | TEST / SIMULATION | Wind-dependent performance, energy and affected trajectory stages are recalculated without unnecessary global recomputation |
| V-M02-09 | Insufficient energy | MT02-T29 | TEST / SIMULATION | Candidates failing protected reserve are rejected |
| V-M02-10 | Multi-UAV 3D allocation | MT02-T30 | TEST / SIMULATION | Task/viewpoint allocation and 4D conflict validation produce an admissible group plan |
| V-M02-11 | Deterministic replay | MT02-T21 + fixed snapshot | TEST / SIMULATION | Same controlled inputs, versions and ordering reproduce the same selected result and provenance |

## 6. Required execution record

Each executed case shall record at minimum:

- verification ID;
- requirement/design basis;
- dataset ID and version;
- mission revision;
- planner/algorithm version;
- algorithm configuration;
- environment snapshot;
- UAV/equipment configuration;
- objective profile;
- execution environment;
- expected result;
- actual observations;
- pass/fail disposition;
- anomalies;
- evidence reference;
- reviewer;
- change/configuration identifier.

## 7. Evidence boundary

The following do **not** constitute execution evidence by themselves:

- algorithm pseudocode;
- expected-result text;
- dataset definition;
- case definition;
- mathematical specification;
- CI compilation success unrelated to the case procedure;
- an unbound simulation output.

Evidence becomes controlled only when the execution, result and configuration are recorded and linked.

## 8. Current state

```
Case identity: CONTROLLED
Case definition: DEFINED
Datasets: DEFINED
Execution configuration: OPEN
Execution result: NOT EXECUTED
Evidence: OPEN
Certification closure: OPEN
```

MT-03 remains blocked until the MT-01/MT-02 completion gate is satisfied.
