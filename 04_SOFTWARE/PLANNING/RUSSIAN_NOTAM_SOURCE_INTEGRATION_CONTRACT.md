# Russian NOTAM and Airspace Source Integration Contract

Status: source-selection and production-integration contract, 2026-11-12.
Target jurisdiction: Russian Federation.
Repository rule: implement on `main`; do not create a feature branch for this work.
Requirement control: this document adds no SYS-REQ/SW-REQ identifiers. Existing requirement ownership and traceability remain in the master requirements register.

## 1. Source authority and verified access model

The designated operational source organization is the branch "Centre of Aeronautical Information" (ЦАИ) of FSUE "State ATM Corporation" (ФГУП «Госкорпорация по ОрВД»), acting within the Russian aeronautical-information publication/distribution system.

Verified public-source facts as of this review:
- Rosaviatsia's official information page directs users to ЦАИ for Russian aeronautical-information products.
- ЦАИ describes round-the-clock NOTAM processing and operational publication/distribution.
- ЦАИ states that NOTAM reception/distribution is performed over AFTN.
- The ЦАИ site exposes an official NOTAM product page and an active-NOTAM list, but access to folders containing official aeronautical-information products is restricted to registered users.
- ЦАИ publishes the Russian AIP and related products, including NOTAM series for en-route/airspace changes and aerodrome changes.
- ЦАИ's product catalogue separately lists restriction zones, prohibited zones, dangerous zones, UAS-flight zones and airspace boundaries. These are not interchangeable with temporary NOTAM records.

Official references:
- Rosaviatsia aeronautical/met information: https://favt.gov.ru/dejatelnost-organizacija-ispolzovanija-vozdushnogo-prostranstva-aeronavi-meteorolog-obespech/
- ЦАИ NOTAM information and series: https://caica.ru/notam/?lang=ru
- ЦАИ official NOTAM list: https://caica.ru/ANI_Official/notam/?lang=ru
- ЦАИ Russian AIP products: https://www.caica.ru/products/
- ЦАИ download portal (registered-user access): https://www.caica.ru/dnload/
- Ministry of Transport Order 05.02.2026 No. 50, Federal Aviation Rules "Procedure for development and provision of aeronautical information": https://www.consultant.ru/document/cons_doc_LAW_531369/2ff7a8c72de3994f30496a0ccbb1ddafdaddf518/

The public website is not verified as a machine-readable operational API. Do not scrape its pages or infer an API endpoint and call that an operational integration. AFTN is the published distribution channel; the production interface, authorization, message format/transport, delivery guarantees, retention and commercial/usage terms must be confirmed directly with ЦАИ. Registered-user downloads may be used only under the applicable access terms and must not be represented as a streaming feed.

## 2. Required source acquisition design

Implement a provider boundary; do not couple the planner to FAA NMS or a specific transport:

- `RussianAisNotamProvider`: production adapter for an approved ЦАИ feed or authorized delivery arrangement.
- `NotamMessageArchive`: durable raw-message archive, immutable and indexed by source, message identity, receipt time and content hash.
- `NotamNormalizer`: converts source messages to canonical NOTAM records and geometry components while preserving raw text and parse diagnostics.
- `RussianAirspaceDataProvider`: separately imports applicable AIP/official datasets for permanent or published prohibited, restricted, dangerous and UAS-flight zones.
- `NotamSnapshotBuilder`: builds a consistent, versioned AOI/time snapshot from the baseline plus all deltas through a known source watermark.

The FAA NMS staging adapter remains test-only and must not be selected as the Russian production provider.

## 3. Data classes that must remain separate

1. Temporary NOTAM restrictions and operational changes.
2. Permanent or periodically amended AIP airspace structures and restrictions.
3. Published prohibited/restricted/dangerous areas and UAS-flight zones from approved Russian aeronautical-information products.
4. Mission-specific operator exclusions and permissions.

Merge them only at the planning constraint layer. Retain source class and legal/procedural provenance on every resulting component. Do not treat a missing NOTAM as proof that permanent airspace restrictions do not exist.

## 4. Canonical record and provenance

Every imported message must retain:
- issuing office / source identifier and source channel;
- complete raw NOTAM text and original encoding;
- NOTAM series, number, year and any linked replacement/cancellation identity;
- Q-line fields when present, including FIR, subject, traffic, purpose, scope, lower/upper limits and coordinates/radius;
- item A location/FIR, item B/C effective interval, item D schedule, item E full text, and item F/G limits when present;
- original altitude units and datum/reference exactly as received;
- original coordinate representation and parsed geometry;
- received-at UTC, source-issued time, effective interval, source watermark/sequence if supplied;
- parser version, normalization version, validation state and content hash.

Each geometry component needs a stable component ID distinct from the parent NOTAM ID. A compound NOTAM may contain multiple independent geometries (for example AREA and WI CIRCLE); do not connect them into one artificial polygon. Preserve holes/rings and all source geometry metadata. Keep the original message available for audit and manual resolution.

The canonical record must distinguish:
- source text from machine-interpreted fields;
- unknown from zero/unlimited;
- missing vertical limits from an unbounded vertical range;
- UTC timestamps from local-time schedules;
- an unparsed message from a message with no geographic restriction.

## 5. Parsing and validation rules

- Parse standard NOTAM structure and Russian/English text without discarding either language.
- Item D schedules must be expanded/evaluated in the source-defined time basis; do not silently treat a schedule as continuously active.
- Handle NOTAMN, NOTAMR and NOTAMC (or their current source equivalents) as lifecycle events. Replacements and cancellations must update the same logical message chain, not create duplicate active restrictions.
- Validate coordinates, radius, polygon closure/topology, FIR/location, effective interval, altitude bounds and units.
- Preserve Q-line limits and item F/G limits independently; resolve precedence only under a documented rule grounded in the source specification.
- Unknown, contradictory or malformed geometry/vertical limits must create a blocking finding for affected missions, not be guessed.
- Convert vertical datums only through a separately validated transformation with the required reference data. Until AGL/AMSL/Flight Level conversion is implemented and verified, unsupported or ambiguous datum remains BLOCKED.
- A NOTAM that cannot be geometrically interpreted must remain visible as an unresolved operational warning and block any candidate whose applicability cannot be excluded.

## 6. Snapshot completeness and persistence

A mission snapshot is complete only when all of the following are known:
- approved source identity and jurisdiction coverage;
- baseline/current-list identity and its publication or retrieval time;
- all subsequent deltas through a source watermark or equivalent completeness marker;
- successful parsing/normalization status for every relevant message;
- spatial coverage of the full AOI, route corridor and configured lateral safety buffer;
- temporal coverage of the complete planned mission interval, including expected wind-driven ETA changes;
- a deterministic snapshot ID and content hash.

Do not clear the last known valid snapshot before a replacement has passed authentication, integrity, completeness and normalization checks. Stage new data separately; atomically promote it only after validation. Retain old snapshots for audit and reproducibility.

If a feed disconnects, an update sequence has a gap, the active list is stale, source completeness cannot be proven, or a full baseline is missing, mark the snapshot STALE/INCOMPLETE and block new route selection. Do not interpret an empty response as a valid empty airspace state unless the source explicitly attests that the query completed successfully and returned zero applicable records.

Incremental updates must preserve previously received still-active records. Expire a record only from its effective end or an authenticated cancellation/replacement event, never because it was absent from an incomplete delta response.

## 7. Planner integration sequence

1. Determine mission AOI, route corridor, planned UTC start/end, altitude envelope and required buffer.
2. Load applicable permanent AIP/airspace datasets and their current effective versions.
3. Acquire the Russian NOTAM baseline and deltas for the mission jurisdiction/AOI and full time interval through the approved ЦАИ channel.
4. Normalize each NOTAM into independent, provenance-bearing geometry components.
5. Display all applicable restrictions before route generation, including unresolved records and source freshness/completeness.
6. Apply vertical/time applicability and approved buffers. Unknown datum or incomplete coverage blocks the affected planning operation.
7. Pass the union of applicable permanent restrictions, NOTAM restrictions and operator exclusions into both coverage-leg and turn/connector generation. Never repair a forbidden intersection by drawing a straight connector through it.
8. Apply wind/performance to each candidate, derive actual segment occupancy times, and re-evaluate NOTAM schedule/effective-time applicability.
9. Validate the final ground-referenced 4D geometry against the same immutable source snapshot or a newer complete snapshot. If wind timing changes activate a restriction, regenerate or reject the candidate.
10. Rank only candidates that pass all airspace, aircraft, terrain and trajectory gates. Keep source provenance and findings with the candidate.

## 8. Acceptance tests

- Baseline import contains several active NOTAM series and preserves full raw message text.
- Incremental NOTAMR/NOTAMC events replace/cancel the correct logical message without duplicate active geometry.
- A delta gap or stale feed marks the snapshot incomplete and blocks planning.
- An empty successful response is distinguishable from authentication, transport, parsing or coverage failure.
- Compound NOTAM geometry yields independent stable components; AREA and WI CIRCLE are not joined.
- Scheduled NOTAM is evaluated at actual segment occupancy time, including after wind/performance timing.
- Unknown altitude datum, missing vertical limits, invalid geometry and unresolved parser output fail closed.
- Permanent AIP restricted/prohibited/dangerous/UAS zones remain active independently of NOTAM expiry.
- Snapshot promotion is atomic; a failed refresh cannot erase the last known valid snapshot.
- Route generation avoids every applicable exclusion for both coverage legs and connectors; final 4D validation catches intersections and timing conflicts.
- Audit replay using the same raw messages, parser version and snapshot hash yields identical normalized records/findings.

## 9. Current implementation boundary and next external dependency

The existing MT-01 path already accepts a normalized immutable NOTAM snapshot, applies time/altitude filtering and buffers before coverage/turn generation, binds the snapshot ID to the route, checks query coverage, and performs a post-wind 4D NOTAM validation before candidate ranking. Its current source adapter path is FAA NMS STAGING/FIT and is not operationally suitable for Russian jurisdiction.

The next production dependency is not another planner algorithm: it is an approved ЦАИ delivery arrangement and its technical specification. Obtain written confirmation of:
1. supported machine delivery protocol (AFTN endpoint/mailbox, file exchange, or another approved interface);
2. authorization and customer registration requirements;
3. message scope/series required for Russian domestic and intended UAS operations;
4. baseline and delta delivery, cancellation/replacement semantics and completeness/watermark guarantees;
5. official coordinate/vertical-limit coding rules and applicable AIP dataset formats;
6. test feed, test messages, SLA/latency, retention and permitted software redistribution/use.

Until these are confirmed and an end-to-end test feed is received, report Russian NOTAM integration as `SOURCE-CONTRACT-DEFINED / OPERATIONAL-FEED-BLOCKED`, not as completed or flight-ready.

## 10. Preserve the currently connected test source

The currently connected FAA NMS adapter and its STAGING/FIT credentials/configuration must remain available and unchanged for engineering tests. Do not disconnect, delete, replace, or silently disable this provider as part of Russian-source work.

Provider selection must be explicit by environment/profile:
- TEST / STAGING: retain the existing FAA NMS provider for parser, snapshot, geometry, coverage, lifecycle, and post-wind validation tests where its dataset is suitable. Every test result must identify the FAA source and must not be represented as Russian airspace coverage.
- PRODUCTION / Russian jurisdiction: select only an approved ЦАИ delivery adapter after its access and completeness contract is confirmed. Do not silently fall back from ЦАИ to FAA NMS for an operational mission.
- OFFLINE / FIXTURE: use controlled, versioned test fixtures with an explicit synthetic/test-source marker.

A test snapshot from FAA NMS may validate software behavior, but it cannot establish the presence or absence of Russian restrictions and cannot authorize flight release in Russian airspace. Keep provider identity, jurisdiction, environment, source watermark and snapshot hash visible in diagnostics and persisted route provenance.
