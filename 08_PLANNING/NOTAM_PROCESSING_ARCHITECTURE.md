# NOTAM Processing Architecture

Status: architecture decision for implementation on `main`.
Scope: shared NOTAM interpretation for mission planning and map presentation.

## Decision

Use one source-ingestion and interpretation pipeline that produces an immutable, versioned canonical NOTAM snapshot. Planning and map presentation are independent consumers of that snapshot. Neither consumer calls the other's code, and neither reparses NOTAM text or rebuilds source geometry during normal operation.

Do not create two independent text parsers. That would allow map and optimizer to disagree about the same restriction. Do not make the map renderer a dependency of the optimizer.

## Processing stages

1. **Source adapters**
   - Fetch source records (FAA NMS now; additional authorities later).
   - Preserve original payload/text, source identity, issue/update timestamps, validity fields and query coverage.
   - No map styling and no mission-planning decisions here.
   - Fetch scope is explicit: map viewport requests and mission AOI requests are separate query intents.

2. **Shared NOTAM interpreter (one pass per new/changed source record)**
   - Parse source fields and text into typed facts: geometry components, vertical limits and datum, validity interval, restriction type/condition, activation schedule, source references.
   - Prefer authoritative structured geometry/fields. Text-derived geometry is a separately provenance-tagged interpretation, never silently treated as source geometry.
   - Preserve raw text and parser evidence.
   - Emit explicit interpretation state: `CONFIRMED`, `AMBIGUOUS`, `UNSUPPORTED`, or `INVALID`.
   - Do not infer `PROHIBITED` merely because a record contains a polygon.
   - No QML, WebEngine, MapLibre, map style or UI dependencies.

3. **Canonical immutable snapshot**
   - Contains source lineage, record/component IDs, typed geometry, vertical reference, effective time interval/schedule, restriction semantics, interpretation confidence/state and raw-source references.
   - Geometry is validated and normalized once. Build a spatial index (e.g. R-tree) and temporal lookup metadata once per snapshot.
   - Snapshot identity is content/version based; identical unchanged inputs reuse cached results.
   - Snapshot publication is atomic: consumers never observe partially interpreted data.

4. **Planning projection**
   - Takes canonical snapshot + AOI + planned time interval + altitude/vertical reference + vehicle/operation context.
   - Performs a cheap indexed query, then exact time/altitude/geometry filtering.
   - Produces a compact immutable planning constraint set: only relevant prohibited/conditional components, buffers and provenance.
   - Ambiguous or unsupported NOTAM that may intersect the AOI/time/altitude is a planning blocker or requires operator resolution; it is not silently ignored.
   - The optimizer receives typed geometry/constraints, not raw NOTAM text and not map display objects.
   - Wind/time correction triggers re-evaluation against the same canonical snapshot, not a complete text reparse.

5. **Map projection**
   - Consumes the same canonical snapshot, but creates display features only: labels, symbols, selected-record details and simplified geometry for current zoom/viewport.
   - Display-only point/line NOTAM remain visible without becoming area exclusions.
   - Map styling, clustering, label placement and WebGL work are isolated from planning.
   - Map updates do not mutate the canonical snapshot or delay planning.

## Runtime and performance rules

- Run source I/O and interpretation off the UI thread.
- Interpret only new or changed records; cache by source ID/version and content hash.
- Never parse NOTAM text, validate polygon topology, or build buffers on every map frame, pan, zoom or optimizer iteration.
- Keep map and planner consumers read-only and independently schedulable.
- Planning queries use spatial/temporal indexes and return only relevant components; exact geometric checks run only on this reduced set.
- Use bounded worker queues and cancellation of obsolete viewport requests. A map refresh must not cancel or replace a mission-AOI snapshot.
- Measure separately: fetch latency, interpretation time, cache hit rate, spatial query time, planner filtering time, map projection time and frame time.

## Safety and semantic rules

- A search radius or Q-line is query coverage metadata, never restriction geometry.
- Structured source geometry takes precedence; text-derived geometry must carry provenance and validation state.
- Restriction class, lower/upper limits, datum and active schedule must be explicit. Unknown values are not converted to zero or assumed AGL.
- Polygon/line/point display geometry is not automatically a prohibited volume.
- Every planning constraint retains NOTAM ID, component ID, source/version and interpretation evidence for audit.
- The planning gate must fail closed when a potentially applicable restriction cannot be interpreted safely.

## Current implementation findings

- `FaaNmsClient.cpp` currently calls `NotamGeometryEngine::normalize()` while ingesting each FAA record.
- `NotamGeometryEngine` is a Qt/QVariant-based HMI implementation that handles source geometry validation and text-derived area/circle geometry.
- `Mt01MappingBridge.cpp::parseNotamSnapshot()` then parses the normalized JSON geometry again into planning `NotamZone` objects.
- The bridge currently assigns `NotamRestrictionType::Prohibited` to each accepted area component; this is too broad until restriction semantics are classified explicitly.
- `MapTilerWebMap.qml` initiates NOTAM requests from viewport changes. A viewport snapshot must not be the authoritative mission-AOI snapshot.
- Therefore, current code has duplicated interpretation/geometry parsing across the HMI and planning boundary, and couples map-driven acquisition to mission planning inputs.

## Implementation sequence

1. Define Qt-independent canonical NOTAM domain types and snapshot contract in the shared core.
2. Extract the one-pass interpreter from the HMI geometry engine; keep a thin Qt adapter only for source/UI integration.
3. Add a snapshot repository/cache with atomic publication and source/version/content-hash lineage.
4. Add separate map and planning projections. Map consumes display features; planner consumes typed constraint sets.
5. Add independent mission-AOI acquisition/query scope; viewport queries remain map-only.
6. Replace `Mt01MappingBridge::parseNotamSnapshot()` with canonical snapshot lookup/projection. Remove duplicate JSON geometry parsing and unconditional prohibited classification.
7. Add contract/performance tests: same snapshot semantics for both consumers, ambiguous/unsupported records, altitude/time filtering, map-query isolation, cache reuse, and no interpretation work on pan/zoom or optimizer iterations.

## Acceptance criteria

- Exactly one interpretation result per unchanged source record/version.
- Map and optimizer derive their respective outputs from the same canonical interpretation and preserve identical NOTAM lineage.
- Viewport movement cannot replace or invalidate the mission planning snapshot.
- Optimizer receives only relevant typed constraints and never parses raw text or map payloads.
- Map projection and rendering cannot block or change optimizer results.
- Ambiguous applicable NOTAM cannot silently pass the planning gate.
- Performance tests demonstrate cached reuse and bounded query/projection cost.
