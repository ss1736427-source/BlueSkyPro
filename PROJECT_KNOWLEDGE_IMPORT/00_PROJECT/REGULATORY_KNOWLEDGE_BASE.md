---
id: REGULATORY-KNOWLEDGE-BASE-001
type: regulatory_knowledge_base
status: draft_for_agreement
purpose: certification_baseline
---

# Regulatory Knowledge Base

## 1. Purpose

Этот документ вводит нормативный слой проекта, потому что конечная цель проекта — подготовка документации и доказательств для сертификации.

Нормативный источник не считается автоматически применимым. Сначала определяется:

```text
Jurisdiction
→ Certification / Operational Path
→ Applicable Category
→ Certification Basis / Requirements
→ Means of Compliance
→ Evidence
→ Compliance Matrix
```

## 2. Critical project decision still required

Нужно формально зафиксировать целевой путь сертификации:

- юрисдикция;
- орган/authority;
- UAS category;
- тип предполагаемого approval;
- является ли BlueSky airborne software, ground/control software, operational support system или частью UAS;
- тип БВС;
- MTOM;
- intended operation;
- уровень риска/SAIL;
- требуется ли type/design approval.

До этого нормативные документы ниже имеют статус `CANDIDATE`, а не окончательно применимых требований.

## 3. Russian regulatory baseline — candidate

### ГОСТ Р 56122-2014

**Беспилотные авиационные системы. Общие требования.**

Росстандарт указывает, что стандарт содержит основные требования к БАС и их элементам при разработке, производстве, сертификации и эксплуатации гражданских БАС.

Статус: `CANDIDATE / REVIEW REQUIRED`.

### ГОСТ Р 59517-2021

**Беспилотные авиационные системы. Классификация и категоризация.**

Стандарт устанавливает классификацию и категории БАС для определения требований по обеспечению безопасности полёта и необходимого объёма сертификационных работ.

Статус: `CANDIDATE / HIGH PRIORITY`.

### ГОСТ Р 59518-2021

**Беспилотные авиационные системы. Порядок разработки.**

Определяет порядок и состав работ при разработке гражданских БАС.

Статус: `CANDIDATE / HIGH PRIORITY`.

### ГОСТ Р 59751-2021

**БАС с БВС самолётного типа. Требования к лётной годности.**

Применим к БВС самолётного типа с MTOM от 30 до 5700 кг.

Статус: `CONDITIONAL — ONLY IF AIRCRAFT TYPE AND MTOM MATCH`.

### ГОСТ Р 71996-2025

**Оценка рисков, связанных с эксплуатацией БАС специальной категории.**

Предназначен для оценки рисков специальной категории и может использоваться как основа доказательной документации безопасности.

Статус: `CANDIDATE / HIGH PRIORITY FOR SPECIAL CATEGORY`.

### ГОСТ Р 72691-2026

**БВС. Компоненты. Требования к навигационной системе.**

Стандарт действует с 15.08.2026 и непосредственно касается навигационной системы БВС.

Статус: `HIGH PRIORITY FOR NAVIGATION SCOPE`.

### ГОСТ Р 72692-2026

**Общие требования к системам информационного обеспечения БВС, не получающих диспетчерского обслуживания по эшелонированию.**

Действует с 15.08.2026.

Статус: `HIGH PRIORITY IF SYSTEM SCOPE MATCHES`.

### ГОСТ Р 72695-2026

**Встроенные средства контроля. Общие технические требования.**

Включает требования, связанные с контролем технического состояния, безопасностью функционирования и кибербезопасностью.

Статус: `CONDITIONAL / REVIEW REQUIRED`.

### ПНСТ 1049-2026

**Классификация и кодификация компонентов БАС и БВС.**

Предназначен, в том числе, для технических заданий, конструкторской и эксплуатационной документации.

Статус: `SUPPORTING / REVIEW REQUIRED`.

## 4. EU / EASA baseline — candidate

### Regulation (EU) 2019/947

Операции UAS.

Ключевой слой для operational authorisation и risk-based operation.

Для Specific category EASA указывает SORA как методику оценки риска.

Статус: `CANDIDATE / HIGH PRIORITY IF EU OPERATION`.

### Regulation (EU) 2019/945

UAS и third-country operators; product requirements and conformity framework.

Статус: `CANDIDATE / REVIEW REQUIRED`.

### Regulation (EU) 2024/1108

Устанавливает требования, при которых design, production and maintenance некоторых UAS должны быть certified, включая отдельные случаи certified/specific operations.

Статус: `HIGH PRIORITY IF EU CERTIFICATION PATH`.

### EASA SORA 2.5

В текущей редакции EASA Easy Access Rules 2026 включён SORA 2.5 как AMC к Article 11 для Specific category.

Статус: `HIGH PRIORITY FOR SPECIFIC CATEGORY`.

### EASA Light-UAS certification material

EASA публикует certification specifications / means of compliance для Light-UAS, включая high-risk SAIL V/VI material.

Статус: `CONDITIONAL — DEPENDS ON CERTIFICATION BASIS`.

## 5. Software assurance — conditional

### EASA AMC 20-115D / ED-12C / DO-178C

EASA признаёт ED-12C/DO-178C как acceptable means of compliance для software aspects of airborne systems/equipment в product certification/ETSO contexts.

Это **не означает автоматически**, что весь BlueSky должен разрабатываться по DO-178C.

Сначала определить:

```text
Is BlueSky airborne?
Is it part of certified equipment?
Does it perform a safety-significant airborne function?
What certification basis applies?
```

Статус: `CONDITIONAL / VERY HIGH PRIORITY TO CLASSIFY`.

### EASA AMC 20-152A / ED-80 / DO-254

Разработка airborne electronic hardware.

Статус: `CONDITIONAL — ONLY IF PROJECT INCLUDES CERTIFICATION OF AEH`.

### ED-79A / ARP4754-family system development

System development assurance may become relevant depending on certification basis and system role.

Статус: `CONDITIONAL / REVIEW REQUIRED`.

### Environmental / equipment qualification

ED-14 / DO-160 family may become relevant to airborne equipment.

Статус: `CONDITIONAL`.

## 6. Certification documentation layers we currently lack

The project currently needs explicit documents for:

1. Certification Strategy.
2. Certification Basis.
3. Applicability / Regulatory Register.
4. Compliance Matrix.
5. Means of Compliance Matrix.
6. System Requirements Specification.
7. Safety Requirements / Safety Assessment.
8. System Architecture Description.
9. Software Requirements Specification.
10. Software Design Description.
11. Software Development Plan.
12. Software Verification Plan.
13. Software Configuration Management Plan.
14. Software Quality Assurance Plan.
15. Requirements Traceability Matrix.
16. Verification / Test Evidence Index.
17. Hazard Log / Safety Case.
18. Configuration Index / Baseline.
19. Change / Problem Reports.
20. Interface Control Documents.
21. Operational / ConOps documentation.
22. Maintenance / continuing-airworthiness documentation where applicable.
23. Cybersecurity / information-security evidence where applicable.
24. Human factors / HMI evidence where applicable.
25. Flight-test / simulation evidence where applicable.
26. Declaration / certification application package.

Not all are automatically mandatory; applicability must be determined by certification path.

## 7. Existing project material that can feed certification

Existing project documents already provide useful source material for:

- mission requirements;
- route planning;
- WP model;
- UAV state and telemetry;
- multi-UAV coordination;
- dynamic return;
- collision avoidance;
- mission calculation;
- mission execution;
- map/dashboard;
- safety gate;
- AI authority boundaries;
- navigation state;
- navigation rules;
- navigation algorithm;
- verification model.

These should be transformed into certification-controlled artifacts rather than duplicated independently.

## 8. Required certification traceability

The project must support:

```text
REGULATION / STANDARD
→ CLAUSE / REQUIREMENT
→ PROJECT REQUIREMENT
→ SYSTEM REQUIREMENT
→ DESIGN
→ IMPLEMENTATION
→ TEST / ANALYSIS / INSPECTION
→ RESULT
→ COMPLIANCE EVIDENCE
```

For safety:

```text
HAZARD
→ SAFETY OBJECTIVE
→ SAFETY REQUIREMENT
→ MITIGATION
→ DESIGN
→ VERIFICATION
→ EVIDENCE
```

## 9. Source hierarchy

For certification work:

```text
Applicable law / regulation
→ Authority certification basis
→ Official certification specification
→ Approved / accepted means of compliance
→ Applicable standard
→ Project requirement
→ Design decision
→ Implementation
→ Test evidence
```

A model-generated interpretation is never treated as normative authority.

## 10. Current gaps

### REG-GAP-001
Target jurisdiction not formally fixed.

### REG-GAP-002
Certification vs operational authorisation path not fixed.

### REG-GAP-003
UAS category not formally fixed.

### REG-GAP-004
BlueSky certification item classification not fixed.

### REG-GAP-005
Certification basis not established.

### REG-GAP-006
Applicable standards have not been mapped clause-by-clause.

### REG-GAP-007
No controlled compliance matrix.

### REG-GAP-008
No controlled means-of-compliance matrix.

### REG-GAP-009
No formal certification requirements baseline.

### REG-GAP-010
No complete safety case / hazard traceability.

### REG-GAP-011
No certification-oriented software lifecycle evidence package.

### REG-GAP-012
No certification configuration baseline / evidence index.

## 11. Immediate priority

Before additional algorithm implementation:

```text
1. Fix certification target/path
2. Build Regulatory Source Register
3. Establish Certification Basis
4. Build Compliance Matrix
5. Classify BlueSky functions
6. Derive certification requirements
7. Connect requirements to current design
8. Build evidence/verification structure
```




