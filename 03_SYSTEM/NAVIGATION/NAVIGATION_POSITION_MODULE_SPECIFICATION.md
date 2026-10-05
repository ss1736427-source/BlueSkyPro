---
id: NAVIGATION-POSITION-MODULE-001
type: navigation_module_specification
status: draft_for_agreement
version: 0.2
parent: NAVIGATION
authority: NAVIGATION_STATE_MODEL; NAVIGATION_RULES; NAVIGATION_ALGORITHM; NAVIGATION_MATHEMATICAL_SPECIFICATION_001
---

# BlueSky PRO — Navigation / Position Module Specification

## 1. Назначение
Определить модуль `Position` блока `Navigation` как источник и обработчик пространственного положения БВС в составе Navigation State.

Модуль не является исполнительным контуром управления, Safety Gate или Authorization layer.

## 2. Архитектурная граница
```text
Position sources
      ↓
 POSITION MODULE
      ↓
 Navigation State
      ↓
 Planning / Guidance / Safety / HMI
```

## 3. Семантика Position
Положение должно однозначно описываться координатами, системой отсчёта, качеством, источником, временной меткой, свежестью, валидностью и confidence.

Сохраняется различие `PLANNED / SIMULATED / ACTUAL`.

## 4. Источники и provenance
Потенциальные источники определяются существующей архитектурой и конфигурацией конкретного БВС. При нескольких источниках применяется согласованная source/fusion policy; конкретные приоритеты не фиксируются без основания.

Для существенного значения сохраняются `source`, `timestamp`, `freshness`, `validity`, `confidence`.

## 5. Quality states
Используется общая модель Navigation State: `VALID / DEGRADED / STALE / INVALID / UNAVAILABLE / CONFLICTING`.

Качество является самостоятельным атрибутом данных.

## 6. Входы и выходы
Входы: position source(s), time, source metadata, altitude/reference information, mission/navigation state.

Выходы: `Position`, `Position Quality`, `Position Provenance`, `Timestamp`, `Freshness`, `Confidence` и явно маркированные derived products.

## 7. Связи
`Position` используется совместно с Velocity/Speed, Heading, Track/Course и Wind через общий Navigation State.

Изменение положения во времени может давать derived Ground Velocity:
```text
Position(t1) + Position(t2) + Δt → derived Ground Velocity
```

Конкретная математика находится в `NAVIGATION_MATHEMATICAL_SPECIFICATION_001.md`.

Route Planning использует Position для текущего положения БВС, контроля маршрута, waypoint proximity, route deviation и связанных расчётов.

Dynamic Return использует актуальное Position с quality/freshness/provenance. Модуль предоставляет данные и не принимает решение о возврате.

## 8. Multi-UAV
Для каждого БВС сохраняется индивидуальное состояние `UAV[i].Position`, включая quality и provenance. Fleet aggregation не должна уничтожать индивидуальные сведения.

## 9. Offline и отказные состояния
При потере внешней связи используются доступные локальные/onboard источники. Состояния явно деградируют:
```text
NO SOURCE → UNAVAILABLE
STALE DATA → STALE
INVALID DATA → INVALID
DEGRADED SOURCE → DEGRADED
```

Конфликт источников обрабатывается установленной fusion policy. Численные пороги без подтверждённого основания не вводятся.

## 10. Safety boundary
```text
Position → Navigation State → Dependent calculations → Validation → Safety / Decision layers
```

Position Module самостоятельно не изменяет маршрут, не разрешает полёт, не выдаёт authorization и не управляет исполнительными механизмами.

## 11. HMI
Эксплуатационный HMI отображает только необходимые оператору пространственные данные и эксплуатационное качество. Конкретный layout и визуальная кодировка определяются общей HMI-проработкой Navigation.

## 12. Verification
Проверки должны охватывать nominal position, updates, source loss, stale/invalid/degraded states, source conflict, time consistency, derived velocity, route deviation, waypoint proximity, Dynamic Return input, offline operation и Multi-UAV state.

Конкретные Test ID назначаются через существующий Verification Register.

## 13. Regulatory applicability
Применимость устанавливается только через:
```text
Official source / clause → regulated object → BlueSky role → system boundary → applicability → requirement → module allocation → verification
```

Тематическое совпадение само по себе не создаёт требования Position.

## 14. Traceability
```text
NAVIGATION_STATE_MODEL
        ↓
NAVIGATION_RULES
        ↓
NAVIGATION_ALGORITHM
        ↓
NAVIGATION_MATHEMATICAL_SPECIFICATION_001
        ↓
POSITION MODULE
        ↓
NAVIGATION HMI / INTERFACES
        ↓
NAVIGATION_VERIFICATION_MODEL
        ↓
NAVIGATION_TRACEABILITY_MATRIX_001
```

Связь с системными и нормативными требованиями определяется записями Master Requirements Register и allocation records; неподтверждённые ссылки не создаются.

## 15. Open items
```text
NAV-POS-OPEN-001  authoritative position source set
NAV-POS-OPEN-002  source priority / fusion policy
NAV-POS-OPEN-003  reference-frame details
NAV-POS-OPEN-004  altitude semantics
NAV-POS-OPEN-005  precision / accuracy thresholds
NAV-POS-OPEN-006  freshness / latency thresholds
NAV-POS-OPEN-007  confidence model
NAV-POS-OPEN-008  exact verification Test IDs
NAV-POS-OPEN-009  HMI thresholds / presentation
```

Открытый вопрос не считается GAP автоматически.

## 16. Completion gate
```text
[ ] responsibility
[ ] position semantics
[ ] coordinate/reference model
[ ] source model
[ ] provenance
[ ] quality model
[ ] temporal consistency
[ ] Velocity relationship
[ ] Heading / Track relationship
[ ] Wind relationship
[ ] Planning interface
[ ] Dynamic Return interface
[ ] failure behaviour
[ ] safety allocation
[ ] HMI allocation
[ ] verification mapping
[ ] regulatory applicability
[ ] requirement traceability
```

До закрытия применимых пунктов статус остаётся `DRAFT_FOR_AGREEMENT`.
