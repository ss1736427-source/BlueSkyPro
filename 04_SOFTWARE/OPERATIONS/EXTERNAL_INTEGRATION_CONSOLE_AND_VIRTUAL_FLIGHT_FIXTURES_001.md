# External Integration Console and Virtual Flight Test Fixtures

**Status:** ADMIN has initial live WX and point-elevation HTTP queries; NOTAM machine feed remains unconnected  
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

ADMIN no longer exposes the TEST STUBS switch as its integration mechanism. It now provides live HTTP queries to Open-Meteo for current weather and point elevation, using operator-entered WGS84 coordinates. This is an initial connectivity probe, not a production adapter: credentials are not required for the public prototype endpoint, but licence, rate limits, provenance, source freshness, schema and commercial-use conditions still require policy enforcement. The NOTAM card links to the official Russian CAI portal but explicitly reports that no machine-readable feed is connected.

## 2. Virtual Flight test console

The VIRTUAL FLT workspace provides selectable mock scenarios for exercising the HMI data path. Current scenario profiles contain illustrative values for mean wind, gust, temperature, visibility, cloud base and the count of synthetic NOTAM test records.

**Data qualification:** these values are demonstration fixtures, not measurements downloaded from an identified station or a verified historical climatology dataset. They are not tied to a geographic location or date range. They must not be described as authoritative historical weather.

The current `SELECT TEST PROFILE` action selects a versioned synthetic fixture for repeatable Virtual Flight testing. It must remain isolated from live provider results and must not imply a current operational weather/NOTAM assessment.

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
- Live weather HTTP queries are available for prototype testing; they are not yet an operational weather service or flight-release input.
- Point elevation can be queried live; it is not a terrain/obstacle profile and is not sufficient for route clearance.
- NOTAM remains unavailable to the automated planning path until an authorized machine-readable source and access terms are confirmed.

## 5. Current implementation checkpoint — 2026-10-19

- `MainContent.qml` stores the shared TEST STUBS switch under `BlueSkyPRO/ExternalIntegrations`; the ADMIN action writes to the settings property rather than breaking the binding to the displayed state.
- Virtual Flight scenarios now use a structured, versioned fixture record (`VF-FIXTURES-1.0`) with numeric weather values, profile ID, `SYNTHETIC_AVERAGE` classification, and explicit `UNSPECIFIED` location/averaging period. UI labels say `SYNTHETIC AVERAGE`, not `HISTORICAL AVERAGE`, because no identified historical dataset, station/grid, averaging period or licence is configured.
- ADMIN live WX/point-elevation requests are now implemented in QML; results remain in the console and are not yet converted into canonical typed source snapshots for the planning core.
- NOTAM/AIP is not connected. The official CAI page is linked for operator access; no HTML scraping or false LIVE status is used.
- Route-based sampling, terrain-profile generation, source freshness/coverage validation, secure provider configuration and end-to-end planning-core consumption remain unimplemented.
- The CMake include-path correction and CI for `daabb8e` passed. CI for the newer toggle, labels, structured fixture and this document update must be checked against their exact SHAs before closure.

## 6. Acceptance criteria for production integration

For each provider, record and verify connection state, authentication, protocol/schema version, provider identity, geographic/time coverage, source timestamp, retrieval timestamp, freshness policy, completeness, integrity, error/retry state, last successful synchronization and audit trail. Exercise timeout, malformed response, partial coverage, stale data, cancellation/replacement, offline startup and recovery. A successful transport exchange is not equivalent to valid data or authorization.


## 7. Initial live-source implementation — 2026-10-19

- `qt/BlueSkyPRO-HMI/qml/IntegrationConsole.qml` exposes WGS84 latitude/longitude inputs and explicit actions to query:
  - Open-Meteo forecast endpoint for current model snapshot (timestamp, temperature, wind speed/direction/gust, cloud cover, humidity and pressure);
  - Open-Meteo Elevation API for point elevation.
- HTTP status and JSON validation errors are displayed. A transport success is not represented as flight readiness.
- The public Open-Meteo endpoint is suitable only for non-commercial prototyping under its published terms; production commercial use requires an appropriate commercial licence or a separately operated deployment.
- Official Russian NOTAM portal: https://aip.caica.ru/notam/. This is a source reference only; automated feed access, machine-readable format, authentication and reuse rights are not verified. The console must continue to report NOTAM as unavailable and keep operational release blocked.
- No user PC drive was accessed. Changes were made directly to canonical `main`; no branch or PR was created.


## 8. Provider connection nodes — 2026-10-19

The ADMIN integration console now includes provider configuration nodes for:

- **NOTAM / NOTAC REST JSON** — editable API base URL (default `https://notac.aero/api/v1`), masked token field, and an explicit connection test. The test sends `GET /notam/?country_code=US&status=active&page=1` with `Authorization: Bearer <token>`, validates the JSON envelope (`count`, `results`) and reports transport/auth/quota/rate-limit/schema status. The US query is only a connectivity fixture; it does not validate Russian NOTAM coverage.
- **ICAO NOTAM API** — masked key-entry field and setup note. No request is sent yet because endpoint parameters, credentials and data coverage must first be confirmed.
- **MapTiler** — masked key-entry field. This is a configuration placeholder; it is not yet bound to the map renderer or the existing map-provider setting.
- **Commercial weather provider** — masked key-entry field for future use. The current Open-Meteo query does not consume this field.

### Operator setup flow

1. Request a NOTAC API token from the provider using its official API documentation/contact channel.
2. Open **ADMIN → External Integrations → Provider Connection Nodes → NOTAM / NOTAC REST JSON**.
3. Verify the API base URL and paste the token into the masked **API TOKEN** field.
4. Select **TEST NOTAM API CONNECTION**.
5. Read the status and HTTP error. A successful test confirms only network/authentication/JSON-envelope handling against the US test query.
6. Use SAVE KEY to store the secret, then TEST API. A saved key is not shown back in the field; the NOTAC test retrieves it from the vault when the field is empty. Use DELETE KEY to remove the saved value.

### Security and implementation boundary

- qt/BlueSkyPRO-HMI/src/CredentialVault.cpp implements a native Windows Credential Manager adapter. It stores per-provider generic credentials for the current Windows user; it does not write secrets to QSettings or repository files.
- Provider identifiers are allow-listed (notam, icao, maptiler, weather). Empty values and unknown identifiers are rejected.
- The HMI exposes saved/not-saved status, save and delete actions. The UI clears the key field after successful save. NOTAC reads the saved key only when the input field is empty.
- MapTiler is loaded from the vault at application startup only when BLUESKY_MAPTILER_API_KEY is not supplied in the environment. The environment value takes precedence.
- Non-Windows platforms report native vault support as unavailable and refuse to save; there is no plaintext fallback. macOS Keychain and Linux Secret Service adapters remain future work.
- Credential vault support is not the same as provider activation: ICAO and commercial-weather keys are stored but not used by live adapters yet.
- The NOTAC key is sent only in the HTTPS Authorization header for the explicit test action, never as a query parameter. ICAO and commercial-weather keys are stored but not currently sent to a provider. The MapTiler key is passed to the map renderer at application startup when no environment-variable key is supplied; the renderer may include it in provider requests as required by the MapTiler URL contract.
- The current console does not yet provide a general provider activation lifecycle, key rotation, provider selection in the planning core, retry/backoff policy, or normalized source snapshots. Secure persistence exists for the four configured provider key slots, but saving a key does not activate an adapter.
- The NOTAC feed is FAA-sourced. It is used as a temporary integration test only; it must not be treated as a Russian lower-airspace restriction source or used to authorize a flight.
- A successful connection must remain distinct from data validity, geographic completeness, freshness, operational authorization and flight-release eligibility.


## 9. Secure credential persistence implementation — 2026-10-19

- Native bridge: qt/BlueSkyPRO-HMI/src/CredentialVault.h/.cpp, exposed to QML as credentialVault.
- Windows backend: Windows Credential Manager generic credentials, scoped by provider identifier and application namespace. Save/replace, read, existence check and delete operations are provided.
- Credential errors expose only provider/backend status and Windows error numbers; secret values are not logged.
- Automated contract test: qt/BlueSkyPRO-HMI/tests/CredentialVaultTest.cpp. On Windows it exercises save → read → delete using an isolated test namespace; on non-Windows it verifies fail-closed behavior.
- The exact resulting commit must be built and tested on the supported CI matrix before this implementation is considered verified. No local Qt build or interactive restart test was performed during this change.
- Remaining platform gap: macOS Keychain and Linux Secret Service are not implemented. This does not justify writing secrets to plain settings or files.
