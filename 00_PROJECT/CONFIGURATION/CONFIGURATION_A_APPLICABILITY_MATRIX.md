---
id: CONFIGURATION-A-APPLICABILITY-001
type: regulatory_applicability_matrix
status: draft_for_agreement
jurisdiction: Russian Federation
configuration: CONFIGURATION-A
system: aviation_system
---

# CONFIGURATION-A — Regulatory Applicability Matrix

## 1. Назначение

Определить применимость российских авиационных требований к первому рыночному профилю BlueSky:

```text
CONFIGURATION-A
MTOM ≤ 30 кг
```

Матрица не является утверждённым certification basis.

Она разделяет:

```text
mandatory certification applicability
регуляторную применимость
voluntary / future certification relevance
configuration constraints
```

## 2. Configuration A

```text
Product:
BlueSky Aviation System

Profile:
CONFIGURATION-A

MTOM:
≤ 30 kg

Potential aircraft integration profiles:
A-MR  Multirotor
A-FW  Fixed-wing
A-VT  VTOL

Jurisdiction:
Russian Federation
```

## 3. Основной вывод по сертификации

По действующей ст. 8 ВК РФ БАС и/или их элементы, включающие БВС с максимальной взлётной массой 30 кг и менее, относятся к исключению из обязательной сертификации, предусмотренному этой статьёй.

Следовательно:

```text
CONFIGURATION-A
≤ 30 кг
        ↓
mandatory type certification
по соответствующему положению ст. 8
НЕ является исходным обязательным путём
```

Это не означает отсутствия иных обязательных требований.

## 4. Applicability status model

```text
APPLICABLE
CONDITIONALLY_APPLICABLE
NOT_APPLICABLE
CERTIFICATION_RELEVANT
FUTURE_PROFILE
TBD
```

## 5. High-level matrix

| ID        | Source / requirement family                             | A-MR                   | A-FW                   | A-VT                   | Current conclusion                                                                        |
| --------- | ------------------------------------------------------- | ---------------------- | ---------------------- | ---------------------- | ----------------------------------------------------------------------------------------- |
| A-REG-001 | ВК РФ ст. 8 — mandatory certification                   | NOT_APPLICABLE*        | NOT_APPLICABLE*        | NOT_APPLICABLE*        | ≤30 кг попадает в исключение по соответствующему положению                                |
| A-REG-002 | ВК РФ ст. 37 — certification basis / type certification | CERTIFICATION_RELEVANT | CERTIFICATION_RELEVANT | CERTIFICATION_RELEVANT | Не превращает A автоматически в mandatory certification target                            |
| A-REG-003 | НЛГ Л-БАС                                               | CONDITIONAL            | CONDITIONAL            | CONDITIONAL            | Может быть relevant для type-certification scope при выполнении всех условий применимости |
| A-REG-004 | НЛГ БАС-СТ                                              | NOT_APPLICABLE         | FUTURE_PROFILE         | NOT_APPLICABLE         | Издание 2 ориентировано на самолётный БАС >30 кг и определённые операции                  |
| A-REG-005 | НЛГ БАС-ВТ                                              | NOT_APPLICABLE         | NOT_APPLICABLE         | FUTURE_PROFILE         | Издание 2 содержит требования для вертолётного БАС >30 кг и определённых операций         |
| A-REG-006 | НЛГ УИ-БАС                                              | CONDITIONAL            | CONDITIONAL            | CONDITIONAL            | Проверяется по наличию/обязательности оборудования удалённой идентификации                |
| A-REG-007 | НЛГ АЗН-УВД                                             | CONDITIONAL            | CONDITIONAL            | CONDITIONAL            | Проверяется по operational concept и установленному оборудованию                          |
| A-REG-008 | ФАП по C2, приказ №142                                  | APPLICABLE/CONDITIONAL | APPLICABLE/CONDITIONAL | APPLICABLE/CONDITIONAL | Проверить конкретную организацию линии C2 и условия эксплуатации                          |
| A-REG-009 | Государственный учёт БВС                                | APPLICABLE/CONDITIONAL | APPLICABLE/CONDITIONAL | APPLICABLE/CONDITIONAL | Проверяется по действующему режиму учёта и конкретному БВС                                |
| A-REG-010 | Радиочастоты / электросвязь                             | CONDITIONAL            | CONDITIONAL            | CONDITIONAL            | Зависит от используемых средств связи и спектра                                           |
| A-REG-011 | Ограничения воздушного пространства                     | CONDITIONAL            | CONDITIONAL            | CONDITIONAL            | Зависит от intended operation и района полётов                                            |
| A-REG-012 | Эксплуатационные ограничения                            | APPLICABLE             | APPLICABLE             | APPLICABLE             | Определяются конкретной эксплуатацией                                                     |

`*` — применительно к mandatory certification exclusion; это не означает отсутствие иных требований.

## 6. НЛГ Л-БАС — ключевой кандидат

НЛГ Л-БАС, утверждённые приказом Росавиации №1186-П от 27.12.2024, действуют с 01.01.2025 и предназначены для выдачи сертификата типа и внесения изменений в сертификат типа.

Область применения Л-БАС.2000 включает одновременно:

```text
MTOM ≤ 600 кг
не перевозят людей
не предназначены для эксплуатации
над населёнными пунктами и местами скопления людей
управляются с участием внешнего пилота
не требуют высокого уровня эксплуатационной надёжности
для снижения риска третьим лицам
полёты ниже 120 м от поверхности земли
в классе G либо в контролируемом пространстве
при соответствующем согласовании / разрешении
```

Следствие:

```text
A ≤30 кг
≠
автоматически НЛГ Л-БАС
```

Но:

```text
A ≤30 кг
+
условия Л-БАС.2000
        ↓
НЛГ Л-БАС = HIGH-VALUE CERTIFICATION CANDIDATE
```

## 7. Значение для продуктовой стратегии

Это открывает более широкий вариант, чем просто «БПЛА до 30 кг».

НЛГ Л-БАС распространяются по массе до 600 кг, но только при выполнении условий применимости.

Поэтому архитектура BlueSky должна сохранять возможность:

```text
A ≤30 кг
        ↓
L-BAS-compatible configuration
        ↓
future configuration
30–600 кг
```

если соответствующий certification scope и правовой режим будут выбраны.

Это является стратегической возможностью, а не текущим заявлением о сертификации.

## 8. A-MR — Multirotor

### Current status

```text
MTOM ≤30 кг
Mandatory type certification:
NOT_APPLICABLE under the relevant ≤30 kg exclusion

NLG L-BAS:
CONDITIONAL

Other regulatory requirements:
APPLICABLE / CONDITIONAL
```

Проверить:

```text
external pilot concept
flight height
airspace class
operation over populated areas
operation near crowds
C2
remote identification
navigation
energy
operational limitations
```

## 9. A-FW — Fixed-wing

### Current status

```text
MTOM ≤30 кг
Mandatory type certification:
NOT_APPLICABLE under the relevant ≤30 kg exclusion

NLG L-BAS:
CONDITIONAL

NLG BАС-СТ:
NOT_APPLICABLE for the current ≤30 kg profile
FUTURE_PROFILE for >30 kg certification
```

НЛГ БАС-СТ издание 2 содержит требования для самолётного БАС с MTOM более 30 кг и определённых операций, поэтому не следует использовать их как действующую certification basis A-FW.

## 10. A-VT — VTOL

### Current status

```text
MTOM ≤30 кг
Mandatory type certification:
NOT_APPLICABLE under the relevant ≤30 kg exclusion

NLG L-BAS:
CONDITIONAL

NLG BАС-ВТ:
NOT_APPLICABLE for the current ≤30 kg profile
FUTURE_PROFILE for >30 kg certification
```

НЛГ БАС-ВТ издание 2 содержит требования для вертолётного типа с MTOM более 30 кг и конкретными категориями эксплуатации.

## 11. C2

Приказ Минтранса №142 от 24.04.2025 устанавливает ФАП по организации и обеспечению функционирования линий управления БАС и контроля БАС для БАС в составе с беспилотными гражданскими воздушными судами.

Приказ вступил в силу 01.03.2026 и действует до 01.03.2032.

Для BlueSky C2 является отдельным compliance domain:

```text
C2 architecture
→ link characteristics
→ availability
→ integrity
→ control authority
→ loss-of-link behaviour
→ recovery
→ configuration
→ evidence
```

Применимость конкретных положений должна проверяться по фактической роли BlueSky и организации линии C2.

## 12. Remote identification

НЛГ УИ-БАС являются отдельным набором норм по оборудованию удалённой идентификации.

Для A:

```text
если оборудование / функция входит в scope
→ applicability review

если не входит
→ NOT_APPLICABLE

если нормативная обязанность зависит от конкретной эксплуатации
→ CONDITIONAL
```

## 13. Surveillance / ATM equipment

НЛГ АЗН-УВД не следует автоматически включать в A.

Применимость определяется:

```text
airspace
operational concept
required surveillance
installed equipment
interaction with ATM
```

## 14. Airspace / operating conditions

Для Configuration A необходимо создать operational constraints:

```text
Maximum altitude
Airspace class
VLOS / other applicable operating concept
Population density / populated areas
Crowd proximity
Weather
Geographical area
C2 coverage
Navigation availability
Emergency procedures
```

Без этих параметров окончательная regulatory applicability невозможна.

## 15. Product boundary

BlueSky должен быть разделён на:

```text
COMMON BLUE SKY CORE
        +
AIRCRAFT ADAPTER
        +
CONFIGURATION DATA
        +
AIRCRAFT-SPECIFIC EQUIPMENT
```

Это особенно важно для дальнейшего перехода к >30 кг.

## 16. Certification-ready requirements for A

Даже если A не является обязательным type-certification target, сохраняем:

```text
Requirements traceability
Safety traceability
Configuration management
Interface control
Verification evidence
Software / hardware identification
Change impact
Operational limitations
C2 traceability
```

Это снижает стоимость последующего certification scope expansion.

## 17. Future >30 kg branch

```text
CONFIGURATION-A
≤30 кг
        │
        │ common architecture
        ↓
CONFIGURATION-B
>30 кг
        ↓
Aircraft type
        ↓
Applicable NLG
        ↓
Certification Basis
        ↓
Compliance Plan
        ↓
Evidence
```

Для B:

```text
A-MR → future multirotor certification scope
A-FW → future fixed-wing certification scope
A-VT → future VTOL certification scope
```

не следует считать автоматически взаимозаменяемыми.

## 18. Current conclusions

### Confirmed

```text
C-A-001
≤30 кг является отдельным regulatory profile.

C-A-002
Для ≤30 кг нельзя автоматически строить mandatory type-certification
как для >30 кг.

C-A-003
НЛГ БАС-СТ и НЛГ БАС-ВТ издания 2 ориентированы на
определённые сертифицируемые категории >30 кг и поэтому
не являются текущей basis для A.

C-A-004
НЛГ Л-БАС является наиболее интересным certification
reference для части будущего/добровольного certification scope,
но только при выполнении условий Л-БАС.2000.

C-A-005
C2 является отдельным regulatory/compliance domain.

C-A-006
BlueSky должен сохранять certification-ready architecture
даже для A.
```

## 19. Open regulatory gaps

```text
A-GAP-001
Точный intended operation Configuration A

A-GAP-002
Точная максимальная высота

A-GAP-003
VLOS / operational concept

A-GAP-004
Эксплуатация над населёнными пунктами

A-GAP-005
Эксплуатация вблизи скоплений людей

A-GAP-006
Конкретный C2 architecture / service model

A-GAP-007
Remote identification applicability

A-GAP-008
Airspace / ATM applicability

A-GAP-009
Radio-frequency / communications applicability

A-GAP-010
Выбор между market-only A и voluntary/type-certification
reference architecture based on НЛГ Л-БАС

A-GAP-011
Определение конкретных A-MR / A-FW / A-VT airframe families
для дальнейшего compliance mapping
```

## 20. Decision

На текущем этапе:

```text
CONFIGURATION-A
= MARKET / REGULATORY PROFILE ≤30 кг

NOT:
= universal type certificate
= mandatory type certification target
= automatic applicability of all NLG
```

НЛГ Л-БАС сохраняются как стратегически важный certification reference.

## 21. Next controlled action

Следующий шаг:

```text
A-MR
→ exact operational concept
→ L-BAS applicability

A-FW
→ exact operational concept
→ L-BAS applicability

A-VT
→ exact operational concept
→ L-BAS applicability

C2
→ clause-level mapping of Order No. 142
```

После этого результаты переносятся в:

```text
CERTIFICATION_BASIS.md
COMPLIANCE_MATRIX.md
CERTIFICATION_REQUIREMENTS_BASELINE.md
MASTER_REQUIREMENTS_REGISTER.md
```

## 22. Status

**DRAFT_FOR_AGREEMENT**
