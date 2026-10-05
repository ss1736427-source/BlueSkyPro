---
id: EXTERNAL-REGULATORY-INTERFACE-ALLOCATION-001
type: external_regulatory_interface_allocation
status: controlled_working_baseline
system: BlueSky PRO
basis: REGULATORY-SOURCE-REGISTER-001
scope: BlueSky-owned system-management interfaces only
---

# BLUE SKY PRO — EXTERNAL REGULATORY INTERFACE ALLOCATION 001

## 1. Purpose

Define the BlueSky-owned side of external interfaces that may be derived from regulatory or operational external sources.

This record does **not** transfer external obligations to BlueSky and does not establish requirements for external organizations, providers, infrastructure or equipment.

## 2. Scope boundary

Included:

- external operational constraints consumed by BlueSky;
- authorized technical-readiness/status inputs consumed by BlueSky, where such an interface is part of the approved system scope;
- C2 service/provider state consumed by BlueSky;
- C2 degradation/loss indications and BlueSky response;
- freshness and validity control for external inputs;
- recording of relevant external state transitions within the BlueSky event/audit model.

Excluded:

- landing-site management and physical landing-site requirements;
- aerodrome/vertiport/landing-site owner obligations;
- maintenance organization obligations and physical maintenance activities;
- C2 provider's own certification, inspection or ground-equipment obligations;
- external radio-frequency/resource-owner obligations;
- any external obligation that is not a BlueSky-owned function.

## 3. Allocation matrix

| Allocation ID | Source domain | External input / condition | BlueSky-owned function | BlueSky requirement layer | Verification |
|---|---|---|---|---|---|
| EXT-ALLOC-001 | Operational restrictions | authorized operational constraint | ingest, validate and apply constraint to planning/flight logic | Interface / System Requirement | EXT-V01 |
| EXT-ALLOC-002 | Technical readiness | authorized status relevant to mission readiness | consume, validate and prevent invalid/stale status from becoming positive readiness | Interface / System Requirement | EXT-V02 |
| EXT-ALLOC-003 | C2 service | provider/service availability or state | consume and represent C2 state | Interface / System Requirement | EXT-V03 |
| EXT-ALLOC-004 | C2 degradation | loss/degradation indication | execute approved BlueSky response and record event | System / Safety Requirement | EXT-V04 |
| EXT-ALLOC-005 | System boundary | external responsibility | maintain responsibility boundary and avoid false compliance attribution | Governance / System Boundary | EXT-V05 |
| EXT-ALLOC-006 | External data | freshness/validity state | detect and handle stale external data | Interface / System Requirement | EXT-V06 |
| EXT-ALLOC-007 | Event source | external state transition | record relevant source, identity, time and state in BlueSky event model | System Requirement | EXT-V07 |

## 4. Regulatory allocation rule

A regulatory source becomes a BlueSky requirement only after the following chain is satisfied:

```text
REGULATORY SOURCE
→ APPLICABLE CLAUSE
→ OBJECT OF REGULATION
→ BLUE SKY ROLE
→ SYSTEM BOUNDARY
→ OWNED FUNCTION
→ REQUIREMENT / INTERFACE REQUIREMENT
→ VERIFICATION
→ EVIDENCE
```

No requirement is created merely because BlueSky exchanges data with an external participant.

## 5. Current source applicability

| Source | Current treatment in BlueSky |
|---|---|
| REG-013 / №142 | Direct/interface/conditional C2 domain; detailed C2 mapping maintained separately |
| REG-017 / №140 | External landing-site domain; excluded from BlueSky system-management verification |
| REG-018 / №312 | External/conditional infrastructure domain; no BlueSky requirement without a demonstrated owned interface |
| REG-019 / №367 | External/conditional maintenance domain; only BlueSky-owned readiness/status interface may be allocated |
| REG-020 / №354 | External/conditional C2-provider equipment domain; provider-side inspection remains external; BlueSky may consume provider status where approved |

## 6. Verification linkage

```text
EXT-ALLOC-001 → EXT-V01
EXT-ALLOC-002 → EXT-V02
EXT-ALLOC-003 → EXT-V03
EXT-ALLOC-004 → EXT-V04
EXT-ALLOC-005 → EXT-V05
EXT-ALLOC-006 → EXT-V06
EXT-ALLOC-007 → EXT-V07
```

The linked verification records are preparation stubs until real integration testing is performed.

## 7. Deferred items

The following are intentionally deferred until the applicable integration/design stage or approved verification basis exists:

- exact external interface schemas;
- protocol and transport selection where not already baselined;
- quantitative freshness/latency thresholds;
- provider-specific implementation details;
- real external datasets;
- execution evidence.

## 8. Status

**CONTROLLED WORKING BASELINE — BLUE SKY-OWNED EXTERNAL INTERFACE ALLOCATION ESTABLISHED; EXTERNAL RESPONSIBILITIES EXCLUDED; REAL INTEGRATION TESTING DEFERRED.**
