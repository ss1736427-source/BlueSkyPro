# BlueSky PRO — Algorithm Framework for 13 Mission Templates

**Status:** Engineering baseline for implementation and verification  
**Date:** 2026-11-09

## 1. Architectural decision

Do not build 13 unrelated end-to-end planners. Templates express operator intent; the Algorithm Orchestrator composes reusable capabilities.

```text
TASK + GEOMETRY → INTENT / CONSTRAINTS → TEMPLATE(S) + OBJECTIVE PROFILE
 → DECOMPOSITION → ALGORITHM ORCHESTRATOR → CANDIDATE PLANS
 → INDEPENDENT HARD GATES → TASK-QUALITY EVALUATION
 → BEST ADMISSIBLE PLAN + EXPLANATION + TRACE
```

**Invariants:** (1) Safety, regulatory, vehicle, payload, C2, separation and minimum reserve are hard feasibility gates, never weighted preferences. (2) Optimize mission success and task quality first; optimize energy/time/resource/smoothness only among admissible candidates. (3) Reproducibility requires versioned inputs, data timestamps, calibration, algorithm and parameters. (4) Prefer deterministic explainable baselines; use learning/metaheuristics only where measured complexity justifies them, with independent validation. (5) Recalculate affected dependencies and visibly version the mission. (6) Operator states intent and confirms; no algorithm selection required. (7) Recommendation is not flight authorization.

**Mandatory safety policy:** All 13 templates and all planning modes are governed by [Flight Safety Constraints Policy](FLIGHT_SAFETY_CONSTRAINTS_POLICY.md). Vehicle speed, bank, acceleration, stall margin, altitude, energy/reserve and full-path spatial/environmental checks are non-bypassable feasibility gates. Template-specific logic may add stricter constraints but may not weaken this common policy. A longer turn-out, extended transition or larger course change may be generated to capture a path line, but may never be achieved by exceeding the aircraft envelope. If no admissible trajectory exists, the candidate is infeasible.

## 2. Reusable algorithm capabilities

| Capability | Method families | What it solves |
|---|---|---|
| Area decomposition | Polygon clipping, exact cellular/boustrophedon decomposition, grid/quadtree | Split AOI into obstacle-aware cells |
| Coverage path planning | Sweep-line, lawn-mower/boustrophedon, contour/spiral, grid coverage | Area coverage with swath, overlap, turns and completeness |
| Graph routing | Dijkstra, A*, visibility graph, navigation mesh | Route through validated graph |
| Motion planning | RRT/RRT*, PRM, kinodynamic variants | Feasible motion in complex 2D/3D spaces; still needs trajectory validation |
| Viewpoint planning | Mesh sampling, surface normals, visibility/occlusion, set-cover/ILP, TSP | Select and sequence sensor viewpoints |
| Sequencing | TSP, orienteering, VRP, time-window VRP | Order targets, sites and deliveries |
| Search | Bayesian probability maps, probability-of-detection (PoD), adaptive sweep, information gain | Allocate search effort and update uncertainty |
| Persistent monitoring | Cyclic route optimization, revisit scheduling, age-of-information | Meet revisit/dwell requirements |
| Change detection | Image/point-cloud registration, differencing, tracking, confidence models | Compare against dated baseline |
| Connectivity | Air-to-ground channel/link budget, relay placement, graph connectivity/network flow | Maintain required communications |
| Energy/trajectory | Vehicle performance model, wind-aware energy, constrained optimization, MPC | Feasible trajectory and bounded updates |
| Fleet coordination | Task partition, assignment/min-cost matching, synchronized scheduling, conflict detection | Coordinate heterogeneous UAVs |

A* does not solve image overlap, visibility, detection probability or radio availability. The orchestrator combines methods based on task structure.

## 3. Mapping each template to algorithms

### MT-01 — Картографирование территории
**Objective:** complete AOI coverage at required GSD, image quality and overlap.  
**Pipeline:** validate polygon/exclusions → terrain-aware decomposition → calculate altitude and camera footprint from calibration/GSD → derive line spacing from swath/overlap → boustrophedon/sweep-line → optimize turns/entry/exit → battery-feasible sorties → coverage and image-geometry validation.  
**Methods:** exact cellular decomposition for obstacle-separated regions; boustrophedon for regular polygons; contour/spiral only where geometry/sensor justify; TSP/VRP for disconnected cells/sorties.  
**Inputs:** AOI, DEM, obstacles, camera calibration, GSD, overlap, wind, aircraft model, launch/recovery.  
**Acceptance:** coverage/GSD/overlap thresholds; uncovered regions reported; hard gates and reserve pass. No universal altitude/overlap.

### MT-02 — 3D-картография / реконструкция
**Objective:** reconstruct required surfaces/volume with adequate geometry and bounded occlusion.  
**Pipeline:** target mesh/BIM → candidate viewpoints → normal/standoff/FOV/collision filtering → visibility/occlusion → set-cover/ILP selection → TSP/graph sequencing → camera/trajectory optimization → unobserved/low-confidence surface report.  
**Methods:** mesh sampling, normal-vector filtering, ray-cast visibility, set-cover/ILP, TSP; RRT*/PRM for constrained 3D transit.  
**Inputs:** target mesh/BIM, DEM, camera/LiDAR, FOV, GSD/point density, overlap, standoff, gimbal, aircraft/wind.  
**Acceptance:** surface coverage, observation angles, overlap/triangulation, occlusion, collision and energy checks. Area coverage is not surface coverage.

### MT-03 — Инспекция объектов и инфраструктуры
**Objective:** collect required evidence for each asset/component/defect zone.  
**Pipeline:** identify assets and criteria → required viewpoints/directions → resolution/angle/standoff/illumination limits → remove occluded/unsafe views → sequence → obstacle-aware transit → link evidence to asset/component.  
**Methods:** viewpoint coverage, visibility/occlusion, set-cover/ILP, graph/TSP, RRT*/PRM for constrained 3D transit.  
**Inputs:** asset geometry/model or selected surfaces, points of interest, sensor calibration, resolution/standoff, obstacles, aircraft/gimbal envelope.  
**Acceptance:** all mandatory views at quality threshold, traceable evidence, clearance and reserve. Without geometry, request it or label output reconnaissance-only.

### MT-04 — Мониторинг строительства
**Objective:** repeatable measurement of progress/change against dated design/baseline.  
**Pipeline:** bind AOI and BIM/CAD revision → repeatable acquisition geometry → imagery/point cloud → co-registration → change/volume/feature differences → confidence/coverage gaps → next observation scheduling.  
**Methods:** MT-01/02 coverage and viewpoints, BIM-aware viewpoint selection, registration, change detection, surface differencing, time-series.  
**Inputs:** site boundary, baseline/revision IDs, BIM/IFC or reference data, CRS, sensor calibration, tolerance, cadence.  
**Acceptance:** registration error within tolerance; changes tied to baseline version/time; confidence/occlusion reported. This is a time-series/digital-twin task, not simply repeated mapping.

### MT-05 — Мониторинг территории и периметра
**Objective:** observe sectors/targets within maximum revisit and response times.  
**Pipeline:** sectors/critical points/priorities → sensor visibility/blind spots → patrol graph → revisit/dwell schedule → cyclic route under endurance/C2 → event reprioritization and local replanning.  
**Methods:** graph patrol, TSP/cyclic optimization, persistent monitoring/max-revisit interval, priority orienteering, multi-UAV partition.  
**Inputs:** perimeter/sectors, priorities, sensor footprint/range, revisit/dwell, obstacles, weather/lighting, C2, endurance/bases.  
**Acceptance:** critical sectors meet revisit/dwell; blind spots/missed windows visible; energy/C2 pass. A one-time sweep is not persistent monitoring.

### MT-06 — Поиск и спасение
**Objective:** maximize probability of detection (PoD) and useful information within available time.  
**Pipeline:** last-known position/evidence → prior probability surface/uncertainty → search cells → sensor PoD model by altitude/speed/terrain/visibility/target → effort allocation for cumulative PoD/information gain → sweep/adaptive re-search → Bayesian update from observations → responder/fleet coordination.  
**Methods:** Bayesian search/probability maps, calibrated PoD, probability-weighted coverage, orienteering/resource allocation, information gain, multi-UAV assignment.  
**Inputs:** search area, last-known point/time, drift/movement model if relevant, target and sensor model, visibility/terrain, time budget, responders/fleet.  
**Acceptance:** PoD assumptions, searched probability mass and residual uncertainty exposed; blind zones/stale evidence flagged; no guaranteed detection. Negative observation updates probability only where coverage and detection conditions were adequate.

### MT-07 — Пожарный мониторинг и ЧС
**Objective:** rapidly observe priority hazards and provide georeferenced/time-stamped situational data.  
**Pipeline:** incident perimeter/hotspots/weather → priority/confidence → RGB/thermal viewpoints → route by priority/time/risk → hazard stand-off/prohibited zones → update from detections and wind/fire spread → short-horizon replanning.  
**Methods:** priority orienteering/TSP, hotspot clustering, thermal/RGB detection, change detection, receding-horizon replanning, multi-UAV sector allocation.  
**Inputs:** incident perimeter, hotspot positions/confidence/time, terrain, wind, sensor model, stand-off, airspace, responder priorities/C2.  
**Acceptance:** priority targets observed in windows; timestamps/confidence recorded; hazard, airspace, wind and energy gates pass. BlueSky plans observation, not authority to enter unsafe areas or conduct response actions.

### MT-08 — Экологический и природный мониторинг
**Objective:** repeatable/statistically comparable observations of environmental indicators.  
**Pipeline:** indicators/sampling design → modality → stratify ecological zones/sampling units → coverage/sample points → repeat geometry/time → classification/index/change → calibration, uncertainty and provenance.  
**Methods:** stratified sampling, coverage planning, adaptive sampling/information gain, classification/change detection, time-series, point/line transects where appropriate.  
**Inputs:** AOI, habitat zones, indicators, calibrated RGB/multispectral/thermal/LiDAR, sampling density, season/time, reference data/uncertainty.  
**Acceptance:** representative sampling, calibration/co-registration, uncertainty/missing data, campaign repeatability. Vegetation indices alone do not prove species/pollution/ecological condition without ground truth.

### MT-09 — Сельское хозяйство
**Objective:** actionable crop information or defined treatment with controlled coverage/dose.  
**Pipeline:** field/crop/operation → management zones/cells → sensor or applicator model → flight lines from swath/GSD/overlap/application width → smooth tracks/headland turns → prescription-map adaptation → coverage/dose/drift validation.  
**Methods:** complete coverage path planning, cellular decomposition, boustrophedon tracks, headland/turn optimization, multispectral classification/prescription zoning, variable-rate coverage if supported.  
**Inputs:** field polygon, crop/stage, sensor/applicator calibration, GSD/overlap or swath/dose, DEM/obstacles, wind, buffers, payload/aircraft.  
**Acceptance:** coverage/image quality or dose within tolerance; buffers/weather pass; records georeferenced/auditable. Monitoring, spraying, seeding and spreading require distinct payload models and constraints.

### MT-10 — Доставка грузов
**Objective:** deliver correct payload to required location/condition with safe recoverable energy.  
**Pipeline:** origin/destination/windows → mass/volume/CG/release validation → airspace/obstacle-constrained graph → aircraft-specific wind/payload energy → launch/landing/handoff/recovery → time-window/fleet schedule → replanning.  
**Methods:** constrained shortest path/A*, VRP/time-window VRP, pickup-and-delivery scheduling, energy-constrained routing, contingency/recovery planning.  
**Inputs:** delivery points/windows, cargo properties, landing/release, aircraft, battery/SOH, wind, terrain, obstacles, airspace/recovery sites.  
**Acceptance:** payload compatibility, delivery feasibility, reserve including recovery/contingency, landing/drop-zone suitability, delivery evidence. Shortest distance alone is not the objective.

### MT-11 — Ретрансляция связи
**Objective:** maintain specified link availability, quality and connectivity over time.  
**Pipeline:** service nodes/required links → terrain/building/radio model → link budget/coverage probability → candidate relay positions/altitudes → placement/trajectory optimization for coverage/capacity/latency/energy → end-to-end topology validation → replan on link changes.  
**Methods:** A2G channel/link budget, relay placement, graph connectivity/network flow, multi-objective placement/trajectory, connectivity-aware fleet coordination.  
**Inputs:** ground nodes, frequency/bandwidth, antenna/power, RSSI/SINR/throughput/latency/availability targets, terrain/buildings, interference assumptions, backhaul, endurance/C2.  
**Acceptance:** link budget and coverage probability with margin, connected network/capacity, assumptions/interference limits disclosed. Line of sight is not a reliable radio-link guarantee.

### MT-12 — Аэрофотосъёмка и медиапроизводство
**Objective:** requested visual content with framing, smooth motion and safe trajectory.  
**Pipeline:** shot intent → framing/altitude/speed → camera/gimbal orientation and motion envelope → shot segments/transitions → smoothness/acceleration/jerk/framing optimization → obstacle/wind/lighting checks → executable route.  
**Methods:** shot/waypoint sequencing, spline/polynomial trajectories, minimum-jerk/time-optimal optimization, camera/gimbal target tracking, obstacle-aware motion planning.  
**Inputs:** scene/subject geometry, shot list, camera/lens, gimbal, framing/motion, lighting/time, wind, obstacles, aircraft dynamics.  
**Acceptance:** framing/motion, gimbal/flight envelope, smoothness, time/georeferenced shots. Cinematic smoothness is soft and never overrides safety.

### MT-13 — C-UAS: обнаружение БПЛА
**Objective:** detect, localize, classify and track aerial objects using authorized compatible sensors, exposing confidence/uncertainty.  
**Pipeline:** sensor coverage → per-modality detection → temporal/cross-sensor association → evidence fusion → calibrated classification → track state/uncertainty → continuity/operator alert → evidence/false-alarm log.  
**Methods:** available radar/EO/IR/RF/acoustic sensor models, signal/image detection, multi-target tracking (Kalman/EKF/IMM only where assumptions fit), data association, calibrated Bayesian or other validated fusion.  
**Inputs:** installed sensor calibration, range/FOV/pose, clutter, authorized scope, geospatial context, synchronized timestamps, track history.  
**Acceptance:** detection/localization/tracking against ground truth; false alarms/misses; limitations/confidence; audit trail. Scope is DTI only: no jamming, spoofing, interception or countermeasure authorization.

## 4. Orchestrator selection procedure

1. Classify geometry: area, corridor, points, surface/volume, asset viewpoints, network nodes or moving targets.
2. Load actual sensor model: footprint, FOV, GSD, overlap, PoD, range, visibility, uncertainty and calibration.
3. Build hard constraints: terrain, obstacles, airspace/NOTAM, aircraft envelope, C2, weather, energy and separation.
4. Select the simplest capability combination able to represent the task.
5. Generate candidates and realize dynamically feasible trajectories for actual aircraft/payload.
6. Run independent hard gates and reject failures.
7. Evaluate task quality, uncertainty and missing coverage.
8. Rank admissible candidates: primary outcome → secondary objectives → tie-breakers.
9. Explain selection, rejected alternatives and limiting factor.
10. Persist inputs/source/time, calibration/config versions, algorithm/version/parameters, candidate metrics, gate findings and operator overrides.

## 5. Gaps to add to BlueSky contracts

Existing Mission Model/Objectives already define hard-gate hierarchy, environment, vehicle/payload, geometry, traceability and heterogeneous fleet. Make these implementation contracts explicit:

1. **Sensor performance model:** calibrated footprint/FOV, GSD, overlap, PoD, range, viewing-angle limits, latency and uncertainty; capability flags alone are insufficient.
2. **Task-quality model:** measurable success criteria and evidence/coverage accounting; route generation alone is not task success.
3. **Uncertainty/confidence:** data quality, stale inputs, model uncertainty, confidence and unobserved regions.
4. **Time model:** observation windows, revisit, dwell, deadlines, synchronization and freshness.
5. **Recovery/contingency:** reserve includes recovery, alternate landing, lost-link and degraded-weather cases where supported.
6. **Dynamic replanning:** event-triggered bounded-horizon updates, dependency invalidation and visible mission-version changes.
7. **Provenance:** coordinate reference, baseline/model/sensor calibration/version/time follow every result.
8. **Fleet coordination:** partition by capability/sensor, synchronize and deconflict; never assume identical aircraft/endurance.
9. **Algorithm capability registry:** supported geometry/dimensions, sensor assumptions, constraints, determinism/realtime properties, failure modes and evidence.
10. **Graceful degradation:** missing data prompts operator or yields explicitly reduced scope; never invent substitutes or silently relax thresholds.

## 6. Implementation order

**A — Shared contracts/primitives:** stable template→profile→capability IDs; geometry/sensor/quality/uncertainty/time/provenance schemas; audit/reuse existing coverage, graph, route, energy, NOTAM and fleet code; deterministic baselines and tests.

**B — First capability chain:** MT-01 mapping → MT-02 3D viewpoint/occlusion → MT-03 inspection/evidence → MT-04 baseline registration/change → MT-09 agriculture/field zones and payload modes.

**C — Temporal/probabilistic/network:** MT-05 persistent monitoring; MT-06 probabilistic search; MT-07 incident/hotspot priorities; MT-08 environmental sampling/time series; MT-11 connectivity-aware relay.

**D — Logistics/media/DTI:** MT-10 recovery-aware VRP; MT-12 camera/trajectory shots; MT-13 authorized DTI fusion/tracking.

**E — Fleet/runtime:** heterogeneous assignment, synchronization, event-triggered replanning with weather/NOTAM/telemetry/geometry; SITL/virtual-flight/field evidence.

## 7. Verification

Every template requires positive, ambiguous/missing-input, incompatible-payload, hard-gate rejection, stale-environment, energy-failure, quality-failure and operator-override scenarios. Multi-UAV tasks additionally require allocation/conflict scenarios. Metrics are measured against reference geometry, calibrated sensor models and representative simulation/field data. A unit test does not prove operational survey or detection performance.

## 8. External technical references

1. Kumar et al., *Comprehensive Review of Path Planning Techniques for UAVs*, ACM Computing Surveys, 2025 — sampling, potential-field, bio-inspired and AI methods; constraints/uncertainty. https://doi.org/10.1145/3737280
2. Sun et al., *A Survey on Coverage Path Planning Method for UAV*, Control Theory & Applications, 2026 — environment models, CPP classes and classical/heuristic/metaheuristic/learning methods. https://doi.org/10.7641/CTA.2024.40006
3. Höffmann, *Optimal guidance track generation for precision agriculture*, Journal of Field Robotics, 2024 — guidance tracks, traversal sequence, smooth path, cellular decomposition. https://doi.org/10.1002/rob.22286
4. *Coverage Path Planning Methods Focusing on Energy Efficient and Cooperative Strategies for UAVs*, 2022 — decomposition, boustrophedon/spiral/grid, multi-UAV trade-offs. https://doi.org/10.3390/s22031235
5. Li et al., *A Path Planning Algorithm for UAV 3D Surface Inspection Based on Normal Vector Filtering and Integrated Viewpoint Evaluation*, 2025 — viewpoint sampling/selection and sequencing. https://doi.org/10.1002/rob.22530
6. SINTEF, *Sampling-Based 3D Aerial Inspection Path Planning*, 2025 — mesh viewpoints, RRT*, collision clearance, camera/gimbal. https://www.sintef.no/en/publications/publication/10265990/
7. Abdulateef et al., *Autonomous BIM-Aware UAV Path Planning for Construction Inspection*, 2025 — IFC/BIM, ILP viewpoints, GSD/stereo quality, TSP, battery epochs. https://www.mdpi.com/2673-7418/5/4/79
8. Hari et al., *Optimal UAV Route Planning for Persistent Monitoring Missions*, IEEE Transactions on Robotics, 2021 — cyclic revisit under endurance constraints. https://doi.org/10.1109/TRO.2020.3032171
9. *Drone Routing for Drone-Based Delivery Systems: A Review*, 2023 — payload, routing, charging and security. https://pmc.ncbi.nlm.nih.gov/articles/PMC9919662/
10. Won et al., *A survey on UAV placement and trajectory optimization in communication networks*, ICT Express, 2023 — A2G channels, relay placement and trajectory. https://doi.org/10.1016/j.icte.2022.01.015
11. *Communication-aware path planning in multi-UAV networks: A systematic review*, Vehicular Communications, 2026 — coupled route/network/resource constraints. https://doi.org/10.1016/j.vehcom.2026.101043
12. European Commission JRC, *Technical developments in counter-drone technology: C-UAS detection, tracking and identification*, 2025 — DTI, sensor modality limitations and fusion. https://publications.jrc.ec.europa.eu/repository/handle/JRC140692
13. ISO/CD 25461, *UAS Counter-UAS functional requirements for detection, localization and identification*, committee draft, 2026 — DTI architecture; countermeasures excluded. https://www.iso.org/standard/90482.html

These sources establish method families, not BlueSky implementation status, certification or universal thresholds. Numeric limits must come from aircraft, payload calibration, task requirements, operating rules and verification evidence.


## 9. MT-01 implementation status — 2026-11-09

A first deterministic 2D sweep-line candidate generator is registered in
`04_SOFTWARE/PLANNING/coverage_path_planner.cpp` and built as part of
`bluesky_planning`. Its public input/result contract is in
`coverage_path_planner.hpp`; the standalone regression target is
`coverage_path_planner_test`.

Implemented baseline:
- camera/GSD-derived AGL altitude and altitude-bound rejection;
- image footprint, cross-track line spacing and forward image-trigger spacing calculations;
- local equirectangular projection with a 50 km extent limit;
- horizontal scanline intersection and subtraction of exclusion polygons;
- deterministic scanline ordering, alternating direction per scanline;
- rejection of degenerate and self-intersecting AOI/exclusion rings;
- normalization of a repeated closing vertex in AOI and exclusion rings;
- rejection of exclusion polygons that cross the AOI boundary, lie partly outside it, or overlap/contain another exclusion polygon;
- explicit coverage-segment and scanline counts;
- coverage-leg-only length/time metrics, excluding turns and transit.

Not implemented and therefore not to be inferred from this baseline:
- camera latency compensation, trigger behavior during turns/transitions, and flight-controller command integration;
- explicit endpoint capture stations are emitted for each straight coverage leg; this does not prove edge coverage because the camera footprint/overscan and trigger timing are not modeled;
- cellular decomposition for complex concave/multi-component operational areas;
- connecting transit/turn trajectories between disjoint segments or scanlines;
- DEM terrain following, obstacle avoidance, wind and aircraft energy model;
- geometric coverage-union proof, achieved coverage percentage or uncovered-area proof;
- NOTAM/airspace, separation, vehicle/payload capability and independent safety gates;
- AUTO application-service integration or MANUAL route-generation integration.

The planner result status `CANDIDATE_2D_COVERAGE_ONLY` is intentionally not a
flight-ready route or authorization. Its time metric is coverage-leg time only.
The regression test is necessary but not sufficient evidence of operational
survey performance. CI build/test must pass before this baseline is accepted.


### Vehicle motion model software baseline — 2026-11-09

A first reusable C++ contract is now present in
`04_SOFTWARE/PLANNING/model/vehicle_motion_model.hpp`, with regression coverage in
`04_SOFTWARE/PLANNING/vehicle_motion_model_test.cpp` and CTest registration.

Implemented:
- fixed-wing coordinated level-turn radius from explicit TAS and bank angle;
- multirotor idealized horizontal-turn radius from explicit air-relative speed and sustained lateral acceleration;
- exact model/version/configuration identity and source provenance requirements;
- speed-envelope and physical-input validation;
- `DerivedScenario` estimates remain advisory and cannot satisfy the vehicle-motion hard gate;
- only `OfficialValidated` or `FlightTestValidated` evidence can set the evaluator's hard-gate eligibility flag.

Not implemented by this evaluator:
- wind-corrected ground-track turn geometry;
- VTOL hover/transition/recovery dynamics;
- bounded-curvature or acceleration-limited trajectory generation;
- collision/restriction checks over generated turn geometry;
- energy and independent route-integrity validation.

The evaluator returns an air-relative idealized radius only. Its hard-gate eligibility flag is necessary but not sufficient for route acceptance. No route is made flight-ready by this calculation.

### MT-01 next engineering gate — vehicle-constrained transitions

The next implementation increment must not connect coverage segments with unqualified straight lines. A transition is a trajectory candidate and must satisfy the same hard feasibility boundary as every other route segment.

Required vehicle motion contract:
- explicit airframe/motion class: fixed-wing, multirotor, or other supported class;
- validated minimum turn radius or a validated speed/bank/lateral-acceleration envelope, with units and source/version;
- applicable airspeed range and the speed at which the turn constraint is valid;
- for fixed-wing, a coordinated-turn model may derive radius from speed and bank angle only when both are authoritative for the configured aircraft and operating condition; do not apply this formula universally to multirotors;
- multirotor stop/yaw/translate behavior requires its own validated dynamic limits and must not be represented as fixed-wing Dubins motion.

**Calculated-data policy:** when manufacturer maneuverability data is absent, the vehicle reference dataset may provide explicitly assumed scenario curves (e.g. fixed-wing bank-angle sweeps or multirotor lateral-acceleration sweeps). These are advisory sensitivity inputs only. They must carry `DERIVED_SCENARIO` / `UNVALIDATED_ASSUMPTION` provenance and cannot satisfy the validated motion-model hard gate, establish a minimum turn radius, or promote a candidate to flight-ready. A scenario-based candidate must remain `MOTION_MODEL_UNVERIFIED` until exact-configuration limits are read back and validated with representative evidence. Wind-dependent ground-track geometry is evaluated separately from air-relative turn geometry.

Required transition result contract:
- separate coverage legs from turn, transit and reposition segments;
- retain segment type, geometry, length, speed profile, vehicle-model identity/version and calculation-input identity;
- report transition distance/time separately from coverage distance/time;
- reject unsupported or incomplete vehicle motion models rather than substituting a generic radius.

Required feasibility checks:
- validate the complete transition geometry, not only its endpoints;
- test the transition against AOI/exclusion geometry and the versioned constrained-environment snapshot, including altitude datum compatibility;
- do not assume turns may leave the AOI; an explicit permitted turn-area/operating-domain policy is required;
- later integrate terrain clearance, wind/performance, energy reserve, NOTAM/airspace and independent route validation before any candidate can be promoted.

Acceptance tests must include radius/curvature bounds, speed-envelope violations, turn-area boundary violations, exclusion intersection, restricted-environment intersection, unsupported vehicle class, missing model provenance and deterministic output. No transition result may imply flight readiness or authorization.

Implementation sequencing:
1. introduce the versioned vehicle motion-model contract and input validation;
2. implement a bounded trajectory primitive for one explicitly supported vehicle class;
3. validate its entire geometry against restrictions and permitted turn domain;
4. add segment-typed route output and separate distance/time metrics;
5. integrate independent route validation and only then connect it to the MT-01 candidate pipeline.

Until these gates are implemented and tested, MT-01 remains a 2D coverage-leg candidate generator. No transition distance, total mission time, turn feasibility or flight-ready route is claimed.


### Adaptive fixed-wing turn speed — stall-margin constraint

The vehicle-motion model now includes a preliminary deterministic speed-selection function for fixed-wing coordinated turns.

Implemented contract:
- retain the current speed when it already meets the requested turn radius; reduce speed only when the current-speed radius is too large, then evaluate the lowest admissible turn speed;
- calculate bank-adjusted stall speed as Vs_bank = Vs_clean / sqrt(cos(bank)) for the coordinated level-turn approximation;
- apply an explicit safety-margin factor greater than 1.0;
- enforce the configured aircraft minimum/maximum speed envelope;
- select the minimum safe speed only if its resulting idealized turn radius meets the requested radius;
- reject the transition if the required radius would demand a speed below the calculated safe minimum or if the stall-margin speed exceeds the configured envelope;
- require exact aircraft/configuration identity, source provenance and an explicit TAS datum for both turn speed and stall-speed input; IAS/CAS values must first be converted using an authoritative atmosphere/configuration model;
- keep derived/unvalidated aircraft data advisory; only official-validated or flight-test-validated inputs can be eligible for the motion hard gate.

Safety and scope limitations:
- the margin factor is an explicit aircraft/operator-approved input, not a universal constant; a sample test value does not establish an operational threshold;
- clean stall speed must correspond to the exact mass, CG, flap/propulsion/airframe configuration and applicable operating condition; use the appropriate configuration-specific stall data;
- the bank-load approximation does not model gusts, angle-of-attack protection, accelerated stall transients, turbulence, sideslip or control-law behavior;
- this function is a ground-planning estimate, not a real-time flight-control command or stall-protection system;
- wind-corrected ground track, turn trajectory integration, altitude/terrain, airspace, obstacle, energy/reserve and independent route-integrity gates remain separate required checks;
- if validated stall data, speed datum, margin or configuration is missing, the planner must not invent a value or authorize speed reduction.

Regression tests cover safe minimum selection, radius infeasibility, missing stall data, invalid margin, mismatched speed datum and an insufficient maximum-speed envelope. The code-level test still requires successful CI execution; it does not establish flight-test validation.
