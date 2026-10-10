---
id: CONFIGURATION-BASELINE-001
type: configuration_baseline
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Configuration Baseline

## 1. Назначение

Определить контролируемую конфигурацию авиационной системы BlueSky, чтобы requirements, design, software, data, verification и certification evidence всегда можно было однозначно связать с конкретным состоянием системы.

Главный принцип:

```text
CONFIGURATION
→ IDENTIFICATION
→ CONTROL
→ CHANGE
→ IMPACT ASSESSMENT
→ VERIFICATION
→ EVIDENCE
```

## 2. Configuration item

Каждый элемент, способный влиять на функцию, безопасность, verification или certification evidence, должен иметь идентификатор и контролируемую версию.

Кандидаты:

```text
SYSTEM
SUBSYSTEM
HARDWARE
SOFTWARE
CONFIGURATION
PARAMETERS
NAVIGATION DATA
MAP DATA
MISSION DATA
ROUTE / WP DATA
INTERFACE DEFINITION
REQUIREMENTS
DESIGN
TEST PROCEDURE
TEST ENVIRONMENT
TEST DATASET
SIMULATION MODEL
VERIFICATION RESULT
SAFETY ANALYSIS
CERTIFICATION DOCUMENT
```

Окончательный Configuration Item List утверждается отдельно.

## 3. Configuration identification

Минимальная структура идентификации:

```text
CI-ID
Name
Type
Version
Status
Parent
Baseline
Owner
Effective Date
Change Record
```

Для software дополнительно:

```text
Build ID
Commit / Source Revision
Compiler / Toolchain
Dependencies
Build Configuration
```

## 4. Baseline types

Используются следующие baseline:

```text
BL-REQ   Requirements Baseline
BL-SYS   System Baseline
BL-DES   Design Baseline
BL-SW    Software Baseline
BL-HW    Hardware Baseline
BL-DATA  Data Baseline
BL-VER   Verification Baseline
BL-SAF   Safety Baseline
BL-CERT  Certification Baseline
```

## 5. Initial certification baseline

На текущем этапе формируется:

```text
CERTIFICATION BASELINE 0.1
```

Состав:

```text
AVIATION_SYSTEM_DEFINITION.md
CERTIFICATION_STRATEGY.md
CERTIFICATION_BASIS.md
REGULATORY_SOURCE_REGISTER.md
COMPLIANCE_MATRIX.md
CERTIFICATION_REQUIREMENTS_BASELINE.md
SAFETY_REQUIREMENTS_BASELINE.md
HAZARD_LOG.md
SAFETY_CASE.md
REQUIREMENTS_TRACEABILITY_MATRIX.md
VERIFICATION_EVIDENCE_INDEX.md
```

Это **рабочая документационная baseline**, а не утверждённая authority certification baseline.

## 6. Existing technical baseline candidates

К существующим проектным материалам, которые должны войти в controlled configuration после проверки:

```text
NAVIGATION_KNOWLEDGE_MAP
NAVIGATION_KNOWLEDGE_REVIEW
NAVIGATION_STATE_MODEL
NAVIGATION_RULES
NAVIGATION_ALGORITHM
NAVIGATION_VERIFICATION_MODEL
```

Также должны быть идентифицированы:

```text
Route model
WP model
Dynamic Return
Collision Avoidance
Mission
Multi-UAV
C2
Safety Gate
AI authority
Map / Dashboard
```

## 7. Configuration status

Допустимые статусы:

```text
DRAFT
UNDER REVIEW
APPROVED
BASELINED
SUPERSEDED
WITHDRAWN
```

## 8. Baseline creation

Baseline создаётся только после:

```text
Content identified
→ Versions recorded
→ Dependencies identified
→ Review completed
→ Configuration ID assigned
→ Baseline approved
```

## 9. Change control

Любое изменение certification-relevant configuration проходит:

```text
Change Request
→ Impact Assessment
→ Approval
→ Implementation
→ Verification
→ Evidence Update
→ Baseline Update
```

Для safety-significant changes обязательно оценивать:

```text
Safety Impact
Certification Impact
Verification Impact
Evidence Impact
```

## 10. Configuration mismatch

Если verification выполняется не на той конфигурации, которая заявлена для результата:

```text
RESULT = INVALID FOR THAT CONFIGURATION
```

Далее:

```text
Mismatch
→ Impact Assessment
→ Re-test / Re-analysis
→ Correct Evidence
```

Нельзя исправлять mismatch только изменением описания evidence.

## 11. Software configuration

Для каждого controlled software build должны быть доступны:

```text
Software Version
Source Revision
Build ID
Build Date
Compiler
Compiler Version
Build Flags
Dependencies
Configuration Parameters
Test Environment
```

Конкретный набор полей будет уточнён требованиями выбранной certification basis.

## 12. Data configuration

Контролируются данные, способные влиять на safety или operational behaviour:

```text
Navigation Data
Map Data
Airspace / Zone Data
Mission Data
Route Data
WP Data
Vehicle Parameters
Performance Parameters
Environment / Weather Inputs
```

Для каждого набора:

```text
Data ID
Version
Source
Validity
Effective Date
Integrity / Checksum where applicable
Configuration
```

## 13. Test environment configuration

Verification evidence должна идентифицировать:

```text
Hardware
Software
OS / Runtime
Toolchain
Simulator
Models
Dataset
Parameters
Network / C2 configuration
External services
```

Если среда materially affects result, её состояние становится частью evidence configuration.

## 14. Configuration and traceability

Основная связь:

```text
Requirement
→ Design
→ Implementation
→ Configuration
→ Verification
→ Evidence
```

Обратная связь:

```text
Evidence
→ Configuration
→ Implementation
→ Design
→ Requirement
```

## 15. Configuration audit

Периодически проверять:

```text
CI exists
Version exists
Baseline assignment exists
Dependencies recorded
Change history exists
Verification linkage exists
Evidence linkage exists
```

Найденное расхождение получает Problem / Change Record.

## 16. Release gate

Новый certification-relevant baseline нельзя объявлять готовым без:

```text
All CIs identified
+
Versions frozen
+
Dependencies recorded
+
Changes reviewed
+
Required verification completed
+
Evidence linked
+
Baseline approved
```

## 17. Certification evidence preservation

Исторические baselines и evidence не удаляются из certification history.

При замене:

```text
OLD BASELINE
→ SUPERSEDED
→ NEW BASELINE
→ CHANGE RECORD
→ IMPACT ASSESSMENT
```

## 18. Configuration roles

Минимально должны быть определены роли:

```text
Configuration Owner
System Owner
Requirements Owner
Safety Owner
Verification Owner
Software Owner
Certification Documentation Owner
Approver
```

Один человек/роль может совмещать функции только если это допускается установленным process и не нарушает требуемую независимость verification.

## 19. Initial configuration IDs

```text
CFG-AVI-001   Aviation System
CFG-CERT-001  Certification Documentation
CFG-REQ-001   Requirements
CFG-SAF-001   Safety
CFG-NAV-001   Navigation
CFG-VER-001   Verification
CFG-DATA-001  Data
CFG-SW-001    Software
```

Эти IDs являются рабочими и могут быть заменены после утверждения configuration management scheme.

## 20. Current gaps

```text
CFG-GAP-001  Formal Configuration Item List not approved
CFG-GAP-002  Versioning scheme not fully approved
CFG-GAP-003  Baseline approval workflow not established
CFG-GAP-004  Software build reproducibility not established
CFG-GAP-005  Data configuration control not established
CFG-GAP-006  Verification environment control not established
CFG-GAP-007  Configuration audit process not established
CFG-GAP-008  Roles and responsibilities not formally assigned
CFG-GAP-009  Certification baseline not authority-approved
```

## 21. Next document

Следующий документ:

`CHANGE_AND_PROBLEM_PROCESS.md`

Он определит управляемую процедуру изменений, дефектов, несоответствий и их влияния на безопасность, требования, verification и certification evidence.

## 22. Status

**DRAFT_FOR_AGREEMENT**



