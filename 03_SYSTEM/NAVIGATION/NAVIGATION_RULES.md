---
id: NAVIGATION-RULES-001
type: navigation_rules
status: draft_for_agreement
authority: derived_from_navigation_state_model
---

# Navigation Rules

## 1. Назначение

Этот документ формализует рабочие правила навигационного слоя на основании:

- накопленной базы знаний проекта;
- `NAVIGATION_KNOWLEDGE_MAP.md`;
- `NAVIGATION_KNOWLEDGE_REVIEW_001.md`;
- `NAVIGATION_STATE_MODEL.md`;
- существующих проектных документов Route Planning, UAV, Mission и Safety.

Документ **не является окончательной реализацией алгоритма**.

---

## 2. Основной принцип

Навигационные правила строятся из проверенных знаний.

```text
Knowledge
→ Concepts
→ Rules
→ Constraints
→ Algorithm
→ Verification
```

Ни одно новое навигационное правило не должно молча заменять ранее принятое.

При конфликте:

```text
CONFLICT
→ RECORD
→ COMPARE SOURCES
→ AGREE
→ UPDATE KNOWLEDGE
```

---

## 3. Rule N-001 — Planned и Actual разделяются

Плановое состояние и фактическое состояние БВС не смешиваются.

```text
PLANNED ≠ ACTUAL
```

`PLANNED` описывает намерение системы.

`ACTUAL` описывает состояние, полученное из фактических источников.

`SIMULATED` представляет расчётное/прогнозируемое состояние и не становится `ACTUAL` без фактического подтверждения.

---

## 4. Rule N-002 — Course, Heading и Track различаются

Запрещено использовать одно поле `course` для разных навигационных смыслов.

```text
Course  → intended path/direction
Heading → direction of aircraft longitudinal axis
Track   → actual ground path
```

Связь между ними может зависеть от ветра и других условий.

---

## 5. Rule N-003 — Bearing является отдельной сущностью

`Bearing` не должен автоматически трактоваться как `Course`, `Heading` или `Track`.

Он описывает направление на объект/точку относительно выбранной системы отсчёта.

---

## 6. Rule N-004 — Airspeed и Groundspeed различаются

```text
Airspeed    ≠ Groundspeed
```

`Airspeed` относится к движению относительно воздушной массы.

`Groundspeed` относится к перемещению относительно поверхности Земли.

Не допускается хранить их в одном универсальном поле `speed`, если это приводит к потере семантики.

---

## 7. Rule N-005 — Drift является производным параметром

`Drift Angle` не должен считаться независимым измеренным состоянием без указания источника.

Рабочая связь:

```text
Heading
+
Track
→ Drift Angle
```

При использовании другого метода оценки источник и метод должны быть явно указаны.

---

## 8. Rule N-006 — Reference Frame обязателен

Для направлений должна быть известна система отсчёта.

Минимально:

```text
TRUE
MAGNETIC
COMPASS
```

Истинные и магнитные значения нельзя молча смешивать.

Преобразование между reference frames должно быть явным.

---

## 9. Rule N-007 — Навигационные данные имеют provenance

Для существенного навигационного значения должна быть возможность определить:

```text
Source
Timestamp
Freshness
Validity
Confidence
```

Это необходимо для последующей validation, safety и traceability.

---

## 10. Rule N-008 — Navigation State не является Execution Authority

Navigation State предоставляет состояние и производные данные.

Он не имеет права непосредственно выполнять действие над БВС.

```text
Navigation
→ Validation
→ Readiness
→ Safety Gate
→ Authorization
→ C++ Core
→ Execution
```

---

## 11. Rule N-009 — Фактическое состояние проверяется относительно плана

При наличии активного маршрута фактическое состояние должно быть сопоставимо с плановым:

```text
ACTUAL
↔
PLANNED ROUTE / ACTIVE WP
```

При существенном отклонении или изменении исходных данных требуется соответствующая повторная проверка.

---

## 12. Rule N-010 — Route и WP имеют версионность контекста

Navigation State должен быть связан с конкретным:

- route;
- route version;
- active waypoint;
- mission version,

если такая идентификация предусмотрена моделью проекта.

Нельзя сравнивать Actual State с неизвестной или устаревшей версией маршрута.

---

## 13. Rule N-011 — Производные значения не маскируются под измеренные

Следует различать:

```text
MEASURED
ESTIMATED
DERIVED
PREDICTED
```

Например:

```text
Position       → measured/estimated
Track          → measured/derived depending on source
Drift Angle    → derived/estimated
CrossTrackError→ derived
ETA            → derived/predicted
ReturnFeasibility → derived/evaluated
```

Конкретная классификация зависит от источника и метода расчёта и должна быть зафиксирована в модели данных.

---

## 14. Rule N-012 — Качество данных влияет на использование

Рабочие состояния:

```text
VALID
DEGRADED
STALE
INVALID
UNAVAILABLE
```

Алгоритм не должен рассматривать `STALE`, `INVALID` или `UNAVAILABLE` как эквивалент актуального `VALID` значения.

Конкретные пороги stale/degraded пока не установлены.

---

## 15. Rule N-013 — Navigation State является общим входом

Navigation State должен использоваться как общий источник навигационного контекста для:

```text
Route Planning
Dynamic Return
Collision Avoidance
Mission Calculation
Mission Execution
Multi-UAV Coordination
Map / Dashboard
Safety Validation
```

Каждый потребитель не должен создавать собственное несовместимое определение базового навигационного состояния.

---

## 16. Rule N-014 — Dynamic Return использует Actual State

Оценка Safe Return должна учитывать актуальное состояние конкретного БВС.

Минимальная логическая связь:

```text
Current Position
+
Current Navigation State
+
Resource/Energy State
+
Communication State
+
Environmental Constraints
+
Mission Constraints
→ Return Feasibility
```

Расчёт только геометрического расстояния не является достаточным определением выполнимости возврата.

---

## 17. Rule N-015 — Multi-UAV сохраняет индивидуальное состояние

При координации нескольких БВС:

```text
Fleet State
```

не заменяет:

```text
UAV-1 Navigation State
UAV-2 Navigation State
...
UAV-N Navigation State
```

Fleet-level decisions должны учитывать состояние каждого участника.

---

## 18. Rule N-016 — Изменение значимого навигационного состояния может инициировать revalidation

При изменении данных, которое способно повлиять на:

- выполнимость миссии;
- маршрут;
- безопасность;
- ресурс;
- связь;
- взаимодействие БВС,

должна быть предусмотрена соответствующая повторная проверка.

```text
STATE CHANGE
→ IMPACT ASSESSMENT
→ REVALIDATION when required
```

---

## 19. Rule N-017 — Map и WP Table не создают собственную навигационную истину

Карта и WP Table должны представлять одно и то же рабочее состояние маршрута.

```text
Authoritative Project State
        ↓
Map
        ↓
WP Table
```

UI-представление не должно становиться самостоятельным источником противоречивой навигационной информации.

---

## 20. Rule N-018 — Safety имеет приоритет

Оптимизация маршрута, AI-предложение или навигационная коррекция не могут обходить Safety Gate.

```text
Navigation optimization
        ↓
Validation
        ↓
Safety Gate
```

Safety-critical constraints имеют приоритет над оптимизацией.

---

## 21. Rule N-019 — AI не получает навигационную authority

AI может:

- анализировать;
- предлагать;
- оценивать;
- формировать proposal.

AI не должен напрямую изменять authoritative Navigation State или выполнять действие над БВС без предусмотренного проектом validation/authorization контура.

---

## 22. Rule N-020 — Источник знания сохраняется

Для каждого существенного правила должна быть возможность установить происхождение:

```text
Source
→ Knowledge Item
→ Rule
→ Requirement / Architecture
→ Implementation
→ Verification
```

Если правило является проектным решением, это должно быть отличимо от утверждения первичного источника.

---

## 23. Правила, которые пока НЕ фиксируем окончательно

Не устанавливаются без дополнительного сравнения и согласования:

- точная система координат;
- vertical datum;
- точная precision отображения/хранения;
- единицы измерения для конкретных навигационных величин;
- допустимые диапазоны;
- stale thresholds;
- confidence calculation;
- source fusion;
- navigation-source priority;
- формула CrossTrackError;
- формула Drift Angle для всех режимов;
- правила true/magnetic conversion;
- точная семантика Course для каждого типа маршрута.

Это **открытые вопросы**, а не недостающие значения, которые следует придумать.

---

## 24. Rule N-021 — Единицы скорости и вертикальной скорости

Внутренняя расчётная единица скорости и вертикальной скорости — **m/s**.

В HMI пользователь может переключать отображение скорости:

```text
m/s ↔ km/h
```

Семантика параметров фиксирована:

```text
GS       → Ground Speed
AS       → Air Speed
CLIMB    → скорость набора
DESCENT  → скорость снижения
```

Термины `CLIMB RATE` и `DESCENT RATE` в HMI не используются.

Для `CLIMB` и `DESCENT` знаки `+` и `-` в индикации не используются: направление определяется названием параметра.

---

## 25. Rule N-022 — Высота HGT / ALT

В навигационной индикации используются два самостоятельных параметра:

```text
HGT → истинная высота над землёй
ALT → барометрическая высота
```

Основное отображение высоты:

```text
HGT < 100 m
→ HGT XX m

HGT ≥ 100 m
→ ALT XXX m
```

Второй параметр остаётся доступным как навигационное значение и не удаляется из Navigation State.

Слово `ALTITUDE` как заголовок основного индикатора не используется.

Формат основного HMI:

```text
HGT 30 m
ALT 115 m
```

Знаки `+` и `-` для HGT/ALT в индикации не используются.

---

## 26. Rule N-023 — Минимальная семантическая индикация

Основные навигационные значения отображаются непосредственно через идентификатор параметра и значение.

Не добавляются:

- отдельный заголовок `ALTITUDE`;
- `RATE` к `CLIMB` или `DESCENT`;
- знаки `+` / `-` для направления CLIMB/DESCENT;
- знаки `+` / `-` для HGT/ALT.

Таким образом, направление и тип величины определяются самим обозначением параметра.

---

## 27. Статус

**DRAFT_FOR_AGREEMENT**

Правила подготовлены из текущей базы знаний и модели Navigation State.

Следующий этап после проверки и согласования:

```text
NAVIGATION RULES
→ NAVIGATION ALGORITHM
```

Но алгоритм строится только из правил, которые прошли согласование.

---

## 28. Контрольная цепочка

Итоговая рабочая цепочка:

```text
SOURCE
→ KNOWLEDGE
→ CONCEPT
→ RULE
→ CONSTRAINT
→ ALGORITHM
→ VALIDATION
→ SAFETY
→ EXECUTION
→ RESULT
→ KNOWLEDGE UPDATE
```
