---
id: PROJECT-WORK-ORDER-001
type: project_work_order
status: controlled_working_order
system: BlueSky PRO
---

# BLUE SKY PRO —  Я Т

## 1. азначение

тот документ является единым рабочим порядком проекта BlueSky PRO.

н определяет последовательность выполнения работ от определения будущего продукта до формирования сертификационного пакета и выпуска контролируемой конфигурации продукта.

## 2. бязательное правило последовательности

ри работе над проектом BlueSky PRO необходимо постоянно обращаться к этому документу и выдерживать установленную последовательность.

ельзя переходить к следующему этапу, если для текущего этапа не определены входные данные, требуемые решения, необходимые документы, результаты и незакрытые вопросы с их статусом.

сли возникает новая задача, сначала определяется её место в настоящем рабочем порядке и только затем выполняется работа.

зменение настоящего порядка выполняется контролируемо и отражается в правилах проекта.

## 3. сновная цепочка

0. PROJECT CONTROL
→ 1. PRODUCT DEFINITION
→ 2. REGULATORY KNOWLEDGE
→ 3. CERTIFICATION PATH
→ 4. CERTIFICATION BASIS
→ 5. CONFIGURATION / APPLICABILITY
→ 6. COMPLIANCE MATRIX
→ 7. CERTIFICATION REQUIREMENTS BASELINE
→ 8. SYSTEM REQUIREMENTS
→ 9. SAFETY
→ 10. SYSTEM ARCHITECTURE
→ 11. SYSTEM DESIGN
→ 12. INTERFACE CONTROL
→ 13. SOFTWARE ENGINEERING
→ 14. VERIFICATION & VALIDATION
→ 15. EVIDENCE
→ 16. CONFIGURATION MANAGEMENT
→ 17. OPERATIONAL DOCUMENTATION
→ 18. CERTIFICATION PACKAGE
→ 19. RELEASE / PRODUCT BASELINE

## 4. тапы

### Т 0 — PROJECT CONTROL
оддерживать MASTER_DOCUMENT_INDEX, MASTER_REQUIREMENTS_REGISTER, BLUE_SKY_DECISIONS_LOG, CONFIGURATION_BASELINE, CHANGE_AND_PROBLEM_PROCESS и Certification Gap Register.

### Т 1 — PRODUCT DEFINITION
пределить авиационную систему, объект и границы, назначение, эксплуатационную концепцию, функции, роли, внешние системы и поддерживаемые конфигурации.

сновные документы:
- AVIATION_SYSTEM_DEFINITION
- CERTIFICATION_OBJECT_AND_SCOPE

### Т 2 — REGULATORY KNOWLEDGE
Собрать и классифицировать нормативные и официальные источники в REGULATORY_SOURCE_REGISTER.

### Т 3 — CERTIFICATION PATH
пределить юрисдикцию, certification object, certification target, применимую процедуру, границы сертификации, конфигурации и внешних участников.

### Т 4 — CERTIFICATION BASIS
Сформировать CERTIFICATION_BASIS:

Official Source → Clause → Applicability → Certification Requirement → Means of Compliance.

### Т 5 — CONFIGURATION / APPLICABILITY
оследовательно обработать применимость конфигураций и требований. езультаты внести в CONFIGURATION_A_APPLICABILITY_MATRIX, COMPLIANCE_MATRIX, CERTIFICATION_REQUIREMENTS_BASELINE и MASTER_REQUIREMENTS_REGISTER.

### Т 6 — COMPLIANCE MATRIX
ля каждого применимого пункта определить требование, применимость, ответственность BlueSky, MoC, verification, evidence, configuration и status.

### Т 7 — CERTIFICATION REQUIREMENTS BASELINE
ывести и зарегистрировать требования BlueSky из regulatory, certification, safety, system, interface, operational и derived sources.

### Т 8 — SYSTEM REQUIREMENTS
Сформировать SYSTEM_REQUIREMENTS_SPECIFICATION и полный набор системных требований с трассировкой.

### Т 9 — SAFETY
ести цепочку:

HAZARD → CAUSE / CONDITION → EFFECT → RISK → SAFETY OBJECTIVE → SAFETY REQUIREMENT → MITIGATION → DESIGN → VERIFICATION → EVIDENCE → RESIDUAL RISK → ACCEPTANCE.

сновные документы:
- HAZARD_LOG
- SAFETY_REQUIREMENTS_BASELINE
- SAFETY_CASE

### Т 10 — SYSTEM ARCHITECTURE
Сформировать SYSTEM_ARCHITECTURE_BASELINE: компоненты, границы ответственности, data/command/state flows, safety boundary, AI boundary, C2 boundary и hardware/software partition.

### Т 11 — SYSTEM DESIGN
Сформировать SYSTEM_DESIGN_DESCRIPTION и детализировать техническую реализацию.

### Т 12 — INTERFACE CONTROL
Сформировать INTERFACE_CONTROL_DOCUMENT и зафиксировать интерфейсы, форматы, единицы, системы координат, timing, validity, error handling, security, failure behaviour и verification.

### Т 13 — SOFTWARE ENGINEERING
осле стабилизации требований, safety и архитектуры выполнять основную реализацию : software requirements, architecture, design, coding standards, source/build configuration, dependencies, tests, reviews, static analysis и change records.

### Т 14 — VERIFICATION & VALIDATION
Requirement → Verification Method → Verification Case → Procedure → Execution → Result → Review.

етоды:
TEST, ANALYSIS, INSPECTION, SIMULATION, DEMONSTRATION, DESIGN REVIEW, DOCUMENT REVIEW, AUDIT, COMBINATION.

сновной документ:
- VERIFICATION_PLAN

### Т 15 — EVIDENCE
ормировать VERIFICATION_EVIDENCE_INDEX:

Requirement → Verification Case → Result → Evidence → Configuration → Review.

### Т 16 — CONFIGURATION MANAGEMENT
иксировать software version, hardware configuration, parameters, data, algorithms, documentation и test environment.

юбое изменение:

Change → Impact Assessment → Approval → Implementation → Verification → Evidence → Baseline Update.

### Т 17 — OPERATIONAL DOCUMENTATION
Сформировать руководства пилота и оператора, эксплуатационные, emergency, C2-loss, GNSS-loss, Dynamic Return, Multi-UAV, maintenance/support и training procedures.

### Т 18 — CERTIFICATION PACKAGE
ормировать только после закрытия предыдущих этапов:

Certification Basis + Compliance Matrix + Requirements Baseline + Safety Case + Hazard Log + Architecture + Design + Software Evidence + Verification Evidence + Configuration Baseline + Operational Documentation.

### Т 19 — RELEASE / PRODUCT BASELINE
афиксировать released configuration, документацию, software build, data/configuration, evidence, release record и обновить Master Document Index и Master Requirements Register.

## 5. ритерий перехода

аждый этап проходит:

INPUTS
→ WORK
→ DOCUMENTS
→ REVIEW
→ DECISION
→ BASELINE
→ NEXT STAGE

е допускается:
- создавать реализацию вместо отсутствующего требования;
- создавать verification вместо отсутствующего требования;
- считать mitigation доказанной без verification;
- считать requirement закрытым без evidence;
- переносить внешнее требование на BlueSky без applicability;
- считать неподтверждённый нормативный источник установленным certification basis;
- менять approved decision без документированного основания.

## 6. Текущая рабочая точка

REGULATORY SOURCE REGISTER
→ CERTIFICATION BASIS
→ CONFIGURATION / APPLICABILITY
→ COMPLIANCE MATRIX
→ CERTIFICATION REQUIREMENTS BASELINE
→ SYSTEM REQUIREMENTS

осле этого:
Safety → Architecture → Design → Interfaces → Software → Verification → Evidence → Configuration → Operational Documentation → Certification Package → Release.

## 7. равило каждой новой задачи

еред каждой новой рабочей задачей необходимо определить её место в настоящем порядке.

осле выполнения задачи необходимо зафиксировать:
- что сделано;
- какие документы созданы/изменены;
- какие требования добавлены/изменены;
- какие решения приняты;
- какие GAP закрыты;
- какая следующая задача является обязательной.

## 8. Статус

CONTROLLED_WORKING_ORDER

зменение последовательности допускается только документированным решением проекта.


## Execution control

Autonomous continuation: follow PROJECT_RULES autonomous continuation rule; stop only when user participation is required.
