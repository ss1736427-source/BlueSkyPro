---
id: WEATHER-SOURCE-PROVIDER-ARCHITECTURE-001
type: architecture
status: BASELINE_FOR_IMPLEMENTATION
version: 1.0
---

# BlueSky PRO — Weather Source Provider Architecture 001

## 1. Purpose

BlueSky PRO shall support multiple interchangeable meteorological data providers. The system must not depend on one vendor, one protocol, one commercial model or one geographic source.

Free public sources are initial adapters only. The architecture must also support paid APIs, national meteorological services, regional providers, private enterprise endpoints, customer-hosted servers and future in-house forecast services.

Provider-specific logic must remain outside the mission model, weather calculation engine, route optimizer and HMI business rules.

## 2. Architectural principles

1. **Provider neutrality.** Consumers request normalized weather products; they do not call provider APIs directly.
2. **Capability-based selection.** A provider is eligible only for the requested product, geography, altitude, time interval, resolution and quality requirements.
3. **Replaceability.** A provider can be disabled or replaced without changing mission or route data.
4. **Multiple simultaneous sources.** Different products and regions may use different providers. One provider need not supply every product.
5. **Fail-closed operation.** Missing, stale, invalid, unverified or conflicting critical data must remain visible and must not be silently substituted or extrapolated.
6. **Provenance.** Every normalized value retains provider, product, issue time, valid time, retrieval time, revision and raw-payload identity.
7. **Cost awareness.** Requests are mission-scoped, deduplicated and cache-aware. Paid-source quotas and cost limits are first-class configuration.
8. **Security.** Credentials are stored in the system credential vault. Configuration files and mission exports contain secret references only, never secret values.
9. **Auditable administration.** Provider changes, priority changes, credentials rotation, test outcomes and enable/disable actions are versioned and audited.
10. **No implicit blending.** Data from different providers or model cycles are not merged unless an explicit product-specific fusion policy is configured and verified.

## 3. Logical architecture

```text
ADMINISTRATOR / EXTERNAL CONNECTIONS / WEATHER
                     |
             Provider Registry
                     |
        Provider Configuration + Vault
                     |
       Capability / Coverage / Cost Rules
                     |
       Provider Adapter Interface (versioned)
          /           |             \
    Free APIs     Paid APIs     Private services
          \           |             /
             Raw Product Acquisition
                     |
          Parse / Decode / Normalize
                     |
       Provenance + Quality + Validation
                     |
           Weather Product Repository
                + Cache
                     |
       Mission-scoped Product Resolver
                     |
            Weather Assessment
                     |
        Algorithm Orchestrator (weather-aware
        candidate ranking + post-wind NOTAM gate)
```

## 3.1 Current implementation boundary

The current HMI acquisition path is a built-in NOAA/NCEP NOMADS GFS 0.25-degree GRIB2 integration. It requests mission/candidate-scoped regional U/V subsets and explicit AGL levels, checks directory-listing availability with a short-lived 10-minute cache, reuses immutable forecast products only when request identity, valid time, byte count and SHA-256 match the manifest, and decodes one coherent run for the candidate set.

The generic `WeatherProviderRegistry` currently validates adapter descriptors and provider capability declarations. It is not yet the runtime acquisition interface used by the HMI: the active HMI download/availability services still call NOMADS directly. A provider-neutral acquisition API, adapter implementations for additional sources, canonical product repository and administrator configuration workspace remain future integration work. Do not describe the multi-provider architecture as operationally complete.

## 4. Provider and adapter model

Separate three entities:

- **Provider definition:** vendor/service identity, documentation, adapter type and supported capabilities.
- **Provider instance:** one configured endpoint/account/region with its own credentials, limits and lifecycle.
- **Adapter:** executable implementation of a versioned interface that communicates with the provider and converts its response to canonical BlueSky weather products.

A provider may have multiple instances (for example, production and test endpoints, separate regional accounts or separate credentials).

Minimum adapter operations:

- describe adapter identity and interface version;
- report supported product types, formats, geographic coverage, altitude levels, time horizons and resolution;
- validate non-secret configuration;
- test authentication and service reachability;
- query product availability;
- acquire a bounded request for area, time interval and required levels;
- cancel an in-flight request;
- return raw payload metadata and normalized product records;
- report structured errors, rate-limit information and retry guidance.

The adapter must not make flight release decisions, modify missions or optimize routes.

### 4.1 Extension mechanism

Initial adapters may be built-in modules. The architecture must permit future separately versioned extensions without changing consumer interfaces.

For third-party extensions, require signed packages, declared permissions/capabilities, compatibility checks and isolated execution (prefer a separate process/service boundary). Do not load arbitrary unsigned native libraries into the HMI process.

An adapter with an incompatible interface version or unverified signature is not eligible for activation.

## 5. Canonical weather product model

All providers normalize into typed product envelopes. A common envelope shall contain:

- product_id and product_type;
- provider_id, provider_instance_id and adapter_id/version;
- source station/grid/model identifiers;
- issue_time_utc, observation_time_utc where applicable, valid_from_utc and valid_to_utc;
- retrieved_at_utc;
- geographic footprint / geometry / grid definition;
- altitude reference, level and units;
- parameter values and quality flags;
- source revision / correction / cancellation relationship;
- raw payload URI or controlled cache key, payload hash and format;
- parse status, validation status and provenance record.

Product-specific schemas remain distinct: METAR/SPECI, TAF, SIGMET/AIRMET, GAMET, SIGWX, upper wind/temperature, numerical model fields and derived route weather. Do not flatten unlike products into a single generic observation record.

Corrections and cancellations must preserve history. A corrected product supersedes the identified earlier product; cancellation is represented as a tombstone/event, not deletion of the audit record.

## 6. Provider capability and eligibility contract

A provider instance declares capabilities for each product:

- product types and supported message variants;
- supported geography and coverage exclusions;
- spatial resolution / grid spacing;
- supported altitude coordinate systems and levels;
- forecast horizon and temporal resolution;
- update cadence and expected publication latency;
- maximum request area, payload size and request rate;
- authentication mode and required credential scopes;
- terms/license classification and permitted operational use;
- estimated request/data cost and quota limits.

The resolver must test the requested mission against these capabilities before scheduling a request. A provider that cannot cover the complete required interval or region is not silently treated as complete coverage.

## 7. Selection, priority and fallback

Priority is configured per product family and optionally per geographic region, not as one global provider ranking.

Selection order:

1. filter enabled instances by product capability;
2. filter by geography, time interval, altitude and resolution;
3. check terms/license and account/quota state;
4. check service health and verified cache;
5. apply administrator priority and quality policy;
6. acquire missing products only.

Fallback is permitted only when the alternate provider supplies a semantically compatible product and meets the same minimum coverage and quality contract. The system records the reason for fallback and the selected provider. It must not silently replace an aviation observation with a numerical forecast or a route-area forecast with an aerodrome report.

If two eligible sources materially disagree, record the conflict and apply a product-specific conflict policy. Critical unresolved conflicts block operational readiness until reviewed.

## 8. Administrator workspace

Location: **ADMINISTRATOR → EXTERNAL CONNECTIONS → WEATHER**.

Required pages/tabs:

### 8.1 Source catalog
- configured provider instances;
- provider type and adapter version;
- products supplied;
- geography and altitude coverage;
- enabled/disabled state;
- connection and data-quality status;
- primary / secondary / standby role per product;
- add, edit, duplicate profile, disable and retire actions.

### 8.2 Add provider / adapter
- select a built-in adapter or install an approved extension;
- select service environment (test / production);
- configure endpoint and protocol;
- configure authentication method;
- enter credentials through a protected secret-entry flow;
- declare region and products;
- validate capabilities and test connection;
- save as disabled until validation passes and an administrator activates it.

### 8.3 Product routing
For each product family, configure preferred provider, fallback sequence, region scope, minimum quality, maximum age where applicable, required coverage and conflict handling.

### 8.4 Coverage and quality
Show last successful retrieval, latest issue/observation time, valid interval, coverage footprint, missing regions/levels, decode/parse status, QC status, source provenance and unresolved conflicts.

### 8.5 Cost and quota
Show plan/tariff label, request quota, data quota, configured spending ceiling, current period usage, estimated cost and threshold alerts. Do not assume a free tier or infer a price from request count; use provider-reported or administrator-entered tariff metadata.

### 8.6 Credentials
Store secrets in CredentialVault or an approved external secrets manager. The UI may show credential presence, scope, last rotation and expiry, but never read back or display secret material. Support rotation and revocation. Audit secret changes without logging secret values.

### 8.7 Diagnostics and journal
Show connection tests, response latency, HTTP/provider errors, rate limiting, retries, cache hits, downloads, parsing failures, adapter version and administrator changes.

## 9. Configuration and persistence model

Recommended logical records:

```text
weather_provider_definition
  id, vendor, service_name, adapter_id, adapter_version,
  interface_version, package_signature, documentation_uri

weather_provider_instance
  id, definition_id, display_name, environment, endpoint,
  enabled, lifecycle_state, config_revision, created_at, updated_at

weather_provider_capability
  provider_instance_id, product_type, geography, altitude_levels,
  spatial_resolution, temporal_resolution, forecast_horizon,
  max_area, max_payload, rate_limit, license_class

weather_provider_credential_ref
  provider_instance_id, credential_ref, auth_type, scopes,
  expires_at, last_rotated_at

weather_product_route
  product_type, region_scope, priority, provider_instance_id,
  fallback_rank, min_quality, conflict_policy, enabled

weather_provider_quota
  provider_instance_id, period, request_limit, data_limit,
  spending_limit, tariff_reference, alert_threshold

weather_provider_health
  provider_instance_id, checked_at, connection_state,
  latency_ms, last_success_at, last_error_code

weather_provider_audit
  actor, action, object_id, old_revision, new_revision,
  reason, timestamp_utc
```

Use revisioned configuration and atomic updates. The storage implementation may use the existing system configuration store, provided it supports transactional writes, access control, revision history and audit records.

## 10. Security and authorization

- Only authorized Administrator roles may add providers, edit routing, activate adapters or change spending limits.
- Credential rotation may be separately delegated to a security administrator.
- Engineers may inspect capabilities and diagnostics according to assigned permissions, but must not automatically receive credential access.
- Pilots/operators see source status and data provenance relevant to their mission, not administrative secrets or account billing details.
- All network endpoints require TLS by default. Plain HTTP is disabled except explicitly approved isolated test environments.
- Prevent endpoint abuse: validate schemes/hosts, restrict redirects, bound response size/time, and protect against SSRF and untrusted adapter payloads.
- Never include credentials in logs, crash reports, source control, mission exports or diagnostics.

## 11. Runtime request lifecycle

A mission weather request is formed from the approved mission area, route geometry, execution interval and required altitude levels.

1. Resolve required product families.
2. Query provider registry and select eligible instances.
3. Check verified cache and deduplicate identical in-flight requests.
4. Request only missing products and required spatial/temporal subsets.
5. Validate payload integrity, parse/decode and normalize.
6. attach provenance, quality and applicability metadata.
7. Persist immutable product versions and cache manifests.
8. Return the resolved dataset to Weather Assessment.
9. Trigger re-evaluation only when a relevant dependency changes.
10. Send material changes to a separately verified weather-aware optimizer integration.

Map pan/zoom must not trigger new acquisition. Cache TTL for availability metadata is not forecast validity. Product validity is determined by each product's own time and coverage semantics.

## 12. Relationship to existing weather implementation

Existing components that should be retained and placed behind this architecture:

- `GfsForecastAvailabilityService`: NOAA NOMADS availability discovery and listing cache.
- `WeatherDataService`: bounded GFS GRIB2 subset acquisition and immutable-file cache/manifest.
- `grib2_decoder` / `grib2_weather_pipeline`: GRIB2 decoding and weather field processing.
- `WeatherApplicabilityPolicy`: METAR/SPECI/TAF applicability rules.
- `WeatherCalculationService`: route weather assessment.
- `WeatherCoverageBridge`: mission weather coverage request boundary.

These components are not yet a complete provider registry. The GFS services currently contain NOAA-specific endpoints and request semantics. The next implementation step is to introduce the provider-neutral interface and wrap the existing NOAA path as the first built-in adapter without changing its validated behavior.

METAR/SPECI/TAF ingestion remains a separate adapter and parser implementation. Do not mark these products operational until live retrieval, parsing, correction/cancellation handling, provenance, cache and tests are complete.

## 12.1 Current NOAA GFS vertical-coverage boundary

The current NOAA GFS 0.25-degree GRIB filter exposes discrete wind levels at 2, 10, 20, 30, 40, 50, 80, 100, 1000 and 4000 metres above ground. The BlueSky request builder and decoder must use only levels confirmed by the live provider inventory.

The current interpolation contract permits vertical interpolation only between bracketing AGL levels separated by no more than 100 m. Therefore:
- low-level requests such as 5 m AGL can be bracketed by 2 m and 10 m;
- requests between 100 m and 1000 m are not covered by this GFS AGL-level set and must be blocked unless an alternate source supplies suitable levels;
- do not widen the 100 m interpolation limit merely to make a request pass;
- pressure-level wind may be used in a future adapter only after geopotential-height conversion and terrain/DEM-based AGL conversion are implemented and validated.

The provider registry must declare discrete levels and its maximum vertical interpolation gap. A continuous minimum/maximum altitude range is not sufficient evidence of vertical coverage.

## 12.2 Route-weather acquisition and assessment boundary

The HMI weather request is formed only after the canonical route candidate set has been validated. The initial time window covers the longest candidate route using the selected, validated and approved UAV performance profile, climb/descent rates and the configured operational reserve (currently 30 minutes). This reserve is a BlueSky planning margin, not a METAR/TAF regulatory validity period.

For GFS 0.25-degree subsetting, the acquisition bounds are built from the union of the candidate-route coverage and expanded by one grid interval around the mission's configured spatial buffer. This retains bracketing grid cells for bilinear interpolation near route and area boundaries. Map pan/zoom does not trigger acquisition.

The current weather trajectory engine:
1. verifies that forecast files belong to one model run and match their manifest identity, byte size and SHA-256;
2. decodes paired U/V fields into one immutable wind snapshot;
3. samples the complete trajectory at no more than 10 km or 5 minutes per subsegment;
4. interpolates only inside horizontal-grid, vertical-level and forecast-time coverage;
5. iterates wind-dependent segment times and energy until convergence, or blocks the assessment.

This first GFS path currently decodes U/V wind only. It assesses wind impact on route segment speed, time and energy; it is not a complete meteorological hazard assessment and does not yet cover visibility, cloud base, precipitation, icing, turbulence, temperature limits or aviation hazard products. The HMI therefore labels the result as a wind assessment, not a full weather clearance.

The application now has a multi-candidate weather reranking path:
1. `PlanningBridge` publishes the external planning result. When it contains `routeCandidates`, `MainContent.syncPlanningRouteCandidate` passes the set to `RouteController.loadCandidateSet`.
2. The route controller accepts only candidates with complete canonical geometry, common mission/calculation lineage, route validation and an explicit ALLOWED flight-safety result for the assigned UAV.
3. `MissionProfileWindow` forms weather coverage from the entire candidate set and calls `WeatherCalculationService.evaluateCandidates`.
4. The service verifies and decodes the downloaded forecast products, constructs `makeCandidateWeatherAssessor`, and runs the candidate set through `AlgorithmOrchestrator` using a `CandidateSetSolver`.
5. Wind-corrected time, energy and remaining energy replace nominal metrics before objective ranking. The final time-dependent NOTAM gate is applied to the wind-corrected 4D trajectory. If the selected candidate changes, the HMI restores that candidate through `RouteController.selectCandidate`.

This is a second-stage optimization over the complete candidate set produced by the first planning stage. It follows the required sequence — generate routes, account for wind, compare corrected candidates, select the best admissible route — without passing a C++ callback across the JSON process boundary. It does not generate new route geometry after weather assessment. If every supplied candidate is rejected, the solve remains blocked; it must not fall back to nominal metrics or an unverified route.

The canonical route waypoint supports optional task execution duration and energy. Both values must be supplied together from validated mission/aircraft/equipment data; the trajectory calculator adds them to elapsed time and energy, and later forecast samples are shifted by the dwell duration. The mission/task model and HMI route-snapshot producer do not yet populate these fields from mission templates, so operational task dwell is supported by the calculation contract but is not yet automatically supplied end-to-end.

## 12.3 Weather-aware candidate contract

The planning orchestrator exposes an optional candidate weather assessor. When configured, it evaluates each structurally valid candidate before objective ranking and replaces nominal time, energy and remaining-energy values with the assessor's verified weather-corrected metrics. A missing, invalid or non-converged assessment blocks that candidate; the optimizer must not rank it using nominal estimates as a fallback.

The candidate contract carries an optional canonical `trajectory_route`. Weather assessment is permitted only when this route is marked `trajectory_geometry_complete` and contains a consistent waypoint/segment chain. The route must represent the complete flown trajectory, including transitions and turns, with explicit AGL heights. The assessor result must include the immutable forecast snapshot identity/version and finite corrected time, energy and remaining-energy values.

The HMI candidate-set contract accepts 1–64 candidates. Every candidate must have unique candidate and route IDs, complete WGS84 geometry, consistent mission/calculation lineage, explicit UAV/safety-envelope identity, route validation and ALLOWED flight-safety validation. Missing geometry is not inferred from a graph-node polyline.

**Known route-generation boundary:** the generic A*/Dijkstra graph solvers still publish node-ID paths only; they do not themselves populate complete canonical flown geometry. Those raw graph candidates cannot be sent directly to the weather assessor. The upstream route-producing stage must publish canonical route candidates with all legs, transitions and turns. The HMI route controller and Python planning bridge validate this contract, but the production source of every candidate geometry still requires operational verification.

A production bridge, `makeCandidateWeatherAssessor`, accepts decoded forecast fields, the assigned UAV performance profile and snapshot identity, then invokes `WeatherTrajectoryCalculator` for each candidate.

A new engine-level integration test, `weather_candidate_reranking_integration_test`, uses the real weather trajectory calculator, candidate assessor and `AlgorithmOrchestrator` with two complete synthetic routes. It verifies that an east-favourable forecast selects the eastbound candidate and a north-favourable forecast selects the northbound candidate. This proves that forecast changes can change the selected candidate in the weather-ranking engine. It does not, by itself, prove live NOAA acquisition, the external planner's route-generation quality or the complete QML/PlanningBridge runtime path.

## 12.4 HMI / planning-process integration boundary

The current application has two connected process stages:

- `PlanningBridge` starts the external planning executable through `QProcess` and exchanges newline-delimited JSON. The planning result may carry one `routeCandidate` or a `routeCandidates` set.
- `MainContent` loads the validated candidate set into the shared `RouteController`. `MissionProfileWindow` then acquires the mission/candidate-scoped forecast and runs `WeatherCalculationService.evaluateCandidates` in the HMI process.

The candidate assessor is a C++ callback (`std::function`) owned by `AlgorithmOrchestrator`; it is not serialized through the JSON process boundary. In the current design this is intentional: the first planning stage supplies canonical candidate routes, and the HMI weather stage performs wind correction and ranking over that set. The HMI is not merely assessing the highlighted route; it can select a different candidate from the supplied set.

This separation does not mean that the external planning executable itself performs weather-aware search. The weather stage cannot change the first-stage search graph or invent a new candidate after the forecast is applied. The external planning stage must provide a sufficiently diverse, fully validated candidate set. If future requirements demand weather-driven route generation beyond reranking, a versioned planning-process contract and an explicit regeneration loop will be required; a callable C++ object must not be passed through JSON.

**Remaining end-to-end verification:** add a test that exercises the actual PlanningBridge result contract through candidate-set loading, weather acquisition/decoding, candidate reranking and HMI selection. It must prove that changing the forecast snapshot can change the selected route, that missing/stale or mismatched forecast products block selection, that post-wind NOTAM validation uses the same snapshot and mission window, and that the operator receives snapshot identity plus corrected metrics. The current engine-level test covers the wind calculation/ranking core, not this complete live HMI-to-planning runtime path. Until the complete runtime scenario passes, the UI must continue to describe the function as wind assessment, not full weather clearance.

## 13. Implementation sequence

1. Define canonical product envelope, capability manifest and versioned adapter interface.
2. Add provider registry/configuration storage, revisioning and audit.
3. Integrate credential references with CredentialVault; no plaintext secrets.
4. Wrap current NOAA GFS availability and GRIB2 download services as the first adapter.
5. Add Administrator Weather source catalog, source details, status and product routing UI.
6. Add AviationWeather.gov as a separate METAR/SPECI/TAF adapter; implement parser and event handling.
7. Add further public, national, commercial and customer-hosted adapters independently.
8. Add tests for capability filtering, provider priority, fallback, stale data, conflicting reports, cancellation, quotas, credential isolation, adapter compatibility and cache reuse.
9. Verify weather assessment and weather-aware optimizer handoff independently.

## 14. Acceptance criteria

- A new provider can be configured without changing mission schema, route optimizer or weather calculation consumers.
- Multiple providers can be active simultaneously and selected per product/region.
- Disabling or replacing one provider does not invalidate already acquired, provenance-complete immutable products.
- No secret appears in configuration exports, logs or mission packages.
- Provider selection is deterministic and explainable.
- Fallback never weakens required product semantics, time coverage, spatial coverage or quality thresholds.
- Paid-source quotas and spending limits can prevent further acquisition before a configured limit is exceeded.
- Every provider action and configuration revision is auditable.
- Existing NOAA GFS behavior remains regression-tested while moved behind the adapter boundary.
