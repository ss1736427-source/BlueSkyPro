---
id: C2-TEXT-RECONCILIATION-001
type: requirements_text_reconciliation
status: controlled_working_draft
system: BlueSky PRO
authority: MASTER-REQUIREMENTS-REGISTER-001
---

# BLUE SKY PRO — C2 TEXT RECONCILIATION 001

## 1. Purpose

Выполнить содержательное сопоставление C2-кандидатов с уже существующими системными и safety requirements, не создавая второй authoritative register.

## 2. Source records actually available

Для сопоставления использованы текущие записи репозитория:

- `MASTER_REQUIREMENTS_REGISTER.md`
- `SYSTEM_REQUIREMENTS_SPECIFICATION.md`
- `SAFETY_REQUIREMENTS_BASELINE.md`
- `SAFETY_CASE.md`
- `INTERFACE_CONTROL_DOCUMENT.md`
- `C2_CLAUSE_LEVEL_MAPPING_001.md`
- `C2_SYSTEM_REQUIREMENTS_BASELINE_001.md`
- `C2_REQUIREMENTS_RECONCILIATION_001.md`
- `C2_TRACEABILITY_BASELINE_001.md`
- `REQUIREMENTS_INVENTORY.md`
- `REQUIREMENTS_RECONCILIATION.csv`

## 3. Important inventory finding

`REQUIREMENTS_INVENTORY.md` содержит идентификаторы и перечень файлов, в которых они встречаются, но не является источником полного controlled wording каждого `SYS-REQ`.

Поэтому сопоставление, требующее дословного текста существующего SYS-REQ, не должно объявляться окончательным, пока controlled wording не извлечён из исходной записи.

Это не является основанием для создания нового SYS-REQ.

## 4. C2 candidate → existing coverage

| Candidate | Existing record(s) | Available evidence | Current disposition | Baseline action |
|---|---|---|---|---|
| C2-REQ-001 C2 operating mode | SYS-REQ-083; DEC-012 | SYS-REQ-083 title = Platform Independence; DEC-012 explicitly establishes DIRECT-C2 / PROVIDER-C2 | DERIVED / OVERLAP | do not create new SYS-REQ yet; reconcile wording |
| C2-REQ-002 C2 NORMAL/DEGRADED/LOST monitoring | SYS-REQ-086; SAF-REQ-013; IF-C2-001 | SYS-REQ-086 = Graceful Degradation; SAF-REQ-013 explicitly covers C2 degradation; ICD defines C2 state model | MERGE / DERIVED | preserve existing IDs; allocate C2 behaviour to them |
| C2-REQ-003 C2 quality/availability parameters | SYS-REQ-091; C2-142-004/011/013/014 | SYS-REQ-091 = Critical Latency; regulatory mapping identifies C2 quality/QoS/availability | PARTIAL / GAP CHECK | no new baseline until parameter source and existing wording are checked |
| C2-REQ-004 Provider-C2 availability/boundary | DEC-012; IF-C2-001 | DEC-012 defines provider boundary; ICD defines provider interface | DERIVED | interface allocation, not new independent SYS-REQ |
| C2-REQ-005 cause/status distinction | SAF-REQ-013; C2 safety allocation C2-F-001..005 | Safety baseline covers C2 degradation; C2 safety model separates failure conditions and states | DERIVED / OVERLAP | use existing safety allocation; create requirement only if a real functional gap remains |
| C2-REQ-006 approved response to C2 loss/degradation | SYS-REQ-081; SYS-REQ-086; SYS-REQ-093; SAF-REQ-013 | existing failure/recovery cluster + C2 safety allocation | MERGE candidate | preserve existing requirement identity; derive scenario/verification |
| C2-REQ-007 C2 event recording | SYS-C2-005; C2-142-021; C2 ICD; archive/data model | Controlled wording defines C2 event logging and the ICD identifies event/state data | DERIVED / COVERAGE CONFIRMED | allocate to existing event/archive model; no new SYS-REQ |
| C2-REQ-008 controlled time reference | SYS-REQ-091; IF-C2-001; SYS-C2-008 | SYS-REQ-091 and controlled wording define timestamp and common time-base semantics for C2 data/events | DERIVED / COVERAGE CONFIRMED | allocate to existing interface/time model; no new SYS-REQ |
| C2-REQ-009 AI/optimization cannot bypass safety authority | SYS-REQ-085; SYS-REQ-082; SAF-REQ-005; SAF-REQ-018; ARCH-DEC-016 | explicit Safety Gate and AI authority boundary | MERGE / DERIVED | no duplicate authority requirement |
| C2-REQ-010 C2 state/warnings to operator | SYS-C2-006; C2-142-020; HMI interface | Controlled wording defines operator warnings/data on C2 loss and HMI allocation is established | DERIVED / COVERAGE CONFIRMED | allocate to existing HMI and safety workflow; no new SYS-REQ |

## 5. Safety reconciliation

C2-REQ-002, C2-REQ-005 and C2-REQ-006 have clear existing safety relationships:

```text
H-C2-001
→ SAF-REQ-013
→ C2 state / degradation handling
→ SYS-REQ-086 / related recovery allocation
→ verification
→ evidence
```

The existing safety baseline explicitly states that loss or degradation of C2 shall be detected and mapped to a defined safety response. The Safety Case repeats the same C2 safety claim. No parallel safety requirement is justified by the current evidence.

## 6. Architecture reconciliation

The existing architecture authority chain remains:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

Therefore C2-REQ-009 is not a new execution-authority branch. It is a derived constraint on existing architecture.

DEC-012 separately establishes the DIRECT-C2 / PROVIDER-C2 boundary. Provider internal implementation is external to BlueSky unless the approved scope changes.

## 7. Interface reconciliation

`IF-C2-001` already defines the controlled C2 interface model, including:

- connection state;
- latency;
- command status;
- telemetry status;
- failure/recovery state;
- provider status where applicable;
- timestamp/temporal validity;
- authentication/integrity where applicable;
- version/configuration control.

Therefore interface content must not be duplicated as independent SYS-REQ merely because it appears in the C2 candidate list.

## 8. Verification reconciliation

Existing verification records are referenced by the project as `TEST-*`, but the current repository-level material available in this pass does not expose enough controlled TEST content to declare equivalence for every C2 candidate.

Disposition:

```text
Equivalent TEST found
→ LINK

No equivalent TEST
→ create verification case

Requirement changed
→ impact + regression review
```

No C2 requirement is marked VERIFIED from the existence of an architecture or ICD record alone.

## 9. Corrections to previous C2 allocation logic

The previous C2 allocation contained candidate system requirements that could be mistaken for new authoritative requirements.

This reconciliation establishes:

1. `C2-REQ-*` remains candidate/derived vocabulary.
2. Existing `SYS-REQ-*` identities have priority.
3. `SAF-REQ-013` already provides explicit C2 degradation safety coverage.
4. `DEC-012` already establishes the C2 provider boundary.
5. `IF-C2-001` already provides the principal C2 interface structure.
6. No new SYS-REQ is baselined by this pass.
7. `C2-REQ-007` and `C2-REQ-010` remain genuine gap candidates only until existing wording is checked.

## 10. Master Register impact

**No authoritative SYS-REQ is created, renumbered, deleted or baselined by this pass.**

The only permitted next change to `MASTER_REQUIREMENTS_REGISTER.md` is a controlled update after the remaining exact-wording comparison is completed.

## 11. Result

```text
C2-REQ-001 → DERIVED / OVERLAP
C2-REQ-002 → MERGE / DERIVED
C2-REQ-003 → PARTIAL / GAP CHECK
C2-REQ-004 → DERIVED
C2-REQ-005 → DERIVED / OVERLAP
C2-REQ-006 → MERGE CANDIDATE
C2-REQ-007 → GAP CANDIDATE
C2-REQ-008 → PARTIAL / GAP CHECK
C2-REQ-009 → MERGE / DERIVED
C2-REQ-010 → GAP CANDIDATE
```

## 12. Gate

**C2 TEXT RECONCILIATION: PARTIAL — NO DUPLICATE BASELINE CREATED**

Remaining blocker is not conceptual: it is extraction of the controlled wording and verification links for the affected existing requirements.

Until that extraction is available, `GAP` means **candidate gap**, not proven missing requirement.

## 13. Next controlled action

```text
affected existing SYS-REQ / SAF-REQ
→ exact wording
→ candidate comparison
→ final KEEP / DERIVED / MERGE / GAP / CONFLICT
→ controlled Master Register update
→ SRS / ICD / Safety / Verification propagation
```
