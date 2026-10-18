# IF-AIRSPACE-ATM — BlueSky Airspace and ATM Interface

**Status:** STRUCTURAL DRAFT  
**Purpose:** Define the external aviation-information and ATM boundary used by BlueSky for planning, validation, authorization and operational status.

## Flow

```text
AIP / NOTAM / Airspace / Restrictions / Weather / ATM sources
                         |
                         v
                External Data Gateway
                         |
                  validation/quality
                         |
                         v
                       HUB
                         |
              +----------+----------+
              |                     |
              v                     v
         Planning/Safety       Authorization
              |                     |
              +----------+----------+
                         |
                         v
                    Flight Mission
```

## Required external domains

- aeronautical information;
- airspace and restrictions;
- NOTAM and temporary changes;
- terrain/obstacle information;
- weather and wind;
- traffic information where required;
- flight-plan submission/status where applicable;
- authorization/approval status;
- operational messages and acknowledgements.

## Required authorization states

DATA_CURRENT, DATA_STALE, SUBMISSION_PENDING, SUBMITTED, ACCEPTED, REJECTED, CANCELLED, AMENDED.

## Principle

BlueSky MUST distinguish between information used for planning and an actual authorization to conduct a flight. Receipt of external data MUST NOT itself be interpreted as permission to fly.

## Open items

- authoritative source and protocol for each Russian operational data source;
- ЕСОРВД/FPL exchange contract;
- authentication and submission identity;
- acknowledgement/status model;
- freshness and validity requirements;
- outage/fallback behavior;
- audit/evidence requirements.


## NOTAM source discovery — 2026-10-18

**Status:** SOURCE IDENTIFIED; MACHINE-TO-MACHINE ACCESS NOT YET QUALIFIED  
**Scope:** Russian national aeronautical information. This is a source investigation result, not an operational approval or a claim that a public API exists.

### Primary source candidate

The Federal Air Transport Agency (Rosaviatsiya) identifies the Aeronautical Information Centre branch of FSUE State ATM Corporation (Филиал «ЦАИ» ФГУП «Госкорпорация по ОрВД») as the location for Russian aeronautical information products. The Centre's official site publishes a dedicated NOTAM information page and states that NOTAM are distributed through AFTN channels.

- Rosaviatsiya aeronautical information page: https://www.favt.gov.ru/dejatelnost-organizacija-ispolzovanija-vozdushnogo-prostranstva-aeronavi-inform/
- CAI/State ATM Corporation official product portal: https://ftp.caica.ru/GKOVD/?lang=en
- CAI NOTAM information page: https://post.caica.ru/notam/?lang=ru

The CAI page describes multiple Russian NOTAM series, including series for temporary airspace restrictions. It also lists separate products for restricted, prohibited, danger and UAV flight zones. Therefore, NOTAM ingestion alone must not be treated as complete coverage of all constraints relevant to a UAV mission.

### External alternative

EUROCONTROL EAD documents worldwide international NOTAM availability, and its availability table lists Russia (UUUU). System-to-system access through MyEAD requires an EAD Data User Agreement and API integration; EAD Basic explicitly says its displayed information is not for operational use. EAD is therefore a possible licensed international-data input, not a substitute for qualifying the Russian national/local restriction sources.

- EAD international NOTAM availability: https://www.ead.eurocontrol.int/cms-eadbasic/opencms/en/ead-operations/data-maintenance/notam-availability-ino/
- MyEAD system-to-system access: https://www.ead.eurocontrol.int/cms-eadbasic/opencms/en/ead-solutions/my-ead/
- EAD access modes and operational-use limitation: https://www.eurocontrol.int/service/european-ais-database

### Engineering decision

1. Treat the CAI branch of State ATM Corporation as the **primary source candidate for Russian official aeronautical information**.
2. Do not implement screen scraping or infer a supported API from the public web pages. Confirm the permitted machine-readable delivery method, authentication, licence/terms, update cadence, cancellation/replacement semantics, coverage and service-level expectations with the provider.
3. Keep a provider-neutral adapter boundary. The adapter must produce a canonical, normalized snapshot only after parsing, provenance and completeness checks.
4. Preserve at least: provider identity, source message/reference, source publication/version, retrieval time, evaluation time, coverage scope, effective intervals, original altitude reference, cancellations/replacements, normalization version and integrity evidence.
5. A route-area query is not a complete snapshot unless the provider confirms the requested geographic/time scope and the adapter can prove that the response is complete. Timeout, partial response, unknown coverage, unsupported altitude datum, stale data or failed integrity checks must block assessment.
6. Keep NOTAM, officially published airspace/restricted-zone datasets, local/temporary UAV restrictions, terrain and obstacle inputs as distinct data dependencies. Each must have its own source and freshness/completeness status.
7. Do not pass a web-page display, manually copied NOTAM, synthetic fixture or unverified response to the operational assessment path. Tests may use fixtures only in isolated test code.
8. The combined `assessManualRouteCandidateAirspaceAndNotams` assessment remains a partial geometry check. It must not set flight-release eligibility; release remains blocked until all required authoritative inputs and authorization gates are connected and pass.

### Provider qualification checklist

- [ ] Provider confirms the authoritative product and permitted machine interface for BlueSky's intended Russian operating use.
- [ ] Delivery method, credentials, licence/redistribution rights and operational-use restrictions are documented.
- [ ] Full/update briefing semantics and cancellation/replacement handling are specified.
- [ ] Geographic and time coverage can be proven for each request.
- [ ] Retrieval timestamp, source timestamp/version and data integrity can be recorded and audited.
- [ ] Parser handles malformed, truncated, duplicate, cancelled, replaced and unsupported messages fail-closed.
- [ ] NOTAM altitude references and effective periods are preserved; no implicit AGL/MSL conversion.
- [ ] Expiry, provider outage, stale data and partial coverage have tested fail-closed behavior.
- [ ] Independent verification confirms the normalized snapshot against the source response.

**Current gate:** provider identity is researched; machine interface, data rights, completeness semantics and operational freshness limits are **unconfirmed**. No live NOTAM feed is connected by this record, and it does not change flight-release behavior.
