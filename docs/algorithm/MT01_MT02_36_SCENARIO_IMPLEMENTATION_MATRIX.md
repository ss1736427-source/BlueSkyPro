# MT-01 / MT-02 — 36-Scenario Implementation and Verification Matrix

**Status:** controlled gap analysis; not an execution report  
**Scope:** the 18 MT-01 and 18 MT-02 scenarios in `MT-01_MT-02_VERIFICATION_TRACEABILITY.md`.  
**Main protection:** this matrix does not change `main`, does not declare any scenario passed, and does not replace controlled execution packages.

## 1. Reading this matrix

- **Direct-partial** means an existing test asserts part of the scenario contract, but not the full template-level behavior.
- **Related-only** means an adjacent component is tested, but the scenario's decisive behavior is not directly asserted.
- **Gap** means no suitable implementation/test pair was identified in the inspected source set.
- C++ planning modules/tests listed as **PR #26 branch** are present on `control/mt01-mt02-requirement-allocation-2026-10-09` at reviewed head `3cf2b3a757cd6d623d3dffc8540a1c332d0dcccb`; its C++ build-and-test workflow [#367](https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37979485939) passed. That run executed 41/41 CTest tests and printed representative MT-01 simulation metrics. These files are **not part of PR #28 head** and are not evidence of integration into PR #28 or `main`.
- The schema-validator tests listed as **PR #28 branch** are in the current algorithm-documentation branch. Passing CI proves only that the configured jobs ran successfully, not that every row below is closed.
- Existing case-level requirement allocation in PR #26 maps `V-M01-01` and `V-M02-01` to `SYS-REQ-008`, `SYS-REQ-035` and `SYS-REQ-076`. That allocation must **not** be silently propagated to all 36 scenarios. Scenario-specific requirement IDs remain open until each relationship is verified against the authoritative requirement wording.

## 2. MT-01 — area mapping

| Scenario | Design basis | Implementation / executable test candidate | Coverage status | Unclosed behavior |
|---|---|---|---|---|
| MT01-V01 — valid AOI and nominal coverage | MT-01 §§4, 8 | PR #26: `acquisition_geometry.*`, `coverage_orientation.*`, `coverage_decomposition.*`, `coverage_transition_graph.*`; `mt01_t01_representative_simulation_test` plus component tests; CI #367 | Direct-partial | A connected representative simulation passes geometry, coverage estimate, wind/energy and route selection; it uses synthetic assumptions and does not bind a real controlled input snapshot, final mission-integrity gate or operational readiness. |
| MT01-V02 — invalid/empty polygon | MT-01 §§2, 4, 8 | PR #26: `coverage_decomposition.*`; `coverage_decomposition_test`; reusable polygon validation tests | Related-only | Need explicit empty, self-intersecting and malformed AOI assertions with stable blocking diagnostics and no released candidate. |
| MT01-V03 — GSD units and invalid inputs | MT-01 §§2, 4, 8 | PR #26: `acquisition_geometry.*`; `acquisition_geometry_test` | Direct-partial | Test now asserts derived GSD/footprint/spacing/timing values from its existing unit-test fixture and rejects zero focal length; unit normalization and sensor-metadata/unit mismatch cases remain open. |
| MT01-V04 — overlap, image step and track spacing | MT-01 §§4, 8 | PR #26: `acquisition_geometry.*`; `acquisition_geometry_test` | Direct-partial | Formula-level expected GSD, footprint, track spacing, image spacing, trigger interval and rate are now asserted; a controlled target-overlap acceptance gate and hardware trigger-capability gate remain open. |
| MT01-V05 — edge/corner coverage | MT-01 §§4, 8 | PR #26: `CoverageEdgeEngine`, `MappingQualityEngine`; `coverage_decomposition_test` | Direct-partial | Endpoint-gap and mandatory-area cases exist; corner footprint union and full boundary/cell coverage are not established. |
| MT01-V06 — trigger interval exceeds hardware capability | MT-01 §§2, 4, 8 | PR #26: `acquisition_geometry.*`; `acquisition_geometry_test` | Related-only | Trigger interval/rate are calculated, but a controlled equipment minimum-interval capability gate is not demonstrated. |
| MT01-V07 — required GSD infeasible for payload | MT-01 §§2, 4, 8 | PR #26: `acquisition_geometry.*`; `acquisition_geometry_test` | Related-only | GSD is calculated; no direct target-GSD versus payload-capability rejection test was identified. |
| MT01-V08 — terrain variation / surface-relative distance | MT-01 §§2, 4, 8 | PR #26: flight-profile/vertical-route infrastructure; `flight_profile_test`, `wind_performance_trajectory_test` | Related-only | No mapping-specific terrain-following test binds local surface range, GSD and clearance across varying terrain. |
| MT01-V09 — prohibited corridor intersects route | MT-01 §§4, 8 | PR #26: `constrained_open_space.*`, `coverage_transition_graph.*`; `coverage_transition_graph_test`, `notam_prohibited_zone_validator_test`; PR #28: `test_geometry_engine.py`, `test_route_in_zone.py` | Direct-partial | Tests cover constrained edges / route containment, but do not establish exhaustive candidate-generation and route-search avoidance for all prohibited corridors. |
| MT01-V10 — restrictions leave no admissible route | MT-01 §§4, 8 | PR #26: `CoverageRouteSelector`; `coverage_transition_graph_test`; `candidate_comparison_test` | Direct-partial | No complete mapping-plan test proves every alternative is rejected with the correct restriction-linked diagnostic and no release eligibility. |
| MT01-V11 — reserve/return infeasible | MT-01 §§4, 8 | PR #26: `wind_performance_trajectory.*`, `coverage_transition_graph.*`; `wind_performance_trajectory_test`, `coverage_transition_graph_test`; PR #28: `test_wind_performance.py`, `test_uav_assignment.py` | Direct-partial | Reusable route-energy checks exist; a complete MT-01 return/recovery and per-UAV reserve acceptance case remains unbound. |
| MT01-V12 — wind change and incremental recalculation | MT-01 §§4, 7, 8 | PR #26: `wind_performance_trajectory.*`, dependency identity in acquisition/coverage stages; related trajectory/orchestrator tests; PR #28: `test_assignment_invalidation.py` | Related-only | No direct MT-01 test proves wind changes cadence/performance while preserving valid coverage geometry and invalidating every dependent result only. |
| MT01-V13 — missing post-flight image sequence | MT-01 §§5–6, 8 | No mapping-specific post-flight image-sequence QA module/test identified | Gap | Implement sequence completeness, affected-cell status, and targeted reacquisition evidence; missing data must not be inferred as coverage. |
| MT01-V14 — actual overlap below requirement | MT-01 §§4, 6, 8 | PR #26: `MappingQualityEngine` in `coverage_decomposition.*`; `coverage_decomposition_test` | Related-only | Current metrics expose minimum overlap, but a controlled required-overlap threshold and explicit post-flight acceptance/failure assertion are not established. |
| MT01-V15 — stale environment snapshot | MT-01 §§2–4, 7–8 | PR #26: environment/dependency identities and final-integrity/readiness modules; `final_planning_integrity_test`, `authorization_readiness_gate_test` | Related-only | Need a direct stale-version/freshness test tied to the mapping candidate and final release decision. |
| MT01-V16 — missing required QA evidence | MT-01 §§3, 6, 8 | PR #26: `MappingQualityEngine`; PR #28: `test_three_d_mapping_adapter.py` checks propagated status only | Related-only | No complete MT-01 test proves missing product evidence leaves the result `UNVERIFIED` and blocks completion/release. |
| MT01-V17 — multi-sortie / multi-UAV partition and stitching | MT-01 §§4, 6, 8 | PR #26: multi-UAV assignment/conflict infrastructure and tests; PR #28: `test_uav_assignment.py`, `test_multi_uav_domain.py` | Related-only | Multi-UAV routing/assignment is not evidence of image-block stitching, shared observation overlap or common-reference QA. |
| MT01-V18 — material input change / selective invalidation | MT-01 §7–8 | PR #26: dependency identities in geometry/coverage modules; PR #28: `test_assignment_invalidation.py`, `test_multi_uav_domain.py` | Direct-partial | Deterministic identity changes are tested in individual stages; complete MT-01 downstream invalidation/reuse is not yet established. |

## 3. MT-02 — 3D reconstruction

| Scenario | Design basis | Implementation / executable test candidate | Coverage status | Unclosed behavior |
|---|---|---|---|---|
| MT02-V01 — flat area and valid nadir observations | MT-02 §§3–4, 8 | PR #28: `three_d_mapping_adapter.py`, `test_three_d_mapping_adapter.py` | Related-only | Adapter aggregates upstream verified artifacts; no patch/viewpoint planner or end-to-end reconstruction geometry test identified. |
| MT02-V02 — vertical facade | MT-02 §§4, 8 | No surface-viewpoint generator identified in inspected planning modules | Gap | Generate and validate feasible oblique/side-looking observations for required facade patches. |
| MT02-V03 — occluded/recessed surface | MT-02 §§4, 6, 8 | No visibility/occlusion engine or direct test identified | Gap | Occluded views must not count as coverage; unresolved required patches must fail or remain unverified. |
| MT02-V04 — disconnected observation graph | MT-02 §§4, 8 | No MT-02 observation-graph builder/connectivity test identified | Gap | Build graph from valid observations and assert disconnection blocks release or causes a justified repair. |
| MT02-V05 — insufficient parallax despite high overlap | MT-02 §§4, 8 | No parallax/baseline quality engine or direct test identified | Gap | Overlap percentage alone must not pass geometric-strength criteria. |
| MT02-V06 — GSD/point density beyond sensor capability | MT-02 §§3–4, 8 | PR #26: `acquisition_geometry.*`, `acquisition_geometry_test`; PR #28: adapter tests | Related-only | Camera GSD calculation does not implement LiDAR point-density or 3D product feasibility gates. |
| MT02-V07 — unsupported camera/gimbal attitude | MT-02 §§3–4, 8 | PR #26: vehicle/equipment capability contracts; `vehicle_equipment_capability_contract_test` | Related-only | No MT-02 candidate-viewpoint test checks camera/gimbal attitude limits and substitutes a valid view. |
| MT02-V08 — unsafe viewpoint | MT-02 §§4, 8 | PR #26: constrained-open-space and route-constraint modules/tests; PR #28: `test_geometry_engine.py`, `test_route_in_zone.py` | Related-only | Route safety infrastructure exists, but no viewpoint candidate is bound to the safe route test and target-patch objective. |
| MT02-V09 — no admissible route to required patch | MT-02 §§4, 8 | PR #26: route candidate/constraint modules; `coverage_transition_graph_test`; PR #28: route containment tests | Related-only | No patch-linked MT-02 test proves an unreachable required patch blocks the reconstruction plan. |
| MT02-V10 — per-UAV reserve/recovery infeasibility | MT-02 §§4, 8 | PR #26: `wind_performance_trajectory.*`, multi-UAV planning tests; PR #28: `test_wind_performance.py`, `test_uav_assignment.py` | Related-only | No MT-02 test binds each vehicle's selected viewpoints, return route and reserve proof to the final result. |
| MT02-V11 — wind changes route performance and acquisition cadence | MT-02 §§4, 7–8 | PR #26: wind/performance trajectory engine; PR #28: `test_wind_performance.py`, `test_assignment_invalidation.py` | Related-only | No MT-02 observation scheduler exists to prove cadence recalculation and quality-gate rerun after wind changes. |
| MT02-V12 — multi-UAV stitching observations | MT-02 §§4, 8 | PR #26: multi-UAV allocation/conflict tests; PR #28: `test_multi_uav_domain.py`, `test_uav_assignment.py` | Related-only | Assignment and conflict tests do not verify common observations, graph connectivity or cross-UAV registration. |
| MT02-V13 — incompatible calibration/sensor metadata | MT-02 §§2–4, 6, 8 | No calibration-consistency/fusion gate or direct test identified | Gap | Reject incompatible metadata or require a controlled correction workflow before fusion. |
| MT02-V14 — missing images / LiDAR swaths | MT-02 §§5–6, 8 | No post-flight sensor-data completeness QA module/test identified | Gap | Mark affected patches incomplete and produce evidence-linked reacquisition tasks. |
| MT02-V15 — georeferencing/checkpoint failure | MT-02 §§6, 8 | No reconstruction georeferencing/checkpoint QA module/test identified | Gap | Product completion must be blocked when configured accuracy evidence fails or is absent. |
| MT02-V16 — unobserved surface patches | MT-02 §§4, 6, 8 | No target-patch state model/coverage test identified; adapter only accepts optional aggregate coverage | Gap | Patch-level state must be `FAIL`/`UNVERIFIED`, never aggregate `PASS` while a required patch is unseen. |
| MT02-V17 — stale environment/authorization snapshot | MT-02 §§2, 4, 7–8 | PR #26: authorization/readiness/final-integrity tests; PR #28: `test_final_gate.py` | Related-only | No MT-02 case binds snapshot freshness and authorization version to viewpoint, route and final quality artifacts. |
| MT02-V18 — material input change / selective invalidation | MT-02 §7–8 | PR #26: dependency identity infrastructure; PR #28: `test_assignment_invalidation.py`, `test_multi_uav_domain.py` | Related-only | No surface/viewpoint/observation-graph dependency graph exists to assert complete and selective MT-02 invalidation. |

## 4. Requirement allocation disposition

1. The **PR #26 controlled case binding** allocates `SYS-REQ-008` (Mission Readiness), `SYS-REQ-035` (Task to Capability Mapping) and `SYS-REQ-076` (UAV Capability Profile) to the **case-level** records `V-M01-01` and `V-M02-01`.
2. That is a valid parent dependency, not a complete requirement allocation for the 36 scenario rows.
3. Do not assign additional SYS-REQ IDs based only on similar wording or module names. The current `01_REQUIREMENTS/TRACEABILITY/REQUIREMENTS_TRACEABILITY_MATRIX.md` still lists `RTE-REQ-001..004`, `WP-REQ-001..003` and `MIS-REQ-001..003` as candidates with status `TBD`; `CONTROLLED_WORDING_RECONCILIATION_001.md` records `MIS-REQ-001`, `MUL-REQ-001` and `RTE-REQ-001` as `MASTER-REGISTER-GAP`. These candidate families are not authoritative scenario allocations. Each scenario-specific allocation must be checked against exact controlled requirement text and the approved algorithm/design basis.
4. Until that audit is complete, the scenario-specific SYS-REQ allocation is **OPEN**. The design-basis references above identify intended behavior but are not substitutes for formal requirement IDs.

## 5. Current closure status

- Scenario rows: 36 identified; no row is declared fully closed by this matrix.
- MT-01 acquisition geometry / track generation / edge evaluation / route-candidate work: first implementation slices exist in PR #26, but integration and full acceptance gates remain open.
- MT-02 visibility, viewpoint generation, parallax, observation-graph connectivity, patch-level completeness and reconstruction QA: implementation gaps remain in the inspected modules.
- Controlled real-data execution: `V-M01-01 / MT01-T01` and `V-M02-01 / MT02-T21` remain unexecuted because their controlled inputs/configurations are incomplete.
- CI status is reported separately per branch/commit; no cross-branch status is inferred.
- MT-03 remains blocked until MT-01 and MT-02 meet their completion gates.

## 6. Required next implementation sequence

1. Reconcile the two open PR branches before treating PR #26 C++ code and PR #28 algorithm documents as one integrated baseline.
2. Bind the MT-01 acquisition engine to canonical mission/equipment configuration and make its tests assert exact formula results from controlled unit-test inputs.
3. Complete coverage-footprint/edge/corner and target-overlap acceptance gates without inventing universal thresholds.
4. Add explicit route-search avoidance evidence for prohibited corridors and full downstream dependency invalidation tests.
5. For MT-02, implement target-surface patches, visibility/occlusion, viewpoint generation, parallax and observation-graph connectivity before claiming reconstruction-plan verification.
6. Keep controlled dataset execution blocked until real, versioned inputs and acceptance criteria are available.
