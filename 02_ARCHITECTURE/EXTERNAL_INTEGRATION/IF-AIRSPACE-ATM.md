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
