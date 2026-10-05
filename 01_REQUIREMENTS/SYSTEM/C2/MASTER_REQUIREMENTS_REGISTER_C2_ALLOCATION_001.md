---
id: MASTER-REQUIREMENTS-C2-ALLOCATION-001
type: requirements_allocation
status: working_draft
system: BlueSky PRO
source: C2_CLAUSE_LEVEL_MAPPING_001.md
---

# BLUE SKY PRO — C2 REQUIREMENTS ALLOCATION 001

## 1. Purpose

Controlled allocation of the C2 regulatory mapping into the project requirements hierarchy. This document is a working allocation record; it does not replace the authoritative `MASTER_REQUIREMENTS_REGISTER.md`, regulatory sources, or an approved certification basis.

## 2. Traceability chain

```text
REGULATORY CLAUSE
→ APPLICABILITY
→ REGULATORY REQUIREMENT
→ EXISTING BLUE SKY REQUIREMENT / DERIVED REQUIREMENT
→ ARCHITECTURE / SAFETY / ICD
→ VERIFICATION
→ EVIDENCE
```

## 3. Candidate allocation

The following are **candidate derived records**, not new authoritative SYS-REQ identities. Existing SYS-REQ records have priority and must be reconciled before any baseline change.

| Candidate | Source | Requirement implication | Existing authoritative coverage | Allocation | Verification | Disposition |
|---|---|---|---|---|---|---|
| C2-CAND-001 | №142 | explicit C2 operational mode/state | SYS-REQ-083 + existing C2 architecture records | FLIGHT / C2 | analysis + test | DERIVED / existing coverage |
| C2-CAND-002 | №142 | monitor C2 path and distinguish normal/degraded/lost | SYS-REQ-086, SYS-REQ-093 | C2 / INTEGRATION | analysis + test | MERGE / existing coverage |
| C2-CAND-003 | №142 | apply approved C2 QoS parameters | SYS-REQ-091 where applicable; quantitative values remain TBD | C2 / CONFIGURATION | inspection + test | DERIVED / GAP only if coverage absent |
| C2-CAND-004 | №142 | enforce defined PROVIDER-C2 service/interface boundary | existing provider/interface allocation; no provider-side requirement assigned to BlueSky | INTEGRATION / ICD | inspection + integration test | DERIVED |
| C2-CAND-005 | №142 | record C2 state transitions and relevant events | existing event/logging records | HUB / JOURNAL | test + log inspection | DERIVED / existing coverage |
| C2-CAND-006 | №142 | invoke approved contingency behaviour on C2 loss/degradation | SYS-REQ-081, SYS-REQ-082, SYS-REQ-086, SYS-REQ-093 | FLIGHT / SAFETY | scenario test | MERGE / existing coverage |
| C2-CAND-007 | №142 | preserve cause/status distinction for C2 failures | SYS-REQ-086, SYS-REQ-093 + diagnostics/safety allocation | HUB / SAFETY | test + evidence review | DERIVED / existing coverage |
| C2-CAND-008 | №142 | use common controlled time reference | existing common time/reference allocation | SYSTEM / HUB | analysis + test | DERIVED / existing coverage |

## 4. Exact existing SYS-REQ reconciliation

The exact requirement records listed by the current C2 reconciliation have been inspected before allocation. Their identities are preserved; no replacement or renumbering is introduced.

```text
SYS-REQ-080  Dynamic Task Reallocation       KEEP
SYS-REQ-081  UAV Failure Tolerance            KEEP
SYS-REQ-082  Safe Mission Completion           KEEP
SYS-REQ-083  Platform Independence             KEEP
SYS-REQ-085  Safety-Critical Priority          KEEP
SYS-REQ-086  Graceful Degradation              KEEP
SYS-REQ-091  Critical Latency                  KEEP
SYS-REQ-092  Redundant HUB Resource Recovery   KEEP
SYS-REQ-093  Controlled Resource Recovery       KEEP
```

The reconciliation confirms that C2 degradation/loss/recovery does not justify a duplicate authoritative SYS-REQ on the evidence currently available.

## 5. Boundary rule

`DIRECT-C2` means the BlueSky-controlled C2 function/interface is inside the system boundary.

`PROVIDER-C2` means the external provider remains responsible for its service while BlueSky manages and verifies only the defined interface obligations inside the BlueSky boundary.

Provider infrastructure and provider internal implementation are not BlueSky SYS-REQ unless the certification scope explicitly assigns those functions to BlueSky.

## 6. Open allocation items

- Final numerical C2 performance limits: OPEN.
- Exact certification-object applicability: OPEN.
- Allocation of contingency behaviour to flight-control software versus external aircraft system: OPEN.
- Final ICD parameter set: OPEN.
- Verification acceptance criteria: OPEN pending approved requirements baseline.

These open items do not block the completed identity/overlap reconciliation above.

## 7. Consolidation rule

```text
CANDIDATE
→ exact wording comparison
→ scope comparison
→ source / basis check
→ safety impact check
→ architecture / ICD check
→ existing verification check
→ KEEP / DERIVED / MERGE / GAP / CONFLICT
```

Only a proven `GAP` may justify creation of a new authoritative requirement.

## 8. Current result

```text
EXACT SYS-REQ CONTENT REVIEW        COMPLETE FOR IDENTIFIED C2 SET
DUPLICATE AUTHORITATIVE SYS-REQ     NONE IDENTIFIED
C2 CANDIDATE ALLOCATION             RECORDED
C2 NUMERICAL CRITERIA               TBD / OPEN
CERTIFICATION APPLICABILITY         OPEN
REAL VERIFICATION / EVIDENCE        DEFERRED TO TEST STAGE
```

## 9. Next controlled work

```text
C2 allocation
→ SAFETY allocation
→ ICD parameter baseline
→ VERIFICATION acceptance criteria
→ REQUIREMENTS TRACEABILITY MATRIX
```

No item is marked `COMPLIANT` or `BASELINED` until the applicable source, requirement decision and controlled verification evidence exist.
