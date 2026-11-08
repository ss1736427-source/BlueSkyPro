# BlueSky PRO — Algorithm Capability Registry Design

**Status:** IMPLEMENTATION DESIGN / PHASE A  
**Scope:** reusable algorithm capabilities for the 13 mission templates  
**Decision:** one versioned capability registry; templates reference capabilities, not concrete solver implementations.

## 1. Architectural decision

BlueSky PRO shall not maintain 13 separate planners. The 13 mission templates define operational intent and required result quality. The Algorithm Orchestrator decomposes each mission into planning subproblems and selects compatible, validated capabilities from a common registry.

The registry is a catalogue of what a planning component can do. It is not itself a route planner, safety authority, or flight-release mechanism.

```text
MISSION TEMPLATE + TASK DATA
          |
          v
OBJECTIVE PROFILE
          |
          v
TASK DECOMPOSITION
          |
          v
CAPABILITY REQUIREMENTS
          |
          v
CAPABILITY REGISTRY
          |
          v
ELIGIBILITY FILTER -> SOLVER EXECUTION
          |
          v
COMMON CANDIDATE CONTRACT
          |
          v
INDEPENDENT HARD-CONSTRAINT VALIDATION
          |
          v
MISSION-QUALITY RANKING
          |
          v
SELECTED PLAN + EXPLANATION + TRACE
```

## 2. Verified repository baseline

The following components already exist on `main`:

- `04_SOFTWARE/PLANNING/orchestrator/algorithm_orchestrator.*` — orchestrator and deterministic/objective/route-validation tests.
- `04_SOFTWARE/PLANNING/solver_contract/solver_contract.hpp` — common solver interface, mission problem, compute budget, candidate and solver metadata.
- `04_SOFTWARE/PLANNING/solvers/astar_solver.*` and `dijkstra_solver.*` — two graph route solvers.
- `04_SOFTWARE/PLANNING/constrained_open_space.*`, `constrained_planning_graph.*`, `route_constraint_validator.*`, `notam_prohibited_zone_validator.*` — spatial constraints and route validation components.
- `wind_performance_trajectory.*`, multi-UAV sequencing/conflict/zonal planning and selected-route components.
- `BLUESKY_ALGORITHM_ORCHESTRATOR_BASELINE.md`, `BLUESKY_SOLVER_CONTRACT_BASELINE.md` and `BLUESKY_SOLVER_PORTFOLIO_AND_DECISION_ENGINE_BASELINE.md` — existing architectural baselines.

Important implementation observations from the current source:

1. The solver interface already has `metadata()`, `eligible()`, `run()` and `cancel()`.
2. Current solver metadata is sparse: solver ID/version, problem classes, incremental-replan flag and deterministic flag. It does not yet describe required inputs, supported constraints, output-quality dimensions, vehicle/equipment requirements, qualification state or resource estimates.
3. The current orchestrator iterates over solvers sequentially. The architecture documents describe bounded parallel/staged evaluation, but that behaviour is not established by the inspected implementation.
4. The current compiled solver portfolio contains A* and Dijkstra. They solve graph point-to-point routing; they do not implement area coverage, 3D viewpoint generation, image-quality planning, search theory, delivery scheduling or relay placement.
5. Candidate output currently has route elements and basic time/energy/reserve/objective fields. It does not yet carry the complete mission-quality, uncertainty, evidence-coverage, provenance and input-dependency record required by all 13 templates.

These are implementation gaps, not reasons to replace the existing architecture. The existing contracts and validated route/safety components should be extended incrementally.

## 3. Registry entry contract

Each capability entry shall have the following logical fields. The exact C++ representation and serialization format must be finalized before runtime integration.

| Field | Meaning |
|---|---|
| `capability_id` | Stable unique identifier, independent of implementation |
| `display_name` | Operator/diagnostic-friendly name |
| `capability_version` | Version of the capability contract |
| `implementation_id` | Solver/plugin implementing the capability |
| `implementation_version` | Exact implementation version |
| `problem_classes` | Supported subproblem types: point, corridor, area, volume, network, schedule, search, etc. |
| `geometry_types` | Supported geometry/topology |
| `required_inputs` | Mandatory data with units, coordinate reference and validity requirements |
| `optional_inputs` | Inputs that improve quality but are not always required |
| `supported_constraints` | Constraints the implementation can correctly consume |
| `vehicle_requirements` | Required aircraft performance or flight mode |
| `equipment_requirements` | Sensor, payload, camera, thermal, communication or other equipment |
| `output_metrics` | Metrics and evidence the capability can produce |
| `uncertainty_model` | Known uncertainty/confidence output, or explicit `NOT_SUPPORTED` |
| `incremental_replan` | Whether and under which changes partial recomputation is supported |
| `determinism` | Deterministic/stochastic behaviour and seed requirements |
| `resource_profile` | Expected compute time/memory and candidate limits |
| `validation_status` | Experimental, tested, qualified, suspended |
| `safety_class` | Planning-only; never grants flight authority |
| `provenance` | Source, test evidence, benchmark and approved version |

Missing required input, unsupported mandatory constraint, invalid data, or an unqualified implementation must make the capability ineligible for that run. The orchestrator must not silently downgrade a mandatory requirement.

## 4. Capability families and template mapping

The mapping below is a target registry taxonomy. It does not claim that all capabilities are already implemented.

| Template | Required capability families | Main output / acceptance evidence |
|---|---|---|
| MT-01 Mapping | AOI decomposition; coverage tracks; sensor footprint/GSD; overlap; constrained routing; sortie sequencing | Coverage percentage, GSD, forward/side overlap, uncovered cells |
| MT-02 3D reconstruction | Surface/mesh sampling; viewpoint generation; normal-angle checks; visibility/occlusion; set cover; viewpoint routing | Surface coverage, view-angle distribution, occluded/unobserved regions |
| MT-03 Inspection | Asset/defect target model; required viewpoints; visibility; evidence capture; viewpoint routing | Required asset faces/points observed, evidence completeness, image quality |
| MT-04 Construction | Repeatable survey; coordinate registration; point-cloud processing; BIM alignment; change detection | Registration error, observed area/volume, change confidence, time-series trace |
| MT-05 Perimeter | Perimeter graph; patrol coverage; cyclic routing; revisit/dwell scheduler | Boundary coverage, revisit interval, blind-time and patrol completion |
| MT-06 Search and rescue | Prior probability map; Bayesian update; sensor detection/PoD model; adaptive search; information gain | Probability mass searched, detection probability estimate, uncertainty and evidence |
| MT-07 Fire / incident | Hotspot/incident prioritization; thermal/RGB sensing; constrained orienteering; receding-horizon replanning | Time-to-observe, hotspot coverage, confidence, freshness of observations |
| MT-08 Ecology | Stratified sampling; habitat/zone coverage; adaptive sampling; repeat observation | Sampling representativeness, spatial coverage, uncertainty and repeatability |
| MT-09 Agriculture | Field decomposition; coverage tracks; headland/turn planning; prescription zoning; task-mode constraints | Field coverage, application/sensing quality, missed strips, prescription compliance |
| MT-10 Delivery | Payload/vehicle compatibility; constrained routing; VRP/assignment; time windows; energy/recovery | Delivery completion, arrival window, payload integrity, reserve and recovery feasibility |
| MT-11 Communication relay | A2G link budget; propagation/channel model; relay placement; network connectivity/capacity | Coverage, link margin, connectivity, latency/capacity and robustness |
| MT-12 Media | Shot/scene sequencing; camera/gimbal framing; visibility; smooth trajectory generation | Shot completion, framing/blur constraints, trajectory smoothness and safety |
| MT-13 C-UAS detection and tracking | Sensor detection; localization; classification; sensor fusion; track association; confidence management | Detection/track confidence, false-alarm handling, coverage and traceability |

MT-13 is limited to detection, tracking and identification support. It does not authorize jamming, spoofing, interception or countermeasures.

## 5. Selection and execution rules

1. Template/objective profile produces a set of required capabilities and measurable quality criteria.
2. The orchestrator filters registry entries by problem class, geometry, inputs, constraints, vehicle/equipment compatibility, validation status and compute budget.
3. If a mandatory capability is unavailable, the system reports the exact missing capability/data and blocks or marks the affected mission result incomplete. It must not substitute an unrelated algorithm.
4. Independent eligible capabilities may run concurrently only after bounded execution, cancellation, thread-safety and deterministic decision rules are implemented and tested. Until then, the existing sequential execution remains the known behaviour.
5. Each solver candidate passes the same independent spatial, regulatory, vehicle, energy, recovery and mission-specific validation gates.
6. Only admissible candidates are ranked. Mission quality is evaluated before efficiency where the objective profile requires it; hard safety constraints remain non-negotiable.
7. The selected result records registry version, capability IDs, implementation versions, input versions, rejected candidates/reasons, validation evidence and decision explanation.

## 6. Data-quality and degradation rules

- Every input declares units, coordinate reference, timestamp/validity interval, source and quality/confidence.
- DEM, weather, NOTAM/airspace, UAV performance and sensor calibration must be tied to the spatial and temporal scope of the mission.
- A missing optional input may reduce a reported quality dimension. A missing mandatory input makes the relevant capability ineligible.
- Uncertainty must be reported; unknown is not equivalent to zero uncertainty.
- A partial solution must be labelled partial and cannot be presented as complete mission coverage.
- Stale environment or authorization dependencies invalidate only the affected results, following the dependency-driven recalculation policy.
- Capability qualification is evidence-based: unit/contract tests alone do not establish operational fitness.

## 7. Implementation sequence

### Phase A1 — Contract and registry
- Extend solver metadata without breaking existing solver implementations.
- Add a registry with stable capability IDs and eligibility explanations.
- Add schema validation and tests for missing inputs, unsupported constraints, incompatible vehicle/equipment and unqualified solvers.

### Phase A2 — Common candidate evidence
- Extend candidate results with task-quality metrics, coverage/evidence, uncertainty, input/dependency identity, provenance and termination state.
- Keep hard-constraint validation independent of solver-reported feasibility.

### Phase B — First operational capabilities
- Start with MT-01 mapping, MT-02 3D and MT-03 inspection.
- Reuse existing constrained-open-space, graph routing, route validation, wind/performance, energy and recovery components.
- Implement actual coverage/viewpoint generation as separate capabilities; do not treat A*/Dijkstra as coverage planners.

### Phase C — Additional domain capabilities
- Add MT-04/05/08/09 and then MT-06/07/10/11/12/13 with domain-specific evidence and qualification plans.

### Phase D — Parallel execution and runtime replanning
- Implement bounded concurrency, cancellation, best-admissible-so-far and incremental dependency invalidation.
- Benchmark preparation time and quality against deterministic reference cases.

## 8. Acceptance criteria

The registry is accepted when:
- all 13 template profiles resolve to explicit capability requirements;
- each capability has machine-readable input/constraint/output declarations;
- the orchestrator can explain why a capability is eligible or rejected;
- missing mandatory data cannot silently produce a complete-looking result;
- only independently validated admissible candidates enter ranking;
- all selected outputs are reproducible and traceable;
- each template has a domain-specific verification dataset and measurable acceptance thresholds;
- no planning capability can release a flight or override authorization/safety gates.

## 9. Explicit implementation status

**Completed:** 13-template objective/capability mapping in the planning documentation; common solver interface; graph routing solvers; route/airspace/terrain constraint components; wind/performance and multi-UAV planning components.

**Not yet demonstrated by the inspected code:** a complete machine-readable capability registry; 13 operational mission planners; domain-specific coverage/viewpoint/search/relay/media solvers; complete evidence/uncertainty metrics for every template; bounded parallel solver execution.

This distinction is mandatory in project status reporting. Architecture mapping is not equivalent to algorithm implementation or operational qualification.
