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
| 5. Cellular decomposition | Decompose AOI into planning cells where required | no MT-01 coverage decomposition engine found | none identified | GAP |
| 6. Coverage tracks | Generate parallel acquisition tracks, clip to geometry, enforce footprint coverage and endpoints | no MT-01 coverage-track generator found | none identified | GAP |
| 7. Edge coverage | Deliberately validate/repair boundary and corner acquisition coverage | no dedicated edge-coverage implementation found | none identified | GAP |
| 8. Transition graph | Connect acquisition tracks with feasible transition costs | generic planning graph / route infrastructure exists; no MT-01 acquisition-transition graph found | graph/route tests | PARTIAL |
| 9. Route candidates | Generate bounded route candidates from coverage/transition alternatives | Algorithm Orchestrator and solver contract can manage candidates; no MT-01 coverage candidate producer found | orchestrator tests | PARTIAL |
| 10. Wind-aware performance | Apply wind/performance model and determine segment time/energy | wind_performance_trajectory.* | wind_performance_trajectory_test.cpp | IMPLEMENTED REUSABLE |
| 11. Mission energy | Include mission phases, reserve and energy feasibility | energy model/source registers exist; trajectory engine enforces usable energy + reserve; complete MT-01 phase model not found | wind/trajectory tests; NAV tests elsewhere | PARTIAL |
| 12. 4D trajectory | Produce time-parameterized route/profile with altitude and energy | wind_performance_trajectory.*, flight_profile.*, vertical route infrastructure | corresponding unit tests | PARTIAL / REUSABLE |
| 13. Acquisition event validation | Validate position, camera-surface distance, orientation, footprint, GSD, overlap and trigger state | no acquisition-event validator/model found | none identified | GAP |
| 14. Mapping quality | Coverage ratio, uncovered geometry, GSD, overlap, acquisition validity, terrain following, sensor compliance | no dedicated MT-01 mapping-quality engine found | none identified | GAP |
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

Status: **CLOSED FOR FIRST GEOMETRIC DECOMPOSITION SLICE; CONSTRAINT INTEGRATION REMAINS OPEN.**

Implemented in `04_SOFTWARE/PLANNING/coverage_decomposition.hpp/.cpp` with `coverage_decomposition_test.cpp` and registered in `CMakeLists.txt`.

The first slice validates AOI, selected orientation, controlled track spacing and explicit altitude band, then performs deterministic strip decomposition in the selected orientation. Cells receive stable generation order, `MT01-CELL-<index>` identity, polygon geometry, area and dependency identity.

The engine is deliberately not declared a complete exclusion-aware decomposition engine yet. The current constrained-environment classification path is present as a reusable boundary check, but full clipping/splitting around internal restricted geometry and terrain/obstacle-aware decomposition remain open.

No universal hard-coded acceptance values were introduced.

### GAP-MT01-004 — Coverage Track Generator

Required: parallel sweep generation, clipping, sensor-footprint coverage, endpoint/turn feasibility and track provenance.

Current state: not identified.

### GAP-MT01-005 — Edge Coverage Engine

Required: boundary/corner coverage evaluation and coverage margin derived from sensor geometry, with local repair that does not silently violate hard constraints.

Current state: not identified.

### GAP-MT01-006 — Acquisition Event Model / Validator

Required: acquisition event identity, sensor state, camera-ground distance, orientation, footprint, GSD, overlap, trigger validity and quality state.

Current state: not identified.

### GAP-MT01-007 — Mapping Quality Engine

Required: area coverage ratio, mandatory-area coverage, uncovered geometry classification, GSD distribution, overlap minima, acquisition-event validity, sensor compliance and terrain-following quality.

Current state: not identified.

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

`GAP-MT01-001` and `GAP-MT01-002` are closed for their first implementation slices. The next dependency is **GAP-MT01-003 — Coverage Decomposition Engine**.

The next implementation must consume the controlled AOI, constrained spatial domain and selected orientation candidates, and produce deterministic planning cells without introducing universal hard-coded acceptance values. GAP-MT01-003 is now closed only for the first geometric decomposition slice; full constrained-domain integration remains open and must not be represented as verification-complete.

Remaining dependency order: MT01-003 → MT01-004 → MT01-005 → MT01-006 → MT01-007.

MT-02 and MT-03 remain outside this step.