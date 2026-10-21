# FAA NMS NOTAM Adapter — Onboarding and Implementation Gate

**Status:** SPECIFICATION RECEIVED; IMPLEMENTATION READY FOR STAGING, CREDENTIALS REQUIRED  
**Scope:** Experimental US NOTAM source for integration testing only  
**Canonical branch:** `main`

## 1. Purpose and boundary

This document records the supplied FAA NMS-API onboarding bundle and the implementation boundary for BlueSky PRO. It does not claim that BlueSky PRO has authenticated to FAA, received live data, or is operationally ready.

FAA NMS is a US source. The supplied bundle does not establish coverage of Russian airspace or suitability for operational decisions under Russian jurisdiction. The adapter must be labelled **FAA NMS — US TEST SOURCE** and remain separate from the Russian NOTAM source decision.

## 2. Supplied onboarding package

The reviewed bundle contains an OpenAPI specification (`nms-api.yaml`), curl examples, a pre-production SoapUI project, FAQ PDF, initial-load examples in JSON/XML, and a checklist example. The spreadsheet was not parsed successfully in the review environment; therefore no credential values were read or verified.

The bundle documents these environments:

| Environment | Base URL |
|---|---|
| FIT | `https://api-fit.cgifederal-aim.com/` |
| Staging / Pre-Production | `https://api-staging.cgifederal-aim.com/` |
| Production | `https://api-nms.aim.faa.gov/` |

Do not use the production host for initial integration tests. Start with the authorized staging environment.

## 3. Authentication contract from the supplied FAQ/examples

The documented token request is a client-credentials exchange. The token endpoint is outside the `/nmsapi` path:

```http
POST https://api-staging.cgifederal-aim.com/v1/auth/token
Content-Type: application/x-www-form-urlencoded
Authorization: Basic <base64(client_id:client_secret)>

grant_type=client_credentials
```

The documented curl equivalent is:

```sh
curl -X POST --location \
  "https://api-staging.cgifederal-aim.com/v1/auth/token" \
  -d grant_type=client_credentials \
  -u "$FAA_NMS_CLIENT_ID:$FAA_NMS_CLIENT_SECRET"
```

The response supplies an `access_token` and `expires_in`; the FAQ's typical example is about 1,799 seconds. The client must renew tokens before expiry and handle `401` by refreshing once before returning an authentication error. An `invalid_client` response points to client credential/configuration failure, not an expired bearer token.

**Security:** never commit, log, or email the client secret or bearer token. Use the native credential vault for the client ID/secret; keep the short-lived access token in memory only. Do not disable TLS certificate validation in production.

## 4. Endpoints documented in the package

All data endpoints below use the environment host plus `/nmsapi` and require `Authorization: Bearer <access_token>`.

| Purpose | Endpoint | Notes |
|---|---|---|
| Connectivity probe | `GET /nmsapi/v1/ping` | First staging smoke test |
| NOTAM query | `GET /nmsapi/v1/notams` | Filterable by location, time, classification, identifiers and other documented fields |
| Checklist | `GET /nmsapi/v1/notams/checklist` | Summary/checklist records |
| Full initial load | `GET /nmsapi/v1/notams/il` | Bulk load; do not poll as a delta feed |
| Initial load by classification | `GET /nmsapi/v1/notams/il/{classification}` | Classification-specific bulk load |
| Initial-load content | `GET /nmsapi/v1/content/{token}` | Fetch content using the token/reference returned by initial-load flow |
| Location series | `GET /nmsapi/v1/locationseries` | Optional location-series data |

For `/v1/notams`, the package specifies the required `nmsResponseFormat` header with `AIXM` or `GEOJSON`. Geographic filters use decimal-degree latitude/longitude and radius in nautical miles. The adapter must follow the OpenAPI schema shipped in the onboarding package rather than infer a response from the sample fixtures.

The signed URL in the sample initial-load JSON is dated sample data and must not be reused as a live endpoint.

## 5. Rate limits and polling policy

The supplied FAQ states:

- Pre-Production: 1 request/second, with content calls allowed at approximately 2 requests/second.
- Production: one data pull every 3 minutes for activity from the preceding 3-minute interval.
- Initial-load bulk pull: at most once per 24 hours unless FAA explicitly approves a different rate.
- More frequent polling may require FAA approval and can trigger rate-limit errors.

Implement a single scheduled poller with a minimum 3-minute production interval, a 3-minute incremental lookback, backoff for `429`/transient failures, and explicit last-success/freshness metadata. Do not run a bulk initial load as a recurring delta query. Respect any stricter rate limits in the issued account agreement.

## 6. BlueSky PRO integration boundary

The current HMI NOTAM test uses the third-party NOTAC REST API and expects a NOTAC-specific JSON envelope. It is not an FAA NMS adapter. Keep the existing NOTAC route/parser separate; do not repurpose it to call FAA.

The FAA client should be implemented in the native C++ layer using Qt Network (or an equivalent existing native service), not by exposing the FAA client secret to QML/JavaScript. The native adapter should:

1. read client credentials through the existing credential-vault bridge;
2. obtain/refresh the bearer token;
3. call staging `ping` and then a bounded NOTAM query;
4. validate HTTP status, content type, required fields and response format;
5. normalize records into the canonical BlueSky NOTAM model while retaining provider ID, classification, issue/update/effective times, cancellation/replacement state, geometry, altitude reference/limits, retrieval time and source provenance;
6. publish explicit states: `NOT_CONFIGURED`, `CONNECTING`, `CONNECTED`, `AUTH_FAILED`, `RATE_LIMITED`, `TIMEOUT`, `INVALID_RESPONSE`, `STALE`, `ERROR`;
7. never treat a failed, stale, partial or malformed response as an empty/clear airspace result.

Prefer GeoJSON only after validating that the returned schema contains adequate geometry and temporal/vertical attributes for the planning core. If AIXM is required for relevant records, implement a versioned parser and test it against the supplied sample before enabling normalization.

## 7. Virtual Flight and safety gate

Virtual Flight remains the default test path until authenticated staging data has been validated. Use deterministic fixture files for successful query, empty result, malformed response, expired/cancelled NOTAM, unsupported geometry/altitude reference, timeout, `401`, `invalid_client`, `429`, and stale/partial data. Label provider records as US test data and keep them separate from synthetic fixtures.

Do not use FAA staging records as authoritative Russian restrictions or as the sole basis for real-mission release. The existing route validator's explicit altitude-reference rules remain in force; records with unsupported or unknown vertical references must not silently pass.

## 8. Verification checklist

- [ ] Staging credentials provisioned and stored in the native credential vault.
- [ ] Token exchange succeeds without logging secrets.
- [ ] Token expiry and one-time refresh after `401` are tested.
- [ ] `/nmsapi/v1/ping` succeeds against staging.
- [ ] A bounded `/nmsapi/v1/notams` query returns a schema-valid response.
- [ ] GeoJSON/AIXM parsing and normalization have deterministic tests.
- [ ] Rate limits, retry/backoff, freshness and source provenance are verified.
- [ ] Admin UI distinguishes transport success from valid NOTAM data.
- [ ] Virtual Flight fixtures remain isolated from provider results.
- [ ] Russian jurisdiction source and coverage are separately established.

**Current blocker:** the onboarding bundle defines the protocol, but this review did not verify that valid staging credentials are available. The credential spreadsheet was unreadable in the review environment. No live connection is claimed. The vault has separate identifiers for `faa_nms_client_id` and `faa_nms_client_secret`, and the ADMIN console now has controls to save/delete them and initiate token-plus-ping testing. These changes do not mean credentials have been entered or validated; CI is still pending.

## 9. Next implementation sequence

1. Store FAA OAuth credentials as two independent Windows Credential Manager entries: `faa_nms_client_id` and `faa_nms_client_secret`. Keep both separate from `faa_nms` and the existing `notam`/NOTAC secret.
2. Add the native client for token exchange and `ping`; expose status and a credential/test control in the ADMIN integration console. This is a connectivity smoke test only, not NOTAM ingestion.
3. Confirm the HMI build and add deterministic fixture-backed tests for auth/error handling before live requests.
4. Add the bounded NOTAM query, response validation and canonical normalization.
5. Connect validated provider records to the ADMIN console with source, status, last successful retrieval and freshness.
6. Keep Virtual Flight as the operationally isolated test path until the complete verification checklist is met.


## 10. Implementation status — 2026-10-21

The current code now includes a native `FaaNmsClient` and a bounded `queryNotams(latitude, longitude, radiusNm)` request, plus an HMI viewport trigger in `qt/BlueSkyPRO-HMI/qml/MapTilerWebMap.qml`. This is an implementation update to the older sequencing notes above; it does not waive the verification checklist.

### Viewport-triggered test query

- The map overlay offers radii of 10, 20, 50 and 100 NM; the default is 20 NM.
- A request is triggered after the map center moves approximately 2 NM, the zoom changes by at least 0.75, or the selected radius changes.
- The query is a circle around the map center, not an exact query for the visible map bounds.
- A failed request clears the remembered query key and allows a retry after a 15-second backoff. If prior records remain displayed, the status text identifies them as a previous snapshot.
- The status handler is connected to the `faaNmsClient.notamQueryFinished` signal, not to the map component itself.
- The displayed provider is FAA NMS staging, a US test source only. This does not establish Russian airspace coverage or authorize a mission.

### Verification evidence and remaining limits

- GitHub Actions at commit `d2168809b005997a16e4ec4e21329e6dfb274a83` completed successfully for HMI Qt Build, Windows Map Build and Multi-UAV Validator.
- These CI results confirm the relevant build/validation workflows, not live staging authentication, actual NOTAM ingestion, or interactive map behavior.
- Still required: run the app with authorized staging credentials and verify pan, zoom, radius change, success, empty result, timeout/auth failure, stale-snapshot messaging and retry behavior. Record the returned snapshot ID and result count. Do not mark the live-query checklist complete based on OAuth/ping alone.
- This viewport query is currently a test-stage mechanism. Confirm provider rate limits and account terms before any sustained use; do not use rapid map gestures as a production polling strategy without request coalescing/debouncing and rate-limit handling.
