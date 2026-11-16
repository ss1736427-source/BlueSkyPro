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
- a typed, versioned per-aircraft vision/avoidance/terrain-follow capability registry connected to mission admission and readiness;
- runtime ingestion of onboard obstacle/perception events and a verified aircraft-specific replan/contingency adapter;
- a production AUTO/MANUAL path assembling all typed terrain, weather, regulatory, vehicle, and task evidence into lifecycle READY.

The current terrain validator is a consumer of decoded grids, not a DEM downloader. Its tests use synthetic fixtures and are contract tests, not real-world terrain evidence.

## 10. Acceptance criteria

1. One primary DEM source is used per mission area; fallback is explicit and quality-gated.
2. MapTiler Terrain RGB is decoded into a normalized immutable planning snapshot; source precision is not represented as source accuracy.
3. Source metadata, CRS, vertical datum, coverage, NoData, revision, and retrieval time are retained.
4. Map, terrain profile, and planner reuse the same snapshot; UI interactions do not trigger redundant provider requests.
5. Terrain, obstacle, NOTAM, and airspace data remain distinct safety inputs.
6. Each aircraft variant has a verified capability profile; UNKNOWN values cannot be promoted to PASS.
7. Onboard perception is treated as complementary local protection, not a substitute for terrain/airspace/energy validation.
8. Degraded sensing invokes only an aircraft- and mission-validated contingency; no generic avoidance behaviour is assumed.
9. Virtual sensor evidence is explicitly SIL and cannot satisfy physical evidence requirements.
10. Performance metrics prove that data acquisition/decoding does not block the UI and that cache reuse prevents repeated downloads.

## 11. External technical references

- MapTiler Terrain RGB: https://docs.maptiler.com/schema-raster/terrain-rgb/
- MapTiler Terrain RGB usage and elevation profiles: https://docs.maptiler.com/guides/map-tiling-hosting/data-hosting/rgb-terrain-by-maptiler/
- DJI Mavic 3 Enterprise specifications: https://enterprise.dji.com/mavic-3-enterprise/specs
- DJI Mavic 3 Enterprise product capabilities and Real-Time Terrain Follow: https://enterprise.dji.com/mavic-3-enterprise
- Skydio X10 Operator Manual: https://support.skydio.com/hc/article_attachments/48924056567067
