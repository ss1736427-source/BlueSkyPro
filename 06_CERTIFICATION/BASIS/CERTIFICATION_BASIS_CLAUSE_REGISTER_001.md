---
id: CERTIFICATION-BASIS-CLAUSE-REGISTER-001
type: certification_basis_clause_register
status: controlled_working_draft
jurisdiction: Russian Federation
system: BlueSky PRO
---

# BLUE SKY PRO — CERTIFICATION BASIS CLAUSE REGISTER 001

## 1. Назначение

Зафиксировать уже подтверждённые нормативные положения, которые могут использоваться как вход Certification Basis, и отделить их от положений, для которых требуется дальнейшая clause-level applicability analysis.

Документ не является утверждённым органом сертификации certification basis и не создаёт новых нормативных требований.

## 2. Правило записи

```text
OFFICIAL SOURCE
→ CLAUSE
→ OBJECT OF REGULATION
→ BLUE SKY ROLE / BOUNDARY
→ APPLICABILITY
→ CERTIFICATION REQUIREMENT
→ MoC
→ VERIFICATION
→ EVIDENCE
```

Если конкретное положение не проверено на уровне официального текста, оно остаётся `OPEN / TBD` и не используется как закрытое основание требования.

## 3. Confirmed working entries

| Record | Official source | Clause / scope | Working interpretation | BlueSky allocation | Status |
|---|---|---|---|---|---|
| CBCR-001 | Воздушный кодекс РФ | ст. 8 | устанавливает случаи обязательной сертификации авиационной техники и применимое исключение для БАС/элементов с БВС MTOM 30 кг и менее | certification applicability / boundary | WORKING |
| CBCR-002 | Воздушный кодекс РФ | ст. 35 | связывает требования к лётной годности с установленными нормами лётной годности | certification basis input | WORKING |
| CBCR-003 | Воздушный кодекс РФ | ст. 37 | определяет обязательную сертификацию БАС нового типа и/или элементов в применимых случаях, certification basis и ответственность разработчика | certification object / basis | WORKING |
| CBCR-004 | ФЗ от 29.09.2025 №360-ФЗ | изменения законодательства о сертификации | используется как источник изменений, влияющих на процесс формирования и применения certification basis | certification process | IN REVIEW |
| CBCR-005 | ФАП, приказ Минтранса №184 | ФАП-21, применимые разделы | процесс сертификации, типовая конструкция, certification basis, доказательная документация и изменения | certification process / evidence / change control | IN REVIEW |
| CBCR-006 | Приказ Минтранса №142 от 24.04.2025 | пп. 1–10 | организация C2 прямым каналом либо через инфраструктуру поставщика; требования к организации и характеристикам C2 | C2 architecture / ICD / planning constraints | WORKING |
| CBCR-007 | Приказ Минтранса №142 от 24.04.2025 | пп. 60–62 | изменения зоны предоставления услуг и качество C2 | Provider-C2 interface / QoS monitoring | WORKING |
| CBCR-008 | Приказ Минтранса №142 от 24.04.2025 | пп. 66–72 | информация пилоту, информационная безопасность, контроль состояния C2 и контрольные сообщения | C2 monitoring / HMI / safety / provider boundary | WORKING |
| CBCR-009 | Приказ Минтранса №142 от 24.04.2025 | пп. 74–75 | документирование C2-событий и внешнее оформление отказов оборудования provider | event logging / external responsibility boundary | WORKING |
| CBCR-010 | Приказ Минтранса №142 от 24.04.2025 | пп. 77–79 | действия при потере C2, классификация причин и timing provider decision | loss-of-C2 workflow / diagnostics / interface | WORKING |
| CBCR-011 | НЛГ Л-БАС, приказ Росавиации №1186-П | область применения | кандидат для проверки применимости к конкретной конфигурации; не объявляется автоматически certification basis | configuration applicability | OPEN |
| CBCR-012 | НЛГ БАС-ВТ, издание 2 | область применения | кандидат для проверки применимости к A-VT | configuration applicability | OPEN |
| CBCR-013 | НЛГ БАС-СТ, издание 2 | область применения | кандидат для проверки применимости к A-FW | configuration applicability | OPEN |
| CBCR-014 | НЛГ УИ-БАС | область применения | применимость зависит от включения remote identification function/equipment в заявленный scope | equipment/function applicability | OPEN |
| CBCR-015 | НЛГ АЗН-УВД | область применения | применимость зависит от operational concept, airspace и фактически включённого оборудования/функции | equipment/function applicability | OPEN |

## 4. External-only sources

Следующие источники не превращаются автоматически в требования BlueSky:

| Source | Current allocation | Rule |
|---|---|---|
| Приказ №140 | EXTERNAL / CONDITIONAL | требования к физическим посадочным площадкам не входят в ключевую систему управления; BlueSky-specific interface допускается только при наличии собственной подтверждённой функции |
| Приказ №312 | EXTERNAL / CONDITIONAL | требования к эксплуатации внешней инфраструктуры не являются SYS-REQ BlueSky; собственная интерфейсная функция требует отдельного подтверждения scope |
| Приказ №367 | EXTERNAL / CONDITIONAL | требования к организации ТО остаются внешними, если соответствующая функция/роль не включена в BlueSky scope |
| Приказ №354 | EXTERNAL / CONDITIONAL | проверки оборудования provider остаются внешней ответственностью; BlueSky может контролировать только собственные status/data interfaces |

## 5. C2 source chain

```text
Воздушный кодекс РФ
  ↓
ст. 78.1 / применимая C2 legal boundary
  ↓
Приказ №142
  ↓
C2 clause-level mapping
  ↓
SYS-C2 / SAF-C2 / ICD
  ↓
Verification Cases
  ↓
Execution Stub
  ↓
Real Evidence
```

Количественные значения C2 QoS не устанавливаются данным документом. Они должны быть получены из применимого certification basis, НЛГ, эксплуатационной документации и иных подтверждённых источников.

## 6. Configuration applicability gate

До окончательного формирования certification basis для Configuration A необходимо подтвердить:

```text
Aircraft type
MTOM
Intended operation
Operating environment
External pilot concept
C2 concept
Navigation
Remote identification
ATM / surveillance equipment
Flight-control architecture
Safety concept
```

Пока эти данные не зафиксированы как применимые для конкретного certification object, записи CBCR-011…015 остаются `OPEN`.

## 7. No-fabrication rule

Запрещается:

- подставлять номер пункта без проверки официального текста;
- превращать `TBD` в нормативное значение;
- создавать certification requirement только из названия документа;
- переносить external responsibility в SYS-REQ;
- объявлять certification basis утверждённым проектной записью.

## 8. Current status

```text
Verified working clause set: ESTABLISHED
C2 clause-level set: ESTABLISHED
Certification-object-specific applicability: OPEN
Full clause-level extraction: IN WORK
Final Certification Basis: NOT BASELINED
Real verification evidence: DEFERRED
```

## 9. Next deterministic action

```text
CLAUSE REGISTER
→ applicability allocation
→ compliance matrix
→ certification requirements baseline
```

Работа продолжается только по тем положениям, для которых существует достаточная нормативная и системная основа. Отсутствующие данные оформляются как controlled stub.
