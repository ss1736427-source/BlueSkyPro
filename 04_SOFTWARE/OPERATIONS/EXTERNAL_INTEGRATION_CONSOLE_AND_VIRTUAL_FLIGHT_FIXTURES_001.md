# External Integration Console and Virtual Flight Test Fixtures

**Status:** HMI prototype implemented; live adapters and planning-kernel binding not implemented  
**Updated:** 2026-10-19  
**Canonical branch:** `main`

## 1. Administrator integration console

The ADMIN workspace contains a dedicated connector catalogue. It is separate from Virtual Flight and is the intended place to inspect and later configure external-system integrations.

The current HMI catalogue covers:

1. NOTAM / AIP
2. Weather / meteorology (METAR, SPECI, TAF, GAMET, SIGWX, SIGMET/AIRMET, wind and hazards)
3. Airspace and restrictions
4. Terrain / DEM
5. Obstacles
6. Traffic / UTM
7. ATM / flight-plan exchange / ЕС ОрВД
8. Authorization / permits
9. GIS / map provider (MapTiler)
10. GNSS / RTK / PPK
11. UAV / FCS / MAVLink
12. C2 / data links
13. Payload / camera / video
14. Identity / access / security
15. Insurance
16. Flight Record / logs
17. Synchronization / external APIs
18. Alerts / operational events

Every catalogue entry currently reports **LIVE: DISCONNECTED**. The TEST STUBS control is stored in local application settings and is shared between ADMIN and VIRTUAL FLT; it does not connect a provider or configure credentials. Endpoint, authentication, licensing, protocol, geographic coverage, update cadence and freshness limits remain unconfigured.

## 2. Virtual Flight test console

The VIRTUAL FLT workspace provides selectable mock scenarios for exercising the HMI data path. Current scenario profiles contain illustrative values for mean wind, gust, temperature, visibility, cloud base and the count of synthetic NOTAM test records.

**Data qualification:** these values are demonstration fixtures, not measurements downloaded from an identified station or a verified historical climatology dataset. They are not tied to a geographic location or date range. They must not be described as authoritative historical weather.

The current `SELECT TEST PROFILE` action only selects a UI fixture. It does not execute the planning kernel, generate a route, assess a real NOTAM snapshot or produce a safety verdict.

## 3. Requirements for the next test-data adapter

Before replacing demonstration values with averaged historical data, define the dataset profile:

- location or area and coordinates;
- averaging period and season/month grouping;
- source/provider and licence;
- station or grid identifier and spatial resolution;
- units, time zone, timestamps and missing-data policy;
- mean and percentile/extreme statistics to expose (mean values alone do not represent operational hazards);
- provenance, retrieval time, dataset version and transformation version;
- coverage/quality flags and explicit synthetic-vs-historical classification.

Historical averages are useful for repeatable performance and interface tests, but they do not establish current flight weather. Current NOTAMs, active airspace restrictions, current wind/weather, terrain/obstacle coverage, aircraft/battery inputs and all required authorization checks remain separate dependencies.

## 4. Safety boundary

- Test fixtures are namespaced and visibly labelled as non-operational.
- Fixture presence must never imply live connectivity, authoritative source validity, regulatory permission or flight-release eligibility.
- The production release path must reject test-profile provenance, stale or incomplete source snapshots, unknown altitude references and unresolved regulatory blockers.
- The planning and safety core remains authoritative; QML must not calculate or infer release status.
- The Admin console is a catalogue/configuration shell until provider adapters and secure credential handling are implemented.
- No live NOTAM or weather feed is connected by this change.

## 5. Current implementation checkpoint — 2026-10-19

- `MainContent.qml` stores the shared TEST STUBS switch under `BlueSkyPRO/ExternalIntegrations`; the ADMIN action writes to the settings property rather than breaking the binding to the displayed state.
- Virtual Flight scenarios now use a structured, versioned fixture record (`VF-FIXTURES-1.0`) with numeric weather values, profile ID, `SYNTHETIC_AVERAGE` classification, and explicit `UNSPECIFIED` location/averaging period. UI labels say `SYNTHETIC AVERAGE`, not `HISTORICAL AVERAGE`, because no identified historical dataset, station/grid, averaging period or licence is configured.
- The HMI fixture selection remains presentation-only. Planning-kernel binding, weather snapshot construction, georeferenced NOTAM fixture geometry and an end-to-end test request are still not implemented.
- The CMake include-path correction and CI for `daabb8e` passed. CI for the newer toggle, labels, structured fixture and this document update must be checked against their exact SHAs before closure.

## 6. Acceptance criteria for production integration

For each provider, record and verify connection state, authentication, protocol/schema version, provider identity, geographic/time coverage, source timestamp, retrieval timestamp, freshness policy, completeness, integrity, error/retry state, last successful synchronization and audit trail. Exercise timeout, malformed response, partial coverage, stale data, cancellation/replacement, offline startup and recovery. A successful transport exchange is not equivalent to valid data or authorization.
