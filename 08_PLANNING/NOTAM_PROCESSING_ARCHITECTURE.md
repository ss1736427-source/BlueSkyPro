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

## Implementation status on main

### Implemented in the first shared-interpreter stage

- `FaaNmsClient.cpp` invokes `NotamGeometryEngine::normalize()` once for each new or changed FAA source record.
- The same interpreter now produces two projections in one pass:
  - `geometryJson` and existing geometry/status fields for the map (kept compatible);
  - versioned `planningComponents` with typed coordinates, altitude bounds/datum, provenance, restriction class and interpretation state for planning.
- The planning bridge consumes `planningComponents`; it no longer reparses `geometryJson` or reconstructs polygon components from JSON.
- Restriction classification is conservative: only explicit closure/prohibition wording is classified `PROHIBITED`; restricted/danger/temporary-reserved wording is `CONDITIONAL`; an area with no explicit class is `AMBIGUOUS`. Conditional and ambiguous applicable records block planning. A polygon alone is never enough to infer prohibition.
- Unsupported interior rings and missing/incompatible vertical bounds fail closed for planning; they remain available to the map through the unchanged map geometry projection.
- FAA record interpretation is cached by environment, NOTAM ID and SHA-256 source fingerprint (bounded to 4096 entries). New/changed records are interpreted on a QtConcurrent worker; completed results are published to the map/planner as one batch on the owning thread.
- The map-facing `geometryJson` generation and existing NOTAM drawing inputs were not replaced by planner-specific structures.

### Remaining work

- The interpreter currently resides in the HMI Qt Core layer. Extracting a Qt-independent domain library is still outstanding; no QML, WebEngine or rendering dependency is introduced into the interpreter.
- FAA acquisition now publishes independent result channels: `notamRecords` / `notamSnapshotId` remain viewport-only for the map; `missionNotamRecords` / `missionNotamSnapshotId` and their own coverage metadata are reserved for planning. A map refresh cannot clear or overwrite the mission snapshot.
- MT-01 manual planning derives a conservative circular query envelope from the canonical mission AOI polygon, adds a 2 NM acquisition margin, and requests a separate mission snapshot when the existing one does not cover the AOI. The 100 NM FAA query limit is enforced. The operator is told to repeat SET after acquisition completes; no route is calculated against a viewport-only snapshot.
- Mission requests are queued behind an active viewport request and take priority immediately after it completes. Query failures are reported on the matching consumer signal. A successful replacement is published atomically; an existing mission snapshot remains available while the replacement is in flight.
- The weather/mission-profile NOTAM inputs now bind only to the mission snapshot. Map display remains bound only to the viewport snapshot.
- Spatial/temporal snapshot indexes and a reusable immutable snapshot repository are not yet implemented. Planning projection currently walks the already interpreted component list.
- Activation schedules and the full NOTAM condition vocabulary are not yet interpreted. Unknown or unsupported area semantics block planning rather than being silently accepted.
- The current explicit semantic classifier is intentionally narrow. It must be expanded against a controlled NOTAM corpus with expected classifications before widening automatic `PROHIBITED` recognition.


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
