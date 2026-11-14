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
- Still pending: deriving camera/turn/aircraft caps automatically; sourcing validated longitudinal dynamics from the assigned vehicle record; regenerating turn geometry when selected speed changes; feeding camera/wind-derived caps into profile input; integration regression test for the full orchestrator path; full CI/build verification.
- This is not yet flight-release eligible and must not be presented as operational end-to-end variable-speed planning.
