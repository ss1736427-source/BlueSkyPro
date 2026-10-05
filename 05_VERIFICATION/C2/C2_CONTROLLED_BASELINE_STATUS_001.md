---
id: C2-CONTROLLED-BASELINE-STATUS-001
type: c2_baseline_status
status: controlled_working_record
system: BlueSky PRO
---

# BLUE SKY PRO — C2 CONTROLLED BASELINE STATUS 001

## 1. Результат прохода

Проведена проверка C2-цепочки по существующим рабочим материалам:

```text
Приказ №142
→ clause mapping
→ C2 candidates
→ existing requirements
→ ICD
→ Safety
→ HMI
→ Verification
```

## 2. Решение по candidate allocation

Кандидатные записи `C2-CAND-*` не получают статус `SYS-REQ` автоматически.

Текущий controlled disposition:

| Candidate | Disposition | Action |
|---|---|---|
| C2-CAND-001 | DERIVED / COVERED | сохранён как derived allocation; exact SYS-REQ reconciliation выполнена |
| C2-CAND-002 | MERGE / COVERED | связан с SYS-REQ-086 и SYS-REQ-093; новый SYS-REQ не создаётся |
| C2-CAND-003 | PARTIAL / CONTROLLED BASIS IDENTIFIED | используется SYS-REQ-091 where applicable, C2-142-004/011/013/014, IF-C2 и IV-C2; количественные значения не назначаются без утверждённой operational/certification basis |
| C2-CAND-004 | DERIVED | сохранён как interface/system allocation; provider-side obligations остаются внешними |
| C2-CAND-005 | DERIVED / COVERED | используется существующий logging/evidence контур |
| C2-CAND-006 | MERGE / COVERED | связан с SYS-REQ-081, SYS-REQ-082, SYS-REQ-086 и SYS-REQ-093; новый SYS-REQ не создаётся |
| C2-CAND-007 | DERIVED / COVERED | используется controlled cause/status distinction и существующая diagnostics/safety allocation |
| C2-CAND-008 | DERIVED / COVERED | используется controlled common time reference и IF-C2 |

## 3. Baseline decision

```text
NEW SYS-REQ: 0
NEW SAF-REQ: 0
RENUMBERING: 0
MASTER REGISTER: no new requirement identities
```

Existing authoritative requirement identities are preserved. The completed exact reconciliation is recorded in:

`01_REQUIREMENTS/SYSTEM/C2/MASTER_REQUIREMENTS_REGISTER_C2_ALLOCATION_001.md`

Ни один кандидат не переводится в baseline без доказанного functional gap и полного набора:

```text
source
→ applicability
→ responsibility
→ requirement
→ interface/design allocation
→ verification
→ evidence
```

## 4. Verification gate

Статус C2 verification остаётся `ALLOCATED / NOT VERIFIED`, пока соответствующие проверки фактически не выполнены и evidence не зарегистрировано.

Наличие verification procedure само по себе не является evidence of compliance.

## 5. Scope gate

Внешние объекты и обязанности не переводятся в BlueSky system requirements.

В частности, требования к физическим посадочным площадкам и их владельцам остаются вне system requirement boundary; они могут учитываться только в соответствующем external/operational documentation контуре, если это необходимо для конкретного certification object.

## 6. Controlled dependency action

Для оставшегося вопроса `C2-CAND-003` создан отдельный controlled placeholder:

`05_VERIFICATION/C2/C2_QUANTITATIVE_PARAMETERS_BASIS_001.md`

Он фиксирует требуемую структуру источника, applicability, параметров, порогов, метода измерения и ответственности, но **не содержит численных значений**.

Количественные C2 quality/QoS parameters остаются на `OPEN / DEFERRED` до появления и проверки authoritative/approved basis.

Это позволяет продолжать независимые подготовительные работы без фиктивного закрытия quantitative dependency.

## 7. Authoritative rule

`MASTER-REQUIREMENTS-REGISTER.md` остаётся единственным authoritative register идентификаторов требований. Рабочие C2 documents не создают вторую независимую базу требований.

## 8. Current controlled chain

```text
C2-CLAUSE-LEVEL-MAPPING-001
→ C2-REQUIREMENTS-RECONCILIATION-001
→ C2-SAFETY-ALLOCATION-001
→ IF-C2
→ C2-VERIFICATION-CASES-001
→ C2-VER-CFG-001
→ C2-VER-EXEC-001
→ C2-EVIDENCE-INDEX-001
→ real execution evidence
```

The clause, requirement, safety and interface pre-execution allocations are controlled. `C2-VER-EXEC-001` and `C2-EVIDENCE-INDEX-001` remain prepared controlled records; factual execution/evidence is not present.

## 9. Checkpoint

```text
CHECKPOINT C2-PREEXEC-001

CLAUSE MAPPING              CLOSED FOR CURRENT WORKING SET
REQUIREMENT RECONCILIATION  CLOSED FOR IDENTIFIED C2 SET
SAFETY ALLOCATION           CLOSED FOR PRE-EXECUTION ALLOCATION
INTERFACE ALLOCATION        CLOSED FOR CURRENT IF-C2 SCOPE
VERIFICATION CASES          DEFINED
CONFIGURATION               CONTROLLED STUB
EXECUTION                   DEFERRED
EVIDENCE                    EMPTY STUB
QUANTITATIVE CRITERIA       DEFERRED
```

## 10. Status

**CONTROLLED WORKING RECORD — C2 PRE-EXECUTION ALLOCATION CLOSED; REAL TEST EXECUTION, QUANTITATIVE CRITERIA AND EVIDENCE REMAIN DEFERRED TO TEST STAGE.**
