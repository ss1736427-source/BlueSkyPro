---
id: CERTIFICATION-STRATEGY-001
type: certification_strategy
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Certification Strategy

## 1. Purpose

Определить стратегию подготовки авиационной системы к сертификации в Российской Федерации.

Документ является верхним уровнем сертификационного планирования. Он не заменяет Certification Basis и не объявляет отдельный стандарт автоматически обязательным.

## 2. Approved foundations

Зафиксировано:

- проект разрабатывает авиационную систему;
- основная юрисдикция — Российская Федерация;
- сертификационный подход начинается с системного уровня;
- знания являются основанием требований, правил и алгоритмов;
- требования должны быть трассируемыми до design, verification и evidence;
- противоречия между источниками выявляются и разрешаются явно;
- внешние источники используются для проверки и сравнения;
- окончательная применимость нормативного требования определяется после установления certification path.

## 3. Certification objective

Цель:

```text
Авиационная система
→ Applicable Regulatory Requirements
→ Certification Basis
→ System Requirements
→ Safety Requirements
→ Design
→ Implementation
→ Verification
→ Compliance Evidence
→ Certification Package
```

До завершения этой цепочки проект не объявляется сертификационно соответствующим.

## 4. Primary jurisdiction

**Российская Федерация.**

Основными источниками для определения требований являются:

1. законодательство Российской Федерации;
2. нормативные акты авиационной области;
3. применимые федеральные авиационные правила;
4. официально установленная certification basis;
5. применимые национальные стандарты;
6. иные документы, признанные применимыми органом сертификации.

Иностранные источники могут использоваться как:

- сравнительные;
- технические;
- explanatory;
- дополнительные evidence references,

но не заменяют российскую нормативную основу без отдельного согласования.

## 5. Certification object

На утверждённом системном уровне объект рассматривается как:

**авиационная система BlueSky.**

Точный состав сертифицируемого изделия/системы, включая физическую границу, распределение airborne/ground functions и состав оборудования, требует отдельной классификации.

## 6. Certification path — current status

Пока не утверждены:

- конкретный вид approval;
- категория БАС;
- класс/категория эксплуатации;
- MTOM и тип БВС;
- точный certification basis;
- роль BlueSky относительно БВС/БАС;
- состав функций, входящих в certification item;
- ответственные организации и роли.

Поэтому эти параметры имеют статус:

**OPEN / TO BE DETERMINED**

## 7. Regulatory baseline candidates

Кандидаты для первичного анализа:

- ГОСТ Р 56122-2014 — общие требования к БАС;
- ГОСТ Р 59517-2021 — классификация и категоризация БАС;
- ГОСТ Р 59518-2021 — порядок разработки БАС;
- ГОСТ Р 71996-2025 — оценка рисков специальной категории;
- ГОСТ Р 72691-2026 — требования к навигационной системе БВС;
- ГОСТ Р 72692-2026 — системы информационного обеспечения БВС;
- ГОСТ Р 72695-2026 — встроенные средства контроля;
- ГОСТ Р 72688-2026 — мониторинг состояния C2 и киберзащита;
- ГОСТ Р 72683-2026 — идентификация и прослеживаемость компонентов;
- ПНСТ 1049-2026 — классификация и кодификация компонентов;
- применимые ФАП и требования к сертификации авиационной техники.

Каждый документ должен пройти отдельную проверку применимости и постатейное извлечение требований.

## 8. Functional certification scope

Кандидаты функций, которые необходимо классифицировать:

### Navigation

- navigation state;
- position;
- course / heading / track;
- wind / drift;
- route deviation;
- navigation quality;
- navigation source handling.

### Route Planning

- route generation;
- WP management;
- route validation;
- route modification;
- 2D/3D representation.

### Safety

- validation;
- readiness;
- safety gate;
- safety-related blocking;
- hazard handling.

### Mission

- mission calculation;
- mission execution;
- runtime adaptation;
- dynamic return;
- mission state.

### Multi-UAV

- coordination;
- individual UAV state;
- separation / collision avoidance;
- resource allocation.

### C2

- communication state;
- communication degradation;
- C2-related safety responses.

### Human interaction

- operator interface;
- map;
- dashboard;
- alerts;
- authorization interaction.

### AI-assisted functions

- analysis;
- proposal generation;
- recommendation;
- bounded automation.

AI is not assumed to be execution authority.

## 9. Safety strategy

Safety shall be established independently from optimisation.

```text
Hazard
→ Risk
→ Safety Objective
→ Safety Requirement
→ Mitigation
→ Implementation
→ Verification
→ Evidence
```

System execution boundary:

```text
Navigation / Mission Proposal
→ Validation
→ Readiness
→ Safety Gate
→ Authorization
→ C++ Core
→ Execution
```

## 10. Requirements strategy

Every certification-relevant requirement must have:

```text
Requirement ID
Source
Clause
Applicability
Requirement Text / Controlled Interpretation
System Allocation
Verification Method
Evidence
Status
```

No requirement is considered closed until evidence is identified and the applicable authority/acceptance process is satisfied.

## 11. Means of Compliance strategy

Potential methods:

```text
TEST
ANALYSIS
INSPECTION
SIMULATION
DEMONSTRATION
REVIEW
COMBINATION
```

The selected method must be justified for the specific requirement.

## 12. Software strategy

Software assurance requirements depend on the final certification classification and system allocation.

Do not automatically impose DO-178C or another foreign software standard on the entire project.

First determine:

```text
System Function
→ Safety Significance
→ Certification Item
→ Applicable Basis
→ Required Assurance
→ Applicable Software Standard / MoC
```

## 13. Configuration and evidence

Certification evidence must be tied to controlled configuration:

```text
System Baseline
→ Software Version
→ Configuration
→ Requirements Baseline
→ Test Environment
→ Test Result
→ Evidence
```

Changes must be traceable.

## 14. Required certification documentation

The target documentation set includes:

```text
CERTIFICATION_STRATEGY
CERTIFICATION_BASIS
REGULATORY_SOURCE_REGISTER
COMPLIANCE_MATRIX
MEANS_OF_COMPLIANCE_MATRIX
CERTIFICATION_REQUIREMENTS_BASELINE
SYSTEM_REQUIREMENTS
SAFETY_REQUIREMENTS
SAFETY_CASE
HAZARD_LOG
SYSTEM_ARCHITECTURE
SOFTWARE_REQUIREMENTS
SOFTWARE_DESIGN
VERIFICATION_PLAN
VERIFICATION_EVIDENCE_INDEX
TRACEABILITY_MATRIX
CONFIGURATION_BASELINE
CHANGE_AND_PROBLEM_RECORD
INTERFACE_CONTROL_DOCUMENTS
OPERATIONAL_DOCUMENTATION
CERTIFICATION_GAP_REGISTER
```

Applicability of each artifact will be determined by the certification path.

## 15. Immediate work sequence

```text
1. Certification Strategy
2. Certification Basis
3. Regulatory Source Register
4. Applicability Matrix
5. Certification Requirements Baseline
6. System Requirements
7. Safety / Hazard Analysis
8. Compliance Matrix
9. Means of Compliance
10. Architecture / Design allocation
11. Verification
12. Evidence
13. Certification package
```

## 16. Current certification gaps

### CERT-GAP-001
Certification authority/path not yet formally selected.

### CERT-GAP-002
UAS category not fixed.

### CERT-GAP-003
Certification item boundary not fixed.

### CERT-GAP-004
System safety classification not completed.

### CERT-GAP-005
Certification basis not established clause-by-clause.

### CERT-GAP-006
No controlled regulatory source register.

### CERT-GAP-007
No controlled compliance matrix.

### CERT-GAP-008
No complete hazard/safety case.

### CERT-GAP-009
No certification-controlled requirements baseline.

### CERT-GAP-010
No certification evidence index.

## 17. Decision principle

Нормативное требование не должно появляться в проекте только потому, что оно «обычно используется».

Для каждого требования:

```text
SOURCE
→ APPLICABILITY
→ REQUIREMENT
→ ALLOCATION
→ MoC
→ EVIDENCE
```

## 18. Status

**DRAFT_FOR_AGREEMENT**

Следующий документ:

`CERTIFICATION_BASIS.md`

Он будет сформирован после постатейного анализа применимых российских источников и определения границ certification item.
