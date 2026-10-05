---
id: REPOSITORY-REQUIREMENTS-AUDIT-001
type: repository_requirements_audit_report
status: controlled_working_report
system: BlueSky PRO
jurisdiction: Russian Federation
date: 2026-09-04
basis:
  - MASTER-REQUIREMENTS-REGISTER-001
  - REQUIREMENTS-INVENTORY-001
  - REQUIREMENTS-CONSOLIDATION-REPORT-001
  - REQUIREMENTS-RECONCILIATION-DECISION-001
  - REGULATORY-SOURCE-REGISTER-001
  - REGULATORY-CLAUSE-MAPPING-001
  - C2-TEXT-RECONCILIATION-001
  - C2-REG-BASELINE-001
---

# BLUE SKY PRO — REPOSITORY REQUIREMENTS / CONTROLLED WORDING AUDIT REPORT 001

## 1. Назначение

Проверить фактическое состояние структуры репозитория BlueSky-PRO-Knowledge перед следующим проходом по controlled wording, source/clause verification и полной трассировке требований.

Цель проверки — установить, где уже имеется достаточная controlled база, где присутствует только предварительное/derived содержание, и какие элементы нельзя переводить в финальный статус без дополнительного извлечения первичного текста или подтверждения источника.

Документ не изменяет идентичность требований и не является certification basis.

## 2. Проверенный репозиторий

Репозиторий:

`ss1736427-source/BlueSky-PRO-Knowledge`

Default branch:

`main`

На момент проверки репозиторий содержит, в частности, следующие контролируемые разделы:

```text
COMPLIANCE
REGULATORY
REGULATORY/C2
SAFETY
SAFETY/C2
SYSTEM
SYSTEM/C2
TRACEABILITY
TRACEABILITY/C2
```

Также присутствуют `PACKAGE_MANIFEST.md` и `README.md`.

Структура подтверждает, что regulatory, system, safety, compliance и traceability уже разделены по отдельным уровням, а C2 выделен в отдельные рабочие контуры.

## 3. Главный результат аудита

Текущая структура пригодна для продолжения работы, однако **полный controlled wording ещё не закрыт**.

Ключевое ограничение установлено непосредственно существующими файлами: `REQUIREMENTS_INVENTORY.md` содержит идентификаторы и места их появления, но сам по себе не является источником полного controlled wording каждого `SYS-REQ`.

Следовательно:

```text
ID inventory
≠
exact controlled requirement text
```

Поэтому окончательная замена предварительных `SUPPORTING/PENDING_WORDING` допустима только после извлечения исходной записи требования и её сопоставления с нормативным положением.

## 4. Master Requirements Register

`SYSTEM/MASTER_REQUIREMENTS_REGISTER.md` определён как authoritative register идентичности требований.

Зафиксировано правило:

```text
ONE REQUIREMENT
→ ONE STABLE ID
→ ONE CONTROLLED WORDING
→ MANY RELATIONSHIPS
```

Существующие `SYS-REQ-*` и `SAF-REQ-*` сохраняются. Новые candidate IDs не должны автоматически становиться самостоятельными baseline requirements.

Это решение подтверждено как в Master Register, так и в существующих reconciliation documents.

### Аудитный вывод

**KEEP:** authoritative identity model.

**НЕ ЗАКРЫТО:** exact wording inventory.

## 5. Requirements Inventory

В `SYSTEM/REQUIREMENTS_INVENTORY.md` зафиксировано:

```text
Unique requirement IDs found: 99
ID occurrences: 399
Duplicate IDs across files: 73
```

Инвентарь показывает множественные появления одних и тех же ID в SRS, master register, traceability, compliance и других документах.

Это не обязательно ошибка: один requirement должен иметь множество relationships. Ошибкой было бы наличие разных независимых текстов, выдаваемых за authoritative wording одного ID.

### Критическое замечание

Сам inventory содержит ссылки на пути вида:

`00_PROJECT\\...`

в то время как актуальная структура доступного репозитория использует отдельные директории верхнего уровня (`SYSTEM`, `TRACEABILITY`, `REGULATORY`, `SAFETY`, `COMPLIANCE` и т.д.).

Это указывает на необходимость отдельной проверки актуальности path references при следующей синхронизации документации.

## 6. Requirements Consolidation

`TRACEABILITY/REQUIREMENTS_CONSOLIDATION_REPORT.md` устанавливает правильную модель:

```text
existing SYS-REQ
+
new candidate families
→ duplicate / overlap / parent-child / source / safety / architecture / verification analysis
```

Candidate families включают:

```text
NAV-REQ-*
RTE-REQ-*
WP-REQ-*
MIS-REQ-*
RET-REQ-*
COL-REQ-*
C2-REQ-*
MUL-REQ-*
RDY-REQ-*
SAF-REQ-*
AUTH-REQ-*
HMI-REQ-*
AI-REQ-*
DATA-REQ-*
CFG-REQ-*
SW-REQ-*
HW-REQ-*
```

Они пока не должны рассматриваться как независимая baseline database.

## 7. Safety / authority reconciliation

Уже установленная архитектурная цепочка:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

и приоритет:

```text
SAFETY
>
MANDATORY SYSTEM CONSTRAINTS
>
MISSION REQUIREMENTS
>
OPERATOR / USER PREFERENCES
>
OPTIMIZATION
```

Поэтому `SAF-REQ-019`, `SAF-REQ-020`, `AUTH-REQ-001`, `AUTH-REQ-002`, `AI-REQ-002` пока корректно остаются candidate derived/clarifying records.

### Вывод

Не создавать параллельную ветку authority requirements.

## 8. C2 controlled reconciliation

В текущем C2-контуре имеются:

```text
C2_CLAUSE_LEVEL_MAPPING_001.md
C2_TEXT_RECONCILIATION_001.md
C2_REGULATORY_REQUIREMENTS_BASELINE_001.md
C2_GAP_CLOSURE_PASS_001.md
C2_REQUIREMENTS_RECONCILIATION_001.md
C2_TRACEABILITY_BASELINE_001.md
```

`C2-TEXT-RECONCILIATION-001` уже выполнил предварительное сопоставление C2 candidates с существующими requirements.

Текущая классификация:

| Candidate | Current disposition |
|---|---|
| C2-REQ-001 | DERIVED / OVERLAP |
| C2-REQ-002 | MERGE / DERIVED |
| C2-REQ-003 | PARTIAL / GAP CHECK |
| C2-REQ-004 | DERIVED |
| C2-REQ-005 | DERIVED / OVERLAP |
| C2-REQ-006 | MERGE CANDIDATE |
| C2-REQ-007 | GAP CANDIDATE |
| C2-REQ-008 | PARTIAL / GAP CHECK |
| C2-REQ-009 | MERGE / DERIVED |
| C2-REQ-010 | GAP CANDIDATE |

Это ещё не финальная wording closure.

## 9. C2 existing allocation

Существующие system IDs, уже выделенные для C2, включают:

```text
SYS-REQ-080
SYS-REQ-081
SYS-REQ-082
SYS-REQ-085
SYS-REQ-086
SYS-REQ-091
SYS-REQ-092
SYS-REQ-093
```

При этом `SAF-REQ-013` уже обеспечивает явную safety-связь для C2 degradation, а `DEC-012` и `IF-C2-001` используются для provider boundary/interface model.

### Вывод

C2 candidate requirements нельзя механически переводить в новые SYS-REQ.

Сначала требуется:

```text
existing exact wording
→ candidate wording
→ source clause
→ applicability
→ allocation
→ verification
```

## 10. Regulatory source register

`REGULATORY/REGULATORY_SOURCE_REGISTER.md` устанавливает обязательное разделение:

```text
DIRECT
INTERFACE
CONDITIONAL
EXTERNAL
REG-GAP / WATCH
```

И правило:

```text
Source
→ Clause
→ Regulatory Object
→ BlueSky Scope
→ Applicability
→ Requirement / Constraint
→ Verification / Evidence
```

Это является корректной основой для controlled wording review.

## 11. Regulatory clause mapping

`REGULATORY/REGULATORY_CLAUSE_MAPPING_001.md` содержит уже проверенные записи по:

- Воздушному кодексу РФ, статья 37;
- статье 8 и анализу порога 30 кг;
- перечню применимых/условно применимых норм лётной годности;
- текущей C2 mapping boundary.

В документе явно установлено:

```text
Official Source
→ Clause
→ Applicability
→ Certification Requirement
→ System Requirement
→ Compliance Method
→ Evidence
```

Если source/clause не подтверждён, запись остаётся `TBD` / `APPLICABILITY_PENDING`.

## 12. C2 regulatory baseline

`C2_REGULATORY_REQUIREMENTS_BASELINE_001.md` содержит рабочий набор `C2-REG-001..010`.

Критически важно, что документ сам запрещает выдумывать универсальный numerical timeout для loss-of-C2. Параметр должен быть выведен из operational requirements, safety analysis и характеристик системы/БВС.

Следовательно, численные параметры пока не должны заменяться произвольными значениями.

## 13. Controlled wording status

### Уже достаточно определено на уровне формулировочного правила

1. Master Register является источником identity.
2. SRS является представлением, а не второй базой.
3. Regulatory source не переносится в SYS-REQ без applicability analysis.
4. Existing IDs сохраняются.
5. Candidate IDs не baseline автоматически.
6. Verification требует acceptance criteria и evidence.
7. Architecture decision не равна verified requirement.
8. AI output не является автоматически authoritative state или execution authority.
9. External provider responsibilities не переносятся в BlueSky requirements без соответствующего scope.

### Требует exact wording closure

1. Полный текст всех затронутых `SYS-REQ-*`.
2. Полный текст затронутых `SAF-REQ-*`.
3. Точная wording C2 candidates после source/clause reconciliation.
4. Точные C2 timing parameters.
5. Pilot indication requirement.
6. Event traceability requirement allocation.
7. Common time-base semantics.
8. HMI warning/indication requirement allocation.
9. Verification acceptance criteria.
10. Evidence identifiers.

## 14. Источники: текущий статус

| Домен | Состояние | Решение |
|---|---|---|
| ВК РФ ст. 37 | VERIFIED SOURCE в проектном mapping | использовать после applicability decision |
| ВК РФ ст. 8 / ≤30 кг | VERIFIED SOURCE в проектном mapping | отдельный applicability analysis |
| Перечень норм лётной годности Росавиации | VERIFIED SOURCE LIST | не считать запись автоматическим applicability |
| Приказ №142 / C2 | WORKING clause-level mapping | продолжить exact clause verification |
| №140 | EXTERNAL / CONDITIONAL | не переносить физические требования площадки в SYS-REQ |
| №312 | EXTERNAL / INTERFACE / CONDITIONAL | выделять только подтверждённую BlueSky-side function |
| №367 | EXTERNAL / CONDITIONAL | не принимать требования ТО как BlueSky requirements без scope |
| №354 | EXTERNAL / INTERFACE / CONDITIONAL | отделять provider equipment от BlueSky boundary |

## 15. Основные обнаруженные проблемы

### AUD-001 — Exact wording gap

Inventory не заменяет первичный controlled wording.

**Статус:** OPEN.

### AUD-002 — Candidate/authoritative смешение в представлениях

Некоторые candidate IDs встречаются в нескольких документах и поэтому визуально выглядят как уже существующие requirements.

**Статус:** CONTROLLED BY MASTER REGISTER, но требует дальнейшей синхронизации.

### AUD-003 — Path references

Часть inventory/reconciliation записей ссылается на старую структуру `00_PROJECT\\...`, тогда как актуальный repository tree использует новые каталоги.

**Статус:** OPEN — требуется path normalization.

### AUD-004 — Verification closure

Наличие `TEST-*` references ещё не доказывает equivalence и coverage каждого требования.

**Статус:** OPEN.

### AUD-005 — Certification wording

Рабочие документы правильно отделяют regulatory source от system requirement, но окончательный compliance claim невозможен без source/clause/applicability/verification/evidence closure.

**Статус:** OPEN.

### AUD-006 — Numerical parameters

Численные safety/C2 thresholds не должны быть изобретены для закрытия таблицы.

**Статус:** CONTROLLED / ACCEPTED RULE.

## 16. Аудитный gate

```text
REPOSITORY STRUCTURE             PASS
MASTER IDENTITY MODEL            PASS
REQUIREMENTS INVENTORY           PASS / INCOMPLETE FOR WORDING
CONSOLIDATION MODEL              PASS
SAFETY AUTHORITY MODEL           PASS
C2 ALLOCATION                    PARTIAL
REGULATORY SOURCE REGISTER       PASS / WORKING
CLAUSE-LEVEL REGULATORY MAPPING  PARTIAL
EXACT CONTROLLED WORDING         OPEN
VERIFICATION COVERAGE            OPEN
EVIDENCE CLOSURE                 OPEN
CERTIFICATION BASELINE           NOT ESTABLISHED
```

## 17. Следующая операция

Следующий controlled pass должен быть выполнен не созданием новых документов требований, а извлечением фактических записей из repository:

```text
ALL authoritative SYS-REQ records
        ↓
exact Requirement Text
        ↓
source / basis
        ↓
linked SAF-REQ
        ↓
linked ARCH-DEC
        ↓
linked IF / design
        ↓
linked TEST / verification
        ↓
linked evidence
        ↓
candidate comparison
        ↓
KEEP / DERIVED / MERGE / GAP / CONFLICT
```

После этого только затронутые записи Master Register должны получить controlled wording update.

## 18. Финальный вывод текущего прохода

Репозиторий уже содержит необходимую логическую основу для controlled requirements process. Главный незакрытый участок — **не структура и не архитектурная концепция, а точное извлечение и окончательное сопоставление controlled wording с первичными источниками и verification evidence**.

Поэтому следующий этап следует выполнять как:

**EXACT WORDING + SOURCE/CLAUSE + APPLICABILITY + TRACEABILITY CLOSURE**.

До завершения этого этапа требования не переводятся в окончательный certification baseline только на основании наличия их ID в нескольких документах.

## 19. Статус отчёта

```text
REPOSITORY AUDIT: COMPLETED
CONTROLLED WORDING AUDIT: PARTIAL
SOURCE/CLAUSE CLOSURE: PARTIAL
BASELINE: NOT ESTABLISHED
NEXT ACTION: EXACT REQUIREMENT TEXT EXTRACTION + RECONCILIATION
```
