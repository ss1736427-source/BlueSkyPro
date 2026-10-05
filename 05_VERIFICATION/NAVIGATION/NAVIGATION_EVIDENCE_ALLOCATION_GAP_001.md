---
id: NAVIGATION-EVIDENCE-ALLOCATION-GAP-001
type: navigation_evidence_allocation_gap
status: working_baseline
---

# BlueSky PRO — Navigation Evidence Allocation GAP 001

## Purpose

Зафиксировать минимальный residual GAP по Navigation verification cases после content-level reconciliation существующих evidence allocations.

## Confirmed gaps

| Navigation case | Requirement | Current evidence | Decision |
|---|---|---|---|
| NAV-V11 | CRB-NAV-002 | none dedicated | GAP — new evidence allocation required |
| NAV-V12 | CRB-NAV-002 | none dedicated | GAP — new evidence allocation required |
| NAV-V14 | Navigation energy degradation | none confirmed | GAP — new evidence allocation required |
| NAV-V20 | CRB-NAV-001 / Navigation state semantics | none dedicated | GAP — new evidence allocation required |

## Control decision

Do not repurpose EVD-007, EVD-014, EVD-016, EVD-017, EVD-018 or EVD-020 merely by thematic similarity. Existing records remain in their original allocation until an explicit content-level reuse decision is made.

Do not mark any evidence as EXECUTED, PASSED, ACCEPTED or CLOSED without an actual verification result and the mandatory evidence fields defined by `VERIFICATION_EVIDENCE_INDEX.md`.

## Required next action

Create the minimum evidence records required for NAV-V11, NAV-V12, NAV-V14 and NAV-V20, with explicit:

- Verification Case ID;
- Requirement ID;
- source/clause basis;
- verification method;
- objective;
- preconditions;
- configuration;
- procedure reference;
- expected result;
- actual result;
- pass/fail;
- anomalies;
- reviewer and acceptance status.

Numerical thresholds remain TBD where the controlled engineering/certification basis has not established them.
