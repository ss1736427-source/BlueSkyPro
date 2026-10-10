---
id: NAVIGATION-EVIDENCE-IDENTITY-RECONCILIATION-001
type: navigation_evidence_identity_reconciliation
status: working_baseline
parent: VERIFICATION-EVIDENCE-INDEX-001; VERIFICATION-REGISTER-001
---

# BlueSky PRO — Navigation Evidence Identity Reconciliation 001

## 1. Purpose

Устранить identity mismatch между Evidence Index и контролируемым Verification Register без создания дублирующих verification-case identities и без изменения execution status.

## 2. Controlled identity basis

`VERIFICATION_REGISTER.md` устанавливает:

```text
NAV-V01 … NAV-V20
```

как единственные контролируемые Navigation verification case IDs. `NAV-TV-001 … NAV-TV-010` остаются отдельными test-vector identities.

## 3. Reconciliation

| Existing reference | Controlled identity | Decision |
|---|---|---|
| V-NAV-001 | NAV-V01 | RECONCILED — legacy reference removed from active allocation |
| V-NAV-002 | NAV-V02 | RECONCILED — legacy reference removed from active allocation |
| V-NAV-003 | NAV-V03 | RECONCILED — legacy reference removed from active allocation |
| V-NAV-004 | NAV-V04 | RECONCILED — legacy reference removed from active allocation |
| V-NAV-005 | NAV-V05 | RECONCILED — legacy reference removed from active allocation |

No new verification case was created. No meaning, method, requirement allocation or status was changed.

## 4. Evidence mapping

Current controlled evidence allocations are:

```text
EVD-006 → NAV-V01
EVD-007 → NAV-V02
EVD-008 → NAV-V03
EVD-009 → NAV-V04
EVD-010 → NAV-V05
EVD-023 → NAV-V11
EVD-024 → NAV-V12
EVD-025 → NAV-V14
EVD-026 → NAV-V20
```

All remain `PLANNED` until actual execution and review are recorded.

## 5. Control rule

```text
ONE VERIFICATION CASE
→ ONE STABLE ID
→ MANY EVIDENCE RELATIONSHIPS
```

Legacy `V-NAV-*` strings shall not be used as parallel active identities.

## 6. Verification boundary

The Evidence Index itself requires evidence to be linked to a requirement, verification method, controlled configuration, procedure/result and review before acceptance. Presence of an evidence allocation does not constitute execution evidence.

## 7. Status

**WORKING BASELINE — IDENTITY RECONCILED**
