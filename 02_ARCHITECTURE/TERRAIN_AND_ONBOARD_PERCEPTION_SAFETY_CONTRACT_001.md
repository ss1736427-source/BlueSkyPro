# BlueSky PRO — Terrain Data and Onboard Perception Safety Contract 001

**Status:** CONTROLLED ARCHITECTURE DECISION  
**Scope:** Terrain data, onboard obstacle perception, terrain following, runtime safety, and resource/performance policy  
**Applies to:** Physical Flight and Virtual Flight, with mode-specific evidence provenance  
**Authority:** End-to-end operational lifecycle baseline and shared mission lifecycle contract

## 1. Decision summary

BlueSkyPro shall use a layered safety model:

1. **Ground planning layer:** a terrain elevation dataset supports preliminary route altitude planning, terrain profiling, clearance assessment, and map visualization.
2. **Onboard perception and autonomy layer:** the aircraft's verified sensors and flight-control/autonomy functions detect local obstacles and, where explicitly supported, perform terrain following or local avoidance during execution.
3. **Mission/runtime supervision layer:** BlueSkyPro uses the aircraft's verified capability contract, telemetry and event interfaces to assess whether the planned mission remains valid, record deviations, and request/accept only supported replanning actions.

Onboard perception complements terrain data; it does not make terrain data unnecessary and does not guarantee detection of every obstacle. A camera or vision sensor being present is not proof of a usable obstacle-avoidance capability.

The planning application must not claim it can directly control a vendor's onboard avoidance function unless the aircraft/autopilot interface explicitly exposes and verifies that control.

## 2. Terrain source decision

MapTiler Terrain RGB v2 is selected as the **first integration candidate** for the common global terrain source, subject to regional coverage, data-quality, licensing, and operational validation.

Documented MapTiler dataset characteristics:
- global land coverage;
- nominal horizontal resolution 30 m globally and 5 m in selected areas;
- vertical resolution/quantization stated by MapTiler as 6 m;
- maximum zoom level 14;
- RGB-encoded elevation values suitable for decoding and elevation-profile use.

The RGB encoding precision must not be confused with source-data accuracy. A fine numerical encoding step does not create finer source measurements. MapTiler Terrain RGB is suitable as a lightweight global baseline for visualization and preliminary terrain profiling; it is not, by itself, accepted as a safety-grade high-resolution obstacle or bare-earth terrain model.

The exact MapTiler product, dataset revision, terms of use, attribution, regional resolution, and vertical reference must be recorded in the source metadata. Do not assume the service is free for every usage or that 5 m coverage exists throughout Russia.

No permanent fan-out to multiple DEM providers is permitted. A secondary provider may be used only through an explicit fallback policy when the selected source is unavailable or fails mission-specific quality requirements. The fallback must not silently lower quality or turn a failed validation into PASS.

## 3. DTM, DSM, and obstacle data are distinct

The system shall distinguish:
- **DTM / bare-earth terrain:** ground elevation with buildings and vegetation removed, where such classification is available and validated;
- **DSM / surface model:** elevation of the upper visible surface, potentially including buildings and vegetation;
- **Obstacle data:** explicit obstacle geometry, height, classification, position, confidence, provenance, and validity interval.

A generic terrain tile must not be treated as a complete obstacle database. MapTiler Terrain RGB product metadata must be treated according to the actual source specification; do not label it DTM unless bare-earth semantics are proven.

### 3.1 Shared obstacle and airspace truth across map and planner

The map and the planning/validation pipeline shall not maintain independent interpretations of the same obstacle or airspace restriction. They are separate consumers of source-grounded domain data, not separate authorities.

The mission environment shall retain a **set of source-specific, immutable and versioned snapshots**. It is not one synthetic snapshot that erases the different authority, coverage, update cadence or quality of terrain, obstacle, permanent airspace and NOTAM sources. Each record/component shall retain, as applicable:

- stable source record and component identity;
- source/provider identity, original reference/payload and source revision;
- retrieval time, publication/update time and effective validity interval;
- geographic geometry and coordinate reference system;
- original vertical limits, units and reference (SFC, AGL, AMSL, FL, etc.), plus any explicitly calculated normalized values and conversion lineage;
- object/restriction class and operational meaning;
- source coverage/completeness, quality/confidence and interpretation state;
- integrity/checksum and normalization/parser version.

The mission environment bundle binds the exact source-specific snapshots used for one planning/validation cycle. A shared bundle identity must never be used to imply that different sources contain identical physical data.

**Consumer consistency rules:**

1. The map projection and planner projection shall derive from the same normalized source record/component semantics and source revision wherever they refer to the same record. They may simplify geometry or select different features for display/query performance, but must not reinterpret restriction class, coordinates, altitude limits, validity or confidence independently.
2. Viewport acquisition and mission-AOI acquisition may have different geographic coverage and query timing. They must remain separately identifiable. A viewport response is not proof of complete mission-area coverage; a map refresh must not replace or clear the mission planning snapshot.
3. Where viewport and mission snapshots overlap, matching source record/component IDs and source revisions must yield matching normalized facts. A newer revision replaces the older fact atomically; the UI must not silently combine incompatible revisions.
4. The map shall distinguish confirmed, ambiguous, unsupported, invalid, stale and unavailable data. Missing or unparseable geometry is shown as unavailable/unknown metadata; no footprint, altitude, clearance or empty-airspace state may be fabricated.
5. A restriction may be displayed even when it cannot yet be normalized for route validation. The display must preserve source wording and identify the unresolved datum/geometry/interpretation. Display visibility is not a PASS and does not authorize flight.
6. The planner shall consume typed constraints and provenance, never map-renderer features or styling objects. The map shall consume display projections, never make planning decisions.
7. Obstacle points/volumes, terrain DTM/DSM, permanent airspace volumes, NOTAM restrictions and onboard detections remain distinct object classes. A terrain surface is not an obstacle register; a NOTAM polygon is not automatically a prohibited volume; an onboard detection is not automatically an authoritative static obstacle record.
8. If sources disagree spatially, vertically, temporally or semantically, preserve both source facts, create an explicit conflict finding and prevent the affected area/route from being reported as clear until the conflict is resolved or conservatively bounded by an approved rule. Do not silently choose the most convenient source.
9. Source freshness and coverage are evaluated against the mission AOI and planned occupancy interval. "No object returned" is a clear result only when the source query is complete, current and valid for that AOI/time; otherwise the result is UNKNOWN/INCOMPLETE.
10. Any source revision, coverage change, expiry, normalization change or mission geometry/time change invalidates only the dependent projections and validation evidence, followed by the required revalidation.

**Vertical and spatial presentation rules:**

- Preserve the source altitude text/value/unit/reference beside normalized altitude values.
- Never compare AGL, AMSL and flight levels as if they shared a datum. Convert only with an identified reference surface/pressure model and a recorded conversion method; otherwise mark vertical overlap UNKNOWN.
- Render vertical extents only when both the value and reference are known. A partially known extent remains partial; never substitute zero or invent a lower/upper bound.
- Keep obstacle top elevation/height, terrain elevation, and airspace/NOTAM vertical limits as separate attributes. Do not infer obstacle height from a terrain tile or DSM without a qualified derivation.
- Map labels, colors, line styles and visibility filters are presentation only. They must not alter the source facts or the planner's constraint set.

### 3.2 Relationship to the existing NOTAM architecture

The existing `08_PLANNING/NOTAM_PROCESSING_ARCHITECTURE.md` remains authoritative for one-pass NOTAM interpretation and separate map/planning projections. Its viewport and mission-AOI snapshots are intentionally separate because their query coverage differs; they must not be mistaken for a shared complete dataset. This contract extends the same consistency principle to permanent airspace and obstacle data without duplicating the NOTAM parser or creating another registry/service/cache.

The NOTAM HMI contract in `02_SYSTEM_DESIGN/PLANNING/NOTAM_PROHIBITED_ZONE_VALIDATION_001.md` remains authoritative for showing source-defined limits, preserving original vertical references, representing uncertainty, and retaining records when display filters change.

## 4. Onboard perception and competitor-derived design principles

Commercial systems demonstrate two complementary patterns:
- DJI Mavic 3 Enterprise documents omnidirectional binocular vision, an infrared sensor, obstacle sensing/avoidance, APAS rerouting, and Real-Time Terrain Follow using onboard sensing without requiring imported elevation data for that feature.
- Skydio X10 documentation describes adaptive obstacle margins and warns about thin obstacles, reflective surfaces, moving objects, and environmental limitations.

These are aircraft-specific capabilities, not generic properties of UAVs. BlueSkyPro shall adopt the architectural pattern, not assume equivalent capability across all models.

### 4.1 Vehicle capability profile

Each aircraft variant and configuration shall have a versioned, provenance-bound perception/autonomy capability profile containing, where applicable:
- sensor type and modality;
- coverage sectors and blind zones;
- field of view by direction;
- detection and measurement ranges (kept as separate properties);
- minimum detectable obstacle size/type and known exclusions;
- effective sensing speed and operating envelope;
- lighting, surface texture, weather, visibility, and other environmental limitations;
- sensor processing/update latency where documented;
- supported obstacle-avoidance modes and their authority boundary;
- supported terrain-following modes and whether they are onboard-autonomous or externally commanded;
- degraded/lost-sensor behaviour;
- source document, firmware/software revision, aircraft variant, and validation status.

Unknown or unverified capability values remain UNKNOWN. Do not infer obstacle avoidance from a camera, GNSS, or generic product family name.

### 4.2 Runtime safety boundary

The onboard flight controller/autonomy system remains responsible for immediate stabilization and any native collision-avoidance action it is certified or documented to perform. BlueSkyPro remains responsible for mission-level planning, constraint validation, monitoring, evidence continuity, and supported replanning.

The safety assessment must consider:
- detection range versus current groundspeed;
- end-to-end sensing and actuation latency;
- braking/deceleration and turning performance;
- required separation margin;
- sensor field of view and blind zones;
- obstacle type and detectability;
- environmental operating limits;
- available manoeuvre space and aircraft dynamics.

If perception becomes degraded, unavailable, or outside its verified envelope, the system must not continue to assume that vision compensates for DEM uncertainty. The configured aircraft contingency policy must select a validated degraded mode, hold/return/land/abort action, or require pilot intervention as appropriate. Exact actions are aircraft- and mission-specific.

Obstacle avoidance does not replace NOTAM/airspace compliance, terrain clearance, energy reserve, or regulatory validation.

## 5. Mavic 3 Enterprise baseline

DJI's published Mavic 3 Enterprise specifications describe an omnidirectional binocular vision system supplemented by a downward infrared sensor. The published sensing ranges, effective sensing speeds, fields of view, and operating conditions differ by direction. DJI also documents Real-Time Terrain Follow for mapping using omnidirectional sensing.

BlueSkyPro may use Mavic 3 Enterprise as the initial reference aircraft, but it must bind the exact aircraft variant, firmware, and capability evidence. Published product specifications do not by themselves prove that BlueSkyPro can invoke, configure, or receive every proprietary DJI autonomy function through an external API.

The capability profile must distinguish:
- obstacle detection range from distance-measurement range;
- onboard automatic avoidance from mission-level route replanning;
- native Real-Time Terrain Follow from an externally controlled terrain-following command.

Until an interface is verified, BlueSkyPro shall record the feature as aircraft-native capability and shall not claim direct external control of it.

## 6. Planning and execution sequence

The common mission sequence is:

`Mission objective → vehicle capability binding → NOTAM/airspace snapshot → terrain source selection → route generation → terrain/clearance validation → weather application → time/energy/performance recalculation → selected route → package validation → readiness → execution`

During execution:

`Telemetry / onboard perception events → capability-envelope check → normal/degraded/critical state → supported local response or replanning → evidence update`

A local obstacle event may trigger a local replan only if the aircraft interface provides sufficient state and control authority, the new route can be validated against airspace/NOTAM, terrain, vehicle dynamics, weather, and energy, and the aircraft can safely accept the revised package/commands. Otherwise the aircraft's approved native contingency behaviour and/or pilot action governs.

## 7. Performance and data-loading policy

The number of external sources is intentionally minimized. The application shall not continuously poll multiple terrain providers.

Required policy:
- select one primary terrain dataset per mission area;
- fetch only the mission area plus a bounded safety/contingency margin;
- use tile/region caching with source and dataset revision keys;
- decode and normalize a terrain snapshot once, then share the immutable snapshot with the map, profile, and planner;
- perform network I/O, decoding, and terrain sampling off the UI thread;
- avoid re-fetching terrain on ordinary map pan/zoom when the required tiles are already cached;
- distinguish map-rendering tiles from authoritative planning snapshots;
- invalidate only affected derived results when source, route, or mission inputs change;
- bound concurrent requests, memory use, and route sample counts;
- expose cache hit/miss, fetch, decode, validation, and UI latency metrics.

A presentation-only map tile response is not automatically an authoritative planning input. A planning snapshot must retain source identity/version, dataset identity/version, CRS, vertical datum, horizontal resolution, vertical accuracy, coverage, NoData mask, and retrieval time.

## 8. Virtual Flight

Virtual Flight uses the same terrain and vehicle capability contracts but must identify all simulated sensor/perception evidence as SIL/simulator provenance. A synthetic sensor result cannot satisfy physical-flight sensor admission or be represented as actual onboard perception evidence.

The selected virtual aircraft model must have an explicit sensor/autonomy simulation profile. Missing simulated sensor behaviour remains UNKNOWN; it must not be filled with assumed real-aircraft capability.

## 9. Current repository implementation status (verified on main)

Implemented:
- `TerrainDemGrid` accepts decoded immutable DEM samples with source/dataset revisions, CRS, vertical datum, horizontal resolution, vertical accuracy, and NoData cells.
- `TerrainProfileValidator` checks complete route geometry, route/snapshot binding, AGL waypoint heights, grid spacing, coverage, NoData, sample workload, and conservative terrain clearance.
- Terrain validation feeds the existing lifecycle terrain evidence adapter.
- Weather candidate processing retains a typed evidence bundle and exact selected route/source lineage.
- Shared physical/virtual lifecycle contract requires terrain validation in both modes and keeps evidence provenance distinct.
- End-to-end lifecycle baseline requires staged planning, environment updates by impact, and avoids unnecessary full recomputation.

Not yet demonstrated as production integration:
- a live MapTiler Terrain RGB v2 acquisition/decoder/provider feeding `TerrainDemGrid`;
- regional source-quality and vertical-datum validation for actual Russian mission areas;
- an authoritative obstacle-data pipeline;
- a production import of permanent Russian AIP/airspace datasets with a source snapshot explicitly bound to regulatory validation;
- a dedicated obstacle snapshot and obstacle-clearance evidence field in `MissionLifecycleEvidence`; the current READY gate requires NOTAM, terrain and regulatory evidence but does not independently prove obstacle-data coverage;
- a typed, versioned per-aircraft vision/avoidance/terrain-follow capability registry connected to mission admission and readiness;
- runtime ingestion of onboard obstacle/perception events and a verified aircraft-specific replan/contingency adapter;
- a production AUTO/MANUAL path assembling all typed terrain, weather, regulatory, vehicle, and task evidence into lifecycle READY.

The current terrain validator is a consumer of decoded grids, not a DEM downloader. Its tests use synthetic fixtures and are contract tests, not real-world terrain evidence.

## 10. Acceptance criteria

1. One primary DEM source is used per mission area; fallback is explicit and quality-gated.
2. MapTiler Terrain RGB is decoded into a normalized immutable planning snapshot; source precision is not represented as source accuracy.
3. Source metadata, CRS, vertical datum, coverage, NoData, revision, and retrieval time are retained.
4. Map, terrain profile, and planner reuse the same snapshot; UI interactions do not trigger redundant provider requests.
5. Terrain, obstacle, NOTAM, and permanent airspace data remain distinct safety inputs with source-specific provenance and validity.
6. Map and planner projections of the same source record/revision preserve identical normalized semantics; differing acquisition coverage is explicit and cannot be represented as complete mission coverage.
7. Source conflicts, incomplete coverage, stale data, unknown vertical references and unsupported geometry remain visible as UNKNOWN/CONFLICT and cannot be silently treated as clear.
8. Map display filtering, simplification and styling never modify the canonical facts or the planner's constraint set.
9. Each aircraft variant has a verified capability profile; UNKNOWN values cannot be promoted to PASS.
10. Onboard perception is treated as complementary local protection, not a substitute for terrain/airspace/energy validation.
11. Degraded sensing invokes only an aircraft- and mission-validated contingency; no generic avoidance behaviour is assumed.
12. Virtual sensor evidence is explicitly SIL and cannot satisfy physical evidence requirements.
13. Performance metrics prove that data acquisition/decoding does not block the UI and that cache reuse prevents repeated downloads.

## 11. External technical references

- MapTiler Terrain RGB: https://docs.maptiler.com/schema-raster/terrain-rgb/
- MapTiler Terrain RGB usage and elevation profiles: https://docs.maptiler.com/guides/map-tiling-hosting/data-hosting/rgb-terrain-by-maptiler/
- DJI Mavic 3 Enterprise specifications: https://enterprise.dji.com/mavic-3-enterprise/specs
- DJI Mavic 3 Enterprise product capabilities and Real-Time Terrain Follow: https://enterprise.dji.com/mavic-3-enterprise
- Skydio X10 Operator Manual: https://support.skydio.com/hc/article_attachments/48924056567067
