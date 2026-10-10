---
id: CERTIFICATION-BASIS-001
type: certification_basis
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
certification_object: CERTIFICATION-OBJECT-AND-SCOPE-001
---

# Certification Basis

## 1. Назначение

Определить основу формирования сертификационного базиса BlueSky и порядок перехода от применимых российских требований к требованиям, доказательствам соответствия и конкретной конфигурации.

Документ не заменяет утверждённый органом сертификации certification basis.

## 2. Рабочий объект

Рабочее определение объекта установлено в:

`CERTIFICATION_OBJECT_AND_SCOPE.md`

```text
БАС нового типа и/или её сертифицируемый элемент
        ↓
BlueSky Aviation System
        ↓
конкретная типовая конструкция / конфигурация
```

Критически важно различать:

```text
BlueSky platform
≠
тип БАС
≠
конкретный экземпляр БВС
≠
сертификационный базис
```

ВК РФ ст. 37 предусматривает обязательную сертификацию БАС нового типа и/или их элементов в применимых случаях; сертификация завершается выдачей сертификата типа при подтверждённом соответствии сертификационному базису и признании конструкции типовой. citeturn0search0

## 3. Юридическое определение certification basis

По ст. 37 ВК РФ сертификационный базис — совокупность требований, необходимых для безопасной эксплуатации БАС и/или их элементов, а также для охраны окружающей среды от воздействия авиационной деятельности.

Для конкретного типа он разрабатывается разработчиком на основании применимых требований к лётной годности и охране окружающей среды и утверждается уполномоченным федеральным органом. citeturn0search0

Следовательно:

```text
CERTIFICATION BASIS
не является
просто перечнем SYS-REQ
```

Он является нормативно-технической основой, из которой формируются certification requirements и compliance arguments.

## 4. Нормативная иерархия

Рабочая цепочка:

```text
Воздушный кодекс РФ
        ↓
Федеральные авиационные правила
        ↓
Применимые НЛГ / специальные требования
        ↓
Certification Basis
        ↓
Certification Requirements
        ↓
System Requirements
        ↓
Compliance Methods
        ↓
Verification
        ↓
Evidence
```

Для каждого элемента цепочки сохраняется первичный источник.

## 5. Основной федеральный закон

### CB-SRC-001 — Воздушный кодекс РФ

Актуальная редакция проверена по внешнему источнику: 25.05.2026.

Статья 8 устанавливает обязательную сертификацию БАС и/или их элементов в предусмотренных случаях и содержит исключение для БАС/элементов, включающих БВС с MTOM 30 кг и менее. citeturn0search1

Статья 37 определяет обязательную сертификацию БАС нового типа и/или их элементов, сертификат типа, certification basis и ответственность разработчика за соответствие типовой конструкции certification basis. citeturn0search0

## 6. Изменения 2025 года

### CB-SRC-002 — Федеральный закон от 29.09.2025 № 360-ФЗ

Официально опубликован 29.09.2025.

Закон внёс изменения в Воздушный кодекс, включая положения статьи 37 о порядке обязательной сертификации и формировании certification basis. citeturn1search0

В соответствии с действующей ст. 37 порядок обязательной сертификации должен включать, в частности:

```text
порядок и последовательность сертификационных работ;
термины и определения;
порядок формирования certification basis;
использование НЛГ;
разработку дополнительных требований;
эквивалентное соответствие;
порядок применения НЛГ.
```

Поэтому проект не должен строить certification basis только на внутренних документах.

## 7. ФАП-21

### CB-SRC-003 — Приказ Минтранса России от 17.06.2019 № 184

ФАП:

`«Сертификация авиационной техники, организаций разработчиков и изготовителей. Часть 21»`

ФАП-21 содержит разделы, относящиеся к:

```text
сертификационным работам;
типовой конструкции;
сертификату типа;
заявке;
требованиям к лётной годности;
certification basis;
доказательной документации;
изменениям типовой конструкции.
```

Текущая найденная редакция — приказ №184 с изменениями от 29.05.2023; срок действия документа указан до 01.09.2029. citeturn1search8

ФАП-21 предусматривает, что заявитель определяет соответствие авиационной техники требованиям certification basis, а Авиарегистр выполняет оценку соответствия в установленном порядке. citeturn1search10

Сертификат типа удостоверяет соответствие изделия certification basis; карта данных сертификата типа содержит сведения о типовой конструкции, certification basis, условиях и ограничениях. Для БАС в карту данных включаются элементы типовой конструкции, включая оборудование управления и контроля, линии управления и контроля. citeturn1search11

## 8. Учебный источник проекта

В предоставленном учебном материале МГТУ им. Н.Э. Баумана «Основы сертификации типа БАС» указаны:

```text
ВК РФ, статьи 8 и 37
ФАП-21, приказ Минтранса №184
НЛГ Л-БАС
НЛГ БАС-ВТ
НЛГ БАС-СТ
НЛГ УИ-БАС
иные применимые нормы по оборудованию, связи,
навигации, ЭМС и охране окружающей среды
```

Материал также описывает последовательность:

```text
заявка
→ определение / утверждение certification basis
→ программа сертификационных работ
→ план подтверждения соответствия
→ расчёты / испытания / проверки
→ доказательная документация
→ устранение несоответствий
→ заключительные документы
→ сертификат типа
```

Это используется в проекте как **вторичный учебный источник для структуры процесса**, а нормативные утверждения подтверждаются первичными официальными источниками. fileciteturn118file0L10-L19 fileciteturn118file0L29-L44

## 9. Требования к лётной годности

Статья 35 ВК РФ связывает требования к лётной годности с нормами лётной годности.

Для БАС Росавиация публикует отдельные комплексы НЛГ, включая:

```text
НЛГ Л-БАС
НЛГ БАС-ВТ
НЛГ БАС-СТ
НЛГ УИ-БАС
НЛГ АЗН-УВД
```

Официальный перечень Росавиации содержит НЛГ БАС-СТ издание 2 (2025), НЛГ БАС-ВТ издание 2 (2024), НЛГ Л-БАС (2024), НЛГ УИ-БАС (2025) и НЛГ АЗН-УВД (2025). citeturn2search0turn2search3

## 10. НЛГ Л-БАС

### CB-SRC-004

НЛГ Л-БАС утверждены приказом Росавиации от 27.12.2024 № 1186-П.

Область применения, согласно официальному тексту:

```text
MTOM не более 600 кг
не перевозят людей
не предназначены для эксплуатации
над населёнными пунктами и местами скопления людей
управляются с участием внешнего пилота
не требуют высокого уровня эксплуатационной надёжности
для снижения соответствующих рисков
```

Это делает НЛГ Л-БАС важным кандидатом для дальнейшего анализа, но не позволяет автоматически объявить их certification basis BlueSky Configuration A. Необходимо сопоставить конкретный тип БВС и intended operation с каждым условием применимости. citeturn2search7

## 11. НЛГ БАС-ВТ

### CB-SRC-005

НЛГ БАС-ВТ издание 2 опубликованы Росавиацией в 2024 году.

Официальный перечень Росавиации подтверждает наличие издания 2. citeturn2search0

Для вертолётного / VTOL профиля Configuration A применимость должна определяться отдельно.

## 12. НЛГ БАС-СТ

### CB-SRC-006

НЛГ БАС-СТ издание 2 опубликованы Росавиацией в 2025 году.

Официальный перечень указывает НЛГ БАС-СТ для БАС с БВС самолётного типа. citeturn2search0

В опубликованном тексте НЛГ БАС-СТ отдельно предусмотрена дифференциация требований по MTOM и условиям применения; например, требования раздела «Прочность» в полном объёме применяются к БВС с MTOM более 150 кг, а для меньшей массы применяются определённые требования в зависимости от условий эксплуатации. citeturn2search10

Следовательно, для Configuration A нельзя использовать только общий порог 30 кг: применимость требований НЛГ должна быть проверена по типу, массе и intended operation.

## 13. НЛГ УИ-БАС

### CB-SRC-007

НЛГ УИ-БАС относятся к требованиям к оборудованию удалённой идентификации.

Официальный перечень Росавиации указывает редакцию 2025 года. citeturn2search0

Применимость зависит от того, входит ли соответствующее оборудование и функция в заявленный configuration / operational scope.

## 14. НЛГ АЗН-УВД

### CB-SRC-008

НЛГ АЗН-УВД относятся к оборудованию автоматического зависимого наблюдения (вещательного) и приёмоответчикам УВД.

Официальный перечень Росавиации указывает редакцию 2025 года. citeturn2search0

Применимость определяется назначением, воздушным пространством, оборудованием и operational concept.

## 15. Configuration A

Текущая стратегия:

```text
CONFIGURATION-A
MTOM ≤30 кг
```

Но это **не certification basis** и не юридическая категория сама по себе.

Для A отдельно определить:

```text
Aircraft type
MTOM
Intended operation
Operating environment
External pilot concept
C2
Navigation
Remote identification
ATM / surveillance equipment
Flight-control architecture
Safety concept
```

Только после этого выбирается применимый набор НЛГ и других требований.

## 16. Configuration A — regulatory status

По ст. 8 ВК РФ БАС/элементы, включающие БВС с MTOM 30 кг и менее, относятся к исключению из обязательной сертификации, указанному в подп. 2.1 пункта 1 статьи 8. citeturn0search1

Поэтому:

```text
PROFILE-A ≤30 кг
→ НЕ объявлять как mandatory type-certification target
→ выполнить regulatory applicability review
→ определить применимые эксплуатационные требования
→ сохранить common certification-ready architecture
```

Это позволяет использовать A как рыночный профиль без ошибочного утверждения о наличии сертификата типа.

## 17. Configuration B — >30 кг

Для >30 кг certification strategy:

```text
Specific aircraft type
→ Applicable airworthiness standards
→ Certification basis
→ Certification requirements
→ Compliance plan
→ Verification
→ Evidence
→ Type certification
```

Common BlueSky functions могут быть переиспользованы только после анализа применимости и конфигурационных различий.

## 18. Certification process

Рабочая последовательность:

```text
1. Определение certification object
2. Определение aircraft type / configuration
3. Определение intended operation
4. Определение применимых НЛГ
5. Определение специальных / дополнительных требований
6. Формирование проекта certification basis
7. Согласование / утверждение certification basis
8. Certification programme / compliance planning
9. Выполнение расчётов и испытаний
10. Формирование доказательной документации
11. Устранение несоответствий
12. Заключительные документы
13. Сертификат типа — если применимо
```

Структура процесса согласуется с учебным материалом проекта и с архитектурой ФАП-21. fileciteturn118file0L29-L44 citeturn1search8

## 19. Compliance methods

Основные методы:

```text
ANALYSIS
INSPECTION
TEST
SIMULATION
DEMONSTRATION
REVIEW_OF_DESIGN
DOCUMENT_REVIEW
```

Для каждого certification requirement метод должен быть обоснован и связан с evidence.

## 20. Эквивалентное соответствие

ФАП-21 предусматривает механизм эквивалентных требований: если соответствие конкретному пункту certification basis установить невозможно, заявитель разрабатывает эквивалентное требование, обеспечивающее не меньший уровень лётной годности, и направляет проект изменения certification basis в уполномоченный орган. citeturn1search20

Это необходимо учитывать как отдельный compliance mechanism:

```text
Applicable Requirement
→ Applicability Problem
→ Equivalent Requirement Proposal
→ Safety / Compliance Justification
→ Authority Approval
→ Updated Certification Basis
```

## 21. Дополнительные требования

Если применимые НЛГ не полностью покрывают конкретную конструкцию / функцию, certification basis должен предусматривать дополнительные требования в предусмотренном порядке.

Кандидаты для BlueSky:

```text
AI-assisted functions
Dynamic mission replanning
Learning / adaptive behaviour
Advanced collision avoidance
Multi-UAV coordination
Novel C2 architecture
Novel authorization architecture
```

Это пока не утверждённые certification requirements.

## 22. Evidence architecture

```text
Certification Requirement
→ Compliance Method
→ Verification Case
→ Result
→ Evidence
→ Configuration Baseline
→ Review
```

Для каждого evidence:

```text
Requirement ID
Configuration ID
Software version
Hardware
Parameters
Test / analysis environment
Result
Reviewer
Date
Baseline
```

## 23. Типовая конструкция

Для certification configuration необходимо явно определить:

```text
Aircraft
Flight-control equipment
Navigation equipment
C2 equipment
Lines of control and control
Relevant software
Relevant hardware
Interfaces
Parameters
Operational limitations
```

ФАП-21 предусматривает отражение элементов типовой конструкции БАС, включая оборудование и линии управления/контроля, в карте данных сертификата типа. citeturn1search11

Следовательно, граница BlueSky должна быть определена через controlled interfaces, а не только маркетинговым названием продукта.

## 24. Common Core / configuration-specific

### Common candidates

```text
Safety architecture
Navigation abstractions
Mission model
Route model
Authorization boundary
C2 abstractions
Configuration management
Verification infrastructure
Data integrity
```

### Configuration-specific

```text
Flight dynamics
Aircraft performance
Mass properties
Energy model
Sensors
Actuators
Flight-control interfaces
Aircraft-specific C2
Environmental limitations
Operational limitations
```

## 25. Certification-ready architecture

Даже для Configuration A, не являющейся текущим обязательным type-certification target по ст. 8, система должна сохранять:

```text
Requirements traceability
Configuration control
Verification evidence
Safety traceability
Interface control
Change impact analysis
Software / hardware identification
```

Это позволяет не переделывать архитектуру и документацию при переходе к >30 кг.

## 26. Source hierarchy

```text
LEVEL 1 — ВК РФ / федеральные законы
LEVEL 2 — постановления Правительства
LEVEL 3 — ФАП / приказы Минтранса
LEVEL 4 — утверждённые НЛГ
LEVEL 5 — официальные методы / материалы Росавиации
LEVEL 6 — technical sources
LEVEL 7 — internal engineering records
```

Учебные материалы используются как secondary reference и не заменяют уровни 1–5.

## 27. Current source set

```text
CB-SRC-001  Воздушный кодекс РФ
CB-SRC-002  ФЗ №360-ФЗ от 29.09.2025
CB-SRC-003  ФАП-21, приказ №184
CB-SRC-004  НЛГ Л-БАС, приказ №1186-П
CB-SRC-005  НЛГ БАС-ВТ, издание 2, №996-П
CB-SRC-006  НЛГ БАС-СТ, издание 2, №613-П
CB-SRC-007  НЛГ УИ-БАС, №829-П
CB-SRC-008  НЛГ АЗН-УВД
```

## 28. Current status

```text
Legal basis: IDENTIFIED
Certification basis concept: IDENTIFIED
FAP-21: IDENTIFIED
Applicable NLG families: IDENTIFIED
Configuration A applicability: NOT YET FINALLY DETERMINED
Configuration B certification path: IDENTIFIED AT HIGH LEVEL

Clause-level applicability: PENDING
Final certification basis: PENDING
Authority-agreed scope: PENDING
```

## 29. Required next action

Не расширять список документов.

Следующая работа:

```text
CONFIGURATION A-MR
→ CONFIGURATION A-FW
→ CONFIGURATION A-VT

для каждого:

Aircraft type
→ Intended operation
→ MTOM
→ Applicable NLG
→ Applicable equipment rules
→ C2
→ Navigation
→ Remote identification
→ Compliance obligations
```

После этого результаты переносятся в:

```text
COMPLIANCE_MATRIX.md
CERTIFICATION_REQUIREMENTS_BASELINE.md
MASTER_REQUIREMENTS_REGISTER.md
```

## 30. Status

**DRAFT_FOR_AGREEMENT**
