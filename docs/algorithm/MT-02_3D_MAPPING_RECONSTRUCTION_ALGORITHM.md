# BlueSky PRO — MT-02 3D Mapping / Reconstruction Mission Algorithm

**Template ID:** MT-02  
**Operator-facing name:** 3D-картография / реконструкция  
**Status:** WORKING ALGORITHM BASELINE — review and verification required  
**Scope:** Planning and validation of image/LiDAR acquisition for 3D surface, structure, object, point-cloud or mesh reconstruction.  
**Parent contracts:** `08_PLANNING/BLUESKY_MISSION_TEMPLATE_CATALOG.md`, `08_PLANNING/BLUESKY_MISSION_MODEL.md`, `08_PLANNING/BLUESKY_MISSION_OBJECTIVE_PROFILES.md`, `08_PLANNING/BLUESKY_ROUTE_CALCULATION_AND_OPTIMIZATION_SPEC.md`, `docs/algorithm/PHOTOGRAMMETRY_SOLUTION_METHOD.md`.

## 1. Purpose and distinction from MT-01

MT-02 plans data acquisition for a geometrically complete and sufficiently constrained 3D reconstruction. The planning target is not merely to cover a ground polygon. It is to observe the required surfaces from useful positions and viewing directions, with sufficient image overlap, parallax, scale consistency, georeferencing and visibility.

The operator defines the reconstruction target, required output and acceptance criteria. BlueSky derives the acquisition strategy, camera/sensor configuration, surface patches, viewpoints, flight paths and resource plan. The algorithm must not require the operator to choose the internal optimization method.

MT-02 reuses the canonical constrained-open-space route pipeline and verified vehicle-performance artifacts. It does not implement a parallel route planner. When a mission needs both a broad orthomosaic and detailed 3D reconstruction, the Common Mission Model may combine MT-01 and MT-02 as complementary task templates under one mission identity.

## 2. Required inputs

Create a versioned acquisition input snapshot containing, where applicable:

- target area, object or structure geometry;
- required product type: point cloud, textured mesh, digital surface model, as-built model or other supported 3D deliverable;
- required completeness, geometric accuracy, resolution/GSD and coordinate reference;
- target surface patches and regions of interest;
- terrain/3D prior model, if available, and its quality;
- camera/sensor intrinsics, calibration, focal length, pixel dimensions and shutter model;
- gimbal range, camera attitude limits and supported oblique/nadir capture modes;
- RTK/PPK/GNSS/INS capabilities and event timing;
- optional LiDAR range, field of view, scan pattern, return characteristics and point-density requirements;
- UAV performance envelope, payload, battery health/degradation and storage;
- terrain, obstacles, airspace, NOTAM and current restrictions;
- wind, weather, illumination and time validity;
- launch/recovery points, operational time window and contingency constraints;
- processing pipeline requirements and supported sensor combinations.

If a required surface, product criterion, sensor capability or coordinate reference is unknown, record a blocking diagnostic. Do not invent a numerical accuracy requirement or treat an unavailable prior model as ground truth.

## 3. State machine

`DRAFT → INPUTS_VALID → PRODUCT_DEFINED → SURFACE_MODEL_READY → SENSOR_CONFIGURED → VIEWPOINTS_GENERATED → ROUTES_VALIDATED → PERFORMANCE_EVALUATED → GEOMETRY_GATES_PASSED → RELEASE_ELIGIBLE → EXECUTING → DATA_QA → RECONSTRUCTION_QA → COMPLETE`

Failure/invalidation states include `BLOCKED_INPUT`, `NO_ADMISSIBLE_PLAN`, `INSUFFICIENT_VISIBILITY`, `GEOMETRY_REQUIREMENTS_UNMET`, `STALE_PLAN`, `REACQUISITION_REQUIRED` and `UNVERIFIED`.

A state cannot advance merely because a route exists or waypoints have been flown. Release eligibility requires predicted acquisition geometry to meet configured criteria; mission completion requires evidence-backed post-flight checks at the level required by the selected product.

### 3.1 Relationship to the Common Mission Model

The states above are **MT-02 internal workflow states**, not a replacement for the canonical mission lifecycle in `08_PLANNING/BLUESKY_MISSION_MODEL.md`. They shall be mapped into the common mission version and its lifecycle: planning/calculation states contribute to `CALCULATING` and `VALIDATING`; `RELEASE_ELIGIBLE` means only that the MT-02 acquisition plan has passed its own geometry, route and resource gates; the overall mission may become `READY` only after all applicable mission-level safety, regulatory, C2, vehicle, energy, authorization and other validation gates pass. Execution and post-flight states update the same traceable mission lineage; MT-02 `COMPLETE` means its required 3D product passed evidence-backed reconstruction QA, not that unrelated mission-level obligations can be skipped.

Candidate ranking follows the Mission Objective Profile hierarchy: hard admissibility first, required surface completeness and reconstruction quality next, then secondary objectives such as energy efficiency and time, with route smoothness or computation cost only as tie-breakers. A soft-objective improvement must never compensate for failed safety, regulatory, vehicle, C2 or minimum-reserve requirements.


## 4. Planning procedure

### Step 1 — Formalize the reconstruction product

1. Resolve the target and required 3D product.
2. Identify required surfaces, edges, recesses, facades, roofs, ground interfaces and other regions of interest.
3. Define acceptance criteria: surface coverage/completeness, minimum observation count or viewing diversity where specified, GSD/point density, georeferencing accuracy and reconstruction quality.
4. Distinguish mandatory product criteria from optimization objectives such as time and energy.
5. Determine whether the requested result is achievable with the installed sensor and available processing workflow.

**Output:** product specification and measurable acceptance criteria.

### Step 2 — Build or import the target surface model

1. Use a suitable terrain/3D prior model when available.
2. If no model exists, construct an initial geometric envelope from the operator's target geometry, map/terrain data or a safe preliminary survey, and explicitly record its uncertainty.
3. Segment the target into surface patches with stable IDs.
4. Estimate each patch's location, normal, slope, aspect, dimensions and visibility limitations when the data supports it.
5. Identify likely occlusion regions: behind edges, under overhangs, between structures, deep recesses and surfaces hidden from nadir views.
6. Mark unknown geometry as uncertain rather than treating it as empty space.
7. Keep the target surface model distinct from the safety/obstacle model; a target surface is not itself authorization to approach it.

**Output:** versioned surface-patch model with quality/uncertainty metadata.

### Step 3 — Select acquisition modality and sensor configuration

Select among supported photogrammetry, LiDAR or a compatible combined strategy based on product criteria, surface texture, geometry, lighting, range and equipment capability.

For photogrammetry, validate camera calibration, focal length, focus, shutter, exposure, image timing and usable gimbal attitudes. For LiDAR, validate range envelope, field of view, scan geometry, point-density capability, motion compensation and positioning/attitude inputs. For mixed sensors, define time/coordinate synchronization and registration requirements.

A modality that cannot demonstrate the required coverage or quality must be rejected or marked as requiring an operator-approved change to product requirements. BlueSky must not silently substitute a lower-quality product.

**Output:** sensor plan and capability verdict.

### Step 4 — Determine surface-relative range, GSD and sampling density

For image-based acquisition, derive nominal GSD and camera-to-surface distance from the calibrated camera model and target resolution. For LiDAR, estimate point spacing/density from range, scan pattern, angular sampling, platform motion and sensor characteristics.

Use local surface geometry rather than a single global altitude for complex targets. Verify that the planned range remains inside both the sensor's useful measurement envelope and the UAV's safe operating envelope.

For photogrammetry, target GSD is necessary but not sufficient: it does not by itself guarantee 3D accuracy. Reconstruction also depends on calibration, parallax, viewing diversity, image quality, control and processing.

**Output:** predicted spatial sampling/range per relevant surface patch, including uncertainty.

### Step 5 — Generate required observation viewpoints

For every required surface patch, generate candidate observations based on:
- visibility and line of sight;
- surface normal and viewing angle;
- required GSD or point density;
- useful baseline/parallax for photogrammetry;
- overlap with neighboring observations;
- camera/gimbal attitude and sensor limits;
- range and incidence-angle constraints;
- illumination and surface texture where relevant;
- safe vehicle clearance and route feasibility.

A candidate viewpoint is useful only if it contributes valid observations of the target surface and can be reached safely. Do not count an occluded or severely oblique observation as coverage merely because its nominal camera footprint intersects the patch.

Nadir observations may cover horizontal surfaces effectively but shall not be assumed sufficient for vertical facades, overhangs or complex objects. Generate oblique, multi-height, orbit or other viewpoints only when supported by the platform, sensor and safe route geometry.

**Output:** candidate observation set with patch IDs, visibility, predicted sampling and viewpoint metadata.

### Step 6 — Build the observation graph and test geometric strength

Build a task-specific graph linking observations that can share features or provide complementary geometry. Evaluate, as applicable:
- image overlap between consecutive and adjacent observations;
- cross-strip/block overlap;
- overlap between nadir and oblique datasets;
- multi-angle observations of each required surface;
- expected parallax and baseline distribution;
- connectedness of the image/observation network;
- weakly textured, repetitive or reflective regions;
- common tie features between separate blocks, sorties or UAVs.

The graph must not assume that high overlap alone guarantees a stable reconstruction. Multiple images from nearly identical positions may provide poor depth geometry; useful viewing diversity and a connected network are required.

Where control points are used, plan distributed GCPs and reserve independent checkpoints for validation. RTK/PPK improves direct georeferencing but does not remove the need to verify reconstruction quality against the mission's accuracy requirements.

**Output:** geometrically connected candidate acquisition graph and predicted weak regions.

### Step 7 — Calculate overlap and acquisition cadence

For image acquisition, derive image footprint and trigger spacing from the selected camera, range, orientation, surface geometry and target overlap. Use the photogrammetry solution method's baseline values as starting points only; demanding 3D structures generally require higher overlap and deliberate multi-angle observations.

For LiDAR, derive scan/track spacing and revisit geometry from the sensor model and target point-density/coverage criteria. Do not apply camera image-overlap percentages as a substitute for LiDAR sampling metrics.

For all modalities:
1. calculate acquisition cadence against actual predicted ground speed;
2. include camera/sensor throughput limits;
3. account for turns and acceleration/deceleration;
4. model coverage of edge and corner patches;
5. include deliberate overlap between blocks, sorties and UAV sectors;
6. estimate the impact of wind and platform motion.

**Output:** modality-specific sampling schedule and coverage predictions.

### Step 8 — Plan routes through the canonical constrained-open-space model

1. Send candidate observation sequences and transitions to the existing route planner.
2. Incorporate current airspace/NOTAM, authorization scope, terrain, obstacles, altitude limits and vehicle performance constraints before route search.
3. Preserve required observation attitude and range constraints while finding feasible paths between viewpoints.
4. Exclude unsafe or prohibited corridors from normal route candidates.
5. If a viewpoint is unreachable, seek a safe alternative viewpoint that still satisfies the surface-coverage requirement.
6. If no alternative meets the criterion, identify the affected patch and report infeasibility rather than silently dropping it.

**Output:** route candidates linked to observation and surface-patch IDs.

### Step 9 — Assign vehicles and estimate wind-adjusted performance

Match each acquisition subtask to a compatible UAV/sensor configuration. Calculate wind-adjusted trajectories, ground speed, segment time, energy, range/point density or image-trigger timing, and return/recovery feasibility.

For multi-UAV operations:
- partition surface patches while preserving deliberate shared observations at sector boundaries;
- consider sensor compatibility, resolution, camera calibration and coordinate/time synchronization;
- avoid treating each UAV dataset as an independent island;
- calculate group makespan and individual energy reserves;
- retain cross-sector registration requirements.

Use incremental recalculation: a wind update normally invalidates affected trajectories/performance and possibly acquisition cadence, not the entire surface-patch model or sensor calibration.

**Output:** vehicle-specific plans, resource estimates and registration/coordination constraints.

### Step 10 — Apply hard feasibility and geometry gates

Reject a candidate if any applicable hard requirement fails, including:
- regulatory/airspace or physical clearance;
- vehicle, payload, sensor range or attitude limits;
- C2 or required operating conditions;
- energy reserve and recovery feasibility;
- required surface patches unreachable or not observable;
- insufficient predicted sampling/GSD or point density;
- disconnected/weak acquisition geometry where the product requires connected reconstruction;
- required overlap/viewing diversity not achieved;
- data storage or sensor throughput insufficient for the acquisition plan.

Hard requirements are not soft penalties. A fast, low-energy plan cannot win if it leaves required surfaces unobserved or lacks the geometric strength required for the target product.

**Output:** admissible candidate set with reason-coded failures per patch and requirement.

### Step 11 — Compare admissible plans

Compare candidates hierarchically:
1. safety, regulatory and vehicle admissibility;
2. required surface completeness and observation visibility;
3. geometric strength, sampling quality and predicted reconstruction suitability;
4. georeferencing/control strategy feasibility;
5. energy reserve and robustness;
6. propulsion/resource use;
7. mission makespan, acquisition time and transition cost.

Use the MT-02 objective profile; do not select solely by shortest route, highest image count or maximum overlap percentage. Excessive acquisition can waste energy and storage without improving the required product. The objective is the smallest robust acquisition plan that satisfies the specified reconstruction criteria with appropriate margins.

If no candidate passes, return `GEOMETRY_REQUIREMENTS_UNMET` or `NO_ADMISSIBLE_PLAN` with the uncovered/weak surface patches and permitted adjustments.

**Output:** selected plan, alternatives, patch-level coverage forecast and rationale.

### Step 12 — Final integrity and release check

Before release:
1. verify the selected routes, viewpoint graph, sensor configuration, surface model and performance artifacts reference compatible versions;
2. verify all required surface patches have a predicted valid observation plan;
3. verify all hard gates and quality gates;
4. verify environment and authorization freshness;
5. invalidate and recalculate only results affected by material input changes;
6. retain the final decision rationale and all rejected-candidate reasons needed for audit.

**Output:** `RELEASE_ELIGIBLE` only when all required gates pass.

## 5. In-flight monitoring

Monitor, where supported:
- actual route and viewpoint completion;
- camera/gimbal attitude and sensor range;
- image capture or LiDAR acquisition acknowledgements;
- image blur/exposure and sensor-health indicators;
- GNSS/RTK/PPK/INS quality and time synchronization;
- storage and data throughput;
- wind, speed and energy margin;
- surface patches whose required observations remain incomplete.

If a viewpoint is missed or sensor quality degrades, update the affected patch's observation state. Replan only affected tasks when safe and feasible. Do not mark a patch complete merely because the UAV passed its waypoint.

Any replacement trajectory must pass the same canonical safety and performance checks before execution.

## 6. Post-flight reconstruction QA

A successful landing or completed route is not proof of a successful 3D product. Verify, as required by the product specification:

1. data completeness and file integrity;
2. image/LiDAR timestamp and pose metadata;
3. calibration and sensor configuration consistency;
4. actual surface coverage and missing patches;
5. image overlap and viewpoint diversity, or LiDAR point density/scan coverage;
6. image sharpness/exposure or sensor quality flags;
7. acquisition graph connectivity and weakly connected blocks;
8. georeferencing and registration quality;
9. GCP/checkpoint residuals where applicable;
10. reconstruction holes, outliers, alignment failures and surface distortion;
11. final product coordinate reference, resolution and required accuracy;
12. cross-UAV/block registration and seam quality where applicable.

Each criterion is recorded as `PASS`, `FAIL` or `UNVERIFIED` with evidence. Planned overlap, expected point density or route completion cannot substitute for actual product evidence.

For failed criteria, identify the smallest affected surface region and produce a targeted reacquisition task with required viewpoints and connection to the existing dataset. Preserve original datasets and mission versions for traceability.

## 7. Incremental recalculation contract

- **Target/product criteria change:** invalidate required patch set, viewpoint selection, geometry gates and dependent routes/performance.
- **Surface model change:** invalidate affected visibility, range, viewpoint, clearance and route results.
- **Camera/lens/calibration change:** invalidate image footprint, GSD, trigger schedule, geometric-strength assessment and dependent routes as needed.
- **Sensor modality change:** invalidate modality-specific sampling and quality calculations; do not reuse incompatible metrics.
- **Wind change:** recalculate affected trajectories, time, energy and cadence where ground speed changes.
- **Airspace/NOTAM change:** update constrained open space and reroute affected segments.
- **UAV/battery change:** recalculate compatibility, performance, energy and assignment; retain unaffected target geometry where still valid.
- **Missing/poor-quality acquisition data:** update actual observation coverage and generate targeted reacquisition tasks; do not rewrite the original plan's predicted state.

Each result must retain input/dependency identity, algorithm version, timestamp, affected patch/route IDs and provenance.

## 8. Minimum verification scenarios

Automated tests shall cover at least:

1. flat area with valid nadir coverage;
2. vertical facade requiring oblique observations;
3. occluded/recessed surface that cannot be counted as covered from a blocked viewpoint;
4. disconnected observation graph;
5. insufficient parallax despite high nominal overlap;
6. target GSD or LiDAR point density beyond sensor capability;
7. unsupported camera/gimbal attitude;
8. unsafe viewpoint excluded by the constrained-open-space planner;
9. no admissible route to a required patch;
10. energy reserve or recovery infeasibility;
11. wind change affecting route/performance and acquisition cadence;
12. multi-UAV sector boundaries with insufficient stitching observations;
13. incompatible calibration or sensor metadata across datasets;
14. missing images or incomplete LiDAR swaths detected after flight;
15. georeferencing/checkpoint failure prevents completion;
16. unobserved patches produce `FAIL` or `UNVERIFIED`, not `PASS`;
17. stale environment/authorization snapshot blocks release;
18. material input change invalidates only dependent artifacts.

Fixtures and simulations are not evidence of real-UAV performance. HIL and field verification remain open until supported by actual records.

## 9. Traceability

The MT-02 record shall link:
- mission ID/version and product requirements;
- target surface model/version and patch IDs;
- sensor/payload/calibration identity;
- candidate and selected viewpoints;
- observation graph and geometric-quality predictions;
- routes and vehicle-performance artifacts;
- energy/reserve calculations;
- safety/authorization and quality-gate results;
- selected-plan rationale;
- raw datasets and actual pose/timing metadata;
- reconstruction outputs, QA evidence and reacquisition tasks.

## 10. Completion criteria

MT-02 planning is `RELEASE_ELIGIBLE` only when required target surfaces have a feasible observation plan, sensor and vehicle capabilities are compatible, predicted sampling and geometry meet configured thresholds, all routes are admissible, resource margins pass, and the final integrity/change-impact check succeeds.

MT-02 is `COMPLETE` only when the required reconstruction product passes its evidence-backed post-flight quality criteria. If the system has only planned routes or raw data without sufficient reconstruction evidence, the state remains `UNVERIFIED` or `REACQUISITION_REQUIRED`, as applicable.

**Verification status:** This document defines intended behavior and minimum verification scenarios. It does not prove that implementation, automated tests, CI, HIL or real-UAV performance has passed.

## 11. Current implementation boundary: 3D Mapping adapter

The current `schemas/validator/three_d_mapping_adapter.py` is a **summary/presentation adapter for upstream verified artifacts**, not the MT-02 viewpoint planner, geometry validator or release authority. Its current result contract contains:

| Adapter field | Current source/meaning | MT-02 requirement not established by this field alone |
|---|---|---|
| `route_length_m` | Sum of verified route lengths | Whether every required surface patch is observed |
| `expected_duration_s` | Time span from earliest trajectory start to latest trajectory end | Whether observation ordering, synchronization and makespan assumptions are valid for the selected plan |
| `expected_energy_wh` | Sum of upstream performance energy values | Whether each UAV individually preserves its required reserve and recovery feasibility |
| `required_reserve_wh` | Optional supplied value | A verified reserve calculation or per-UAV reserve proof |
| `line_spacing_m`, `line_count` | Optional supplied acquisition summary | Viewpoint visibility, parallax, occlusion handling or 3D observation-graph connectivity |
| `expected_frames`, `expected_coverage_percent`, `data_volume_mb` | Optional supplied acquisition estimates | Evidence that the estimates were derived from the selected sensor/model or satisfy product acceptance criteria |
| `release_status`, `verified` | Status supplied to the adapter; `verified` is true only for `FINAL_CHECK_PASS` or `RELEASE_ELIGIBLE` | Independent proof that the MT-02 hard gates and patch-level geometry criteria actually passed |

The adapter currently checks that route, performance and trajectory lists are non-empty/aligned and that their items are marked verified; it also rejects a non-positive aggregate route length and an inverted overall time range. It does **not** independently validate surface visibility, viewpoint diversity, observation-graph connectivity, patch-level coverage, modality-specific sampling, per-UAV reserve, or reconstruction QA. Those checks must remain in their authoritative upstream planning/validation stages and be tied to the same mission version.

### Required integration rule

1. MT-02 produces or references versioned target patches, candidate/selected viewpoints, visibility and sampling evidence, observation-graph checks, and patch-level predicted coverage.
2. The canonical constrained-open-space planner produces admissible routes; the existing verified performance/trajectory pipeline supplies route-level performance artifacts.
3. MT-02 hard gates evaluate the complete evidence set, including per-UAV energy/reserve and required geometry/coverage criteria.
4. Only after those gates pass may the upstream release authority supply `RELEASE_ELIGIBLE` to the adapter. The adapter's `verified` flag is a propagated summary status, not a substitute for gate evaluation.
5. Post-flight `COMPLETE` requires separate reconstruction and QA evidence; adapter verification or route completion alone must never set it.

**Implementation gap:** the current adapter result schema does not expose the MT-02 observation graph, patch-level coverage, visibility/viewpoint diversity, sensor modality, quality-gate evidence, or dependency/version identifiers. Do not infer these capabilities from the presence of the adapter. A future integration change should add a versioned contract for these artifacts or link to their authoritative records without duplicating the route planner or treating optional numeric summary fields as proof of quality.

