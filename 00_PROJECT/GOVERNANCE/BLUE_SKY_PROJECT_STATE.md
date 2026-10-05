# BLUE SKY — PROJECT STATE

> Контрольное состояние проекта для передачи между чатами.
> Дата состояния: 2026-09-20.

## 1. Project Identity

**Project:** BlueSky  
**System type:** авиационная система управления БАС, включающая программно-аппаратные и внешние интеграционные интерфейсы.  
**Primary jurisdiction:** Российская Федерация.

BlueSky не рассматривается как изолированное прикладное ПО. При подготовке к сертификации учитываются функции системы, её интерфейсы, safety, C2, эксплуатационные процессы, конфигурация и доказательства соответствия.

## 2. Certification Strategy

- Формируется широкое семейство продукта с поддержкой нескольких типов/конфигураций БАС.
- Первый профиль: `CONFIGURATION-A`.
- `MTOM ≤ 30 kg` — первый рыночный/регуляторный профиль.
- Порог ≤30 kg не должен называться автоматически «сертифицированным»; применимость обязательной сертификации определяется действующим законодательством и конкретным объектом/условиями эксплуатации.
- >30 kg рассматривается как отдельный будущий сертификационный путь.
- Архитектура должна оставаться пригодной для расширения семейства и последующих сертификационных профилей.

## 3. Current Configuration

### CONFIGURATION-A

Поддерживаемые интеграционные профили:
- `A-MR` — multirotor;
- `A-FW` — fixed-wing;
- `A-VT` — VTOL.

Конфигурация и область применимости должны быть зафиксированы через Configuration Baseline и Applicability Matrix.

## 4. System Purpose and Scope

BlueSky должен обеспечивать, в зависимости от утверждённого состава системы:

- приём задачи на естественном языке;
- формирование и подтверждение понимания задачи;
- предложение оптимизационного профиля;
- планирование миссии;
- работу с departure/destination;
- формирование рабочей области;
- проверку готовности БАС под конкретную миссию;
- распределение задания между несколькими БАС;
- обязательную разведку при отсутствии необходимых внешних/объектных данных;
- построение 3D mission space + time;
- учёт высот, ветра, ограничений, препятствий и других факторов;
- построение и динамический пересчёт маршрута;
- контроль временной модели миссии;
- мониторинг состояния БАС;
- C2 integration;
- аварийную логику;
- архивирование первичной записи миссии и полёта;
- постполётный анализ и производные AI-данные.

## 5. Approved Operational Logic

### Task intake

AI:
1. принимает задачу;
2. формирует понимание;
3. показывает его оператору;
4. получает подтверждение;
5. предлагает профиль оптимизации:
   - быстрее;
   - безопаснее;
   - экономичнее;
   - выше качество.

После подтверждения — переход в пилотскую кабину.

### Departure / Destination

- Departure по умолчанию — GPS оператора/передатчика.
- Departure является началом миссии и не тождественен текущей позиции оператора.
- Пользователь может выбрать другой Departure удобным способом.
- Destination по умолчанию — оператор/передатчик; допускается установка перетаскиванием, координатами или существующей точкой.
- Emergency landing point — отдельная фиксированная безопасная точка.

### Working area

Система предлагает шаблон рабочей области. Оператор корректирует граничные маркеры.

### UAV readiness

Проверяется не только наличие БАС, а соответствие конкретной миссии:
- установленное оборудование;
- техническое состояние;
- готовность;
- доступный ресурс;
- пригодность для задачи.

Один пригодный БАС → выполнение миссии.  
Несколько → распределение задания.  
Нет пригодного → объяснение отсутствующей возможности/ресурса.

### Reconnaissance

Если отсутствуют необходимые данные:
`READY → ВЗЛЁТ → РАЗВЕДКА → ПОЛУЧЕНИЕ ДАННЫХ → ПРОВЕРКА ДАННЫХ → ПЕРЕРАСЧЁТ МИССИИ → ОСНОВНАЯ МИССИЯ`

Другие БАС:
`READY → STBY → READY / ВЫПОЛНЕНИЕ МИССИИ`

`STBY` должен быть видимым состоянием.

### Mission space

Миссия рассматривается как пространство 3D + время. Учитываются:
- высоты;
- внешние данные;
- разведданные;
- ветер;
- зоны ограничений/запрета;
- здания;
- препятствия;
- провода;
- опоры и другие опасности.

### Flight state indicator

Компактные состояния, включая:
`Подготовка`, `READY`, `РАЗВЕДКА`, `ВЫПОЛНЕНИЕ МИССИИ`, `ВОЗВРАТ`, `LANDED`, `STBY`.

### C2 / Communications

Потеря/ухудшение связи не должны автоматически означать немедленный возврат.

Предпочтительная логика:
`Адаптация коммуникационного канала` → оценка возможности продолжения.

Если продолжение невозможно, но безопасный возврат возможен:
- автоматический возврат;
- уведомление пилота.

Если безопасный возврат невозможен:
- переход к emergency/search support;
- приоритет сохранения БАС и уже собранных данных над дальнейшим сбором данных.

Поддерживаются:
- `DIRECT-C2`;
- `PROVIDER-C2`.

BlueSky не должен иметь необоснованной зависимости от одного конкретного C2 provider.

### Dynamic Return

Возврат постоянно пересчитывается по:
- фактической позиции БАС;
- доступной безопасной траектории;
- ограничениям;
- препятствиям;
- ветру;
- ресурсу/батарее;
- техническому состоянию;
- состоянию двигателей и другим релевантным параметрам.

Если безопасный возврат становится невозможным, emergency logic должна срабатывать до исчерпания ресурса.

### Time model

В верхней панели миссии:
- ETD;
- TOT;
- ETA;
- Trip Time;
- Total Time.

До взлёта используется ETD; после фактического взлёта — реальный TOT.

Каждый БАС имеет собственный TOT и индивидуальные времена WP. Общий Mission TOT не заменяет индивидуальные времена БАС.

### UAV selection / flight profile

- одиночный клик — выбор/подсветка связи БАС с миссией;
- повторный клик — снятие выбора;
- остальные БАС не исчезают;
- автоматического zoom нет;
- двойной клик — профиль полёта;
- одиночный клик закрывает/восстанавливает профиль;
- профиль накладывается на карту;
- начальный размер около половины высоты карты;
- размер/позиция сохраняются.

### Route table

По запросу открывается отдельное окно:
- WP sequence;
- course;
- altitude;
- speed.

Редактирование:
- перетаскивание точки на карте;
- фиксация точки после drag;
- отдельное редактирование горизонтальной позиции и высоты.

Проверяются:
- горизонтальные коллизии;
- вертикальные коллизии;
- временные коллизии.

## 6. Safety Concept

Safety является частью системы, а не отдельным приложением после разработки.

Основные принципы:
- mission-specific readiness;
- reconnaissance before execution when critical data are missing;
- dynamic route and return recalculation;
- explicit emergency states;
- сохранение БАС и данных при невозможности безопасного продолжения;
- трассируемость hazards → requirements → verification → evidence.

## 7. Data / Archive

Архив — неизменяемая первичная запись.

Исправления:
- отдельными записями;
- без изменения первичной записи.

Рабочий маршрут отделён от архива.

AI training использует производные данные и не изменяет первичную запись.

Data classes:
- Mission Plan;
- Actual Flight;
- Telemetry / Objective Control;
- Official Flight Plan;
- Mission Results;
- AI Analysis;
- Mission Events.

Срок хранения определяется по классу данных и применимым требованиям, а не единым универсальным сроком.

## 8. Regulatory Basis

Основное ядро:
- Воздушный кодекс РФ;
- ФАП-21 (Приказ Минтранса №184);
- применимые НЛГ БАС.

Из рассматриваемого перечня:
- Приказ №142 от 24.04.2025 — ключевой C2;
- Приказ №140 от 21.04.2025 — посадочные площадки, conditional/external interface;
- Приказ №312 от 01.10.2025 — эксплуатация аэродромов/вертодромов/посадочных площадок, conditional/external interface;
- Приказ №367 от 18.10.2024 — ТО обязательносертифицируемых БАС/элементов, conditional;
- Приказ №354 от 07.10.2024 — проверки оборудования, в том числе релевантного C2 provider, conditional.

Статус и обоснование каждого источника ведутся в `REGULATORY_SOURCE_REGISTER.md`.

### C2 regulatory boundary

Воздушный кодекс ст. 78.1 охватывает не только поставщика услуг C2, но и более широкий контур функционирования C2.

Приказ №142 регулирует функционирование C2 и PROVIDER-C2, но не должен трактоваться как замена отдельной процедуре подтверждения соответствия/реестру поставщиков.

Приказ №354 — это правила проверок оборудования, а не универсальная «сертификация C2 provider».

По отдельной процедуре подтверждения соответствия/реестру поставщиков C2 сохраняется:
`REG-GAP / REGULATORY WATCH`.

Если BlueSky в будущем сам станет C2 provider, открывается отдельный нормативный и организационный scope.

## 9. Certification Documentation Status

Уже сформированы/предусмотрены:

- `CERTIFICATION_BASIS.md`
- `REGULATORY_SOURCE_REGISTER.md`
- `COMPLIANCE_MATRIX.md`
- `CERTIFICATION_REQUIREMENTS_BASELINE.md`
- `SAFETY_REQUIREMENTS_BASELINE.md`
- `HAZARD_LOG.md`
- `SAFETY_CASE.md`
- `REQUIREMENTS_TRACEABILITY_MATRIX.md`
- `VERIFICATION_EVIDENCE_INDEX.md`
- `CONFIGURATION_BASELINE.md`
- `CHANGE_AND_PROBLEM_PROCESS.md`
- `INTERFACE_CONTROL_DOCUMENT.md`
- `SYSTEM_REQUIREMENTS_SPECIFICATION.md`
- `SYSTEM_ARCHITECTURE_BASELINE.md`
- `SYSTEM_DESIGN_DESCRIPTION.md`
- `VERIFICATION_PLAN.md`
- `CONFIGURATION_A_APPLICABILITY_MATRIX.md`
- `C2_PROVIDER_REGULATORY_ASSESSMENT.md`
- `Mission Data Retention Matrix.md`
- `Master Document Index`
- `Master Requirements Register`
- `CONFIGURATION_OBJECT_AND_SCOPE.md`
- `CERTIFICATION_OBJECT_AND_SCOPE.md`

## 10. Approved Decisions

- BlueSky рассматривается как авиационная система.
- CONFIGURATION-A: MTOM ≤30 kg.
- A-MR/A-FW/A-VT остаются поддерживаемыми профилями.
- >30 kg — отдельный будущий certification path.
- Departure и operator position — разные сущности.
- Mission map — функциональный инструмент.
- Readiness — mission-specific.
- Reconnaissance обязательна при отсутствии критических данных.
- STBY — отдельное видимое состояние.
- C2 deterioration не означает автоматический return.
- Dynamic return пересчитывается постоянно.
- Mission-level TOT не заменяет UAV-level timing.
- Archive primary record immutable.
- Corrections — отдельные записи.
- C2 provider — внешний участник, если BlueSky сам им не является.
- Требования должны иметь трассировку до verification/evidence.

Подробный журнал: `BLUE_SKY_DECISIONS_LOG.md`.

## 11. Open Issues / REG-GAP

- окончательная determination of applicability конкретных НЛГ для каждого certification profile;
- формирование окончательного Certification Basis;
- clause-by-clause decomposition Приказа №142;
- определение точных C2 performance/monitoring parameters;
- границы BlueSky и external C2 provider;
- applicability №140/№312 к конкретной эксплуатации;
- applicability №367/№354;
- regulatory watch по отдельной процедуре подтверждения соответствия/реестру C2 providers;
- подтверждение применимости связанных frequency / communications requirements;
- окончательная verification evidence structure.

## 12. Current Repository State and Work Package

Последнее проверенное состояние main:

- current HEAD: 69fe8dfe51b3aa18a56009c178f8243babe6b99a;
- HMI reconciliation HMI-RECONCILIATION-001;
- operational execution architecture through ARCH-OPS-051;
- PH4-INT-005 failure-path propagation fixture;
- PH4-INT-006 operational failure evidence binding;
- PH4-INT-007 failure/evidence execution binding;
- PH4-INT-025 clean-close and archive lifecycle — CLOSED — CI VERIFIED;
- PH4-INT-026 capability manifest and compatibility gate — CLOSED — CI VERIFIED;
- current documentation reconciliation: REPOSITORY-DOCUMENTATION-AUDIT-004.

### Verified PH4 closure evidence

- PH4-INT-025 implementation commit 608785d3dbb703033231db5adfb8643096feba27 has a directly retrievable GitHub Actions run #570 with conclusion success.
- PH4-INT-025 merged main commit: b298a1ea9beac3502bf779b6ac28c8940b5e52a5.
- PH4-INT-026 is merged by PR #55 into main at 5a9ba793c05ed12f5180280b657d7eeb83887085; its controlled status record states CI run #578 succeeded. A direct commit-workflow lookup for the merge commit did not return a workflow run, so this audit does not independently promote that CI claim beyond the controlled project record.
- Both stages remain bounded to SIL_FIXTURE_ONLY; no HIL, physical-UAV, flight-test, certification or real transport evidence is claimed.

### Current deterministic sequence

1. Complete repository/documentation reconciliation against current HEAD.
2. Reconcile requirements ↔ architecture ↔ software ↔ verification.
3. Continue C2 source/clause/applicability closure.
4. Reconcile safety/certification/evidence dependencies.
5. Reconcile AI/data/offline layers.
6. Reconcile HMI/UI traceability.
7. Continue Phase 4 from PH4-INT-027 only after the affected documentation/evidence chain is consistent.
8. Freeze the next implementation work package only after its requirements, design, verification and evidence records are aligned.

The repository-wide audit does not promote draft or working records to certification baseline.

## 13. Project Rules

1. Не называть систему или конфигурацию сертифицированной до получения соответствующего основания/сертификата.
2. Не переносить требования внешней организации автоматически на BlueSky.
3. Разделять:
   - system requirements;
   - external interface requirements;
   - operator requirements;
   - provider requirements;
   - infrastructure requirements.
4. Каждый certification requirement должен иметь applicability и verification method.
5. Regulatory gaps маркировать как `REG-GAP / REGULATORY WATCH`.
6. Не заменять нормативный источник учебными материалами.
7. Не пересматривать approved decisions без документированного основания.
8. При изменении baseline обновлять затронутые связанные документы и traceability.
