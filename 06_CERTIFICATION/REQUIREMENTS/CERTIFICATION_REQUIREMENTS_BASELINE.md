---
id: CERTIFICATION-REQUIREMENTS-BASELINE-001
type: certification_requirements_baseline
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Certification Requirements Baseline

## 1. Purpose

Сформировать контролируемый набор требований авиационной системы, производных от утверждённой Certification Basis.

Этот документ является связующим уровнем:

```text
Regulatory Source
→ Clause
→ Compliance Record
→ Certification Requirement
→ System Requirement
→ Design
→ Verification
→ Evidence
```

## 2. Important status rule

Этот baseline **не содержит выдуманных нормативных требований**.

Пока clause-by-clause review официальных текстов не завершён, строки с `TBD` являются открытыми элементами.

Нельзя считать `TBD` закрытым требованием и нельзя объявлять соответствие только по названию ГОСТ.

## 3. Requirement ID convention

```text
CRB-SYS-xxx   System certification requirement
CRB-SAF-xxx   Safety requirement
CRB-NAV-xxx   Navigation
CRB-RTE-xxx   Route planning
CRB-WP-xxx    Waypoint
CRB-MIS-xxx   Mission
CRB-RET-xxx   Dynamic Return
CRB-COL-xxx   Collision Avoidance
CRB-C2-xxx    C2 / Communication
CRB-MUL-xxx   Multi-UAV
CRB-HMI-xxx   Human Interface
CRB-AI-xxx    AI-assisted functions
CRB-CFG-xxx   Configuration
CRB-SW-xxx    Software
CRB-HW-xxx    Hardware
CRB-OPS-xxx   Operations
```

## 4. Requirement record

Каждое требование должно содержать:

```text
Requirement ID
Source ID
Document
Edition / Status
Clause
Normative requirement
Applicability rationale
System allocation
Safety relevance
Derived project requirement
Verification method
Evidence
Configuration
Status
```

## 5. Baseline records

### CRB-SYS-001 — System definition

**Source:** Certification Strategy / Aviation System Definition

**Requirement:**

Авиационная система должна иметь определённые границы, функции, подсистемы, интерфейсы и allocation ответственности.

**Status:** OPEN

**Verification:** Design Review / Inspection

**Evidence:** Aviation System Definition + System Architecture

---

### CRB-SYS-002 — Requirements traceability

**Source:** Certification Strategy / Compliance Matrix

**Requirement:**

Каждое certification-relevant requirement должно быть трассируемо от нормативного источника до design, verification и evidence.

**Status:** OPEN

**Verification:** Inspection / Traceability Review

**Evidence:** Requirements Traceability Matrix

---

### CRB-SYS-003 — Configuration identification

**Source:** Certification Strategy / Compliance Matrix

**Requirement:**

Сертификационные evidence должны быть однозначно связаны с контролируемой конфигурацией системы.

**Status:** OPEN

**Verification:** Configuration Audit

**Evidence:** Configuration Baseline + Evidence Index

---

### CRB-SYS-004 — Change control

**Source:** Certification Strategy / Compliance Matrix

**Requirement:**

Изменения в certification-relevant системе, требованиях, software, configuration и verification evidence должны быть контролируемыми и трассируемыми.

**Status:** OPEN

**Verification:** Process Audit / Inspection

**Evidence:** Change and Problem Records

---

### CRB-SYS-005 — Verification evidence

**Source:** Certification Strategy / Compliance Matrix

**Requirement:**

Каждое закрываемое certification requirement должно иметь определённый и контролируемый способ подтверждения соответствия.

**Status:** OPEN

**Verification:** Review

**Evidence:** Compliance Matrix + Verification Evidence Index

---

## 6. Navigation baseline

### CRB-NAV-001 — Navigation State

Navigation subsystem должен иметь определённое состояние, включающее применимые actual, planned и derived values, а также quality/provenance information.

**Source:** REG-RU-009 — clause TBD

**Status:** OPEN

**Verification:** Analysis / Test / Review

---

### CRB-NAV-002 — Navigation data quality

Критические navigation inputs должны иметь контролируемый статус качества, включая valid/degraded/stale/invalid/unavailable.

**Source:** REG-RU-009 — clause TBD

**Status:** OPEN

**Verification:** Test

---

### CRB-NAV-003 — Reference frame

Directional/navigation quantities должны иметь явно определённый reference frame.

**Source:** REG-RU-009 — clause TBD

**Status:** OPEN

**Verification:** Analysis / Test

---

### CRB-NAV-004 — Course / Heading / Track distinction

Система не должна безусловно считать Course, Heading и Track взаимозаменяемыми величинами.

**Source:** Navigation Rules + regulatory applicability review

**Status:** OPEN

**Verification:** Analysis / Test

---

### CRB-NAV-005 — Navigation deviation

Система должна выявлять применимое отклонение фактического состояния от планового маршрута/сегмента.

**Source:** REG-RU-009 — clause TBD

**Status:** OPEN

**Verification:** Test / Simulation

---

## 7. Route / WP baseline

### CRB-RTE-001

Route должен иметь контролируемую идентичность/версию для сопоставления с Actual State.

**Source:** Project architecture + regulatory review

**Status:** OPEN

---

### CRB-WP-001

Active WP и переходы между WP должны быть однозначно определены и трассируемы.

**Source:** Project architecture + regulatory review

**Status:** OPEN

---

### CRB-RTE-002

Изменение route, которое влияет на safety или mission feasibility, должно инициировать установленную процедуру revalidation.

**Source:** Project runtime adaptation model

**Status:** OPEN

---

## 8. Safety baseline

### CRB-SAF-001

Safety-significant функции должны иметь определённые safety requirements и verification evidence.

**Source:** Certification Strategy / applicable safety basis

**Status:** OPEN

---

### CRB-SAF-002

Safety Gate не должен быть обходиться функцией оптимизации или AI proposal.

**Source:** Approved system architecture

**Status:** OPEN

**Verification:** Negative Test / Architecture Review

---

### CRB-SAF-003

AI-assisted proposal не должен автоматически становиться execution authority там, где требуется validation/authorization.

**Source:** Approved system architecture

**Status:** OPEN

**Verification:** Negative Test / Architecture Review

---

### CRB-SAF-004

Небезопасный или недостаточно подтверждённый результат должен быть способен блокировать выполнение в пределах установленной safety authority.

**Source:** Safety architecture

**Status:** OPEN

**Verification:** Safety Test

---

## 9. Dynamic Return baseline

### CRB-RET-001

Return feasibility должна оцениваться по текущему состоянию конкретного БВС и применимым ограничениям.

**Source:** Project Dynamic Return architecture + regulatory review

**Status:** OPEN

**Verification:** Analysis / Simulation / Test

---

### CRB-RET-002

Изменение энергии, ветра, groundspeed, communication или других safety-significant условий должно учитываться при повторной оценке return feasibility.

**Status:** OPEN

**Verification:** Scenario Test

---

## 10. C2 baseline

### CRB-C2-001

Состояние C2/communication должно быть частью системного состояния там, где оно влияет на mission или safety.

**Source:** REG-RU-008 — clause TBD

**Status:** OPEN

**Verification:** Test / Analysis

---

### CRB-C2-002

Потеря или деградация C2 должна иметь определённую реакцию системы.

**Source:** REG-RU-008 — clause TBD + system safety analysis

**Status:** OPEN

**Verification:** Failure Scenario Test

---

## 11. Multi-UAV baseline

### CRB-MUL-001

Navigation/mission state каждого БВС должен оставаться индивидуально идентифицируемым.

**Status:** OPEN

**Verification:** Integration Test

---

### CRB-MUL-002

Safety-significant coordination между БВС должна быть проверяема на уровне system behaviour.

**Status:** OPEN

**Verification:** Simulation / Integration Test

---

## 12. Software baseline

### CRB-SW-001

Software requirements должны быть трассируемы к system requirements.

**Status:** OPEN

**Verification:** Requirements Review

---

### CRB-SW-002

Software configuration, version и verification evidence должны быть связаны.

**Status:** OPEN

**Verification:** Configuration Audit

---

### CRB-SW-003

Применимость конкретного software assurance standard должна определяться certification basis и system allocation, а не предполагаться заранее.

**Status:** OPEN

**Verification:** Certification Review

---

## 13. Human / AI baseline

### CRB-HMI-001

Human interaction для safety-significant функций должна быть определена и проверяема.

**Status:** OPEN

**Verification:** HMI Review / Demonstration

---

### CRB-AI-001

AI-assisted функции должны иметь явно определённые границы полномочий.

**Status:** OPEN

**Verification:** Architecture Review / Negative Test

---

## 14. Configuration baseline

### CRB-CFG-001

Для certification evidence должна существовать идентифицируемая configuration baseline.

**Status:** OPEN

**Verification:** Audit

---

### CRB-CFG-002

Изменение baseline должно иметь change record и impact assessment.

**Status:** OPEN

**Verification:** Audit

---

## 15. Requirement derivation rule

Нормативное требование и проектное требование не являются одним и тем же.

```text
Normative Clause
        ↓
Applicability
        ↓
Interpretation
        ↓
Certification Requirement
        ↓
System Requirement
        ↓
Subsystem Requirement
```

Каждый переход должен быть документирован.

## 16. Verification allocation

Каждое требование получает один или несколько методов:

```text
TEST
ANALYSIS
INSPECTION
SIMULATION
DEMONSTRATION
REVIEW
COMBINATION
```

Метод выбирается по характеру требования.

## 17. Current baseline status

Состояние:

```text
STRUCTURE CREATED
NORMATIVE CLAUSE EXTRACTION PENDING
FINAL CERTIFICATION BASELINE NOT YET ESTABLISHED
```

Это намеренно консервативный статус.

## 18. Open gaps

```text
CRB-GAP-001  Certification item not finalized
CRB-GAP-002  UAS category not finalized
CRB-GAP-003  Certification path not finalized
CRB-GAP-004  Official normative texts not fully extracted
CRB-GAP-005  Clause-level applicability not completed
CRB-GAP-006  Safety classification not completed
CRB-GAP-007  Software/hardware assurance classification not completed
CRB-GAP-008  MoC allocation not finalized
CRB-GAP-009  Evidence identifiers not finalized
```

## 19. Next step

```text
CERTIFICATION REQUIREMENTS BASELINE
        ↓
SAFETY REQUIREMENTS / HAZARD LOG
        ↓
SYSTEM REQUIREMENTS
        ↓
TRACEABILITY MATRIX
        ↓
DESIGN ALLOCATION
        ↓
VERIFICATION EVIDENCE
```

## 20. Status

**DRAFT_FOR_AGREEMENT**


## C2 № 142 — CONTROLLED INTEGRATION
Source: Приказ Минтранса России от 24.04.2025 № 142.
Integration record: C2-WP142-CLAUSE-RECON-001.
Status: WORKING / NOT BASELINED.

Existing SYS-REQ identities are preserved. C2 allocation:
SYS-REQ-080, SYS-REQ-081, SYS-REQ-082, SYS-REQ-085, SYS-REQ-086, SYS-REQ-091, SYS-REQ-092, SYS-REQ-093.

Derived SRS records: SRS-C2-001..007.

Baseline gate:
authority в†’ applicability в†’ safety в†’ architecture в†’ verification в†’ evidence в†’ configuration.

No certification claim is made by this integration.
