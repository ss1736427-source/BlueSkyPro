# SPPI / CAI — Research Findings and BlueSky PRO Integration Implications

**Status:** RESEARCH NOTE — official-source desk research; not an API specification  
**Canonical branch:** `main`  
**Research target:** `https://new.ivprf.ru/` and the Russian SPPI / aeronautical-information ecosystem  
**Important limitation:** the target site could not be inspected directly in this research session. Findings below are based on official indexed publications and publicly accessible source references, not reverse-engineered browser traffic or verified internal API calls.

## 1. Executive conclusion

SPPI should be treated as a regulatory workflow service, not merely as a map. Public official materials describe a user journey that includes checking UAV restrictions, deciding whether a flight plan or airspace regime representation is required, submitting relevant information, and tracking the resulting workflow. Separate official aeronautical-information resources publish airspace data, NOTAM and related documents.

The available evidence does **not** establish the exact internal map provider, SPPI backend topology, API endpoints, data schemas, refresh intervals, or whether the currently targeted `new.ivprf.ru` deployment has a public integration API. BlueSky PRO must not assume these details.

## 2. Findings supported by official public materials

### 2.1 Restriction data and official publication channels

The Ministry of Transport FAQ explains that UAV flight zones/restrictions are established through the applicable procedure and published in the Russian Aeronautical Information Publication/collection through the Center of Aeronautical Information (CAI). The same guidance directs users to SPPI's UAV check and its map layers for regional UAV prohibitions.

Public CAI pages provide several classes of aeronautical material, including NOTAM, current NOTAM lists, aeronautical documents, restriction/prohibited/danger zones, UAV flight zones, airspace boundaries, AIP material and database downloads.

This supports a **source-of-record distinction**:
- CAI/AIP publications and NOTAM are authoritative publication channels for the applicable aeronautical information.
- SPPI is a user-facing operational service for checking information and conducting submission workflows.
- The exact mechanism by which SPPI obtains, caches, transforms or displays each dataset remains unverified.

### 2.2 Flight-plan and airspace-regime workflows

Official Ministry of Transport guidance states that flight-plan information is submitted to operational bodies of the Unified Air Traffic Management System (ЕС ОрВД) in applicable cases. The guidance also describes SPPI registration and the submission of temporary/local regime representations and flight plans.

Rosaviatsia's publicly available instructions describe a map-based preflight check: select an area, define a circle, inspect restrictions and conditions, and determine whether a plan or regime representation is required. The described plan workflow includes choosing a plan category, selecting a registered UAV, entering times and altitude, providing the relevant regime reference or geometry, forming the telegraphic message, submitting it, tracking status, and sending departure/arrival messages where applicable.

These are descriptions of the published workflow, not proof that every listed step or field is identical in the current `new.ivprf.ru` build. The exact message format and procedural requirements must be checked against current official instructions before implementation.

### 2.3 Data sources to investigate

| Data class | Publicly evidenced source/channel | BlueSky interpretation |
|---|---|---|
| Published airspace boundaries and zone data | CAI aeronautical publications and database pages | Versioned geospatial source adapter |
| NOTAM and current NOTAM lists | CAI NOTAM publications; other providers may exist | Separate NOTAM adapter; retain original record and validity |
| Regional UAV restrictions/zones | Ministry of Transport decisions/publications, made available through aeronautical information channels | Distinguish source publication from SPPI presentation |
| Temporary/local regimes and related conditions | Applicable ЕС ОрВД / SPPI submission workflow | Model as time-bounded operational constraints, not permanent zones |
| Flight-plan submission and responses | SPPI workflow and applicable ЕС ОрВД procedure | Submission adapter and explicit lifecycle state |
| Departure/arrival messages | Described in Rosaviatsia workflow instructions | Separate operational event/message actions |

The table identifies evidenced public channels; it does not assert that SPPI directly calls each source in real time.

## 3. What remains unconfirmed

The following must be considered unknown until verified directly:

1. Which map renderer/provider and tile/style services are used by `new.ivprf.ru`.
2. Whether restriction overlays come from vector tiles, GeoJSON, WMS/WMTS, database queries, or a mixed architecture.
3. The actual upstream endpoints, authentication scheme, API availability, schemas, rate limits and licensing terms.
4. Update cadence, cache TTL, invalidation behavior and the relationship between SPPI data and the latest CAI publication.
5. The exact geometry/altitude/time fields rendered for each zone class.
6. Whether the interface's status indicators represent only successful submission/receipt or a formal authorization decision.
7. The complete state machine and response semantics of the current SPPI deployment.
8. Whether automated submission by a third-party client is supported or permitted.

Do not infer an API from browser-visible network traffic alone without checking authorization, terms of use and whether the endpoint is stable and intended for external clients.

## 4. Recommended BlueSky PRO architecture

### 4.1 Separate data ingestion from submission

Use independent interfaces rather than a single “SPPI integration” monolith:

- **Aeronautical publication adapter:** obtains authorized CAI/AIP data and records the publication/version.
- **NOTAM adapter:** ingests NOTAM independently, including cancellation/replacement state and effective times.
- **Normalization service:** maps provider records to the canonical BlueSky airspace model while retaining original fields and provenance.
- **Airspace map projection:** renders normalized records but does not become the source of truth.
- **Constraint evaluator:** checks mission geometry against horizontal, vertical and temporal constraints.
- **Regulatory procedure/rules service:** decides which procedure/actions apply; rules must be versioned and tied to jurisdiction.
- **Flight-plan message generator and validator:** produces a message from the mission model using the currently applicable official specification.
- **Submission adapter:** only enabled after an official, permitted interface and authentication procedure are confirmed.
- **Submission lifecycle and evidence store:** records requests, acknowledgements, responses, conditions, timestamps and source references.

### 4.2 Recommended map-layer taxonomy

Keep these as distinct filterable classes rather than merging them into one generic “restricted area” overlay:

1. Permanent prohibited/restricted/danger areas.
2. Published UAV flight zones and regional UAV restrictions.
3. NOTAM-defined temporary restrictions.
4. Temporary/local regimes and reservations.
5. Aerodrome and controlled-airspace constraints, where relevant.
6. Data-quality and freshness status.

Filtering must not delete source records from the safety/validation model. The map may declutter presentation, but the route validator must evaluate all applicable records.

### 4.3 Geometry, vertical reference and validity

Every normalized record should preserve:
- provider/source and original identifier;
- source publication/version and retrieval timestamp;
- geometry and geometry-quality status;
- effective start/end and time zone/UTC interpretation;
- lower and upper vertical limits with original notation, units and datum/reference;
- cancellation, replacement and current-status information;
- normalization schema/version and any conversion uncertainty.

Do not silently convert an unknown vertical reference, replace missing limits with zero, or invent a footprint when geometry is absent. Where a source boundary cannot be converted to the profile datum, show the original source datum and uncertainty. A rendered zone is not, by itself, authorization to fly.

### 4.4 Flight-plan lifecycle

Use an internal, provider-neutral state model and map external statuses into it only when the real response contract is known. A suggested internal model is:

`DRAFT → VALIDATED → READY_FOR_SUBMISSION → SUBMITTED → RECEIVED → UNDER_REVIEW → ACCEPTED/CONDITIONAL/REJECTED`

Also model amendment, cancellation, expiry and closure where applicable. These are **BlueSky design recommendations**, not a claim that SPPI uses these exact labels. Submission receipt must not be equated with permission or authorization.

## 5. Verification plan before any live connector

1. Manually inspect the current SPPI UI and record the visible layer names, legend, zone attributes, timestamps and source references.
2. With authorized access, use browser developer tools to record network requests for map styles/tiles and restriction data; redact session tokens and personal data.
3. Determine whether endpoints are documented and intended for third-party use; ask the SPPI operator/CAI for a formal API specification and access terms.
4. Compare selected SPPI records with the corresponding CAI/AIP/NOTAM source publication and verify geometry, altitude limits, validity and update behavior.
5. Capture representative cases: permanent zone, time-bounded NOTAM, cancelled/replaced NOTAM, unknown vertical limit, missing geometry and stale data.
6. Confirm the legal procedure and exact FPL/message format against current official documents before building a submission adapter.
7. Implement first against deterministic Virtual Flight fixtures. Do not submit live plans until the interface, authorization and operational/legal controls are confirmed.

## 6. Official references

1. Ministry of Transport — FAQ, page 15: https://mintrans.gov.ru/appeals/faq?page=15  
   Public guidance on UAV restriction zones and the SPPI “Проверка БВС” / “Слои” workflow.
2. Ministry of Transport — FAQ, page 40: https://mintrans.gov.ru/appeals/faq?page=40  
   Public guidance on flight-plan submission to ЕС ОрВД and SPPI workflows.
3. Rosaviatsia — UAV/SPPI instruction post: https://t.me/s/favt_drone/233?q=%23%D0%A1%D0%9F%D0%9F%D0%98  
   Publicly available description of map-based restriction checks and flight-plan workflow.
4. Rosaviatsia — related UAV restriction-check post: https://t.me/s/favt_drone/388  
   Public description of the preflight check.
5. CAI — aeronautical information map and source index: https://aip.caica.ru/map_sait/?lang=ru  
   Index of aeronautical publications and map/data resources.
6. CAI — map database downloads: https://aip.caica.ru/dnlddb_map/?lang=ru  
   Public aeronautical database and publication download entry point.
7. CAI — UAV flight-zone documents: https://post.caica.ru/DocNorm/ZonesFlight/?lang=en  
   Public document listing for UAV flight zones.
8. Rostransnadzor — regulatory context: https://rostransnadzor.gov.ru/news/16701  
   Additional official context on airspace boundaries and aeronautical information.

These references establish public procedures and publication channels. They do not disclose or verify SPPI's private/internal API implementation.

## 7. Decision

**Adopt the SPPI workflow as a product/process reference, not as a presumed technical integration endpoint.** For BlueSky PRO, first complete source-of-record ingestion, normalization, temporal/vertical constraint validation and traceable FPL generation. Investigate a live SPPI submission adapter only after its supported interface, legal conditions, authentication and response semantics are confirmed.

**Implementation status:** this document records research and architecture recommendations only. It does not implement an SPPI connection, certify regulatory compliance, or claim live access to SPPI/CAI services.
