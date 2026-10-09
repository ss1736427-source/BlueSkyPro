# BlueSky PRO — MT-01 / MT-02 Verification Traceability

**Status:** INITIAL VERIFICATION GAP REGISTER — not a test execution report  
**Scope:** Trace the minimum algorithm scenarios defined by MT-01 and MT-02 to explicit verification evidence.  
**Related specifications:**
- `docs/algorithm/MT-01_MAPPING_MISSION_ALGORITHM.md`
- `docs/algorithm/MT-02_3D_MAPPING_RECONSTRUCTION_ALGORITHM.md`
- `08_PLANNING/BLUESKY_MISSION_MODEL.md`
- `08_PLANNING/BLUESKY_MISSION_OBJECTIVE_PROFILES.md`
- `08_PLANNING/BLUESKY_ROUTE_CALCULATION_AND_OPTIMIZATION_SPEC.md`
- `docs/algorithm/PHOTOGRAMMETRY_SOLUTION_METHOD.md`

## 1. Interpretation rules

This register distinguishes algorithm requirements from implementation and verification evidence.

- **Specified** means the required behavior is described in the algorithm document.
- **Direct test evidence found** means a reviewed test explicitly exercises the scenario and asserts the required behavior.
- **Unverified in this review** means no direct scenario-to-test link was established from the files inspected. It does **not** prove that no related test exists elsewhere in the repository.
- CI success proves only that the configured CI steps passed; it does not prove that every scenario in this register is implemented or covered.
- Unit-test fixtures, simulation, HIL and real-UAV evidence are separate evidence classes and must not be conflated.

## 2. Existing directly reviewed test evidence

Reviewed file: `schemas/validator/test_three_d_mapping_adapter.py`.

| Test | What it demonstrates | Remaining limitation |
|---|---|---|
| `test_maps_verified_pipeline_outputs` | Aggregates verified route/performance/trajectory objects; checks route length, duration, energy, a supplied coverage value and propagated release status | Does not derive/validate coverage, surface visibility, viewpoint geometry, reserve feasibility, reconstruction quality or product QA |
| `test_rejects_unverified_route` | Rejects a route whose `verified` attribute is false | Does not test the complete MT-02 geometry or reconstruction pipeline |
| `test_rejects_empty_routes` | Rejects an empty route set with `3D_MAPPING_NO_ROUTES` | Does not validate the geometric reason a route is needed |
| `test_rejects_route_performance_mismatch` | Rejects unequal route/performance counts | Does not prove each performance result is correctly linked by ID/version |
| `test_rejects_performance_trajectory_mismatch` | Rejects unequal performance/trajectory counts | Does not prove semantic correspondence or timing synchronization |
| `test_rejects_unverified_performance` | Rejects a performance artifact marked unverified | Does not independently verify its underlying calculation |
| `test_rejects_unverified_trajectory` | Rejects a trajectory artifact marked unverified | Does not independently verify trajectory safety |
| `test_rejects_zero_or_negative_aggregate_route_length` | Rejects a non-positive aggregate route length | Does not validate each segment's geometry or physical plausibility |
| `test_rejects_inverted_time_range` | Rejects an inverted aggregate time range | Does not validate per-UAV sequencing or synchronization |
| `test_unknown_release_status_does_not_mark_result_verified` | Ensures an unknown status such as `PENDING` does not set the adapter's `verified` flag | Does not make the adapter an independent release authority |

Eight additional adapter-guard tests were added in the current PR branch, bringing this file to ten test functions. The CI workflow executes each `test_*.py` file directly with Python rather than invoking pytest, so a `main()` runner was added to explicitly execute all ten adapter test functions under that workflow. They test the adapter's existing contract; they do **not** implement or verify the MT-01/MT-02 acquisition algorithms.

Other related tests inspected:
- `test_geometry_engine.py`: preserves the fail-closed contract-engine test and directly exercises the Shapely-backed constrained-space path. The cases verify an interior exclusion becomes a hole with area conserved, a boundary-to-boundary exclusion splits the operational area into deterministic non-overlapping components, an exclusion outside the source is rejected, and an exclusion that removes all usable space cannot be marked verified. An integration assertion passes the clipped component into `build_route_in_zone`: a route segment crossing the exclusion is rejected even when its endpoints lie inside the outer polygon, while a route around the exclusion is accepted. This verifies the geometry-to-route-containment boundary, not candidate generation or route-search avoidance.
- `test_constrained_space.py`: verifies a nominal constrained-space result and confirms the default contract engine fails explicitly when polygon subtraction is unsupported.
- `test_route_in_zone.py`: verifies an in-zone route and rejects an outside/crossing route.
- `test_final_gate.py`: verifies release blocking for unresolved conflicts and invalidated routes; it also demonstrates that technical release eligibility is separate from authorization status.
- `test_wind_performance.py`: checks headwind/tailwind ground-speed differences, a negative reserve margin and wind-limit rejection.
- `test_uav_assignment.py`: checks multi-UAV zone assignment, global matching, reserve infeasibility and unauthorized-UAV rejection.
- `test_assignment_invalidation.py` and `test_multi_uav_domain.py`: check downstream dependency invalidation and retention of unaffected artifacts.
- `test_multi_uav_regression_fixtures.py`: checks deterministic conflict, release-boundary, invalidation, replay and version-mismatch fixtures.
- `test_polygon_splitter.py` and `test_zone_partition.py`: check geometric partition completeness and rejection of invalid partition cases.

These are **related component tests**, not direct evidence that all 36 template-specific scenarios pass. The register below therefore retains the template-level rows as unverified until a direct test ID is linked to each required behavior. This is a targeted review of the listed validator files, not a claim that no other relevant tests exist elsewhere in the repository.

## 3. Controlled execution package status

The first controlled execution package is documented in `docs/algorithm/MT01-T01_CONTROLLED_EXECUTION_PACKAGE.md`.

| Verification case | Dataset/package | Requirement allocation | Current state | Reason |
|---|---|---|---|---|
| `V-M01-01` | `MT01-T01` | `SYS-REQ-008`, `SYS-REQ-035`, `SYS-REQ-076` | `BLOCKED_MISSING_CONTROLLED_INPUTS` / `NOT_EXECUTED` | No complete versioned AOI/CRS, terrain, restrictions, time-matched weather/wind, selected aircraft/readiness, installed camera calibration, mission quality profile, C2/launch-recovery data and execution configuration were found in the reviewed project data. Manufacturer reference facts alone are insufficient. |

This is a package-readiness finding, not a failed algorithm execution. The test remains unexecuted until the missing controlled artifacts are supplied. No synthetic AOI or invented operational values may be substituted.

## 4. MT-01 — area mapping scenarios

| ID | Minimum scenario | Required observable result | Evidence status in this review |
|---|---|---|---|
| MT01-V01 | Valid polygon and nominal coverage | Coverage geometry generated and edge/cell requirements represented | Unverified in this review |
| MT01-V02 | Invalid or empty polygon | Input rejected with a blocking diagnostic; no candidate released | Unverified in this review |
| MT01-V03 | GSD units and invalid inputs | Unit-normalized result; invalid camera/GSD inputs rejected | Unverified in this review |
| MT01-V04 | Overlap, image step and line spacing | Derived spacing matches selected footprint and overlap inputs | Unverified in this review |
| MT01-V05 | Edge/corner coverage | Boundary gaps are detected or covered by a justified margin | Unverified in this review |
| MT01-V06 | Trigger interval exceeds hardware capability | Payload configuration rejected or plan blocked | Unverified in this review |
| MT01-V07 | Required GSD is infeasible for selected payload | Infeasibility is explicit; no silent relaxation of product criteria | Unverified in this review |
| MT01-V08 | Terrain variation changes surface-relative distance | GSD/clearance evaluated against terrain variation; datum altitude is not substituted for range | Unverified in this review |
| MT01-V09 | Prohibited corridor intersects candidate route | Corridor excluded during constrained-open-space search | Unverified in this review |
| MT01-V10 | Hard restrictions leave no admissible route | No release-eligible candidate; specific infeasibility diagnostic | Unverified in this review |
| MT01-V11 | Reserve/return infeasible | Candidate rejected by energy/recovery gate | Unverified in this review |
| MT01-V12 | Wind changes performance dependencies | Affected performance/trajectory recalculated; unaffected coverage geometry reused | Unverified in this review |
| MT01-V13 | Missing image sequence after flight | Affected coverage cells flagged incomplete/uncertain | Unverified in this review |
| MT01-V14 | Actual overlap below requirement | QA fails or requests targeted reacquisition despite nominal planned overlap | Unverified in this review |
| MT01-V15 | Stale environment snapshot | Release blocked until refreshed/revalidated | Unverified in this review |
| MT01-V16 | Required QA evidence absent | Result remains `UNVERIFIED`, never inferred `PASS` | Unverified in this review |
| MT01-V17 | Multi-sortie/multi-UAV partition | Deliberate stitching overlap and common references preserved | Unverified in this review |
| MT01-V18 | Material input change | Only dependent artifacts invalidated; unrelated artifacts retained | Unverified in this review |

## 5. MT-02 — 3D reconstruction scenarios

| ID | Minimum scenario | Required observable result | Evidence status in this review |
|---|---|---|---|
| MT02-V01 | Flat area with valid nadir coverage | Required patches receive valid observations and geometry gate passes | Unverified in this review |
| MT02-V02 | Vertical facade | Oblique/alternative viewpoints generated where supported; nadir-only insufficiency identified | Unverified in this review |
| MT02-V03 | Occluded/recessed surface | Blocked observation does not count as valid surface coverage | Unverified in this review |
| MT02-V04 | Disconnected observation graph | Geometry gate blocks release or requests additional connecting observations | Unverified in this review |
| MT02-V05 | High overlap but insufficient parallax | Plan rejected or modified; overlap alone cannot pass geometric-strength gate | Unverified in this review |
| MT02-V06 | GSD/point density beyond sensor capability | Sensor/product incompatibility reported; no silent quality downgrade | Unverified in this review |
| MT02-V07 | Unsupported camera/gimbal attitude | Candidate viewpoint rejected or replaced by a valid supported attitude | Unverified in this review |
| MT02-V08 | Unsafe viewpoint | Canonical constrained-open-space planner excludes the unsafe path/viewpoint | Unverified in this review |
| MT02-V09 | No admissible route to required patch | Plan blocked or explicit product-scope decision required | Unverified in this review |
| MT02-V10 | Reserve/recovery infeasibility | Candidate rejected by per-UAV energy and recovery gate | Unverified in this review |
| MT02-V11 | Wind changes route performance/cadence | Dependent performance and acquisition timing recalculated; safety and quality gates rerun | Unverified in this review |
| MT02-V12 | Insufficient multi-UAV stitching observations | Sector assignment fails stitching gate or adds feasible common observations | Unverified in this review |
| MT02-V13 | Incompatible calibration/sensor metadata | Dataset fusion blocked or explicit correction workflow required | Unverified in this review |
| MT02-V14 | Missing images or LiDAR swaths | Affected patches remain incomplete and targeted reacquisition is identified | Unverified in this review |
| MT02-V15 | Georeferencing/checkpoint failure | Product completion blocked by accuracy evidence | Unverified in this review |
| MT02-V16 | Unobserved surface patches | Patch state is `FAIL`/`UNVERIFIED`, not `PASS` | Unverified in this review |
| MT02-V17 | Stale environment/authorization snapshot | Release blocked until the snapshot is refreshed and revalidated | Unverified in this review |
| MT02-V18 | Material input change | Only dependent surface, viewpoint, route and performance artifacts invalidated | Unverified in this review |

## 6. Cross-cutting verification obligations

The scenario suites must additionally verify the contracts that span both templates:

1. Hard constraints cannot be compensated for by lower time, distance or energy cost.
2. Template-level `RELEASE_ELIGIBLE` does not independently promote the Common Mission Model mission to `READY`.
3. Every candidate, selected plan, gate result and post-flight QA record is tied to a mission version and relevant input/model versions.
4. The canonical constrained-open-space route planner remains the sole authoritative route-feasibility path.
5. Incremental recalculation invalidates all dependent results and does not reuse stale results.
6. Post-flight `COMPLETE` requires product-specific evidence; successful route execution/landing is insufficient.
7. Multi-UAV plans preserve individual vehicle/payload/performance/energy checks and parent-mission traceability.
8. Failure cases produce stable diagnostic identifiers suitable for automated assertions and operational explanation.

## 7. Exit criteria for closing a row

A scenario is not closed merely because its specification exists or CI is green. Each row requires:
- a stable automated test ID and deterministic fixture;
- an assertion for the expected state/result and the failure path where applicable;
- evidence that the test runs in the configured CI suite;
- traceability to the relevant requirement/algorithm section;
- HIL or field evidence when the claim concerns real vehicle, sensor or environmental behavior.

**Current conclusion:** MT-01 and MT-02 algorithm specifications define minimum scenario sets. The Shapely geometry regression tests now establish constrained-space clipping, area/component invariants, fail-closed conditions, and route-containment rejection when a segment crosses a clipped exclusion. They still do not close MT01-V09: candidate generation and route-search avoidance of a prohibited corridor remain unverified. Direct test traceability remains unestablished for the full template-specific scenario sets. The 3D adapter test module demonstrates summary aggregation and contract guards; it is not an implementation of the 3D reconstruction algorithm. Do not mark either template fully verified until the scenario rows are linked to executed evidence.
