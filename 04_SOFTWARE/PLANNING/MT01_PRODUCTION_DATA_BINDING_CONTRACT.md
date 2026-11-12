# MT-01 Production Data Binding Contract

Status: engineering gap analysis against `main` at commit `d286f88cefa1280169a9073ca8fc5111f224f8fa`.

This document records the production inputs that must be bound before MT-01 may promote a structural route candidate to a complete, safety-attested trajectory. It does not grant flight-release eligibility.

## 1. Current verified pipeline boundary

`Mt01MappingPipeline::calculate()` currently:
- validates the canonical mission identity, MT-01 template selection, one assigned vehicle, one camera, AOI, and equipment compatibility/verification states;
- accepts `VehicleMotionModel` and `FlightSafetyEnvelope` as separate request inputs;
- generates coverage stripes, then fixed-wing Dubins connector geometry;
- validates segment speed, bank and lateral acceleration against the supplied safety envelope;
- builds a canonical `Route` with waypoint/segment lineage;
- deliberately leaves `route.trajectory_geometry_complete = false` and `flight_release_eligible = false`.

The current route candidate is therefore a structural/kinematic analysis candidate, not a validated flight trajectory.

## 2. Canonical vehicle profile is not yet a complete safety-model source

The canonical `VehicleEquipmentCapabilityProfile` currently exposes:
- identity: vehicle ID, vehicle type/model, airframe and propulsion configuration, autopilot and firmware;
- flight capability: minimum/maximum/cruise/economic airspeed, climb/descent limits, altitude limits, turning limit, endurance and wind tolerance;
- energy capability: usable energy, battery state/health, degradation, equipment mass/power, reserve and recovery requirements;
- configuration and equipment capability records.

The current profile contract does not, by itself, provide all fields required by `VehicleMotionModel` and `FlightSafetyEnvelope` with auditable evidence. In particular, it does not explicitly establish:
- model revision and exact immutable configuration ID binding for the motion model;
- speed reference (TAS/IAS/air-relative) and units for each speed field;
- validated motion class and evidence class (officially validated / flight-test validated);
- configuration-specific fixed-wing bank limit and validated turn model inputs;
- the minimum-speed basis required by the safety validator (including stall speed at bank and safety margin);
- validated maximum lateral acceleration where required;
- source document/reference and revision for each safety limit.

The profile's `turning_limit` is not safe to interpret as bank angle, radius, or lateral acceleration without an explicit typed unit/meaning contract.

**Binding rule:** do not infer or manufacture these values from generic profile fields. A production adapter must return `UNKNOWN/BLOCKED` when any required evidence, unit, reference, revision or exact configuration binding is absent. Test fixtures must remain test-only.

## 3. Required production binding

Implement a dedicated, versioned resolver that takes:
1. canonical vehicle profile ID and immutable profile revision;
2. exact assigned vehicle ID and configuration ID/version from the mission;
3. explicit motion-model record with motion class, speed reference, reference speed and turn dynamics;
4. independently approved safety-envelope record with all applicable hard limits;
5. evidence references, approval state, effective interval and content hashes for the source records.

It must return one immutable binding object containing:
- `VehicleMotionModel`;
- `FlightSafetyEnvelope`;
- source profile/configuration IDs and versions;
- evidence references and validation state;
- a deterministic dependency identity/hash.

Hard gates:
- exact vehicle ID, configuration ID and configuration version match;
- motion and safety records refer to the same aircraft/configuration revision;
- speed units and reference are explicit and compatible;
- only official or flight-test validated evidence can satisfy flight-safety hard gates;
- expired, revoked, draft, derived-only or missing evidence blocks;
- custom equipment/configurations remain supported, but require their own compatibility, mass/balance, propulsion and safety validation;
- no fallback to synthetic/default limits.

## 4. Wind processing boundary

`WindPerformanceTrajectory::calculate()` evaluates wind samples per existing route segment and computes ground speed, track, elapsed time and energy. It does not currently move route waypoint coordinates to construct a wind-corrected ground-track geometry.

Therefore its current output is a performance assessment of supplied geometry, not a replacement ground-track route. Before NOTAM/obstacle intersection checks are treated as final, the system must either:
- produce a wind-corrected ground-track geometry with explicit segment/waypoint lineage and revalidate it; or
- formally establish that the route geometry is ground-referenced and wind effects only change timing/performance, with evidence supporting that model.

Do not describe the current wind calculation as geometric wind correction.

## 5. Terrain/DEM and vertical trajectory boundary

The MT-01 route candidate currently assigns the camera-derived target AGL value to each waypoint. This is not terrain following and does not establish actual clearance above terrain.

A production terrain stage must provide:
- DEM source, dataset/version, CRS, vertical datum, resolution, coverage bounds, acquisition/effective time and integrity metadata;
- deterministic sampling along every route segment and transition, at a spacing tied to DEM resolution and required clearance;
- explicit handling of voids, nodata, water, cliffs, interpolation uncertainty and datum conversion;
- minimum terrain/obstacle clearance evaluation over the swept path, not only at waypoints;
- vertical profile and climb/descent transition geometry, checked against the assigned aircraft's validated performance envelope;
- snapshot identity propagated into `RouteEnvironmentReference::terrain_snapshot_id`.

Missing coverage, unsupported datum or unresolved nodata must block the candidate. A constant AGL waypoint field is not evidence of terrain clearance.

## 6. NOTAM/airspace integration boundary

`NotamProhibitedZoneValidator` accepts a `Route` and a versioned `NotamSnapshot`, checks snapshot identity/time, altitude reference and prohibited-zone intersections. The MT-01 pipeline currently does not call it.

Integration must:
- use the exact current NOTAM/airspace snapshot referenced by the route;
- run against the final ground-referenced trajectory geometry after any wind/terrain geometry transformation;
- preserve altitude datum and vertical overlap semantics;
- block on missing/stale/mismatched snapshots, unsupported altitude references and invalid zone records;
- retain findings and snapshot provenance in the candidate's validation evidence.

The validator currently requires AGL when zones are present. Until vertical datum conversion is implemented, unsupported NOTAM altitude references must remain blocked rather than being numerically compared.

## 7. Energy and reserve integration boundary

`WindPerformanceTrajectory` can calculate segment time/energy from explicit performance inputs and wind samples, including task execution energy/time. Production use still requires an aircraft/configuration-specific performance source, battery state/health and degradation, equipment power/mass, recovery requirement and reserve policy, all with provenance.

The calculation must cover the complete trajectory, including turns, transitions, climb/descent, task execution and recovery. Missing segment samples or unsupported energy model inputs must block; no generic energy-per-metre defaults.

## 8. Promotion criteria for a complete route candidate

Only after all of the following pass may a separate promotion stage set `trajectory_geometry_complete = true` and provide a `VALIDATED/ALLOWED` candidate to the multi-candidate weather ranking path:

1. canonical vehicle/configuration and equipment binding;
2. complete 3D/4D trajectory geometry including takeoff, transitions, turns, task legs and recovery;
3. validated dynamic envelope for every segment;
4. DEM/terrain and obstacle clearance over the complete swept path;
5. wind-ground-track semantics and geometry resolved;
6. current NOTAM/airspace validation against the final geometry;
7. complete energy, reserve and recovery assessment;
8. complete lineage, source snapshots, versions, hashes and independent validation evidence.

Any missing/unknown item remains a hard block. AUTO and MANUAL must use the same safety and release gates; only their route-input processes differ.

## 9. Next implementation sequence

1. Locate and expose the authoritative vehicle/equipment profile repository or service used by the production application.
2. Add the versioned profile-to-motion/safety binding and contract tests, including mismatch, stale evidence, unit ambiguity and custom configuration cases.
3. Locate the authoritative DEM service/data interface; if none exists, define the provider contract before implementing a sampler.
4. Add wind-ground-track geometry semantics and tests; do not conflate track angle/performance output with corrected waypoint coordinates.
5. Integrate NOTAM/airspace validation after final geometry generation.
6. Integrate full-trajectory energy/reserve and recovery assessment.
7. Add the promotion gate and multi-candidate publication into the existing weather reranking path.
8. Verify the Qt production runtime actually invokes the task-aware generator and publishes its candidate set.

No stage may silently substitute test fixtures or structural graph paths for production aircraft data or complete flight trajectories.
