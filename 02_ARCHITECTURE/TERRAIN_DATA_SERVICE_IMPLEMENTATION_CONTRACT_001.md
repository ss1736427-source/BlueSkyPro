# Terrain Data Service — Acquisition and Profile Integration Contract 001

Status: implementation contract  
Scope: mission-area terrain acquisition, immutable terrain snapshots, route-profile sampling, and separation of display data from flight-safety evidence.

## 1. Non-negotiable source separation

BlueSky PRO shall distinguish two terrain data classes:

- **Authoritative terrain**: a decoded, georeferenced DTM/DEM with verified horizontal CRS, vertical datum, dataset revision, coverage, and declared vertical accuracy. Only this class may produce `TerrainDemGrid`, terrain-clearance assessments, or lifecycle terrain evidence.
- **Visual terrain context**: elevation tiles or other terrain products whose source accuracy, vertical datum, or bare-earth semantics are not sufficiently established for safety use. This class may be shown as a clearly labelled contextual profile, but shall never be converted into authoritative terrain evidence.

MapTiler Terrain RGB v2 is initially classified as **visual terrain context**. Its RGB encoding precision is not source accuracy. The service shall not invent or assume its vertical datum or vertical accuracy.

The two classes must have separate snapshot identifiers and explicit source-kind metadata. A visual snapshot identifier must never be copied into `Route.environment.terrain_snapshot_id` or used to satisfy a terrain lifecycle gate.

## 2. Route-profile presentation

The profile shall support two independent states:

1. **Authoritative profile** — bound to the exact `terrainSnapshotId` in the canonical route. It may be labelled with a vertical datum only when the provider metadata and conversion are verified.
2. **Visual context profile** — bound to its own visual snapshot and route revision. It must carry a visible status such as `VISUAL ONLY · ACCURACY / DATUM NOT VERIFIED`. It shall not be displayed as an authoritative AMSL series and shall not imply terrain clearance.

If both sources are available, they may be displayed as separate series with distinct labels. They must not be silently blended, substituted, or compared as if they shared a datum. Missing or stale data is represented as unavailable, never as zero elevation or synthetic terrain.

The route AGL series and terrain elevation series use separate vertical axes unless a verified transformation permits a common reference. Screen-space distance between independent axes is not clearance.

## 3. Acquisition and cache ownership

The mission terrain pipeline is independent of map viewport rendering:

- Request coverage for the full route/AOI plus a bounded margin.
- Plan XYZ tiles using the existing `TerrainRgbTileMatrix`.
- Download asynchronously with bounded concurrency, request timeout, retry policy, and cancellation.
- Never log API keys or full credential-bearing URLs.
- Keep mission acquisition alive when the user pans or zooms the map.
- Cache decoded immutable tiles by provider, dataset/style revision, zoom, x, and y. Reuse only where provider licence and cache directives permit.
- Do not reuse `TileCacheManager::beginViewUpdate()` as the mission acquisition lifecycle: it intentionally drops queued requests for the previous viewport.
- Invalidate a mission acquisition only when mission identity, route revision, AOI, provider revision, or relevant source metadata changes.

The service must report partial coverage and failed tiles explicitly. A partial response must not be represented as a complete profile.

## 4. Geospatial sampling

The service shall preserve a documented mapping between WGS84 coordinates and XYZ tile pixels:

- Web Mercator latitude clamp and XYZ row convention must match the tile matrix.
- Pixel coordinates must account for the decoded tile dimensions (256 or 512), not assume one fixed size.
- Adjacent tile edges must be sampled without gaps or duplicated progress values.
- Longitude wrapping at the antimeridian must be handled consistently with AOI planning.
- Elevation interpolation must fail closed on transparent/NoData pixels.
- Route samples must include endpoints and intermediate positions along every segment. Sampling density must be bounded by a configured maximum count and justified against the source's effective spatial resolution.
- Route progress is cumulative geodesic distance divided by total route distance, not waypoint index.
- Each published sample set carries route ID/version, mission ID/version, source snapshot ID, source kind, units, datum state, and coverage state.

For authoritative sampling, the shared immutable terrain snapshot must be the same snapshot consumed by the map, profile, and terrain validator. No component may independently fetch or reconstruct a different terrain surface for the same validation cycle.

## 5. MapTiler Terrain RGB v2 restrictions

The existing decoder's elevation formula is a pixel encoding contract, not a statement of terrain accuracy. The source must remain visual-only until the exact product metadata for the requested area/revision establishes:
- vertical datum and any required transformation;
- whether the surface is bare-earth DTM or includes buildings/vegetation;
- effective horizontal resolution and source lineage;
- vertical accuracy and its statistical definition;
- coverage and NoData semantics.

A 30 m nominal/global product or a selected-area higher-resolution product is not automatically suitable for low-altitude UAV clearance validation. Encoding increments must not be presented as measurement accuracy.

## 6. Failure and lifecycle behaviour

The service shall fail closed for invalid credentials, HTTP errors, malformed images, unsupported dimensions, invalid tile matrix, NoData at required samples, incomplete AOI coverage, unknown authoritative datum, stale route lineage, and sample-count limits.

Visual context failures affect only the visual context display. They must not mutate route validation or lifecycle state.

Authoritative terrain failures must prevent terrain validation evidence from being created and therefore must continue to block READY. A visual context snapshot cannot clear that blocker.

## 7. Required automated contracts

Tests shall cover:
- XYZ-to-pixel mapping for tile corners and centre at 256 and 512 dimensions;
- adjacent-tile boundaries and antimeridian routes;
- cumulative-distance progress and endpoint inclusion;
- multi-segment sampling with bounded sample count;
- NoData and incomplete tile coverage;
- stale mission/route/provider revision invalidation;
- cancellation and out-of-order network completion;
- API key redaction in diagnostics;
- visual snapshot cannot satisfy authoritative terrain evidence;
- authoritative snapshot identity is preserved through profile, map, and validator adapters.

## 8. Implementation sequence

1. Implement typed terrain source/snapshot metadata and sample-set contracts.
2. Implement asynchronous mission-scoped tile acquisition, separate from viewport tile loading.
3. Implement georeferenced mosaic/sampling and focused unit tests.
4. Add a separate visual-context profile input; retain the existing authoritative profile binding unchanged.
5. Connect the same authoritative immutable snapshot to map, profile, and terrain validator.
6. Add integration tests proving visual-only terrain cannot grant terrain evidence or READY.
