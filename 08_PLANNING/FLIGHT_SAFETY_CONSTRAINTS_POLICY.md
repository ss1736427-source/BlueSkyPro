# BlueSky PRO — Flight Safety Constraints Policy

**Document ID:** BSP-SAFETY-FLIGHT-CONSTRAINTS-001  
**Status:** Mandatory system-wide engineering policy  
**Applies to:** All 13 mission templates, all planning algorithms, AUTO and MANUAL modes  
**Policy class:** Non-bypassable hard constraints

## 1. Purpose and authority

This document defines the common flight-safety constraints that every BlueSky PRO route and trajectory candidate must satisfy, regardless of mission template, algorithm, launch mode or optimization objective.

These constraints are feasibility conditions, not weighted preferences. A candidate that violates a mandatory constraint must be rejected before task-quality scoring or time/energy optimization. A recommendation or validated planning result is not flight authorization.

The policy applies to the complete planned flight path: coverage legs, turns, transitions, repositioning, transit, climb/descent, approach and recovery, to the extent those phases are represented by the supported vehicle model.

## 2. Mandatory vehicle-envelope constraints

Every trajectory segment and transition must be checked against the exact assigned aircraft configuration and its evidence-backed operating envelope.

At minimum, the applicable checks include:

1. **Speed envelope:** do not exceed the validated maximum speed and do not descend below the applicable minimum safe speed. For fixed-wing aircraft, the minimum must account for configuration-specific stall speed, bank/load factor and an explicit safety margin.
2. **Bank/roll envelope:** do not exceed the validated maximum bank angle for the applicable flight phase and configuration.
3. **Acceleration and maneuver envelope:** respect validated lateral/longitudinal acceleration, angular-rate and other applicable dynamic limits. A yaw-rate limit must not be misrepresented as a lateral-turn capability.
4. **Altitude and vertical envelope:** respect aircraft climb/descent capability, terrain clearance, altitude references and phase-specific limits.
5. **Energy and endurance:** account for the full path, including turns, speed changes, climb/descent, transit, stabilization, wind, payload and recovery/contingency reserve.
6. **Configuration validity:** bind every limit to the aircraft model, exact configuration, payload/mass and relevant operating state. Unknown or unvalidated limits must not be silently replaced with assumed values.

For fixed-wing turns, speed, bank, load factor and turn radius are coupled. A planner must not force a requested radius by exceeding the bank limit or violating the stall margin. For multirotor aircraft, use a validated translational acceleration/velocity envelope; yaw rate alone does not establish horizontal path curvature. VTOL aircraft require phase-specific envelopes for hover, transition, wing-borne flight and recovery. Unsupported phases or missing validated data block a flight-ready result.

## 3. Permitted response when a standard maneuver does not fit

If a standard turn or transition cannot safely capture the next required path line, the planner may generate a longer alternative, provided the entire alternative remains within the permitted maneuvering domain and all hard constraints.

Permitted geometric options may include:
- initiating the maneuver earlier;
- extending the turn or adding intermediate arcs;
- increasing the total course change / executing a larger turn-out;
- adding straight transit or stabilization segments;
- using an offset, teardrop or repositioning maneuver;
- reordering coverage passes when task constraints allow.

Increasing the total heading/course change is a change to path geometry. It is **not** permission to increase bank angle, speed, acceleration or any other aircraft limit.

The planner must evaluate the complete swept trajectory, not only its endpoints. It must not assume that a maneuver may leave the AOI: the permitted maneuvering domain must be explicitly defined by mission/operator policy and validated against airspace, NOTAM, obstacle, terrain and other spatial constraints.

## 4. Camera and mission-task continuity

For imaging and sensing missions, turns and repositioning must not be counted as valid coverage or evidence unless the sensor model explicitly validates the resulting data quality.

Capture may be suspended during a turn or unstable transition. Coverage capture may resume only after the required straight-line stabilization distance/time and camera/gimbal readiness conditions are met. Re-entry must preserve required GSD, overlap, trigger spacing, latency and viewing geometry. The planner must account for the resulting coverage gaps and add valid replacement coverage where feasible.

Equivalent mission-specific quality rules apply to non-imaging templates: maneuver geometry must not be counted as successful observation, delivery, search effort, patrol dwell or other task outcome unless its relevant model supports that claim.

## 5. Independent spatial and environmental validation

The full trajectory, including the swept volume and vehicle clearance where supported, must be validated against the same versioned mission environment used for planning:
- permitted maneuvering domain and AOI policy;
- airspace restrictions and NOTAM/prohibited zones;
- terrain/DEM and vertical datum;
- obstacles and required clearance;
- weather and wind, including ground-track effects;
- C2/navigation constraints and fleet separation;
- launch, landing, alternate and recovery constraints.

An air-relative turn-radius estimate is not by itself proof that the ground track is feasible in wind. Wind-aware trajectory validation is required. Stale, incomplete or incompatible mandatory environment data blocks the affected result.

## 6. Feasibility, optimization and failure behaviour

The required order is:

1. Generate candidate geometry/trajectories using a supported vehicle model.
2. Reject candidates violating any mandatory vehicle, spatial, environmental, mission or energy constraint.
3. Validate task quality and required coverage/evidence for remaining candidates.
4. Optimize time, energy, path length, smoothness and maneuver count only among admissible candidates, according to the mission objective hierarchy.
5. Return the selected candidate with constraint evidence, provenance and rejected-candidate reasons.

If no candidate satisfies all mandatory constraints, return an explicit infeasible/blocked result and identify the binding constraint(s). Do not silently relax limits, substitute unvalidated data, omit a difficult segment or label an advisory estimate as flight-ready. An operator cannot waive mandatory safety constraints through a preference or scoring override.

## 7. System-wide application and mode invariants

This policy is mandatory for all 13 mission templates:

- MT-01 Mapping
- MT-02 3D Mapping / Reconstruction
- MT-03 Inspection
- MT-04 Construction Monitoring
- MT-05 Area / Perimeter Monitoring
- MT-06 Search and Rescue
- MT-07 Fire / Incident Monitoring
- MT-08 Ecology / Environmental Monitoring
- MT-09 Agriculture
- MT-10 Delivery
- MT-11 Communications Relay
- MT-12 Aerial Photography / Media
- MT-13 Authorized C-UAS Detection, Tracking and Identification

AUTO and MANUAL must use the same downstream safety constraints and independent validation services. Their launch actions, input responsibilities and operator workflows remain distinct. Shared safety validation does not allow AUTO intent inference to overwrite MANUAL geometry or MANUAL input to invoke task-text interpretation.

No template-specific planner may weaken, bypass or redefine this policy. Template-specific constraints may add stricter requirements, but cannot reduce the common safety baseline.

## 8. Required software architecture

Implement the policy as a shared, independently invoked safety-validation layer, not as duplicated checks embedded separately in each template.

The shared layer must consume:
- canonical mission and explicit launch mode;
- exact aircraft/payload/configuration identity and validated motion/performance model;
- typed trajectory segments and speed/altitude profiles;
- permitted maneuvering domain;
- versioned terrain, obstacle, airspace/NOTAM, weather/wind and C2/fleet inputs;
- task-specific sensor/quality requirements;
- energy model and reserve/recovery policy.

Every finding must include a stable constraint ID, pass/fail/unknown status, measured value, permitted bound, units, source/provenance, model/configuration version and affected trajectory segment. Unknown mandatory evidence is not a pass.

The safety layer must be called after candidate generation and before ranking/selection, and again at the applicable final mission-package validation boundary. It must be impossible for a template, optimizer or UI preference to convert a failed/unknown mandatory gate into a pass.

## 9. Minimum verification requirements

Tests must demonstrate, across shared validation and template integration:

- maximum speed and minimum safe speed boundaries;
- bank-limit enforcement;
- fixed-wing stall margin under bank/load factor;
- multirotor acceleration/translation limits without using yaw rate as a turn-radius proxy;
- VTOL phase gating and rejection of unsupported transitions;
- safe extended-turn / larger-course-change alternatives;
- rejection when the maneuvering domain or swept path intersects a prohibited/obstructed region;
- wind effects on ground-track feasibility;
- energy/reserve and recovery checks for the complete path;
- camera stabilization and coverage-gap accounting;
- explicit infeasible results when no safe candidate exists;
- identical downstream safety-gate behaviour for AUTO and MANUAL;
- all 13 templates routed through the shared non-bypassable validation layer.

Unit tests of individual formulas are necessary but do not establish operational readiness. Flight-ready claims require representative simulation and/or controlled flight evidence for the exact aircraft configuration and operating envelope.

## 10. Implementation status rule

This document establishes the mandatory policy and target architecture. It does not, by itself, prove that all existing planners enforce every constraint. Each integration must be tracked and verified. Until the shared validator and required template-level tests are implemented, the affected planning capability must remain explicitly limited, blocked or advisory; it must not be represented as flight-ready.
