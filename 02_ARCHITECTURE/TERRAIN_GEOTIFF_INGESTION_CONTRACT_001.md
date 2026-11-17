# Authoritative DTM GeoTIFF ingestion contract 001

Status: implementation contract; not evidence that a production DTM provider is connected.

## 1. Objective

Provide a local, provider-neutral ingestion path from georeferenced raster datasets into the existing `TerrainDemGrid` consumed by `TerrainProfileValidator`. The first supported interchange format is GeoTIFF, decoded through GDAL. Acquisition, licensing, source approval, vertical-datum normalization, and mission lifecycle remain separate responsibilities.

The importer is not a data provider and must not download terrain or select a dataset automatically.

## 2. Trust boundary

A GeoTIFF file is untrusted input. Raster tags, file names, embedded descriptions, and user-entered sidecars do not prove that a surface is bare-earth DTM or that its accuracy is a conservative absolute bound.

The importer must receive an immutable source attestation from an application-controlled, approved source registry. The attestation must bind:
- registry record ID and approval/revision;
- source and dataset identifiers and versions;
- exact dataset checksum (SHA-256);
- declared surface type, which must be `BARE_EARTH_DTM`;
- vertical datum and the applied datum-normalization operation/version;
- vertical accuracy model, which must be `CONSERVATIVE_ABSOLUTE_BOUND`, and its positive bound in metres;
- licensing/usage authorization and validity interval.

A free-form manifest supplied alongside the raster must never be sufficient to create safety evidence. Until the source registry and checksum binding are implemented, GeoTIFF imports are inspection-only and must not be wired to READY.

## 3. Supported raster contract

Initial importer scope:
- single-band elevation raster;
- GeoTIFF readable by GDAL;
- horizontal CRS exactly EPSG:4326 after semantic CRS comparison;
- north-up, unrotated affine geotransform;
- monotonic regular pixel spacing and pixel-centre coordinates;
- elevation values convertible to metres using the raster band scale/offset and declared units;
- explicit NoData preserved as NaN;
- finite values only outside NoData;
- complete raster dimensions and bounded total cell count.

Unsupported inputs fail closed. Reprojection, resampling, vertical datum conversion, mosaicking, and gap filling are separate preprocessing operations with their own provenance. The importer must not silently perform them.

## 4. Grid orientation and geometry

GDAL north-up GeoTIFF row zero is the northernmost row. The existing `TerrainDemGrid` contract is south-up (row zero is the southernmost row), with origin coordinates at pixel centres. The importer must reverse raster rows exactly once and calculate:
- `origin_longitude_deg = geotransform[0] + pixel_width / 2`;
- `origin_latitude_deg = geotransform[3] + pixel_height * (rows - 0.5)`;
- positive latitude and longitude step magnitudes;
- conservative horizontal resolution from the smaller ground spacing at the raster's representative latitude.

Reject rotated, skewed, south-up, non-regular, or non-geographic grids in the first version. Do not infer a vertical datum from EPSG:4326; horizontal CRS and vertical datum are independent.

## 5. Safety eligibility

A decoded raster becomes eligible for terrain validation only if all checks pass:
1. checksum matches the approved registry record;
2. registry attestation is current and explicitly approved;
3. dataset identity/version matches the attestation;
4. surface type is bare-earth DTM;
5. vertical datum is known and already normalized to the route's required datum;
6. accuracy model is a conservative absolute bound, not RMSE, LE90, CE90, or an unspecified statistic;
7. raster geometry, dimensions, scale, units, and NoData are valid;
8. route AOI is fully covered without NoData at any required sample;
9. immutable snapshot identity is carried unchanged through profile, map, validator, and lifecycle evidence.

Failure at any stage leaves terrain evidence absent and READY blocked. A successful file decode alone is not a terrain clearance PASS.

## 6. Resource and security limits

- Open datasets read-only.
- Enforce maximum raster width, height, and total cell count before allocating the output grid.
- Reject unsupported band counts, data types, invalid scale/offset, and non-finite decoded elevations.
- Do not execute scripts, VRT references, external overviews, or remote subdatasets from untrusted files.
- Do not log local sensitive paths or credentials.
- Keep import work off the UI thread and make cancellation explicit.
- Use GDAL/PROJ runtime data packaged with the Windows application and verify that the packaged executable can open a test GeoTIFF without relying on developer-machine paths.

## 7. Required tests

- Valid EPSG:4326 north-up raster imports with correct pixel-centre origin.
- North-to-south raster row order is reversed into the south-up planning grid.
- Scale, offset, and metre units are applied correctly.
- NoData becomes NaN and is rejected by the route validator when sampled.
- Wrong CRS, missing CRS, rotation/skew, invalid geotransform, non-metre units, and unsupported band count fail.
- Raster size limits are enforced before allocation.
- Checksum mismatch and unapproved/expired source attestation fail.
- DSM, UNKNOWN surface, RMSE, LE90, missing vertical datum, and missing accuracy bound cannot produce safety-eligible output.
- Vertical datum mismatch is rejected; no implicit EGM2008/Baltic-1977 conversion.
- A complete import does not itself mark a mission READY; the existing route-vs-terrain validator and lifecycle coordinator remain mandatory.

## 8. Integration sequence

1. Add GDAL as a pinned build dependency on Linux and Windows; package GDAL and PROJ runtime data for Windows.
2. Implement a bounded GeoTIFF decoder that returns a typed decoded raster, not lifecycle evidence.
3. Keep source metadata and eligibility checks inside the existing `TerrainDataService`/terrain ingestion path; do not introduce a separately administered source-registry subsystem.
4. Normalize datum and raster orientation only when a real trusted DTM provider requires it.
5. Adapt the trusted decoded raster to the existing `TerrainDemGrid` contract.
6. Pass one immutable snapshot reference to the map, profile, and `TerrainProfileValidator`; reuse existing decoded/cached data rather than adding another acquisition pipeline.
7. Add end-to-end lifecycle tests proving that visual-only, untrusted, stale, incomplete, or mismatched inputs cannot satisfy READY.

## 9. Current system boundary

The current MapTiler Terrain RGB service remains `VISUAL_ONLY` with unknown datum and accuracy. It is not a substitute for this importer. The current terrain validator accepts an already decoded grid but does not acquire or decode a production DTM. No real DTM provider, approved source registry, or GeoTIFF ingestion is claimed by this contract.

## 10. MapTiler visual-profile snapshot identity

The MapTiler Terrain RGB v2 route-profile path now computes a deterministic visual snapshot identity from:
- provider/version and zoom;
- mission, route, route-version, and calculation-input lineage;
- ordered route waypoint IDs and coordinates;
- sorted tile coordinates, dimensions, and normalized decoded RGBA pixel content.

Every generated profile sample carries the same `snapshotId`, `sourceId`, `evidenceClass=VISUAL_ONLY`, `verticalDatum=UNKNOWN`, and `verticalAccuracy=UNKNOWN`. The HMI rejects a sample series with a missing or mixed snapshot identity and displays the snapshot suffix beside the profile.

This closes profile-local visual lineage only. It does not claim that MapTiler tiles are the authoritative DTM, does not bind the map renderer and lifecycle validator to this visual snapshot, and cannot satisfy terrain clearance or READY. The authoritative registry, approved DTM attestation, checksum binding, and common map/profile/validator snapshot remain outstanding.


## 11. Existing integration boundaries and implementation constraint

The current code has three distinct paths; they must not be described as one shared snapshot yet:

- **Map:** `FlightChart.ui.qml` hosts `MapTilerWebMap`. MapLibre requests MapTiler Terrain RGB directly for 3D terrain rendering and visual NOTAM terrain sampling.
- **Profile:** `MissionProfileWindow.qml` calls the existing `TerrainDataService`, which acquires/caches the same MapTiler Terrain RGB v2 tiles and samples the route. Its generated snapshot ID is profile-local and explicitly `VISUAL_ONLY`.
- **Safety validation:** `TerrainProfileValidator` evaluates a supplied `TerrainDemGrid`. `GeoTiffTerrainReader` currently produces inspection-only grids with unknown surface, datum, and accuracy; no production DTM acquisition is connected.

Implementation constraint: preserve these boundaries until a real, qualified DTM source is available. Do not route MapTiler visual elevations into safety validation. Do not create a second registry/service or a second terrain cache. The next integration work should reuse `TerrainDataService` and the existing `TerrainDemGrid` contract, and only connect the map to the same snapshot when the map can actually consume that snapshot's tile/raster data—not merely copy its ID.
