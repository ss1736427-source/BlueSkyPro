---
id: NAVIGATION-EVIDENCE-ALLOCATION-RECONCILIATION-001
type: navigation_evidence_allocation_reconciliation
status: working_baseline
parent: NAVIGATION-VERIFICATION-COVERAGE-RECONCILIATION-001
---

# BlueSky PRO — Navigation Evidence Allocation Reconciliation 001

## 1. Purpose

Проверить, может ли `NAV-V11…NAV-V20` использовать существующие non-Navigation evidence allocations, прежде чем создавать новые `EVD-*` IDs.

## 2. Controlled inputs

`VERIFICATION_REGISTER-001` устанавливает `NAV-V01…NAV-V20` как стабильные Navigation verification cases и отдельно сохраняет `NAV-TV-001…NAV-TV-010` как test-vector identities.

`VERIFICATION_EVIDENCE_INDEX-001` содержит текущие evidence allocations `EVD-001…EVD-022`. Для `NAV-V11…NAV-V20` отдельные evidence records в текущем индексе не назначены.

## 3. Reconciliation result

| Navigation case | Existing potentially related evidence | Coverage decision | Rationale |
|---|---|---|---|
| NAV-V11 Conflicting sources | EVD-007 / V-NAV-002 only | PARTIAL / NOT REUSABLE AS PRIMARY | V-NAV-002 предназначен для invalid/stale handling; conflict handling является отдельным сценарием `NAV-V11`. |
| NAV-V12 Degraded source | EVD-007 / V-NAV-002 | PARTIAL / NOT REUSABLE AS PRIMARY | EVD-007 не идентифицирован как доказательство отдельного `NAV-V12`; degraded source имеет собственный verification case. |
| NAV-V13 Communication degradation | EVD-014 / V-C2-002 | RELATED, NOT EQUIVALENT | C2 degradation может быть смежным evidence, но не подтверждает автоматически navigation-specific communication degradation. |
| NAV-V14 Energy degradation | none confirmed | GAP | `NAV-TV-009` требует energy model; отдельного evidence allocation нет. |
| NAV-V15 Dynamic Return degradation | EVD-017 / EVD-018 | RELATED, NOT EQUIVALENT | Dynamic Return evidence существует, но не закреплено за `NAV-V15`; прямое соответствие должно быть подтверждено содержанием case/result. |
| NAV-V16 Material runtime change | EVD-018 / V-RET-002 may be related | RELATED, NOT EQUIVALENT | Reassessment after degradation не идентично общему material runtime change. |
| NAV-V17 Revalidation required | EVD-018 may be related | RELATED, NOT EQUIVALENT | Revalidation authority path должен проверяться отдельным verification objective. |
| NAV-V18 Safety rejection | EVD-016 / V-SAF-002 | RELATED, CANDIDATE REUSE | EVD-016 проверяет невозможность обхода Safety Gate; это близкая, но не автоматически полная реализация `NAV-V18`. Требуется content-level confirmation. |
| NAV-V19 Multi-UAV | EVD-020 / V-MUL-001 | RELATED, CANDIDATE REUSE | EVD-020 проверяет individual UAV state isolation и может покрывать часть `NAV-V19`; полное соответствие требует подтверждения scenario/result. |
| NAV-V20 Planned vs Simulated vs Actual | none confirmed | GAP | `NAV-V20` является отдельным state-semantics case; текущий EVD index не содержит выделенного evidence. |

## 4. Decision

Не создаём новые `EVD-*` IDs на основании одного совпадения тематики.

Существующие `EVD-016`, `EVD-020` и, условно, `EVD-017/018` являются кандидатами на переиспользование после content-level verification of objectives, procedures, expected results and configuration linkage.

`NAV-V11`, `NAV-V12`, `NAV-V14`, `NAV-V20` имеют подтверждённый residual evidence-allocation gap по текущему индексу.

`NAV-V13`, `NAV-V15`, `NAV-V16`, `NAV-V17`, `NAV-V18`, `NAV-V19` требуют content-level reconciliation с существующими evidence records до решения о создании новых EVD.

## 5. Evidence control boundary

До фактического execution:

```text
DEFINED / READY / PLANNED ≠ EXECUTED ≠ PASSED ≠ ACCEPTED
```

Текущий Evidence Index требует configuration, procedure/result, review and acceptance для принятия evidence.

## 6. Next controlled action

Проверить содержимое существующих evidence-associated verification records:

```text
EVD-014 / V-C2-002
EVD-016 / V-SAF-002
EVD-017 / V-RET-001
EVD-018 / V-RET-002
EVD-020 / V-MUL-001
```

После этой проверки:

1. подтвердить reuse, если coverage фактически полное;
2. иначе создать минимальный набор новых EVD IDs только для residual gaps;
3. не изменять статусы execution без фактических результатов.

## 7. Status

**WORKING BASELINE — CONTENT RECONCILIATION REQUIRED**
