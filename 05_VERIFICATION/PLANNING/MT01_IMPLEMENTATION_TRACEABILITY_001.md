---
id: MT01-IMPLEMENTATION-TRACEABILITY-001
type: implementation_traceability
status: draft_for_agreement
scope: MT-01
---

# MT-01 Implementation Traceability 001

## 1. Purpose

Controlled mapping of the MT-01 algorithm specification (Sections 24 and 27) to the current C++ planning implementation and existing unit tests.

This document distinguishes implemented reusable infrastructure, partial implementation, missing MT-01-specific implementation, and existing lower-level tests that cannot substitute for the MT-01 verification case.

## 2. Controlled mapping

| MT-01 algorithm stage | Required function | Current implementation | Existing test | Status |
|---|---|---|---|---|
| 0. Input integrity | Validate mission, geometry, CRS, terrain, equipment, energy, environment and authorization inputs | Mission model and several lower-level validators exist; no complete MT-01 input-integrity orchestrator found | mission_model_contract_test; lower-level tests | PARTIAL |
| 1. Constrained open space | Build feasible spatial domain from restrictions, altitude, terrain/obstacle, UAV and safety constraints | constrained_open_space.*, constrained_planning_graph.*, notam_prohibited_zone_validator.* | corresponding unit tests | PARTIAL / REUSABLE |
| 2. Acquisition geometry | Derive GSD, footprint, track spacing, image spacing, trigger/frame requirements from controlled camera model | `acquisition_geometry.*` implements controlled sensor geometry, GSD, footprint, spacing and trigger timing with fail-closed input validation | `acquisition_geometry_test.cpp` | IMPLEMENTED — first implementation slice |
| 3. Terrain following | Maintain controlled camera-to-surface distance under terrain/obstacle/altitude/vehicle constraints | flight_profile.*, vertical/route infrastructure exists; no photogrammetry terrain-following planner found | flight-profile / vertical tests | PARTIAL |
| 4. Orientation candidates | Generate and evaluate bounded survey orientations using controlled geometry/search parameters | `coverage_orientation.*` generates deterministic bounded orientation candidates, projected width and estimated track count; downstream wind/energy evaluation remains separate | `coverage_orientation_test.cpp` | IMPLEMENTED — first generation slice |
| 5. Cellular decomposition | Decompose AOI into planning cells where required | `coverage_decomposition.*` provides deterministic orientation-aligned strip cells with explicit altitude band and dependency identity; controlled polygon splitting for active polygon restrictions is integrated, while terrain/obstacle clipping and full multi-level decomposition remain open | `coverage_decomposition_test.cpp` | IMPLEMENTED — first geometric slice + polygon split |
| 6. Coverage tracks | Generate parallel acquisition tracks, clip to geometry, enforce footprint coverage and endpoints | `CoverageTrackGenerator` generates deterministic sweep-line intervals, splits intervals around active polygon/circle restrictions, preserves cell/orientation identity and records track length/altitude; footprint/endpoint feasibility remains open | `coverage_decomposition_test.cpp` | IMPLEMENTED — first generation slice |
| 7. Edge coverage | Deliberately validate/repair boundary and corner acquisition coverage | `CoverageEdgeEngine` evaluates track endpoint distance to cell boundary against the controlled half-footprint requirement and retains localized gap diagnostics; repair is not yet claimed | `coverage_decomposition_test.cpp` | IMPLEMENTED — first evaluation slice |
| 8. Transition graph | Connect acquisition tracks with feasible transition costs | generic planning graph / route infrastructure exists; no MT-01 acquisition-transition graph found | graph/route tests | PARTIAL |
| 9. Route candidates | Generate bounded route candidates from coverage/transition alternatives | Algorithm Orchestrator and solver contract can manage candidates; no MT-01 coverage candidate producer found | orchestrator tests | PARTIAL |
| 10. Wind-aware performance | Apply wind/performance model and determine segment time/energy | wind_performance_trajectory.* | wind_performance_trajectory_test.cpp | IMPLEMENTED REUSABLE |
| 11. Mission energy | Include mission phases, reserve and energy feasibility | energy model/source registers exist; trajectory engine enforces usable energy + reserve; complete MT-01 phase model not found | wind/trajectory tests; NAV tests elsewhere | PARTIAL |
| 12. 4D trajectory | Produce time-parameterized route/profile with altitude and energy | wind_performance_trajectory.*, flight_profile.*, vertical route infrastructure | corresponding unit tests | PARTIAL / REUSABLE |
| 13. Acquisition event validation | Validate position, camera-surface distance, orientation, footprint, GSD, overlap and trigger state | `AcquisitionEventValidator` generates deterministic event records from controlled tracks and `AcquisitionGeometryResult`, preserving camera-ground distance, GSD, footprint and trigger state; orientation/overlap quality gates remain open | `coverage_decomposition_test.cpp` | IMPLEMENTED — first event-model slice |
| 14. Mapping quality | Coverage ratio, uncovered geometry, GSD, overlap, acquisition validity, terrain following, sensor compliance | `MappingQualityEngine` derives explicit `covered_geometry` from deterministic footprint union, then evaluates coverage ratio and uncovered geometry; full quality gates remain open | `coverage_decomposition_test.cpp` | IMPLEMENTED — first exact covered-geometry slice |
| 15. Candidate selection | Hard admissibility first, then objective comparison/tie-break | candidate_comparison.*, orchestrator/* | candidate/orchestrator tests | IMPLEMENTED REUSABLE |
| 16. Final integrity | Verify mission identity, route/configuration/input lineage | final_planning_integrity.*, mission/compiler infrastructure | final_planning_integrity_test.cpp | IMPLEMENTED REUSABLE |

## 3. Existing reusable implementation

### 3.1 Spatial constraints

`constrained_open_space.*` provides coordinate validity, polygon/circle restrictions, altitude overlap, fail-closed invalid geometry behaviour and dependency identity. `constrained_planning_graph.*` filters graph edges against the constrained environment.

This is reusable infrastructure, not an AOI coverage planner.

### 3.2 Route validation

`route_constraint_validator.*` validates waypoint references, altitude limits and route connectivity. It does not generate acquisition geometry.

### 3.3 Wind/performance/trajectory

`wind_performance_trajectory.*` provides segment wind input, wind tolerance, ground speed/track, traversal time, climb/descent timing, segment energy, protected reserve gate, 4D points and dependency identity.

This is the strongest directly reusable MT-01 implementation block currently identified.

### 3.4 Candidate orchestration

`candidate_comparison.*` and `orchestrator/*` provide candidate feasibility/ranking infrastructure and deterministic selection behaviour. They do not generate MT-01 coverage candidates.

### 3.5 Final integrity

`final_planning_integrity.*` verifies mission identity, route/profile/vehicle binding and calculation-input identity. It is a downstream integrity gate, not a substitute for mapping-quality validation.

## 4. Critical implementation gaps

### GAP-MT01-001 — Acquisition Geometry Engine

Status: **CLOSED FOR FIRST IMPLEMENTATION SLICE; INTEGRATION REMAINS OPEN.**

Implemented in `04_SOFTWARE/PLANNING/acquisition_geometry.hpp/.cpp` with contract test `acquisition_geometry_test.cpp` and registered in `CMakeLists.txt`.

Implemented calculations: GSD width/height, sensor footprint width/height, side-overlap track spacing, frontal-overlap image spacing, trigger interval and trigger rate. Invalid/missing controlled inputs fail closed.

Remaining integration work: bind the engine to the canonical mission/equipment configuration and subsequent coverage-track generation. No universal acceptance thresholds were introduced.

### GAP-MT01-002 — Coverage Orientation Generator

Status: **CLOSED FOR FIRST GENERATION SLICE; EVALUATION/INTEGRATION REMAINS OPEN.**

Implemented in `04_SOFTWARE/PLANNING/coverage_orientation.hpp/.cpp` and registered in `CMakeLists.txt`, with `coverage_orientation_test.cpp`.

The generator validates AOI and acquisition geometry, consumes an explicit controlled angle range and step, emits deterministic candidate IDs/order, calculates projected AOI width and estimated track count from controlled track spacing, and records dependency identity.

Wind, energy, turn-cost and terrain-complexity scoring are intentionally not embedded in this generator; those belong to subsequent candidate evaluation stages.

### GAP-MT01-003 — Coverage Decomposition Engine

Status: **CLOSED FOR FIRST GEOMETRIC DECOMPOSITION SLICE; FULL TERRAIN/OBSTACLE INTEGRATION REMAINS OPEN.**

Implemented in `04_SOFTWARE/PLANNING/coverage_decomposition.hpp/.cpp` with `coverage_decomposition_test.cpp` and registered in `CMakeLists.txt`.

The first slice validates AOI, selected orientation, controlled track spacing and explicit altitude band, then performs deterministic strip decomposition in the selected orientation. Cells receive stable generation order, `MT01-CELL-<index>` identity, polygon geometry, area and dependency identity.

The engine is deliberately not declared a complete terrain/obstacle-aware decomposition engine. Active polygon restrictions are now handled through the controlled `CoveragePolygonSplitter`, including convex and non-convex polygon regression coverage; terrain/obstacle-aware clipping, footprint-level clearance and complete multi-level decomposition remain open.

No universal hard-coded acceptance values were introduced.

### GAP-MT01-004 — Coverage Track Generator

Status: **CLOSED FOR FIRST GENERATION SLICE; COVERAGE/ENDPOINT FEASIBILITY INTEGRATION REMAINS OPEN.**

Implemented as `CoverageTrackGenerator` in the existing `coverage_decomposition.*` planning module. It consumes the controlled decomposition, generates deterministic orientation-aligned scanline intervals per cell, emits stable `MT01-TRACK-<index>` identities, preserves cell identity, calculates track length and records altitude/dependency identity.

The slice does not yet claim sensor-footprint coverage validation, terrain-following endpoint feasibility, turn-radius feasibility or constrained-domain clipping beyond the decomposition input. Those remain downstream integration work.

No universal acceptance thresholds were introduced.

### GAP-MT01-005 — Edge Coverage Engine

Status: **CLOSED FOR FIRST EVALUATION SLICE; LOCAL REPAIR REMAINS OPEN.**

Implemented as `CoverageEdgeEngine` in the existing `coverage_decomposition.*` planning module. The engine requires controlled footprint width/height, evaluates each generated track endpoint against its planning-cell boundary, applies the controlled half-footprint requirement for end coverage, and retains explicit gap diagnostics and margins.

The current slice is diagnostic only. It does not silently repair gaps, does not invent a margin/tolerance, and does not claim corner-specific footprint union or full sensor-footprint polygon coverage. Those remain open integration work.

### GAP-MT01-006 — Acquisition Event Model / Validator

Status: **CLOSED FOR FIRST EVENT-MODEL SLICE; FULL EVENT VALIDATION REMAINS OPEN.**

Implemented as `AcquisitionEventValidator` in the existing planning module. It consumes controlled track output and `AcquisitionGeometryResult`, creates stable `MT01-EVENT-<index>` identities, preserves track identity, camera-ground distance, GSD, footprint and trigger timing, and records sensor/trigger state.

The current slice does not yet model camera orientation/gimbal state, individual acquisition positions along the track, overlap validation per event, or terrain-following surface distance. Those remain open and belong to subsequent integration/quality stages.

### GAP-MT01-008 — Constrained-Domain Integration

Status: **CLOSED FOR CONTROLLED POLYGON-SPLITTING SLICE; FULL CONSTRAINED-DOMAIN INTEGRATION REMAINS OPEN.**

The first integration pass now uses `ConstrainedOpenSpace::evaluatePolygon(...)` for cell-level detection and `CoverageTrackGenerator` splits sweep-line intervals at intersections with active polygon/circle restrictions. Each resulting free interval is independently checked through `ConstrainedOpenSpace::evaluateSegment(...)`, so the unrestricted remainder of a constrained cell can continue to generate acquisition tracks.

The new `CoveragePolygonSplitter` performs deterministic sweep-slab subtraction of active polygon restrictions, handles convex and non-convex restriction geometry, preserves restriction/source provenance in its split result, and fails closed on ambiguous topology. The splitter is integrated into decomposition; fully covered cells are removed without producing invalid geometry. Full footprint-level clearance, terrain/obstacle clipping, circle-geometry polygon splitting and complete multi-level decomposition remain open. Execution of the new regression tests is not claimed here because no runnable CI/test result is available for this branch.

### GAP-MT01-007 — Mapping Quality Engine

Status: **CLOSED FOR FIRST EXACT COVERAGE + UNCOVERED-GEOMETRY SLICE; FULL MAPPING QUALITY GATE REMAINS OPEN.**

Implemented as `MappingQualityEngine` in the existing planning module. It validates upstream decomposition, tracks, acquisition events and acquisition geometry; calculates AOI area, a deterministic exact union area for the generated rectangular sweep footprints in the common orientation coordinate system, retains the prior track-footprint estimate for comparison, derives the current bounded coverage ratio from the exact footprint union, and calculates GSD minimum/maximum, controlled overlap minima and invalid-event count.

Uncovered-geometry cause classification, mandatory-area coverage, corner footprint union, terrain-following quality and complete sensor-compliance gating remain open. No hidden raster resolution or universal acceptance threshold was introduced.

## 5. Non-substitution rule

A*/Dijkstra tests, Algorithm Orchestrator tests, constrained-open-space tests, route-constraint tests, wind/performance trajectory tests and final-planning-integrity tests verify reusable components. They do not establish the complete MT-01 algorithm contract.

## 6. Required implementation dependency chain

```
Mission / Task
→ Input Integrity
→ Constrained Open Space
→ Acquisition Geometry
→ Orientation Candidates
→ Coverage Decomposition
→ Coverage Tracks
→ Edge Coverage
→ Transition Graph
→ Coverage Route Candidates
→ Wind / Performance
→ Energy / Reserve
→ 4D Trajectory
→ Acquisition Events
→ Mapping Quality
→ Candidate Comparison
→ Final Planning Integrity
```

Only blocks explicitly identified as reusable implementation infrastructure may currently be treated as implemented.

## 7. Verification consequence

`V-M01-01 / MT01-T01` is currently:

```
SPECIFICATION: READY
REQUIREMENT BINDING: READY
DATA PACKAGE: NOT READY
IMPLEMENTATION: NOT READY
EXECUTION: NOT DONE
EVIDENCE: NOT AVAILABLE
```

No MT-01 execution result shall be claimed yet.

## 8. Next deterministic implementation step

`GAP-MT01-001` through `GAP-MT01-008` are closed only for their first implementation/evaluation slices. The constrained-domain integration now detects internal restricted geometry at cell level and prevents constrained cells from producing acquisition tracks.

The remaining MT-01 integration order is:

`general polygon clipping/splitting → complete coverage-footprint union → acquisition-event expansion/validation → transition graph → route candidates → wind/energy/trajectory integration → full mapping-quality gate`.

The current constrained-domain slice must not be represented as exact clipping or full spatial-coverage compliance. MT-02 and MT-03 remain outside this step.

### GAP-MT01-009 — Uncovered-Geometry Cause Classification

Status: **CLOSED FOR FIRST PROVENANCE-AWARE CLASSIFICATION SLICE; FULL CAUSE CLASSIFICATION REMAINS OPEN.**

MappingQualityResult now exposes explicit uncovered-geometry components containing polygon, area, classification and source identifiers.

Controlled classifications currently supported:
- **BoundaryGap** — component touches the AOI boundary within the controlled implementation tolerance;
- **ExclusionInduced** — component overlaps an active polygon/circle restriction in the bound ConstrainedEnvironmentSnapshot;
- **UnclassifiedSourceNotBound** — the currently bound inputs do not provide sufficient authoritative provenance for a stronger attribution.

The implementation intentionally does not infer terrain/obstacle-induced, trajectory-infeasibility, sensor/acquisition-infeasibility or intentional-non-required causes. Those remain open until authoritative provenance for those causes is added to the Mapping Quality input contract.

No universal quality threshold was introduced by this slice.
