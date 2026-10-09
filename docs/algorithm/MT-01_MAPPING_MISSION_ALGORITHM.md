# BlueSky PRO — MT-01 Mapping Mission Algorithm

**Template ID:** MT-01  
**Operator-facing name:** Картографирование территории  
**Status:** WORKING ALGORITHM BASELINE — review and verification required  
**Scope:** Planning, validating, executing and assessing an area-mapping mission that produces geospatial imagery/products.  
**Parent contracts:** `08_PLANNING/BLUESKY_MISSION_TEMPLATE_CATALOG.md`, `08_PLANNING/BLUESKY_MISSION_MODEL.md`, `08_PLANNING/BLUESKY_MISSION_OBJECTIVE_PROFILES.md`, `08_PLANNING/BLUESKY_ROUTE_CALCULATION_AND_OPTIMIZATION_SPEC.md`, `docs/algorithm/PHOTOGRAMMETRY_SOLUTION_METHOD.md`.

## 1. Purpose and boundary

MT-01 converts the operator's mapping task into an admissible, measurable acquisition plan. The operator specifies the required product and acceptance criteria; BlueSky derives the camera configuration, surface-relative acquisition geometry, coverage tracks, image-trigger schedule, UAV assignment and execution sequence.

MT-01 does not create a parallel route planner. It generates mapping-specific coverage requirements and acquisition geometry, then uses the canonical constrained-open-space route pipeline for every generated route, transition, return and recovery segment.

MT-01 is distinct from MT-02. MT-01 primarily seeks complete, sufficiently uniform area coverage and fit-for-purpose 2D/terrain products. If the requested deliverable requires reconstruction of building facades, complex objects, multiple viewing directions or a 3D mesh/point cloud with geometric-completeness criteria, BlueSky shall additionally activate MT-02 or explicitly identify the product requirement as requiring 3D acquisition.

## 2. Inputs and provenance

The planner shall create a versioned input snapshot containing, where applicable:

- mission ID/version, operator and task boundary;
- required deliverable(s), coordinate reference system and vertical datum;
- target GSD and product accuracy/quality requirements;
- required area coverage and edge-coverage criteria;
- camera/sensor, lens, calibration and shutter model;
- image dimensions, focal length and pixel pitch or equivalent sensor metadata;
- camera orientation and gimbal limits;
- supported image-trigger modes and minimum trigger interval;
- UAV performance envelope, payload, battery state/health and degradation coefficient;
- terrain/surface model, obstacles, airspace, NOTAM and other applicable restrictions;
- wind/weather and illumination forecast with validity timestamps;
- positioning strategy (GNSS, RTK, PPK, GCP/checkpoint or hybrid);
- storage capacity, expected data rate and available mission time;
- operational launch, recovery and contingency locations.

Each source records identity, version or timestamp, validity/freshness and quality where available. Missing mandatory inputs produce a specific blocking diagnostic; the planner must not silently substitute an assumed value. Optional missing inputs may use a documented conservative fallback only when the governing requirements permit it, and the plan must expose that fallback.

## 3. State machine

The mission progresses through explicit states:

`DRAFT → INPUTS_VALID → PRODUCT_DEFINED → ACQUISITION_CONFIGURED → COVERAGE_GENERATED → ROUTES_VALIDATED → PERFORMANCE_EVALUATED → QUALITY_GATES_PASSED → RELEASE_ELIGIBLE → EXECUTING → POSTFLIGHT_QA → COMPLETE`

Failure or invalidation states include `BLOCKED_INPUT`, `NO_ADMISSIBLE_PLAN`, `QUALITY_REQUIREMENTS_UNMET`, `STALE_PLAN`, `ABORTED` and `REACQUISITION_REQUIRED`.

A state transition is allowed only when its entry criteria are satisfied. A highlighted template in the UI does not mean the mission is authorized, validated or ready for flight.

## 4. Deterministic planning procedure

### Step 1 — Normalize the mapping request

1. Validate polygon/area geometry, coordinate reference system, geometry validity and units.
2. Resolve the required product class: orthomosaic, 2D map imagery, digital surface/terrain model, or another explicitly supported mapping deliverable.
3. Resolve measurable acceptance criteria: target GSD, minimum coverage, positional accuracy, image/metadata completeness and any processing-specific conditions.
4. Separate mandatory constraints from optimization objectives.
5. Record any unresolved ambiguity as a blocking input rather than inventing a requirement.

**Output:** normalized task definition and acceptance-criteria set.

### Step 2 — Build the environment snapshot

1. Retrieve the current terrain/surface model, obstacles, airspace restrictions, NOTAM and applicable operating constraints.
2. Resolve authorization only from explicit, current, scope-matching evidence; never infer an exemption.
3. Construct the constrained-open-space model before route search.
4. Identify areas that are physically inaccessible, prohibited or altitude/time constrained.
5. Record source freshness and mark the plan stale if required data is invalid or expired.

**Output:** versioned environment snapshot and feasible spatial domain.

### Step 3 — Select and validate the payload configuration

1. Match the installed camera/sensor to the required product and target GSD.
2. Verify sensor dimensions/resolution, focal length, calibration identity, shutter characteristics, focus mode, image format, trigger interval, gimbal range and storage requirements.
3. Select a stable focal length for a mapping block where supported; do not vary zoom within a block unless the processing/calibration workflow explicitly supports it.
4. Lock focus where supported and validated; define exposure, shutter and ISO limits for the actual sensor and operating conditions.
5. Verify that the planned image rate and data volume are within camera, storage and transfer limits.
6. Reject a configuration that cannot satisfy the required acquisition conditions.

**Output:** versioned payload configuration and capability verdict.

### Step 4 — Derive camera-to-surface distance from GSD

For a near-nadir camera over a locally planar surface, the nominal relationship is:

`GSD = (H_s × p) / f`

where:
- `GSD` is ground sampling distance in the same linear units as `p`;
- `H_s` is camera-to-surface distance;
- `p` is sensor pixel pitch;
- `f` is focal length.

Therefore:

`H_s = (GSD_target × f) / p`

If pixel pitch is unavailable but sensor width and image width in pixels are known, derive pixel pitch from those values. The implementation must normalize units before calculation and validate all denominators and input ranges.

This relation is a first-order estimate, not a complete accuracy guarantee. The planner must account for camera orientation, lens model, terrain elevation, surface slope, focal-length configuration and the required product accuracy. For non-flat terrain, compute local camera-to-surface distance along the planned acquisition geometry.

Do not confuse altitude above a reference datum with distance to the mapped surface. The latter controls nominal GSD; terrain clearance and legal altitude limits remain independent hard constraints.

**Output:** target surface-relative distance, predicted GSD and uncertainty/limitations.

### Step 5 — Derive image footprint and trigger spacing

Using the selected camera model and surface-relative distance, calculate the usable ground footprint in the along-track and cross-track directions. Use the calibrated camera model and orientation when available; do not assume that sensor width and height map to the same ground axes for oblique or rotated cameras.

For nominal footprint dimensions `L` along track and `W` across track:

`image_step = L × (1 − front_overlap)`

`flight_line_spacing = W × (1 − side_overlap)`

`trigger_interval = image_step / planned_ground_speed`

These equations are nominal geometric relations. The final trigger schedule must also respect the camera's minimum interval, motion blur, image write time, GNSS/event timing, terrain variation and the actual ground speed under wind.

Baseline values from the photogrammetry design method are at least 75% forward and 60% side overlap for ordinary nadir mapping. Higher starting targets may be required for complex terrain, vegetation, low-texture surfaces or demanding products. These are configurable planning baselines, not universal constants. The selected values and rationale must be stored in the mission record.

If the required overlap cannot be achieved at the proposed speed, BlueSky shall adjust speed, height, trigger interval or line spacing within validated equipment and safety limits. It must not report the target overlap as achieved solely because it was configured.

**Output:** predicted footprint, line spacing, trigger schedule and overlap targets.

### Step 6 — Generate coverage geometry

1. Distinguish the operator's task polygon from the image-coverage boundary.
2. Compute the perimeter extension needed to cover the task boundary using the actual footprint and geometry; do not use an arbitrary fixed buffer.
3. Generate candidate line orientations from polygon geometry, terrain/surface shape, number and length of lines, turn count, wind, illumination, overlap consistency and expected energy/time.
4. Select a candidate orientation using the MT-01 objective profile; the longest polygon axis is a heuristic, not a mandatory rule.
5. Generate parallel coverage tracks and exposure locations.
6. Check first/last line, first/last exposure, corners and edge strips for coverage holes.
7. Model turns and acceleration/deceleration separately from the image-acquisition segment. Do not count turn areas as covered unless usable imagery actually covers them.
8. Split the area into coverage cells only when necessary for geometry, route feasibility, vehicle capability, endurance or computational limits.
9. Preserve explicit coverage-cell and track identifiers for later progress and post-flight QA.

**Output:** coverage geometry with per-cell requirements and predicted coverage.

### Step 7 — Route the coverage tracks through constrained open space

1. Pass each generated track, transition and required mission segment to the canonical route calculation pipeline.
2. Incorporate airspace/NOTAM, authorization scope, terrain, obstacles, altitude limits and mandatory points before route search.
3. Do not generate a prohibited corridor as a normal selectable candidate and rely on a later warning to reject it.
4. Preserve the intended image-acquisition geometry while planning safe transit and turn paths.
5. If a coverage track cannot be routed safely, mark the affected cell infeasible and attempt a permitted alternative orientation, cell decomposition or altitude strategy.
6. Do not shrink the required survey area or lower quality criteria silently to make the route feasible.

**Output:** route candidates tied to coverage tracks, plus explicit infeasibility diagnostics.

### Step 8 — Assign UAV and evaluate wind/performance

1. Match the payload and coverage workload to a compatible UAV configuration.
2. Evaluate each relevant route/UAV state using the current wind and vehicle-performance model.
3. Calculate ground speed, track, segment time, energy, climb/descent effects and the resulting 4D trajectory.
4. Recompute image trigger timing when actual predicted ground speed changes.
5. Validate the energy requirement, required reserve, return/recovery feasibility, battery health/degradation and applicable propulsion limits.
6. If one flight cannot cover the task within admissible limits, partition it into multiple sorties or compatible UAV subtasks, preserving deliberate overlap between adjacent blocks.
7. Reuse unaffected calculation artifacts. Recalculate only results whose declared dependencies changed.

**Output:** vehicle-specific route/performance/trajectory artifacts and an energy feasibility verdict.

### Step 9 — Estimate data volume and mission duration

Estimate, at minimum:
- image count by track and coverage cell;
- expected image size and total data volume;
- storage margin;
- acquisition time, transit time, turns and expected recovery time;
- transfer time where relevant;
- expected energy and reserve.

Do not double-count overlapping trajectory time when aggregating concurrent UAVs. For multi-UAV missions, report both total vehicle effort and mission makespan. If image size or write latency is not known precisely, identify the estimate as provisional and retain its source.

**Output:** resource estimate with assumptions and uncertainty.

### Step 10 — Apply hard gates before optimization

A candidate is inadmissible if it violates any applicable hard constraint, including:
- regulatory or airspace restrictions without a valid matching authorization;
- terrain/obstacle clearance or altitude envelope;
- UAV, camera, gimbal or payload capability;
- C2 or operational constraints;
- minimum energy reserve or return feasibility;
- camera trigger/storage limits that prevent the required acquisition;
- a mandatory quality requirement that the acquisition geometry cannot satisfy.

Hard gates are not weighted penalties. A shorter route, lower estimated energy or faster completion cannot compensate for a failed gate.

**Output:** admissible candidate set and reason-coded rejection list.

### Step 11 — Compare admissible candidates

Compare candidates hierarchically:
1. required coverage completeness and product-quality feasibility;
2. consistency of target GSD and overlap across the survey area;
3. positioning/georeferencing strategy feasibility;
4. energy reserve and operational robustness;
5. propulsion/resource consumption;
6. total mission time and makespan;
7. route length, turn count and computational cost as tie-breakers.

The exact ranking between quality criteria must come from the mission's acceptance criteria and objective profile. Do not collapse the decision into a single weighted score that allows an unacceptable product to win because it saves energy.

If no candidate meets mandatory product criteria, return `QUALITY_REQUIREMENTS_UNMET` or `NO_ADMISSIBLE_PLAN` with the failed criteria and permitted adjustments.

**Output:** selected admissible plan, alternatives and concise decision rationale.

### Step 12 — Final integrity and change-impact check

Before release:
1. verify all required calculation artifacts are present and reference compatible input versions;
2. verify the selected route set, coverage geometry, camera configuration and performance results are mutually consistent;
3. verify no relevant environment, UAV, payload, weather or task input changed after calculation;
4. invalidate and recalculate only affected dependent results if a material input changed;
5. confirm all required gates pass and the authorization state is separately resolved.

Final validation checks integrity and changed inputs; it must not rerun every calculation unconditionally.

**Output:** `RELEASE_ELIGIBLE` only when all required checks pass; otherwise a specific blocked state.

## 5. In-flight monitoring and adaptation

Monitor, where supported:
- position and distance to the planned surface;
- ground speed and actual trigger cadence;
- image capture acknowledgements and sequence gaps;
- camera orientation, exposure and blur indicators;
- GNSS/RTK/PPK state and event timing;
- storage capacity;
- wind deviation;
- battery/energy margin;
- coverage-cell progress and route deviation.

When a deviation occurs, determine its impact on coverage, overlap, energy and safety. Do not automatically replan the whole mission for an isolated non-material change. Recalculate affected segments and downstream results only. Any replacement route must pass the same hard constraints before being sent to execution.

If image acquisition is missing or quality is below a required threshold, mark the affected coverage cells as uncertain or incomplete and schedule a safe reacquisition only if remaining resources and operating conditions permit.

## 6. Post-flight quality assurance

Landing alone does not complete MT-01. Compare planned and actual acquisition data:

1. planned versus captured image count by track/cell;
2. missing, duplicate or corrupt images;
3. timestamp, geotag and camera metadata integrity;
4. actual footprint and coverage completeness;
5. forward, side and block overlap;
6. sharpness, motion blur and exposure quality;
7. GSD consistency;
8. RTK/PPK status and georeferencing quality;
9. GCP/checkpoint residuals where applicable;
10. holes, weakly observed edges and processing failures;
11. final product coordinate reference and required accuracy.

Classify each criterion as `PASS`, `FAIL`, or `UNVERIFIED` with evidence. Do not assign `PASS` when only a planned value exists and no actual acquisition/product evidence is available.

If a required criterion fails, identify the affected area and produce a targeted reacquisition task. Preserve links to the original mission version and dataset; reacquisition creates a traceable revision rather than overwriting historical evidence.

## 7. Incremental recalculation contract

Every material result shall identify its inputs/dependencies, algorithm version, timestamp and affected coverage/route IDs.

Examples:
- **Wind update:** recalculate affected trajectory, ground speed, trigger timing, time and energy; retain coverage geometry if the surface-relative acquisition geometry remains valid.
- **Camera/lens/GSD change:** invalidate footprint, trigger spacing, line spacing, coverage geometry and downstream route/performance estimates as applicable.
- **Terrain/surface model change:** invalidate affected surface distances, clearance checks, footprint/overlap predictions and affected route/performance artifacts.
- **Boundary change:** invalidate affected coverage cells, tracks, routes and all dependent estimates.
- **Battery health/state change:** invalidate affected performance, energy feasibility and candidate ranking; do not rebuild unrelated coverage geometry.
- **Airspace/NOTAM change:** update the constrained-open-space model and reroute affected segments; preserve unrelated acquisition parameters unless their dependencies also changed.

Dependency invalidation must be explicit. Cached results may be reused only when their input/dependency identity matches the current plan.

## 8. Minimum verification scenarios

The implementation shall have automated tests for at least:

1. valid polygon and complete nominal coverage generation;
2. invalid/empty polygon rejection;
3. GSD calculation with unit normalization and invalid-input rejection;
4. overlap-to-image-step and line-spacing calculation;
5. edge/corner coverage margin;
6. camera trigger interval below hardware capability;
7. inability to meet required GSD with the selected payload;
8. terrain variation that changes surface-relative distance;
9. prohibited route corridor excluded before candidate selection;
10. no admissible route due to a hard restriction;
11. insufficient energy reserve and infeasible return;
12. wind change invalidating performance but not unaffected geometry;
13. missing image sequence detected in post-flight QA;
14. insufficient actual overlap detected despite a planned overlap target;
15. stale environment snapshot blocking release;
16. required QA evidence absent results in `UNVERIFIED`, not `PASS`;
17. multi-sortie or multi-UAV partition preserves deliberate stitching overlap;
18. material input change invalidates only dependent artifacts.

Test fixtures are not flight evidence. Real-UAV/HIL acceptance remains open until supported by actual test records.

## 9. Traceability and mission outputs

The MT-01 result shall link:
- mission ID and version;
- task and product requirements;
- environment snapshot/source versions;
- payload and calibration identity;
- coverage cells, tracks and route IDs;
- GSD/overlap/trigger calculations;
- vehicle assignment and performance artifacts;
- energy/reserve calculations;
- hard-gate results and rejection reasons;
- selected-plan rationale;
- execution logs and captured dataset;
- post-flight QA results and reacquisition tasks.

## 10. Completion criteria

MT-01 planning is `RELEASE_ELIGIBLE` only when:
- the task and product acceptance criteria are explicit;
- payload capability and acquisition configuration are valid;
- predicted coverage/GSD/overlap meet required thresholds;
- all route segments are feasible in the current constrained environment;
- wind-adjusted performance and energy reserve are admissible;
- data/storage limits are feasible;
- required regulatory authorization is separately resolved;
- all calculation artifacts refer to compatible input versions;
- the final integrity/change-impact check passes.

MT-01 is `COMPLETE` only when the post-flight quality criteria required by the mission have evidence-backed results. A mission can be safely flown but remain incomplete if the required mapping product is not demonstrated to meet its acceptance criteria.

**Verification status:** This document defines the intended algorithm and minimum verification scenarios. It does not itself prove that software implementation, automated tests, CI, HIL or real-UAV performance has passed.
