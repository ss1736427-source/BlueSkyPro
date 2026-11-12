# MT-01 Production Data Binding Contract

Status: engineering gap analysis; implementation baseline reviewed on `main` at 2026-11-12.

This document records the production inputs that must be bound before MT-01 may promote a structural route candidate to a complete, safety-attested trajectory. It does not grant flight-release eligibility.

## Requirement ownership and duplicate control

This is the MT-01 production integration contract, not a second system-requirements baseline. Existing requirement identities remain controlled by `01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md` and the repository requirements traceability process. This document adds no new SYS-REQ or SW-REQ identifiers.

The related documents have distinct responsibilities:
- `02_SYSTEM_DESIGN/PLANNING/CONSTRAINED_ROUTE_GENERATOR_001.md` owns the constrained spatial-search boundary: only feasible/open-domain edges enter route search.
- `02_SYSTEM_DESIGN/PLANNING/NOTAM_PROHIBITED_ZONE_VALIDATION_001.md` owns final NOTAM intersection findings, full segment occupancy-time checks after wind timing, and NOTAM boundary presentation semantics.
- `02_SYSTEM_DESIGN/PLANNING/WIND_PERFORMANCE_4D_TRAJECTORY_001.md` owns the single wind/performance calculation and its time/energy result contract.
- `02_SYSTEM_DESIGN/PLANNING/CANDIDATE_COMPARISON_OPTIMIZATION_001.md` owns comparison/ranking of candidates that have already passed feasibility gates.
- `02_SYSTEM_DESIGN/PLANNING/FINAL_PLANNING_INTEGRITY_CHECK_001.md` owns final identity, lineage and dependency-integrity checks.

MT-01-specific responsibility is to bind those contracts into one executable sequence: show/acquire the applicable NOTAM snapshot; generate coverage and connectors in the constrained domain; calculate route performance with wind; re-evaluate NOTAM applicability using the resulting segment occupancy times; then validate and promote only candidates that pass all required gates. The detailed rules remain in their owning documents; they must be referenced, not copied into parallel requirement sets.

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

## 6. NOTAM-aware geometry generation (mandatory, before optimization)

This is a generation constraint, not merely a post-generation validation step.

The mission-planning sequence must:
1. Determine the mission AOI, planned time window, altitude range and intended trajectory corridor.
2. Acquire the current authoritative NOTAM/airspace snapshot covering the AOI plus the required lateral safety buffer.
3. Normalize each relevant restriction into independent geometry components. Preserve each component's NOTAM ID, restriction type, geometry (polygon/circle), lower/upper altitude, altitude datum, effective interval, source/version and snapshot identity. A compound NOTAM must not connect separate AREA and WI CIRCLE components into one artificial polygon.
4. Resolve whether each component is active for the planned segment time and whether its vertical band overlaps the candidate's altitude profile. Do not treat every 2D footprint as an all-altitude permanent exclusion.
5. Convert active, vertically relevant prohibited components into buffered exclusion geometry and feed those constraints into the route generator before it creates coverage legs and turn connectors.
6. Generate/modify candidate geometry around those exclusions. If a safe path cannot be generated, reject that candidate or return BLOCKED; never draw a straight connector through a restricted area.
7. Re-evaluate time-dependent NOTAM applicability after wind/performance timing changes and validate the final complete trajectory against the same referenced snapshot (or a newer explicitly acquired snapshot). If the timing change activates a restriction, regenerate/reject the affected candidate.
8. Preserve NOTAM IDs, component IDs, buffers, altitude/time decisions and snapshot provenance in candidate findings and lineage.

The current implementation now has a first executable NOTAM-to-geometry path:
- `Mt01MappingRequest` carries the immutable `NotamSnapshot`, explicit planned mission start/end UTC epochs and an explicit positive lateral safety buffer for applicable zones.
- `NotamGeometryAdapter` validates snapshot/source identity and zone records, filters prohibited zones by overlap with the planned time window and the camera-derived target AGL altitude, and fails closed for applicable zones whose vertical datum is not AGL.
- Each normalized `NotamZone` carries both a NOTAM ID and a stable `component_id`; separate AREA and WI CIRCLE components remain independent even when they share one NOTAM ID. Both IDs are returned by the geometry adapter and retained in final validator findings.
- Circular restrictions are converted to circumscribed 32-vertex polygons using the NOTAM radius plus the supplied buffer. Polygon restrictions are represented as the source polygon plus edge strips and circumscribed vertex disks. The coverage planner unions overlapping components by scanline subtraction.
- The combined operator exclusions and NOTAM-derived exclusions are passed to both `CoveragePathPlanner` and `Mt01KinematicGeometryBuilder`; the resulting canonical route carries the exact NOTAM snapshot ID in `RouteEnvironmentReference::airspace_snapshot_id`.
- The HMI weather path normalizes the same FAA NMS snapshot used by MT-01 geometry, verifies mission geometry and all candidate routes against the source query circle plus the configured buffer, and passes that immutable snapshot into `AlgorithmOrchestrator` with `MissionProblem::requires_notam_validation` enabled. The time-aware `NotamProhibitedZoneValidator::validate(route, snapshot, trajectory)` runs after wind/performance has produced the 4D `TrajectoryResult` and before candidate ranking. Missing snapshot, missing 4D result, mismatched snapshot identity, or incomplete query coverage blocks selection.
- `NotamProhibitedZoneValidator` and the geometry adapter currently accept AGL only. AMSL/Flight Level conversion is not implemented.

Known limitations remain explicit:
- The adapter consumes a normalized snapshot; authoritative NOTAM retrieval, free-text interpretation, component normalization and AOI-scoped source acquisition remain upstream responsibilities.
- The buffer is a single explicit request value, not yet a policy-resolved per-NOTAM/per-operation value. No implicit buffer default is invented.
- Polygon buffering is a conservative local-planar approximation made from overlapping components, not a geodesic polygon-offset engine. The existing local projection limits still apply.
- The pipeline does not yet prove a wind-corrected ground-track geometry. The current wind stage updates performance/time for supplied geometry; if the route is not ground-referenced, a geometric wind correction stage remains necessary.
- The Qt MT-01 bridge now accepts the FAA NMS normalized record list, snapshot ID/source/version/evaluation epoch, explicit planned UTC start/end and a separate NOTAM buffer from the mission profile. It maps each Polygon/MultiPolygon outer component to a stable per-NOTAM component ID and returns those IDs with the planning result. Non-area display-only records are not treated as exclusion polygons; missing or incompatible vertical bounds remain blocked.
- The FAA NMS client is currently STAGING/FIT test-only. Each successful query retains the exact center and radius sent to the source (including request rounding), and the MT-01 bridge passes that coverage metadata into the snapshot. Before geometry generation, NotamGeometryAdapter requires a valid AOI boundary and verifies that every AOI vertex plus the configured safety buffer fits inside the source query circle; missing or incomplete coverage blocks the candidate. A new query clears the previous snapshot and coverage metadata before validation/authentication, preventing stale coverage from being reused. This proves the requested circular query extent only; it does not prove that FAA NMS returned every applicable NOTAM or that the source is authoritative for the mission jurisdiction. Production NOTAM source coverage and AOI-scoped acquisition remain open integration gates. CI tests exercise the coverage contract, but do not by themselves prove live source completeness or flight-release suitability.

The architecture is now: `AOI/time/target altitude -> normalized NOTAM snapshot with source-query coverage and mission time window -> buffered exclusions -> MT-01 coverage and turn generation -> wind/performance 4D timing -> final time-dependent NOTAM validation (including snapshot-window containment) -> candidate ranking`.

The 4D validator requires the trajectory start to match the snapshot's planned mission start and the wind-corrected trajectory end to remain within the snapshot's planned mission end. A route that runs beyond the time window used to filter NOTAM records is rejected; it cannot be accepted on the basis of an incomplete time slice.

## 7. NOTAM/airspace integration boundary

`NotamProhibitedZoneValidator` has a time-aware overload that consumes the exact route, snapshot and wind/performance `TrajectoryResult`. The `AlgorithmOrchestrator` invokes it after weather assessment and before ranking when `MissionProblem::requires_notam_validation` is true.

The production caller must:
- bind the same NOTAM snapshot ID to the route and the `MissionProblem`;
- require the final 4D trajectory to remain inside the exact mission time window used to filter the snapshot;
- set `requires_notam_validation = true` for flight-planning candidates;
- provide a weather assessor that returns the actual 4D trajectory, not only summary time/energy values;
- ensure the route geometry is ground-referenced or provide a validated wind-corrected geometry before treating the intersection result as final;
- retain the returned NOTAM findings and snapshot provenance in the candidate evidence.

The validator requires AGL when zones are present. Until vertical datum conversion is implemented, unsupported NOTAM altitude references remain blocked rather than being numerically compared. The orchestrator fails closed if the required snapshot or 4D weather trajectory is missing.

## 8. Energy and reserve integration boundary

`WindPerformanceTrajectory` can calculate segment time/energy from explicit performance inputs and wind samples, including task execution energy/time. Production use still requires an aircraft/configuration-specific performance source, battery state/health and degradation, equipment power/mass, recovery requirement and reserve policy, all with provenance.

The calculation must cover the complete trajectory, including turns, transitions, climb/descent, task execution and recovery. Missing segment samples or unsupported energy model inputs must block; no generic energy-per-metre defaults.

## 9. Promotion criteria for a complete route candidate

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

## 10. Next implementation sequence

1. Locate and expose the authoritative vehicle/equipment profile repository or service used by the production application.
2. Add the versioned profile-to-motion/safety binding and contract tests, including mismatch, stale evidence, unit ambiguity and custom configuration cases.
3. Locate the authoritative DEM service/data interface; if none exists, define the provider contract before implementing a sampler.
4. Complete operational NOTAM integration: replace the FAA NMS STAGING/FIT test source with an approved authoritative source for the mission jurisdiction, establish an approved buffer policy, and implement required AGL/AMSL/FL datum conversions. The HMI snapshot-to-geometry path, query-coverage proof, mission-window binding and post-wind 4D NOTAM gate are now connected; the FAA staging endpoint does not establish operational clearance.
5. Add wind-ground-track geometry semantics and tests; after wind changes segment ETA, re-evaluate NOTAM validity and regenerate/reject affected candidates.
6. Run operational acceptance scenarios with an approved NOTAM source and retain evidence proving that the same source snapshot, mission time window and final ground-referenced 4D trajectory reach the orchestrator gate.
7. Integrate full-trajectory energy/reserve and recovery assessment.
8. Add the promotion gate and multi-candidate publication into the existing weather reranking path.
9. Verify the Qt production runtime actually invokes the task-aware generator and publishes its candidate set.

No stage may silently substitute test fixtures or structural graph paths for production aircraft data or complete flight trajectories.
