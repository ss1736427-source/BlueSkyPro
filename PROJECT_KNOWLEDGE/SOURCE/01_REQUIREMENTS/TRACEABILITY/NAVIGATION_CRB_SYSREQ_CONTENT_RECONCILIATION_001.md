---
id: NAVIGATION-CRB-SYSREQ-RECON-001
type: navigation_requirement_content_reconciliation
status: WORKING
scope: CRB-NAV-001..005; SYS-REQ-081,082,085,086,091,093
authority: MASTER-REQUIREMENTS-REGISTER-001
---

# BLUE SKY PRO — NAVIGATION CRB ↔ SYS-REQ CONTENT RECONCILIATION 001

Date: 2026-09-06

## 1. Purpose

Фактическая содержательная сверка CRB-NAV-001..005 с существующими SYS-REQ-081/082/085/086/091/093. Новые requirement IDs не создаются; эквивалентность определяется только по содержанию.

## 2. Navigation baseline

| CRB | Controlled content | Verification |
|---|---|---|
| CRB-NAV-001 | Navigation State: actual, planned, derived values + quality/provenance | Analysis / Test / Review |
| CRB-NAV-002 | Critical navigation inputs: valid/degraded/stale/invalid/unavailable quality states | Test |
| CRB-NAV-003 | Directional/navigation quantities have an explicitly defined reference frame | Analysis / Test |
| CRB-NAV-004 | Course, Heading and Track are not treated as interchangeable | Analysis / Test |
| CRB-NAV-005 | Detect applicable deviation of actual state from planned route/segment | Test / Simulation |

Source records show these five items as OPEN navigation baseline items. CRB-NAV-001..003 cite REG-RU-009 clause TBD; CRB-NAV-004 cites Navigation Rules + regulatory applicability review; CRB-NAV-005 cites REG-RU-009 clause TBD.

## 3. Existing SYS-REQ content

### SYS-REQ-081 — UAV Failure Tolerance

The requirement addresses failure/loss of an individual element, assessment of failure impact on current tasks and mission state, and transfer of critical tasks to a reserve executor when provided by the mission profile.

### SYS-REQ-082 — Safe Mission Completion

The requirement addresses safe mission completion when full execution becomes impossible, considering UAV state, task criticality, available resources, energy, communications and risks. It defines an emergency-completion priority including safe return, validated landing point, and prescribed emergency behaviour.

### SYS-REQ-085 — Safety-Critical Priority

P0/P1 functions have priority over P2–P5 under resource contention; P0/P1 include at least safety, flight control, navigation and C2. Lower-priority functions must not obstruct P0/P1.

### SYS-REQ-086 — Graceful Degradation

Under resource shortage, HUB shall provide controlled degradation by priority. P5/P4 are restricted first, P3 may be restricted, P2 is retained as required for safe mission execution, and P0/P1 remain at the required level regardless of low-priority load.

### SYS-REQ-091 — Critical Latency

Critical messages shall have controlled permissible latency values. P0/P1 shall have stricter latency requirements than P3–P5. When a critical-service threshold is exceeded, the system shall take protective action.

### SYS-REQ-093 — Controlled Resource Recovery

After overload is removed, HUB shall restore restricted services in a controlled manner without secondary overload; critical functions recover before low-priority functions; degradation and recovery state shall be recorded.

## 4. Content comparison

| CRB | Closest SYS-REQ | Result | Controlled conclusion |
|---|---|---|---|
| CRB-NAV-001 Navigation State | 081, 082, 086, 091 | RELATED, NOT EQUIVALENT | Navigation State is a defined Navigation object. SYS requirements use system/UAV/mission state and do not define the Navigation State model. |
| CRB-NAV-002 Navigation data quality | 081, 086, 091 | RELATED, NOT EQUIVALENT | Degradation and latency requirements can consume navigation quality/freshness, but do not define the Navigation quality state taxonomy. |
| CRB-NAV-003 Reference frame | none demonstrated | NO DIRECT COVERAGE | No supplied SYS-REQ wording defines navigation reference frame. Keep as Navigation baseline item. |
| CRB-NAV-004 Course / Heading / Track distinction | none demonstrated | NO DIRECT COVERAGE | No supplied SYS-REQ wording distinguishes Course, Heading and Track. Keep as Navigation baseline item. |
| CRB-NAV-005 Navigation deviation | 082, possibly 081 | RELATED, NOT EQUIVALENT | Safe completion/failure handling may consume route/navigation state, but no supplied SYS-REQ defines route/segment deviation detection. |

## 5. Decision

```text
CRB-NAV-001 → KEEP; supporting allocation candidates only
CRB-NAV-002 → KEEP; supporting allocation candidates only
CRB-NAV-003 → KEEP; no direct SYS-REQ coverage demonstrated
CRB-NAV-004 → KEEP; no direct SYS-REQ coverage demonstrated
CRB-NAV-005 → KEEP; supporting allocation candidates only
```

No merge is authorized.

## 6. Traceability gaps

1. CRB-NAV-001..005 remain OPEN.
2. Regulatory source clauses remain TBD where specified by the source records.
3. Direct allocation from the CRBs to the six SYS-REQ records is not yet established as a controlled relationship.
4. Verification linkage for CRB-NAV-001..005 remains to be completed against existing TEST-* records.
5. Evidence and configuration linkage remain open.

## 7. Important boundary

The comparison confirms that the existing SYS-REQ records are system-level requirements with broader scope. The Navigation baseline defines navigation-specific state, quality, reference-frame, directional-quantity and deviation semantics. Therefore a Navigation CRB is not replaced by an existing SYS-REQ merely because the SYS-REQ depends on Navigation.

## 8. Status

**WORKING — content comparison completed for CRB-NAV-001..005 against SYS-REQ-081/082/085/086/091/093. No equivalence or merge established.**
