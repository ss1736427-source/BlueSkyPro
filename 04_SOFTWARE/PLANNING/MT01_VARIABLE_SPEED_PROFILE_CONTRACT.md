# MT-01 variable-speed profile contract

**Status:** PARTIAL RUNTIME IMPLEMENTATION — solver and wind/performance adapter integrated; MT-01 geometry/orchestrator binding and CI verification pending  
**Scope:** Segment-level air-relative speed planning for coverage and turn transitions.

## 1. Purpose

Introduce one deterministic segment-speed profile between MT-01 geometry generation and the existing wind/performance trajectory calculation. This is not a second optimizer and does not replace route generation, weather assessment, energy calculation, camera validation or candidate comparison.

## 2. Inputs

All inputs are versioned and bound to the exact assigned aircraft and configuration:
- ordered route segments, lengths and turn/coverage classification;
- desired coverage speed and camera-derived ground-speed limits;
- vehicle minimum/maximum air-relative speed and explicit speed reference;
- validated longitudinal acceleration and deceleration limits;
- turn curvature/radius and validated lateral-dynamics limit;
- camera trigger spacing, minimum trigger interval, exposure and blur tolerance;
- wind snapshot identity is a downstream dependency, not an excuse to mix airspeed and ground speed.

Missing required dynamic evidence makes the profile advisory or infeasible; it cannot satisfy a hard motion gate.

## 3. Outputs

For each segment:
- selected air-relative speed and reference;
- applicable speed cap and limiting constraint;
- entry/exit speed;
- acceleration/deceleration feasibility and distance;
- evidence/provenance and profile version.

The profile also returns deterministic findings, feasibility state and dependency identity.

## 4. Solver rules

1. Derive per-segment speed caps from camera, aircraft, curvature and mission hard limits.
2. Run a backward braking pass to ensure the next segment's cap can be reached before entry.
3. Run a forward acceleration pass to ensure speed increases are reachable.
4. Enforce aircraft minimum/maximum speed bounds.
5. Reject a candidate if the speed change cannot be completed in available distance, or if the required turn speed conflicts with minimum safe speed.
6. Do not assume instantaneous speed changes. Do not invent acceleration, deceleration or jerk values.
7. Jerk-limited shaping is optional and requires validated aircraft response/jerk evidence.

## 5. Coupling with existing modules

- Geometry builder supplies candidate turn geometry and must verify curvature at selected turn speed.
- This profile component selects segment airspeeds only.
- Existing WindPerformanceTrajectory converts airspeed plus segment wind into ground speed/track, then calculates time, energy, reserve and camera trigger/blur constraints.
- Existing weather time-indexing and convergence remain authoritative.
- Existing orchestrator applies hard feasibility before objective comparison.
- A change to profile, geometry, vehicle configuration, camera configuration or wind snapshot invalidates only dependent calculations.

## 6. Airspeed / ground speed separation

Vehicle motion and turn limits use the speed reference required by the validated aircraft model (TAS or AIR_RELATIVE). Camera trigger interval and motion blur use wind-corrected ground speed. The two must never be substituted for one another.

## 7. Mavic 3E evidence status

Published DJI speed and camera specifications do not establish sustained lateral acceleration, braking/deceleration response or minimum turn radius. Scenario assumptions can be used only for sensitivity analysis. They cannot pass the vehicle-motion hard gate or produce a release-eligible profile.

## 8. Verification

Tests must cover camera and curvature speed caps, backward braking, forward acceleration, insufficient braking distance, minimum-speed conflicts, segment-boundary continuity, wind direction cases, camera-only-on-coverage checks, energy/reserve recalculation, deterministic replay and fail-closed missing evidence.

**Implementation status (2026-11-14):**
- Deterministic longitudinal envelope solver implemented with backward braking and forward acceleration passes.
- Each segment result retains distance, entry/exit speed, peak speed, acceleration/braking distances and validated dynamics limits.
- WindPerformanceTrajectory accepts a complete, ordered, versioned air-relative speed profile and validates segment identity, continuity, kinematic distances and geometry correspondence.
- Wind/performance integration numerically samples acceleration, cruise and braking phases against the segment wind. Traversal time, payload energy, average ground speed and track are recalculated. Camera trigger interval and motion blur use the maximum sampled/profile-extreme ground speed.
- Regression tests cover solver-to-wind handoff, variable-speed timing, camera limits and fail-closed profile mismatch.
- Canonical `Route` now has an explicit, optional, versioned speed-profile payload. `Mt01RouteCandidateBuilder` binds a supplied solver result only when vehicle/configuration identity, segment order, route distances, phase distances and boundary continuity match the generated geometry.
- `AlgorithmOrchestrator` rechecks an attached profile against the assigned flight-safety envelope before ranking: exact aircraft/configuration identity, TAS/AIR_RELATIVE reference, complete segment coverage, continuity, speed envelope and acceleration/braking distance equations.
- Integrated: the MT-01 weather binding now requires a canonical route speed profile and validated longitudinal-dynamics provenance whenever camera/weather feedback is enabled. MT-01 also solves a baseline profile when validated dynamics are available, even when the initial connector speed equals coverage speed.
- Integrated: the time-dependent weather sampler now derives segment camera ground-speed limits from trigger interval and motion-blur limits, resolves each sampled wind vector against the route ground track using crab correction, and aggregates the most restrictive airspeed envelope onto each source coverage segment.
- Integrated: the constrained speed profile is re-solved during the weather/time iteration. It is projected back onto the along-track weather subdivisions for 4D trajectory evaluation, while the corrected canonical route retains its original lineage and geometry with the updated source-segment profile.
- Integrated: weather/time and profile iteration must converge within 12 passes; acceptance requires both traversal-time delta <= 1.0 s and profile peak-speed delta <= 0.05 m/s. Missing wind, missing speed evidence, infeasible camera caps or non-convergence block the candidate.
- Integrated: the orchestrator accepts the corrected route only when mission/route lineage and complete geometry are preserved, repeats route-speed and custom constraint validation, then performs final time-dependent NOTAM validation before ranking.
- Remaining: source validated acceleration/deceleration and minimum-speed envelopes directly from authoritative assigned-aircraft records; independently verify the new end-to-end feedback path with controlled aircraft/camera/forecast fixtures and full CI; complete Qt/HMI application build and independent flight-release verification.
- Turn geometry is not regenerated by this weather feedback because camera capture segments are coverage legs and connector arcs retain their validated fixed speed. Any future change to connector/turn speed must rebuild and revalidate the corresponding geometry before publication.
- This is not flight-release eligible and must not be presented as operationally ready.


## Wind-corrected camera ground-speed feedback (2026-11-14)

The camera limit is a ground-speed constraint. For a requested ground track, the weather stage resolves along-track and cross-track wind components. Crosswind is compensated by crab angle; it is not treated as an along-track speed penalty. A camera cap below crosswind magnitude is not automatically infeasible: feasibility depends on the available airspeed envelope and positive ground progress. Excessive tailwind can make the camera cap impossible even at the lowest permitted airspeed.

For each camera capture segment, the ground-speed ceiling is the minimum of:
- forward trigger spacing / minimum hardware trigger interval;
- maximum permitted motion blur * GSD / exposure time.

The time-dependent weather sampler supplies wind at each along-track subdivision. The camera adapter converts the ground-speed ceiling into an air-relative speed envelope for the sampled track and wind. The most restrictive envelope across the subdivisions is applied to the original coverage segment, then the longitudinal solver recomputes acceleration, cruise and braking. This conservative aggregation prevents a locally permissive wind sample from hiding a restrictive part of the same camera leg.

The new profile is projected back onto the weather subdivisions for 4D time/energy/camera evaluation. The outer weather iteration repeats when traversal times or profile peaks change and fails closed after 12 iterations. The corrected canonical route is returned to the orchestrator; route lineage, geometry, mission constraints and the matching wind/profile snapshot identity are checked, route-speed/custom constraints are rerun, and NOTAM validation uses the corrected route and its matching 4D trajectory.

The route profile retains both the baseline geometry/aircraft speed envelope and the weather-applied envelope per segment. This separation is intentional: when a forecast snapshot expires or is replaced, the next assessment can discard the previous wind-derived cap and solve again from the baseline envelope. The new profile records its own ID/version, wind snapshot ID/version and camera configuration identity.

Turn connector arcs are not camera-capture legs and remain at their geometry-validated fixed speed. Therefore this feedback does not silently change turn radius. If a future requirement changes connector speed, geometry must be rebuilt and revalidated as a separate dependency before the route can be accepted.
