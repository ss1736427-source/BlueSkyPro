---
id: CERTIFICATION-OBJECT-AND-SCOPE-001
type: certification_object_and_scope
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Certification Object and Scope

## 1. Назначение

Зафиксировать рабочее определение объекта сертификации, границ сертификационного scope и продуктовой архитектуры BlueSky.

Документ является исходной точкой для:

```text
CERTIFICATION_BASIS
COMPLIANCE_MATRIX
SYSTEM_REQUIREMENTS
SAFETY
ARCHITECTURE
VERIFICATION
CONFIGURATION
```

## 2. Основной принцип

BlueSky разрабатывается как **платформенная авиационная система**, предназначенная для интеграции с беспилотными авиационными системами различных типов и конфигураций.

BlueSky не следует заранее определять как систему только одного типа БПЛА или только одной массы.

При этом сертификация осуществляется не по заявлению об «универсальности», а по конкретному заявленному certification scope и конкретным конфигурациям.

## 3. Рабочее определение объекта

Рабочее определение:

> **Объект сертификации проекта — беспилотная авиационная система нового типа и/или её сертифицируемый элемент, в состав которого входит разрабатываемая авиационная система BlueSky. Точный certification scope, состав типовой конструкции и перечень сертифицируемых элементов устанавливаются отдельно для заявленной конфигурации на основании применимого российского регулирования, категории БАС, максимальной взлётной массы, назначения и условий эксплуатации.**

Это определение является рабочим и не заменяет официального определения объекта органом сертификации.

## 4. System Under Development

```text
BlueSky Aviation System
```

является разрабатываемой авиационной системой.

Она включает, в зависимости от конфигурации:

```text
Navigation
Route Planning
Mission Management
Readiness
Safety / Safety Gate
Authorization Interface
C2
Collision Avoidance
Multi-UAV Functions
HMI
Data Management
Configuration Management
AI-assisted Functions
Verification Interfaces
```

Конкретный состав функций конкретной конфигурации является частью configuration baseline.

## 5. Product family

BlueSky проектируется как product family:

```text
                    BLUESKY
                       │
             UNIVERSAL AVIATION SYSTEM
                       │
          ┌────────────┴────────────┐
          │                         │
     COMMON CORE              CONFIGURATION
          │                         │
   common functions          aircraft-specific
   common interfaces         integration
   common safety             limitations
   common verification       equipment
          │                   parameters
          └────────────┬────────────┘
                       ↓
              конкретная конфигурация
```

## 6. Aircraft / UAV type independence

На уровне архитектуры предусматривается возможность применения с различными типами БПЛА, в том числе:

```text
Multirotor
Fixed-wing
VTOL
Other applicable configurations
```

Это является **архитектурной целью**, а не утверждением о наличии certification approval для каждого типа.

Для каждой конфигурации должны быть определены:

```text
Aircraft type
Flight characteristics
Flight-control interface
Navigation equipment
C2 equipment
Energy model
Sensors
Actuators
Environmental limitations
Operational limitations
Applicable safety constraints
Verification configuration
```

## 7. MTOM strategy

Для product strategy выделяются конфигурационные профили:

```text
PROFILE-A
БАС до 30 кг включительно

PROFILE-B
БАС свыше 30 кг

PROFILE-C
Дополнительные категории / конфигурации,
если они применимы к заявленному scope
```

Порог 30 кг используется как **регуляторно-стратегическая граница**, а не как архитектурное ограничение BlueSky.

Конкретная применимость обязательной сертификации определяется действующим российским законодательством и применимым certification regime.

## 8. Initial market / regulatory profile

Первичный профиль:

```text
PROFILE-A
БАС до 30 кг включительно
```

Цель:

```text
Быстрый и управляемый выход
на применимый рынок
```

при сохранении архитектурной возможности последующего расширения.

Это не означает автоматического распространения какого-либо статуса или разрешения с одной конфигурации на другую.

## 9. >30 kg strategy

Для:

```text
PROFILE-B
БАС свыше 30 кг
```

архитектура и development process должны изначально обеспечивать возможность последующего certification scope expansion.

До определения конкретного certification basis нельзя утверждать:

```text
что существующая сертификация ≤30 кг автоматически покрывает >30 кг;
что все функции BlueSky будут сертифицируемыми без дополнительных ограничений;
что один комплект evidence будет достаточен для всех конфигураций.
```

Стратегия:

```text
COMMON CORE
+
CONFIGURATION-SPECIFIC EVIDENCE
+
AIRCRAFT-SPECIFIC INTEGRATION
+
APPLICABLE CERTIFICATION BASIS
```

## 10. Common Core

К потенциально общим функциям относятся:

```text
Navigation abstraction
Route / mission abstraction
State management
Safety architecture
Safety Gate
Authorization boundary
C2 abstraction
Data integrity mechanisms
Configuration management
Verification infrastructure
Observability
```

Для каждой функции требуется определить, какая часть действительно может быть common across configurations.

## 11. Aircraft-specific layer

Конфигурационно-зависимыми могут быть:

```text
Flight-control interfaces
Sensors
Actuators
Energy / endurance model
Aircraft dynamics
Mass properties
Performance limits
Navigation equipment
C2 equipment
Environmental limits
Operational limitations
Aircraft-specific safety constraints
```

Эти элементы должны быть отделены от common core архитектурно и документально.

## 12. Certification scope

Certification scope должен определяться отдельно для каждой заявленной конфигурации.

Минимальная структура:

```text
Certification Object
→ Product / Model
→ Configuration
→ Aircraft Type
→ MTOM
→ Intended Operation
→ Operating Environment
→ Included Functions
→ Included Equipment
→ Exclusions
→ Applicable Certification Basis
→ Compliance Evidence
```

## 13. Configuration principle

Одна и та же BlueSky platform может иметь несколько configuration baselines:

```text
BLUESKY-CONFIG-A
BLUESKY-CONFIG-B
BLUESKY-CONFIG-C
```

Каждая конфигурация должна иметь однозначную связь:

```text
Aircraft
+
BlueSky Version
+
Hardware
+
Software
+
Interfaces
+
Parameters
+
Data
+
Verification Evidence
```

## 14. Certification evidence strategy

Общий принцип:

```text
COMMON REQUIREMENT
→ COMMON EVIDENCE WHERE VALID

AIRCRAFT-SPECIFIC REQUIREMENT
→ AIRCRAFT-SPECIFIC EVIDENCE

CONFIGURATION CHANGE
→ IMPACT ASSESSMENT
→ ADDITIONAL / REGRESSION VERIFICATION
```

Нельзя предполагать переносимость evidence без анализа applicability.

## 15. Certification boundary

Не считать автоматически частью certification object:

```text
Knowledge Base
Engineering notes
Development tools
Research artefacts
Non-deployed simulation tools
Internal management documents
```

если они не входят в заявленный certification scope.

При этом они могут являться частью certification evidence / development assurance records, если применимо.

## 16. Knowledge boundary

Navigation Knowledge:

```text
Original Source
→ Specialized Knowledge Base
→ Navigation Knowledge Map
→ Engineering Rule
→ Requirement
→ Design
→ Verification
```

Knowledge Base не является сама по себе certification object.

## 17. Regulatory boundary

Regulatory source:

```text
Official Russian Source
→ Applicability
→ Requirement
→ Certification Basis
→ Compliance Method
→ Evidence
```

Технический источник:

```text
Technical Source
→ Engineering Knowledge
→ Requirement / Design Basis
```

Эти две цепочки не смешиваются.

## 18. Certification strategy

Стратегически:

```text
                BLUESKY PLATFORM
                       │
             ┌─────────┴─────────┐
             │                   │
          ≤30 kg              >30 kg
        initial profile       expansion
             │                   │
      applicable regime     certification basis
             │                   │
      configuration A       configuration B...
```

Расширение должно выполняться через controlled configuration and certification change process.

## 19. Product positioning

Внешнее инженерно-продуктовое позиционирование:

> **BlueSky — платформенная авиационная система для интеграции и эксплуатации БАС различных типов и конфигураций с конфигурационно-зависимой адаптацией и управляемым расширением области применения.**

Не заявлять:

```text
«сертифицирована для всех БПЛА»
```

до получения соответствующих approvals.

## 20. Required certification questions

Перед утверждением certification basis необходимо определить для каждой заявляемой конфигурации:

```text
[ ] Legal classification
[ ] Aircraft / UAV type
[ ] MTOM
[ ] Intended operation
[ ] Operating environment
[ ] Applicant / certificate holder
[ ] Certification object
[ ] Included elements
[ ] Excluded elements
[ ] Applicable certification procedure
[ ] Applicable certification basis
[ ] Compliance methods
[ ] Authority interaction
```

## 21. Current decisions

```text
DEC-CO-001
BlueSky is designed as a platform, not a single-aircraft product.

DEC-CO-002
The first strategic profile is UAVs up to 30 kg inclusive,
subject to the applicable Russian legal regime.

DEC-CO-003
The architecture shall support future configurations above 30 kg.

DEC-CO-004
Different aircraft types are treated as configurations / integration profiles.

DEC-CO-005
Certification scope is established per declared configuration.

DEC-CO-006
No certification claim is generalized from one configuration
to another without an applicability and compliance assessment.
```

## 22. Open issues

```text
CO-GAP-001 Exact legal certification regime for each target configuration
CO-GAP-002 Exact certification object wording for the first application
CO-GAP-003 Applicant / certificate-holder model
CO-GAP-004 Complete aircraft type matrix
CO-GAP-005 MTOM / category matrix
CO-GAP-006 Intended operation matrix
CO-GAP-007 Configuration-specific certification basis
CO-GAP-008 Authority interaction plan
```

## 23. Effect on existing documents

This document becomes the basis for synchronization of:

```text
CERTIFICATION_BASIS.md
COMPLIANCE_MATRIX.md
CERTIFICATION_REQUIREMENTS_BASELINE.md
SYSTEM_REQUIREMENTS_SPECIFICATION.md
SYSTEM_ARCHITECTURE_BASELINE.md
CONFIGURATION_BASELINE.md
VERIFICATION_PLAN.md
```

No unrelated document should be rewritten solely because this document exists. Only affected records are updated through the change-control process.

## 24. Status

**DRAFT_FOR_AGREEMENT**
