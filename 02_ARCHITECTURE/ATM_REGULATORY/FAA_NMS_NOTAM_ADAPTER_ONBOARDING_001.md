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

**Current blocker:** the onboarding bundle defines the protocol, but this review did not verify that valid staging credentials are available. The credential spreadsheet was unreadable in the review environment. No live connection is claimed.

## 9. Next implementation sequence

1. Verify the credential-vault bridge's read/write behavior and add a dedicated `faa_nms` credential namespace without altering the existing `notam`/NOTAC secret.
2. Implement a native FAA NMS client for token exchange, token renewal and `ping`.
3. Add fixture-backed tests for auth/error handling before making live requests.
4. Add the bounded NOTAM query, response validation and canonical normalization.
5. Connect the result to the ADMIN integration console with source, status, last successful retrieval and freshness.
6. Keep Virtual Flight as the operationally isolated test path until the complete verification checklist is met.
