---
id: REGULATORY-CLAUSE-MAPPING-001
type: regulatory_clause_mapping
status: working_draft
jurisdiction: Russian Federation
system: BlueSky PRO
source_register: REGULATORY_SOURCE_REGISTER.md
certification_basis: CERTIFICATION_BASIS.md
---

# BLUE SKY PRO — REGULATORY CLAUSE MAPPING 001

## 1. Назначение

Документ выполняет clause-level mapping между проверенным нормативным источником и границей ответственности BlueSky.

Он не заменяет `CERTIFICATION_BASIS.md`, `COMPLIANCE_MATRIX.md` или `CERTIFICATION_REQUIREMENTS_BASELINE.md` и не является утверждённым certification basis.

## 2. Метод

Для каждого положения фиксируется:

```text
SOURCE
→ CLAUSE
→ REGULATORY OBJECT
→ BLUE SKY SCOPE
→ APPLICABILITY
→ REQUIRED ACTION
→ TARGET DOCUMENT
→ VERIFICATION
→ EVIDENCE
```

Если нормативное положение не позволяет сделать однозначный вывод о применимости BlueSky, статус остаётся `APPLICABILITY_PENDING`.

## 3. ВК РФ — статья 37

### SRC-001 / CLAUSE-001

**Источник:** Воздушный кодекс Российской Федерации, статья 37.

**Проверка:** действующая редакция источника на 04.09.2026.

**Положение:** обязательная сертификация БАС нового типа и/или их элементов проводится в соответствии с федеральными авиационными правилами, устанавливающими порядок обязательной сертификации; завершение сертификации связано с выдачей сертификата типа при установлении соответствия требованиям certification basis и признании конструкции типовой.

**Регуляторный объект:** БАС нового типа и/или их элементы, подпадающие под обязательную сертификацию.

**BlueSky relevance:** CONDITIONAL.

**Applicability:** зависит от установленного certification object и конкретного сертификационного пути BlueSky.

**Action:** не объявлять BlueSky автоматически сертификационным объектом только на основании наличия функций управления/планирования. Сначала зафиксировать границы объекта и роль BlueSky.

**Target:**

```text
CERTIFICATION_OBJECT_AND_SCOPE.md
CERTIFICATION_BASIS.md
COMPLIANCE_MATRIX.md
```

**Verification:** legal/regulatory review.

**Evidence:** официальный текст ВК РФ + запись applicability decision.

### SRC-001 / CLAUSE-002

**Положение:** статья 37 определяет certification basis как совокупность требований, необходимых для обеспечения безопасной эксплуатации БАС и/или их элементов и охраны окружающей среды от воздействия деятельности в области авиации.

**Регуляторный объект:** certification basis конкретного БАС/элемента.

**BlueSky relevance:** DIRECT at process/document level; CONDITIONAL at product-certification level.

**Applicability:** применяется как правило формирования certification basis только при наличии соответствующего certification scope.

**Action:** сохранять отдельный слой certification basis и не смешивать его с внутренним каталогом SYS-REQ.

**Target:**

```text
CERTIFICATION_BASIS.md
CERTIFICATION_BASIS_WORKING_REGISTER.md
```

**Verification:** document review / regulatory review.

**Evidence:** approved source reference and controlled project record.

### SRC-001 / CLAUSE-003

**Положение:** certification basis разрабатывается юридическим лицом-разработчиком на основании требований к лётной годности и охране окружающей среды, применимых к конкретному типу БАС и/или элемента, и утверждается уполномоченным федеральным органом.

**BlueSky relevance:** DIRECT to certification documentation governance when BlueSky is within certification scope; otherwise CONDITIONAL.

**Applicability:** PENDING until certification object is frozen.

**Action:** maintain traceability from applicable airworthiness/environmental requirements to certification basis and then to compliance evidence.

**Target:**

```text
CERTIFICATION_BASIS.md
COMPLIANCE_MATRIX.md
MASTER_REQUIREMENTS_REGISTER.md
```

**Verification:** review of source-to-basis traceability.

**Evidence:** controlled matrix and approval record when available.

### SRC-001 / CLAUSE-004

**Положение:** порядок проведения обязательной сертификации должен содержать, в числе прочего, порядок и последовательность сертификационных работ, порядок формирования certification basis, применение норм лётной годности, разработку дополнительных требований и установление эквивалентного соответствия.

**BlueSky relevance:** DIRECT to certification workflow/documentation where certification applies.

**Applicability:** CONDITIONAL.

**Action:** project certification workflow shall preserve these decision points and shall not reduce certification to a simple software requirements checklist.

**Target:**

```text
CERTIFICATION_BASIS.md
COMPLIANCE_MATRIX.md
VERIFICATION_PLAN.md
```

**Verification:** process/document review.

**Evidence:** certification workflow records and compliance matrix.

## 4. ВК РФ — статья 8 / порог 30 кг

### SRC-001 / CLAUSE-005

**Положение:** статья 8 содержит исключение из обязательной сертификации для БАС/элементов, включающих БВС с MTOM 30 кг и менее, в предусмотренном законом случае.

**BlueSky relevance:** DIRECT to applicability analysis; not by itself a product exemption from all aviation regulation.

**Applicability:** PROFILE-A requires separate applicability review.

**Action:** keep the following distinction:

```text
No mandatory certification under applicable provision
≠
No regulatory obligations
≠
No product requirements
≠
No evidence
```

**Target:**

```text
COMPLIANCE_MATRIX.md
CERTIFICATION_BASIS.md
```

**Verification:** legal applicability review.

**Evidence:** current VК РФ source and signed/controlled applicability decision.

## 5. Нормы лётной годности — applicability register

Официальный перечень Росавиации подтверждает наличие следующих действующих комплексов, относящихся к БАС:

| Source ID | Норма | Edition / year shown in official list | BlueSky applicability |
|---|---|---|---|
| CB-SRC-004 | НЛГ Л-БАС | 2024 | CONDITIONAL / OPEN |
| CB-SRC-005 | НЛГ БАС-ВТ | Издание 2 / 2024 | CONDITIONAL / OPEN |
| CB-SRC-006 | НЛГ БАС-СТ | Издание 2 / 2025 | CONDITIONAL / OPEN |
| CB-SRC-007 | НЛГ УИ-БАС | 2025 | CONDITIONAL / OPEN |
| CB-SRC-008 | НЛГ АЗН-УВД | 2025 | CONDITIONAL / OPEN |

Источник перечня: Росавиация, раздел авиационной техники / сертификации. Данные используются как source-status evidence; сама запись в перечне не означает автоматически применимость конкретной нормы к BlueSky.

## 6. C2 — текущий mapping boundary

В соответствии с установленной в проекте моделью C2 сохраняются два режима:

```text
DIRECT-C2
PROVIDER-C2
```

На уровне требований BlueSky необходимо разделять:

1. собственные функции BlueSky;
2. контролируемый интерфейс с внешним C2 provider;
3. функции и ответственность внешнего C2 provider;
4. оборудование/инфраструктуру внешнего участника.

До clause-level проверки конкретного текста приказа №142 все такие записи остаются `IN REVIEW` / `APPLICABILITY_PENDING` и не переводятся в статус `COMPLIANT`.

## 7. Решение по текущему проходу

```text
ВК РФ ст. 37 → VERIFIED SOURCE
ВК РФ ст. 8 / ≤30 кг → VERIFIED SOURCE
НЛГ перечень Росавиации → VERIFIED SOURCE LIST
C2 №142 → NEXT CLAUSE-LEVEL PASS
```

## 8. Следующий проход

Следующая работа выполняется в таком порядке:

```text
Приказ №142
→ конкретные пункты
→ C2 obligation
→ BlueSky DIRECT / INTERFACE / EXTERNAL
→ C2 System Requirements
→ Safety allocation
→ ICD
→ Verification
→ Evidence
```

После этого аналогично разбираются №140 и №312.

## 9. Контроль версии

```text
Status: WORKING_DRAFT
No certification claim
No final applicability decision
No replacement of authoritative regulatory text
```
