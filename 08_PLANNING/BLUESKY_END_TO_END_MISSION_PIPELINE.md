# BlueSky PRO — End-to-End Mission Planning Pipeline

**Status:** IMPLEMENTATION CONTROL DOCUMENT — P0

## 1. Required behaviour

The operator describes the intended work and supplies or confirms the operating area. BlueSky PRO must interpret the task, recommend templates, request missing information, build a canonical versioned mission, acquire mission-scoped inputs, decompose the work, resolve algorithm capabilities, calculate candidates, independently validate them, rank admissible candidates and present the selected plan with evidence and decision trace.

A recommendation is not a plan. A calculated candidate is not a validated mission. A validated mission is not an authorization to fly.

## 2. End-to-end state machine

```text
TASK_DRAFT -> INTENT_ANALYSIS -> CLARIFICATION_REQUIRED | INTENT_CONFIRMED
-> MISSION_MODEL_BUILD -> INPUT_ACQUISITION -> INPUT_VALIDATION
-> DECOMPOSITION -> CAPABILITY_RESOLUTION -> CANDIDATE_CALCULATION
-> INDEPENDENT_VALIDATION -> OBJECTIVE_RANKING -> OPERATOR_REVIEW
-> MISSION_PACKAGE_VALIDATION -> AUTHORIZATION -> READY_FOR_TRANSFER

Missing/invalid mandatory dependency -> NEEDS_INPUT or BLOCKED.
Changed dependency -> invalidate affected downstream calculations.
```

## 3. Verified integration baseline

| Stage | Existing component | Actual gap |
|---|---|---|
| Task input | qt/BlueSkyPRO-HMI/qml/TaskCreation.ui.qml | Captures free text and template choices; AUTO path does not submit a canonical planning request |
| Template suggestion | MissionTemplateResolver | Explainable lexical baseline, not semantic intent understanding |
| Mission model | ManualMissionModelAdapter; 04_SOFTWARE/PLANNING/model/mission_model.hpp | MANUAL adapter builds a structurally checked model, but emits reduced JSON; canonical contract reconciliation remains open |
| Schema | 08_PLANNING/schemas/mission-model.schema.json | Schema and C++ Mission/adapter serialization require reconciliation |
| Geometry | ManualGeometryBridge | Manual map geometry exists; AUTO task flow is not joined to the same aggregate |
| Environment | WeatherDataService, FaaNmsClient and planning validators | No proven single immutable mission-scoped DEM/NOTAM/weather/wind snapshot assembled end-to-end |
| Planning core | 04_SOFTWARE/PLANNING/orchestrator/algorithm_orchestrator.* | Shared planning library is linked to HMI; a planning application service connecting both launch modes to calculation is not yet implemented |
| Solvers | A*, Dijkstra and other planning components | A*/Dijkstra are graph route solvers, not coverage or 3D viewpoint planners |
| Assessment | VirtualFlightAssessmentBridge | Current assessment uses synthetic airspace/NOTAM fixtures; not authoritative operational validation |
| PlanningBridge | PlanningBridge | Process/JSON result bridge, not the planning engine |

MainContent.qml records AUTO task text/template IDs and opens the workspace. It also has a distinct MANUAL state and manual-geometry handling. The manual set handler builds a structurally valid model but explicitly leaves route generation disconnected and blocks completion. These are confirmed breaks in the user-visible chain.

## 4. One canonical mission model

AUTO and MANUAL entry modes must converge on one versioned mission aggregate containing:
- original task description, normalized intent and intent-confirmation state;
- mission identity/version, selected template IDs and objective-profile version;
- typed AOI/route/corridor/inspection/search/delivery geometry with CRS, altitude reference and geometry version;
- mandatory constraints separately from optimization preferences;
- mission-scoped environment snapshot IDs/versions for DEM, airspace/NOTAM, weather/wind and obstacles;
- vehicle and payload assignments with configuration versions;
- required capabilities, solver versions and compute budgets;
- candidates, route/trajectory/coverage and task-quality metrics;
- uncertainty, validation findings, rejected-candidate reasons and provenance;
- selected solution and rationale; package/authorization/execution references only at their respective later stages.

The original user description is immutable source input. Derived fields must not replace it.

## 5. Intent and clarification

The lexical resolver may recommend templates but must not directly create a flight-ready mission. Intent analysis must preserve detected objectives, evidence, ambiguity, conflicting phrases, extracted entities/quantities/units, required geometry and equipment, missing inputs and operator confirmation.

Compound tasks may create multiple dependent subtasks. For example, bridge inspection requires target extent/faces, observation requirements, sensor quality, stand-off constraints, altitude reference and a compatible aircraft/payload. If these are not known, the system asks for them rather than inventing them.

## 6. Mission-scoped inputs

Environment data must be bound to the AOI/route, expected execution time and selected vehicle. Each source records provider, retrieval time, valid interval, spatial coverage, CRS/vertical datum, units, version, completeness, freshness and quality. Each calculation uses an immutable snapshot identity. Refreshing weather, NOTAM, DEM, geometry, payload or vehicle configuration invalidates dependent results.

Missing mandatory data blocks the affected capability. Missing optional data is disclosed and may lower quality. Unknown uncertainty is never treated as zero uncertainty.

## 7. Decomposition and capability resolution

Objective profiles generate typed subproblems. MT-01: AOI decomposition, sensor footprint/GSD, coverage tracks, constrained transit, sortie sequence. MT-02: surface model, viewpoints, visibility/occlusion, set cover, viewpoint routing. MT-03: asset requirements, viewpoints/evidence, visibility and route. MT-06: search prior, sensor detection model, search pattern and Bayesian update. MT-09: field zoning, operation-specific coverage and headland turns. MT-10: payload/vehicle compatibility, assignment, time windows, route and recovery.

The capability registry selects compatible algorithms. The operator does not choose algorithms.

## 8. Candidate and validation separation

Every candidate must pass independent checks for geometry/altitude references, DEM/obstacles, current airspace/NOTAM, weather/wind, vehicle performance, payload/sensor suitability, energy/reserve/recovery, C2/navigation, timing/fleet separation and mission-specific coverage/evidence.

A solver-reported FEASIBLE value is not independent validation. Hard constraints are gates. Only admissible candidates enter objective ranking. INFEASIBLE, UNCERTAIN, PARTIAL and FEASIBLE remain distinct.

## 8A. System-wide flight safety constraints

All 13 mission templates and both AUTO and MANUAL launch modes are subject to the same mandatory [Flight Safety Constraints Policy](FLIGHT_SAFETY_CONSTRAINTS_POLICY.md). The policy is a non-bypassable hard gate, not a template-specific optimization preference.

Every generated trajectory must remain within the exact aircraft configuration's validated speed, bank/roll, acceleration, altitude and energy/reserve envelope. Fixed-wing speed selection must preserve the configuration-specific stall margin under bank/load factor. Multirotor and VTOL vehicles require their own validated, phase-appropriate motion envelopes; yaw rate alone is not a substitute for translational turn capability.

When a standard transition cannot capture the next path line, the planner may generate an earlier, longer or multi-stage maneuver, increase total course change, or add straight transit/stabilization segments. This is geometric path extension, not permission to exceed bank, speed or other aircraft limits. The entire swept path must remain inside an explicitly permitted maneuvering domain and pass the versioned terrain, obstacle, airspace/NOTAM, weather/wind, C2, separation and energy checks.

If no candidate passes all mandatory gates, return BLOCKED/INFEASIBLE with the binding reason(s). Do not silently relax a limit or rank an unsafe candidate. If the selected admissible trajectory reaches one or more validated maximum aircraft limits, the result and HMI must show the warning **«ВНИМАНИЕ: расчёт выполнен с использованием максимально допустимых режимов БВС»** and enumerate each exact parameter, planned value, limit, units and affected segment. No special warning is required for parameters merely approaching a limit. No parameter may exceed its applicable validated limit. The same downstream validator applies to AUTO and MANUAL; their separate entry workflows and input responsibilities remain unchanged.

## 9. Operator workflow

1. Describe task and define AOI/target.
2. Review interpreted objective and recommended templates.
3. Resolve missing inputs and check source freshness.
4. Confirm vehicle/equipment compatibility.
5. Observe calculation stages and blocked reasons.
6. Compare candidates using mission-relevant metrics.
7. Review selected route/coverage/3D and decision explanation.
8. Review independent validation evidence.
9. Proceed to separate mission-package and authorization gates.

The operator may correct intent, geometry, vehicle assignment and preferences, but cannot waive mandatory safety constraints.

## 10. Separate AUTO and MANUAL launch modes

AUTO and MANUAL are two distinct operator launch paths, not two labels for the same screen and not a mode inferred silently from the data. The HMI must expose and preserve a deliberate choice of launch mode before mission creation begins.

### AUTO — task-driven launch
- Entry point starts from a free-text task description and/or operator-selected task templates.
- The system interprets intent, recommends templates, requests missing information and asks the operator to confirm the interpreted task.
- The operator is not required to construct geometry manually as the primary way to express the task; the system may request or support AOI confirmation as an input step.

### MANUAL — operator-driven launch
- Entry point is separate from AUTO and explicitly starts manual mission composition.
- The operator directly defines mission geometry and selects applicable mission parameters, constraints, vehicle and payload inputs.
- The system performs structural and completeness checks, then submits the manually authored mission for the same planning and independent-validation pipeline.
- AUTO intent inference must not overwrite manual geometry or silently change the selected mode. Manual inputs must remain editable until the operator submits them.

### Shared downstream services
Both launch paths create the same versioned canonical Mission aggregate, with `mission_mode` set explicitly to `AUTO` or `MANUAL`. They may share environment acquisition, capability resolution, planning algorithms, candidate evaluation, hard gates, provenance and mission-package controls. Shared downstream services do not remove the separate HMI launch paths or their distinct input responsibilities.

### Required mode invariants
1. The selected launch mode is explicit, visible and persisted in the mission model and audit trace.
2. Starting AUTO cannot silently enter manual composition; starting MANUAL cannot trigger task-text interpretation as a substitute for operator input.
3. Each mode has its own entry action, input workflow, validation feedback and return/cancel behaviour.
4. Both modes use the same canonical model version and the same non-bypassable safety gates after their mode-specific inputs are assembled.
5. Switching modes must be deliberate. The system must not discard unsaved geometry, task text or confirmed inputs without warning and an explicit operator decision.
6. Tests must prove that each launch action enters the correct workflow and that the resulting mission carries the matching mode through planning requests and results.

## 11. First operational vertical slice

Implement MT-01 mapping for one UAV, one polygon AOI and one configured camera before attempting all 13 planners.

Controlled inputs: task text, AOI, UAV/payload configuration and versioned DEM/airspace/weather snapshots.

Required output: mapping decomposition, coverage candidate, route/trajectory, GSD/overlap/coverage metrics, energy/time estimates, independent hard-gate report and provenance.

This slice proves the interfaces among HMI, canonical model, environment services, registry, planning core and validation. MT-02 and MT-03 then reuse the same pipeline with domain-specific capabilities.

## 12. Immediate implementation blockers

1. Reconcile JSON Schema, C++ Mission and adapter serialization.
2. Replace AUTO hand-off that only stores task text/template IDs with canonical task submission.
3. Join AUTO task and MANUAL geometry into the same mission aggregate.
4. Add a planning application service consuming a versioned Mission Model and returning structured stage results.
5. Link the planning library to the application as a stable CMake target; do not duplicate source files across targets.
6. Keep synthetic assessment as test-only; implement mission-scoped authoritative input adapters.
7. Implement the capability registry and eligibility explanations.
8. Implement MT-01 coverage and candidate evidence before claiming automatic mapping support.
9. Add end-to-end tests for task-to-validated-candidate, missing/stale inputs and dependency changes.

## 13. Acceptance criteria

- AUTO and MANUAL have distinct, explicit HMI launch actions and mode-specific input workflows.
- The selected mode is retained in the canonical model, planning request/result and audit trace.
- Switching modes cannot silently discard unsaved operator input.
- One mission ID/version is traceable from task submission to selected candidate.
- Original task, templates and objective profile are retained.
- Geometry and environmental sources are scoped and versioned.
- Capability eligibility is machine-readable and explainable.
- Candidates are reproducible and independently validated.
- Hard-gate failures cannot be overridden by soft scoring.
- Missing inputs yield NEEDS_INPUT or BLOCKED.
- Changing any dependency invalidates affected calculations.
- No solver or template recommendation grants flight release.

## 14. Status reporting rule

Do not report the full chain as implemented until an automated end-to-end test demonstrates task description -> canonical model -> scoped inputs -> decomposition -> solver -> independent validation -> selected candidate. Architecture documents and isolated unit tests are not proof of operational planning.

## 8B. Shared dynamic safety validator — implementation status

A reusable C++ hard-gate primitive is now registered in the planning library and its CTest suite. It validates per-segment speed and the motion-class-specific dynamic limit against official/flight-test validated aircraft configuration data. It rejects over-limit and unknown/unvalidated inputs. Exact use of a permitted maximum is returned as structured data for HMI warning; approaching a limit does not trigger this warning. A route adapter applies the same check to every route segment.

The solver contract now carries an explicit `requires_flight_safety_validation` flag, an optional validated aircraft envelope and per-candidate segment dynamics. When the flag is set, the orchestrator invokes the shared validator before structural acceptance and objective ranking. Missing envelope or missing segment dynamics blocks the candidate; unvalidated evidence and over-limit values also block it. The current contract is deliberately limited to exactly one assigned aircraft and requires the safety envelope to be bound to that exact `vehicle_ids` entry; multi-aircraft safety envelopes are not yet supported and fail closed. The selected decision exposes structured maximum-regime uses and the Russian operator warning. Tests cover exact-limit warning, missing envelope rejection and over-bank rejection. Generic non-flight graph tasks retain their existing path when the flag is false.

This is an orchestrator-level integration seam, not yet an end-to-end flight planner. Solver producers must populate complete segment dynamics from the actual trajectory; the application service must set the flag and bind the correct aircraft configuration. Coupled dynamics (including bank-dependent stall margin), wind/ground-track, spatial swept-path, energy, environment gates and HMI display remain open. The gate validates only the fields represented in its current envelope; it does not prove every template or the entire mission safe.
