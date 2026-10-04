---
id: REGULATORY-SOURCE-REGISTER-001
type: regulatory_source_register
status: controlled_working_baseline
jurisdiction: Russian Federation
system: BlueSky PRO
scope: BlueSky system of control / mission management / C2 integration
---

# Regulatory Source Register — BlueSky PRO

## 1. Назначение

Реестр нормативных источников, используемых при формировании certification basis и комплекта документации BlueSky PRO.

**Ключевое правило:** наличие нормативного источника или требования в реестре не означает его автоматическую применимость к BlueSky.

Каждое положение сначала проходит applicability analysis относительно установленной **System Boundary**.

## 2. Классификация

- **DIRECT** — требование непосредственно относится к функции или элементу BlueSky и может быть распределено в SYS-REQ.
- **INTERFACE** — требование относится к контролируемому интерфейсу BlueSky с внешней системой/участником; в SYS-REQ попадает только собственная сторона интерфейса.
- **CONDITIONAL** — применимость зависит от certification object, конфигурации, роли BlueSky или условий эксплуатации.
- **EXTERNAL** — требование относится к внешнему объекту/организации и не является требованием к BlueSky.
- **REG-GAP / WATCH** — область требует дальнейшего анализа.

## 3. Единая цепочка applicability

```text
Нормативный источник
        ↓
Конкретное положение / пункт
        ↓
Объект регулирования
        ↓
Роль BlueSky
        ↓
System Boundary
        ↓
DIRECT / INTERFACE / CONDITIONAL / EXTERNAL
        ↓
System Requirement / Interface Requirement / Operational Constraint
        ↓
Verification / Evidence
```

Если объект регулирования находится вне System Boundary, требование **не превращается в SYS-REQ BlueSky**.

BlueSky не принимает на себя требования к БАС, C2 provider, владельцу посадочной площадки, организации ТО или владельцу внешнего оборудования, если соответствующий объект не входит в scope BlueSky.

## 4. Источники

| ID | Источник | Предварительная роль | Область |
|---|---|---|---|
| REG-001 | Воздушный кодекс РФ | DIRECT / INTERFACE / CONDITIONAL | legal basis, certification, operation, C2, safety |
| REG-002 | ФАП-21, приказ Минтранса №184 | CONDITIONAL | certification process, compliance, evidence, change control |
| REG-004 | НЛГ Л-БАС, приказ №1186-П | CONDITIONAL | airworthiness |
| REG-005 | НЛГ БАС-ВТ, издание 2 | CONDITIONAL | airworthiness |
| REG-006 | НЛГ БАС-СТ, издание 2 | CONDITIONAL | airworthiness |
| REG-007 | НЛГ УИ-БАС | CONDITIONAL | equipment / identification |
| REG-008 | НЛГ АЗН-УВД | CONDITIONAL | surveillance / ATM |
| REG-013 | Приказ Минтранса №142 | DIRECT / INTERFACE / CONDITIONAL | C2 / control / communication |
| REG-017 | Приказ Минтранса №140 | EXTERNAL / DOCUMENTATION / CONDITIONAL | landing-site regulation |
| REG-018 | Приказ Минтранса №312 | EXTERNAL / INTERFACE / CONDITIONAL | operational/infrastructure regulation |
| REG-019 | Приказ №367 | EXTERNAL / INTERFACE / CONDITIONAL | maintenance |
| REG-020 | Приказ №354 | EXTERNAL / INTERFACE / CONDITIONAL | C2-provider ground equipment |

## 5. Приказ №142 — основной непосредственный нормативный домен C2

№142 рассматривается как приоритетный источник для функций C2, находящихся в системной границе BlueSky.

Рабочая декомпозиция выполняется по направлениям:

- DIRECT-C2;
- PROVIDER-C2;
- мониторинг состояния C2;
- деградация и потеря C2;
- безопасная реакция;
- C2 data/interface control;
- временная модель;
- регистрация значимых C2-событий;
- security controls в пределах BlueSky scope.

Внешними остаются функции и ответственность оператора сети, C2 provider, владельца радиочастотного ресурса и физического оборудования provider, если они не включены в scope BlueSky.

**Текущий результат:** clause-level mapping для №142 сформирован в `C2_CLAUSE_LEVEL_MAPPING_001.md`; C2 requirement/interface/verification reconciliation подготовлена. Quantitative criteria и real-test evidence остаются deferred.

## 6. Приказ №140 — посадочные площадки

Официальный источник подтверждает предмет регулирования: требования к посадочным площадкам, предназначенным для взлёта, посадки, руления и стоянки гражданских воздушных судов. citeturn1search0

### Решение по scope

**№140 не является самостоятельным источником прямых требований к системе управления и планирования BlueSky PRO.**

Требования к физической посадочной площадке, её владельцу, строительству, состоянию, маркировке, оборудованию и организационной эксплуатации площадки находятся **вне системной границы BlueSky**, если соответствующие объекты не включены в утверждённый certification scope.

Они могут быть представлены в certification/operational documentation как:

- внешний нормативный источник;
- external responsibility;
- внешнее условие или ограничение эксплуатации;
- исходные данные, если конкретный параметр площадки действительно используется системой.

### Запрещённый перенос

```text
требование к посадочной площадке
        ✕
        ↓
SYS-REQ BlueSky
```

Нельзя автоматически создавать SYS-REQ о строительстве, состоянии, оборудовании или соответствии площадки.

### Возможная системная связь

Если конкретная функция BlueSky **действительно** использует сведения о месте взлёта/посадки, в SRS допускается требование только к самой функции BlueSky, например обработке внешнего входного параметра или применению заранее определённого operational constraint. Такое требование возникает из **функциональной потребности и установленной границы**, а не из автоматического переноса №140.

**Статус applicability:** EXTERNAL / CONDITIONAL — clause-level allocation в BlueSky не создана; требуется только при подтверждении соответствующей функции/сертификационного scope.

## 7. Приказ №312

Официальный источник подтверждает предмет регулирования: правила эксплуатации аэродромов, вертодромов и посадочных площадок для гражданских воздушных судов в зависимости от видов полётов и характеристик обслуживаемых воздушных судов. citeturn1search1

Рассматривается аналогично: требования к физической/организационной эксплуатации внешней инфраструктуры остаются внешними. В BlueSky могут быть выделены только собственные интерфейсные функции, если они подтверждены System Boundary и operational concept.

**Статус applicability:** EXTERNAL / INTERFACE / CONDITIONAL — отдельная системная allocation не создаётся до подтверждения конкретного интерфейса BlueSky с аэродромной/площадочной инфраструктурой.

## 8. Приказ №367 — техническое обслуживание

В текущем repository register этот источник классифицирован как `EXTERNAL / INTERFACE / CONDITIONAL`.

Требования к организации ТО, персоналу, производственной среде и физическому обслуживанию БАС не являются требованиями BlueSky, если BlueSky не принимает соответствующую роль и объект не входит в scope.

При наличии соответствующей функции допускается интерфейс технической готовности: статус, ограничения, данные о состоянии и события — только в пределах собственной ответственности BlueSky.

**Статус applicability:** CONDITIONAL / EXTERNAL — не создавать SYS-REQ без подтверждения функционального scope и точного официального положения источника.

## 9. Приказ №354 — оборудование C2 provider

Официальный источник подтверждает, что приказ устанавливает порядок проведения наземных и лётных проверок, в том числе наземного оборудования, необходимого поставщикам услуг по обслуживанию линий управления БАС и контроля БАС. citeturn1search2

Физическое наземное оборудование и его проверки остаются внешними, если это оборудование не входит в BlueSky scope.

При PROVIDER-C2 BlueSky может иметь собственные интерфейсные функции по получению статуса доступности/готовности услуги и регистрации соответствующих событий. Это не делает BlueSky владельцем или оператором оборудования provider.

**Статус applicability:** EXTERNAL / INTERFACE / CONDITIONAL — собственная BlueSky interface allocation возможна только для явно определённых provider status/data inputs; provider-side inspection remains external.

## 10. Что входит в системные требования BlueSky

Нормативный материал попадает в `MASTER_REQUIREMENTS_REGISTER` и `SYSTEM_REQUIREMENTS_SPECIFICATION` только после доказанного соответствия собственной System Boundary.

Минимальная трассировка:

```text
SOURCE
→ CLAUSE
→ APPLICABILITY
→ BLUE SKY SCOPE
→ REQUIREMENT
→ DESIGN / INTERFACE
→ VERIFICATION
→ EVIDENCE
```

## 11. Что входит в сертификационную документацию, но не обязательно в SYS-REQ

В документах проекта могут оставаться нормативные положения, которые:

- описывают внешний объект;
- определяют ответственность другого участника;
- задают предпосылку эксплуатации;
- нужны для обоснования applicability;
- формируют external constraint или certification assumption.

Их присутствие в certification documentation **не означает**, что они являются требованиями к программному обеспечению BlueSky.

## 12. Документный контур

Реестр является входом для:

```text
CERTIFICATION_BASIS
COMPLIANCE_MATRIX
CERTIFICATION_REQUIREMENTS_BASELINE
MASTER_REQUIREMENTS_REGISTER
SYSTEM_REQUIREMENTS_SPECIFICATION
SYSTEM_ARCHITECTURE_BASELINE
INTERFACE_CONTROL_DOCUMENT
SAFETY_REQUIREMENTS_BASELINE
VERIFICATION_PLAN
VERIFICATION_EVIDENCE_INDEX
```

Но переход из Regulatory Source Register в системные документы допускается только через applicability и scope allocation.

## 13. Решение

1. **№142/C2** — текущий основной непосредственный нормативный домен системы управления в установленной части; clause-level mapping и pre-execution verification chain подготовлены.
2. **№140** — внешний нормативный контур посадочных площадок; не источник прямых SYS-REQ BlueSky без подтверждённой собственной функции.
3. **№312** — внешний/условный нормативный контур; системные требования только после отдельного подтверждения интерфейсной функции.
4. **№367** — внешний/условный контур ТО; требования BlueSky только при наличии соответствующего функционального scope.
5. **№354** — преимущественно внешний контур оборудования C2 provider; собственные интерфейсные функции выделяются отдельно.

## 14. Контроль изменений

Данный регистр является controlled baseline. Любое последующее добавление нормативного требования должно содержать applicability rationale и явное указание, в какой документный уровень оно распределяется.

## 15. Current controlled status

```text
№142 / C2
  clause mapping              = PREPARED
  requirement reconciliation  = PREPARED
  interface allocation        = PREPARED
  safety allocation           = PREPARED
  verification cases          = DEFINED
  execution configuration     = STUB
  execution result            = STUB
  evidence                    = EMPTY STUB
  quantitative basis          = DEFERRED
  real testing                = DEFERRED

№140 / №312 / №367 / №354
  source register             = CONTROLLED
  high-level applicability   = ALLOCATED
  clause-level BlueSky scope = DEFERRED / CONDITIONAL
  new SYS-REQ                 = NONE
```

This status permits the project to move to the next deterministic regulatory/architecture work package without fabricating test evidence or prematurely baselining conditional external requirements.
