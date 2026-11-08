# BlueSky PRO — Weather Coverage Manager / Mission Binding 001

**Status:** DESIGN PROPOSAL — based on current canonical mission and route models  
**Scope:** spatial-temporal weather coverage for a complete mission  
**Implementation status:** specification only; no runtime or HMI changes in this change

## 1. Objective

Weather acquisition and validation shall be driven by the canonical BlueSky mission and its selected planning solution. Weather is not a route-only download and not a map overlay.

For each mission revision, the system shall determine the required weather coverage for:
- operational area / AOI and task geometry;
- selected route and its complete trajectory;
- planned origin / takeoff;
- destination, landing and recovery locations when defined;
- alternate recovery locations, holding/orbit areas and contingency paths when defined;
- each task phase and its planned execution interval.

The resulting weather evidence shall be linked to the exact mission ID, mission version, selected route ID/version and calculation inputs. Missing or stale coverage shall be explicit and shall block any planning calculation that requires it.

## 2. Existing canonical model — repository reconnaissance

The following existing contracts were inspected on `main`:

| Existing contract | Confirmed capability | Limitation relevant to weather |
|---|---|---|
| `04_SOFTWARE/PLANNING/model/mission_model.hpp` | Mission ID/version, mission origin, geometry objects, intent/task description, constraints, environment snapshots, vehicle/payload assignments and route-candidate references | `TaskDefinition` currently contains only a description; it has no structured phases, geometry references or execution intervals |
| `MissionGeometryObject` | Geometry ID/type, CRS, coordinate vertices, template association and geometry version | Geometry is a flat vertex vector; ring boundaries, holes and multipolygon parts are not represented explicitly |
| `04_SOFTWARE/PLANNING/model/route_model.hpp` | Route ID/version, mission ID/version lineage, ordered waypoints/segments, explicit altitude datum, constraints and environment snapshot references | No per-waypoint planned passage time or segment duration/ETA fields in the route model |
| `04_SOFTWARE/PLANNING/model/mission_model_validation.hpp` | Existing geometry types include POINT, WAYPOINT_ROUTE, POLYGON_AOI, COVERAGE_GRID, CORRIDOR, ORBIT, INSPECTION_POINTS, SEARCH_AREA, DELIVERY_ROUTE and OBSERVATION_POINTS | Structural type validation does not define operational role semantics for each POINT or geometry object |
| `qt/BlueSkyPRO-HMI/src/ManualMissionModelAdapter.cpp` | Converts map-captured geometry and planned origin into the canonical typed Mission model | Adapter currently carries geometry vertices and optional altitude metadata; it does not define weather coverage semantics |
| `04_SOFTWARE/PLANNING/manual_route_candidate_builder.cpp` | Converts supported ordered line geometries into a route with mission lineage and explicit AGL waypoint heights | Explicitly does not build AOI coverage/orbit/unordered observation routes; these require task-specific generators |
| `02_ARCHITECTURE/BLUESKY_MISSION_MODEL.md` | Environment inputs are versioned snapshots; wind is a dynamic planning input applied before trajectory/time/energy evaluation | Does not yet define a spatial-temporal weather coverage contract |
| `02_SYSTEM_DESIGN/MISSION/MISSION_TIME_MODEL_001.md` | Mission time calculations use UTC; planned and actual times are separate | Document status is DRAFT_FOR_AGREEMENT; weather adapter must consume authoritative schedule/ETA values, not invent them |

The current route wind adapter samples one midpoint per segment at one forecast valid time. This is a first-stage approximation and is not sufficient to establish coverage for AOI/task operations or time-varying route weather.

## 3. Architectural placement

The Weather Coverage Manager (WCM) is an application/domain service between the canonical mission/route models, the acquisition service and the normalized weather engine.

```text
Canonical Mission (ID + version)
          |
          +---- Mission geometry / AOI / task areas
          +---- Mission origin / recovery locations
          +---- Selected Route (ID + version)
          +---- Task schedule / planned UTC intervals
          +---- Vehicle altitude and mission constraints
          |
          v
Weather Coverage Request Builder
          |
          v
Weather Coverage Manager
  |       |          |
  |       |          +-- Coverage completeness / freshness / QC
  |       +------------- Shared forecast cache and provenance
  +--------------------- Provider request planning (NOAA GFS first)
          |
          v
GRIB2 acquisition -> ecCodes decoder -> normalized fields
          |
          v
Spatial / vertical / temporal sampling and coverage assessment
          |
          v
Immutable Mission Weather Snapshot
          |
          +---- mission ID/version
          +---- route ID/version
          +---- task/geometry references
          +---- forecast model, cycle and valid times
          +---- source file hashes and decoder/QC versions
          +---- coverage matrix and rejected/missing regions
          |
          v
Planning / trajectory / task feasibility
```

The WCM shall not own mission geometry or replace the canonical Mission model. It consumes the existing model through a read-only request adapter and returns a versioned weather snapshot/reference.

## 4. Coverage request contract

A coverage request shall be an immutable description of what must be evaluated, derived from one exact mission revision and selected solution.

Required identity:
- `mission_id`, `mission_version`;
- `route_id`, `route_version` when a route is selected;
- calculation input version;
- request ID and request schema version.

Spatial components:
- AOI/task geometry IDs and geometry versions;
- route corridor, not only the route centreline;
- mission origin;
- destination/recovery and alternate locations when explicitly defined;
- holding/orbit areas and contingency routes when explicitly defined;
- coordinate reference system for every component;
- configured safety/deviation buffer and its policy/version.

Temporal components:
- mission planned start / ETD in UTC;
- task phase start/end or planned time window in UTC;
- expected route segment passage time and duration, when available;
- forecast valid-time window and required lead times;
- actual execution time must remain separate from planned time.

Vertical components:
- planned altitude profile;
- explicit altitude reference (AGL/AMSL/flight level);
- terrain/DEM snapshot identity when AGL conversion or terrain-relative coverage is required;
- required weather model levels and interpolation policy.

No missing value may be silently defaulted. If the mission does not define a required time, geometry role, altitude datum or recovery point, the request builder shall report a typed input deficiency rather than infer one.

## 5. Coverage construction rules

1. Build the spatial union of all explicitly identified mission weather-relevant objects.
2. Expand the union by a configurable buffer/corridor. Buffer policy must be versioned and justified by the operational use case; do not hard-code an arbitrary radius.
3. Include the AOI interior and boundary, not only its bounding rectangle.
4. Include the full route corridor and sample the route at a resolution derived from grid spacing, segment length, vertical profile and a configured maximum sample spacing.
5. Include takeoff/climb, landing/approach and recovery points where their operational role is explicit.
6. Include alternates, holding areas and contingency paths only when represented by explicit mission data.
7. Convert the resulting geometry to provider request bounds. NOAA NOMADS currently accepts rectangular subsets; downloading extra grid cells is acceptable, but post-decode coverage must be checked against the original geometry.
8. Split antimeridian-crossing requests into supported sub-requests when the provider and decoder contract support it. Until implemented, return an explicit unsupported-coverage result; never wrap coordinates silently.
9. Deduplicate overlapping provider requests through the shared cache. Cache reuse must not remove the mission-specific snapshot and provenance link.

## 6. Time and altitude sampling

- Forecast selection is based on expected mission execution time, not only download time or a single static model timestamp.
- Each route sample is associated with expected UTC passage time. Each task area is evaluated over its planned execution interval.
- Time interpolation is permitted only between compatible forecast valid times and under a documented maximum time-gap policy. No extrapolation beyond available forecast valid times.
- Vertical interpolation uses explicit AGL/AMSL datum conversion and approved model levels only. The existing 100 m maximum vertical interpolation gap remains the default until a separate evidence-based change is approved.
- AGL sampling depends on the applicable terrain/DEM snapshot. The system must not treat AGL as AMSL or use one terrain height for an entire large AOI.
- The current GFS U/V data path supplies horizontal wind only. Vertical wind is not inferred. Other task-weather variables (gusts, precipitation, visibility, cloud ceiling, temperature, turbulence, etc.) require explicit provider products and separate variable/QC contracts.

## 7. Coverage result and fail-closed behavior

The WCM shall return a typed result, not a boolean alone:

- `COMPLETE`: all required spatial, temporal and vertical samples meet configured quality/freshness rules;
- `PARTIAL`: some required samples are valid, but one or more required components are missing;
- `MISSING`: no usable data for a required component;
- `STALE`: data exist but fall outside approved freshness/forecast validity policy;
- `REJECTED`: data fail decoder, pairing, coordinate, level, unit or QC rules;
- `UNSUPPORTED`: request geometry, CRS, time or vertical reference cannot be processed by the configured provider/engine.

Each result shall identify the affected geometry/task/route segment/time interval and the reason. A planner requiring weather shall not consume PARTIAL, MISSING, STALE, REJECTED or UNSUPPORTED coverage as if it were complete.

The result must distinguish:
- provider acquisition success;
- GRIB2 container validation;
- scientific field decoding;
- spatial/vertical/temporal interpolation;
- coverage completeness;
- mission-specific operational acceptance.

A successful download or a valid GRIB2 signature is not proof of valid mission weather coverage.

## 8. Snapshot, cache and reproducibility

The immutable mission weather snapshot shall retain:
- snapshot ID/version;
- mission ID/version and route ID/version;
- all source geometry IDs/versions and request parameters;
- provider/model/product, forecast cycle and valid times;
- source file identifiers, SHA-256 hashes and acquisition timestamps;
- decoder, interpolation and QC implementation versions;
- requested and actual coverage bounds;
- sample counts and coverage result by mission component;
- missing/rejected sample inventory and diagnostic reasons.

Shared cache keys shall be provider/model/product + forecast cycle + forecast lead + requested variables/levels + normalized spatial tile/subset identity. Cache reuse is a storage optimization only; it shall never conflate two mission snapshots or omit per-mission lineage.

## 9. Integration boundaries and non-regression

- Do not change the canonical Mission or Route model merely to fit the weather downloader.
- Do not modify map rendering, map providers, QML panels or existing HMI interactions in the WCM implementation.
- Do not change `WindPerformanceTrajectory` physics as part of weather coverage integration.
- The WCM must be independently testable without a live network and must support deterministic synthetic weather fixtures.
- NOAA acquisition remains asynchronous and must not block the HMI thread.
- Weather snapshot references shall be attached to the mission environment snapshot and selected route lineage through explicit versioned integration points.

## 10. Required model gaps before full mission coverage

The reconnaissance found several domain gaps that must be resolved through explicit contracts before claiming full coverage:

1. **Task phases and time windows:** current `TaskDefinition` is free text. A structured task/phase model or an existing authoritative task contract must supply geometry references and planned UTC intervals.
2. **Geometry topology:** current geometry vertices do not encode polygon rings, holes or multipolygon parts. The WCM adapter must not guess these boundaries. Reuse an existing GIS geometry contract if one exists; otherwise propose a versioned geometry extension separately.
3. **Operational point roles:** mission origin is represented, but destination, landing, alternate and holding roles are not explicitly modeled in `Mission`. Do not infer these from arbitrary POINT geometry IDs.
4. **Route schedule:** route waypoints/segments have no expected passage timestamps. The WCM needs an authoritative predicted trajectory/time result or a versioned schedule extension.
5. **Terrain linkage:** route altitude is explicit AGL, but area-wide AGL sampling needs a terrain snapshot and a defined conversion policy.
6. **Provider variable coverage:** current NOAA integration acquires U/V wind only. It cannot yet support all meteorological constraints relevant to every task type.

These are integration gaps, not reasons to create a second mission model. Resolve them by mapping to existing authoritative subsystem contracts first.

## 11. Verification plan

Offline contract tests shall cover:
- request creation from a mission with AOI + selected route + origin;
- mission ID/version and route ID/version mismatch rejection;
- task geometry and time-window association;
- route corridor and AOI boundary/interior coverage;
- buffer policy versioning and deterministic geometry expansion;
- explicit handling of missing alternate/landing/task data (no inference);
- multiple forecast valid times and bounded temporal interpolation;
- altitude datum conversion and terrain snapshot mismatch;
- missing grid cells, unsupported levels, stale forecast and incomplete time coverage;
- cache reuse across overlapping missions without losing snapshot lineage;
- antimeridian rejection/splitting behavior;
- fail-closed planner gate for every non-COMPLETE result;
- unchanged route physics and no HMI/map regressions.

Live-provider validation is a separate acceptance stage and must use a real NOAA GRIB2 sample, not only synthetic fixtures.

## 12. Recommended implementation sequence

1. Complete and pass the Windows/MSVC ecCodes build/runtime validation independently.
2. Add read-only mission-to-weather request adapter using current Mission, MissionGeometryObject, MissionOrigin and Route contracts.
3. Implement request validation and coverage-result types before any network orchestration.
4. Implement deterministic spatial coverage calculation and unit tests.
5. Add forecast time selection and per-route/task temporal sampling after authoritative time inputs are identified.
6. Add cache/provenance and immutable mission weather snapshot.
7. Connect asynchronous acquisition and decoder.
8. Add planner admission gate requiring COMPLETE coverage for the relevant variables and intervals.
9. Validate using controlled mission fixtures, then real NOAA data.

**Acceptance boundary:** this design does not declare the current weather engine operationally ready. Flight use remains blocked until real-data validation, mission coverage completeness, temporal sampling, Windows runtime packaging and the applicable safety/verification gates pass.
