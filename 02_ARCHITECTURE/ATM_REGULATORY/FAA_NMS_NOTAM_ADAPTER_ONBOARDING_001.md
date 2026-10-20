# FAA NMS NOTAM Adapter — Onboarding and Implementation Gate

**Status:** BLOCKED ON OFFICIAL API ACCESS / CONTRACT  
**Scope:** Experimental US NOTAM source for integration testing only  
**Repository rule:** This record is maintained on the canonical `main` branch.

## 1. Purpose

Evaluate the FAA source shown in the BlueSky PRO integration setup and define the evidence required before implementing a dedicated FAA NMS adapter. This record does not claim a live FAA API connection or operational readiness.

## 2. Official sources

- Public legacy FNS NOTAM Search UI: https://notams.aim.faa.gov/notamSearch/nsapp.html
- FAA NOTAM Management Service portal: https://nms.aim.faa.gov/
- FAA NMS overview and API access entry point: https://www.faa.gov/about/initiatives/notam
- FAA NMS FAQs: https://www.faa.gov/about/initiatives/notam/faqs

The FAA NMS portal describes a modernized NOTAM service. The FAA FAQ says external users should contact `7-AWA-NAIMES@faa.gov` to request access to the NMS-API distribution interface. The portal's public pages do not provide sufficient unauthenticated API endpoint and response-schema detail to implement and verify a production client.

## 2.1 Publicly confirmed API capabilities

The FAA's public NOTAM modernization material describes the NMS-API as available for onboarding and identifies these capabilities:

- query/filter by time and location, including radius-based geospatial queries;
- retrieve recently created or cancelled NOTAMs;
- response formats including AIXM 5.1 and GeoJSON;
- delta queries for incremental updates;
- support for real-time and batch processing.

These are capability-level statements, not a complete endpoint contract. The precise URL paths, authentication exchange, request parameters, pagination, response envelopes, and schema constraints must come from the official onboarding package. Do not implement guessed URLs or assume OAuth details from third-party code.

## 2.2 Access request checklist

Send an access request to `7-AWA-NAIMES@faa.gov` or use the API access path linked from the official NMS portal. Ask for:
- developer/onboarding instructions and the current NMS-API specification;
- access to an approved test/sandbox environment, if available;
- authentication and credential provisioning instructions;
- approved usage terms for an external flight-planning application;
- clarification of whether API access can be granted to an organization based outside the United States.

Suggested request text is maintained below for convenience.

**Subject:** Request for FAA NMS-API access and technical documentation — BlueSky PRO integration

> Hello FAA NOTAM Service Center,
>
> We are evaluating an integration between the FAA NOTAM Management Service API and BlueSky PRO, a UAV flight-planning application. Our initial scope is limited to software integration testing using U.S. NOTAM data; it is not intended to replace official FAA services or represent coverage of non-U.S. airspace.
>
> Please advise how to request NMS-API access and provide the current technical documentation, including supported environments, authentication, endpoint/query specifications, response schemas, update/delta behavior, rate limits, and applicable terms of use. If a sandbox or non-operational test environment is available, please provide its onboarding procedure.
>
> Please also confirm any eligibility requirements for an applicant organization based outside the United States.
>
> Regards,  
> BlueSky PRO Integration Team

## 3. Current BlueSky PRO state

The current HMI NOTAM test uses the third-party NOTAC REST API, with a default base URL of `https://notac.aero/api/v1`. It sends a bearer token and requests active US notices using the NOTAC-specific route and response envelope. This is **not** a direct FAA NMS adapter.

The public FAA search page is a human-facing web application. Its presence does not establish that its internal web requests are a supported public API. Do not bind BlueSky Core to undocumented page internals or scrape the UI as a substitute for an authorized interface.

## 4. Required onboarding evidence

Before implementing the FAA adapter, obtain and archive the official onboarding response and technical specification supplied by FAA. Confirm:

1. API base URL(s) and environment(s) for test and production.
2. Authentication method, token lifecycle, and required headers.
3. Supported query filters (including geography, time validity, status and pagination).
4. Response format(s), schema version(s), and geometry representation.
5. Rate limits, update/delta semantics, freshness expectations, and service error behavior.
6. Terms of use and whether the requested access permits the intended integration and data retention.

Do not store credentials in source control. Use the existing native credential vault mechanism once the authentication contract is confirmed.

## 5. Adapter implementation contract

When the official specification is available, implement a dedicated FAA NMS adapter behind the common NOTAM source boundary. Keep the existing NOTAC adapter available for independent testing; do not silently repurpose its endpoint, token handling, route, or parser.

The FAA adapter must:

- validate HTTP status, content type, schema/version and required fields;
- normalize source records into the BlueSky NOTAM model without discarding source identifiers, validity intervals, cancellation/replacement state, geometry, retrieval time or provenance;
- reject malformed or incomplete safety-relevant records rather than treating them as an empty result;
- expose explicit states for authorization failure, timeout, rate limit, invalid response and successful retrieval;
- include deterministic fixture tests for normal, empty, malformed, expired/cancelled, paginated and error responses;
- mark FAA data as a US test source and never imply Russian airspace coverage.

## 6. Verification gate

The adapter is not considered connected merely because the portal opens or an HTTP request returns 2xx. Acceptance requires an authorized API call against the documented endpoint, a schema-validated response, normalization tests, and evidence of data provenance and validity handling.

Until then, Virtual Flight should continue to use labelled fixtures/synthetic data. FAA test data must not be used as authoritative Russian operational restrictions or as the sole basis for releasing a real mission.

## 7. Next action

Request NMS-API access and the current technical specification through the official FAA NMS portal/contact. Once the specification is available, implement and test the dedicated adapter on `main`, without creating a branch or PR unless a real technical constraint requires one.
