---
id: NAVIGATION-HEADING-MODULE-001
type: navigation_module_specification
status: draft_for_agreement
version: 0.3
parent: NAVIGATION
authority: NAVIGATION_STATE_MODEL; NAVIGATION_RULES; NAVIGATION_ALGORITHM; NAVIGATION_MATHEMATICAL_SPECIFICATION_001; MASTER_REQUIREMENTS_REGISTER
---

# BlueSky PRO — Navigation / Heading Module Specification

## 1. Назначение

Модуль `Heading` блока `Navigation` формирует и предоставляет текущее, плановое и расчётное направление продольной оси БВС относительно согласованной системы отсчёта, а также качество и происхождение соответствующего значения.

Модуль не принимает самостоятельно решений о маршруте, не является модулем управления исполнительными органами и не заменяет Navigation State или Flight Control.

## 2. Граница модуля

Внутри:

- получение/приём Heading от доступных навигационных источников;
- нормализация и представление значения;
- различение `PLANNED / SIMULATED / ACTUAL`;
- оценка качества и актуальности;
- фиксация source/provenance;
- предоставление Heading потребителям Navigation, Planning, Flight, Safety и HMI.

Вне модуля:

- управление рулевыми поверхностями, двигателем или иными исполнительными механизмами;
- самостоятельное построение маршрута;
- определение разрешённости воздушного пространства;
- принятие внешних регуляторных решений.

## 3. Модель данных

Минимальная логическая сущность `Heading` должна иметь:

```text
value
angle normalization
reference frame
timestamp
source
freshness
validity
quality state
provenance
context = PLANNED | SIMULATED | ACTUAL
```

Точный API schema и storage schema являются предметом Software Design.

## 4. Источники и provenance

Конкретный перечень датчиков и внешних источников определяется конфигурацией конкретного БВС и интеграционным контрактом.

Источник должен быть идентифицируемым. Для существенного значения сохраняются:

```text
source
timestamp
freshness
validity
quality
confidence / provenance, where applicable
```

При наличии нескольких источников переход между источниками должен быть наблюдаемым и проверяемым. Политика выбора и fusion не фиксируется предположением до согласования.

## 5. Семантика состояний

### 5.1 PLANNED

Плановое направление, полученное из плановой траектории или соответствующего расчёта. Не является фактическим Heading.

### 5.2 SIMULATED

Расчётное направление в моделировании/прогнозировании. Не является фактическим состоянием БВС.

### 5.3 ACTUAL

Фактическое Heading, полученное из соответствующего навигационного источника и сопровождаемое временной меткой, источником и качеством.

```text
PLANNED ≠ SIMULATED ≠ ACTUAL
```

## 6. Связь Heading с Track / Course

`Heading`, `Track` и `Course` являются различными величинами и не должны подменять друг друга.

При наличии ветра направление продольной оси БВС и направление его движения относительно земли могут различаться. Поэтому:

```text
Heading ≠ Track
```

`Course` используется в соответствии с согласованной терминологией и математической моделью маршрута; окончательная терминологическая фиксация должна быть единой во всей Navigation documentation.

## 7. Reference Frames

Все значения Heading должны быть однозначно привязаны к согласованной системе отсчёта.

Преобразование между системами отсчёта выполняется через определённый интерфейс/сервис, а не неявно внутри HMI.

Конкретный перечень reference frames и правила преобразования являются общей ответственностью навигационной спецификации и не дублируются здесь.

До их утверждения конкретные значения остаются `TBD`.

## 8. Quality / Validity

Применяется общая модель качества Navigation:

```text
VALID
DEGRADED
INVALID
STALE
UNAVAILABLE
```

Критерии и численные thresholds устанавливаются только на основании согласованных requirements/design и характеристик конкретного источника.

Устаревшее или недостоверное значение не должно незаметно использоваться как актуальное.

## 9. Входы

```text
Actual Heading source
Simulated Heading
Planned Heading
Timestamp
Source metadata
Quality / validity
Reference frame
Navigation State context
```

Каждый вход должен быть проверен по validity, quality и temporal consistency до использования в зависимых расчётах.

## 10. Выходы

```text
Heading data object
Heading quality
Heading validity
Heading provenance
Heading timestamp/freshness
Context state
```

Контракт интерфейса должен быть versioned и обратно совместимым в пределах утверждённой политики совместимости.

## 11. Связи с другими модулями

```text
Position ───────┐
Velocity/Speed ─┤
Wind ───────────┤
                ↓
             HEADING
                ↓
     ┌──────────┼──────────┐
     ↓          ↓          ↓
Track/Course  Flight     Safety
     ↓          ↓          ↓
Planning       HMI    Navigation State
```

Связь с Wind не означает, что Heading автоматически рассчитывается из Wind. Wind необходим для анализа взаимосвязи Heading, Airspeed и движения относительно земли.

## 12. Failure behaviour

Обрабатываются как минимум:

- отсутствие источника;
- недействительное значение;
- устаревшее значение;
- потеря обновлений;
- конфликт источников;
- некорректный reference frame;
- резкое/физически неправдоподобное изменение;
- ошибка преобразования.

Базовая реакция:

```text
missing source       → UNAVAILABLE
stale value          → STALE
invalid value        → INVALID
source degradation   → DEGRADED
source conflict      → conflict handling / defined fusion policy
```

Модуль не придумывает собственную contingency strategy; downstream response определяется системными и safety требованиями.

## 13. Safety boundary

```text
Heading
  ↓
Navigation State
  ↓
Validation
  ↓
Safety / Decision layers
  ↓
Execution Authority
```

Heading Module самостоятельно не:

- разрешает выполнение миссии;
- изменяет маршрут;
- выдаёт authorization;
- управляет исполнительными механизмами.

## 14. Route Planning / Dynamic Return

Heading может использоваться Planning и Dynamic Return как навигационный вход совместно с Position, Velocity/Speed, Wind, Energy/Resource State и другими необходимыми данными.

Модуль предоставляет состояние; решение об изменении маршрута или возврате принимает соответствующий вышестоящий контур.

## 15. Multi-UAV

Для каждого БВС сохраняется индивидуальное состояние:

```text
UAV[i].Heading
UAV[i].HeadingQuality
UAV[i].HeadingProvenance
```

Fleet-level aggregation не должна уничтожать индивидуальную provenance и quality information.

## 16. HMI allocation

Heading отображается оператору только там, где он необходим для принятия решения или контроля состояния полёта.

HMI должен различать фактическое и плановое/расчётное значение и показывать эксплуатационно значимую деградацию.

Конкретные размеры, typography, расположение и interaction model наследуются от общей HMI Design System; функциональная спецификация не дублирует эти параметры.

## 17. Verification

Verification должна охватывать:

- диапазон и нормализацию угла;
- reference frame;
- `PLANNED / SIMULATED / ACTUAL`;
- timestamp/source/provenance;
- validity/quality;
- stale/unavailable;
- деградацию;
- конфликт источников;
- интерфейсный контракт;
- интеграцию с Position, Velocity/Speed, Wind и Track/Course;
- использование Heading в зависимых Flight/Safety функциях.

Каждое проверяемое требование получает конкретную связь с Test Case/Test Vector и Evidence через Verification layer.

## 18. Regulatory applicability

Применимые нормативные требования определяются через существующий `MASTER_REQUIREMENTS_REGISTER` и certification basis.

Нормативное требование не переносится в Heading автоматически по тематическому совпадению. Сначала устанавливается:

```text
Official clause
→ regulated object
→ BlueSky role
→ system boundary
→ applicability
→ requirement
→ module allocation
→ verification
```

Если требование относится к внешнему БВС, автопилоту, воздушному пространству или иному внешнему объекту, оно не становится функцией BlueSky PRO без доказанной связи с границей ответственности системы.

## 19. Traceability / controlled references

Модуль связан с существующей документальной цепочкой:

```text
01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md
        ↓
01_REQUIREMENTS/SYSTEM/SYSTEM_REQUIREMENTS_SPECIFICATION.md
        ↓
02_ARCHITECTURE/SYSTEM/AVIATION_SYSTEM_DEFINITION.md
        ↓
03_SYSTEM/NAVIGATION/NAVIGATION_STATE_MODEL.md
        ↓
03_SYSTEM/NAVIGATION/NAVIGATION_RULES.md
        ↓
03_SYSTEM/NAVIGATION/NAVIGATION_ALGORITHM.md
        ↓
03_SYSTEM/NAVIGATION/NAVIGATION_MATHEMATICAL_SPECIFICATION_001.md
        ↓
NAVIGATION-HEADING-MODULE-001
        ↓
05_VERIFICATION/NAVIGATION/NAVIGATION_VERIFICATION_MODEL.md
        ↓
05_VERIFICATION/NAVIGATION/NAVIGATION_TRACEABILITY_MATRIX_001.md
        ↓
05_VERIFICATION/NAVIGATION/TEST_VECTORS/NAVIGATION_TEST_VECTORS_001.md
        ↓
05_VERIFICATION/EVIDENCE/VERIFICATION_EVIDENCE_INDEX.md
```

HMI allocation выполняется через `08_HMI/` и не дублирует общий HMI Design System внутри Navigation.

## 20. Open Questions

```text
NAV-HDG-OPEN-001  source set
NAV-HDG-OPEN-002  source switching / fusion policy
NAV-HDG-OPEN-003  quality thresholds
NAV-HDG-OPEN-004  source disagreement limits
NAV-HDG-OPEN-005  reference frames / conversions
NAV-HDG-OPEN-006  update frequency and latency
NAV-HDG-OPEN-007  responsibility split Navigation / Flight / Safety
NAV-HDG-OPEN-008  exact verification Test IDs
NAV-HDG-OPEN-009  HMI presentation rules
```

Открытый вопрос не считается GAP автоматически.

## 21. Completion gate

```text
[ ] responsibility
[ ] data semantics
[ ] source/provenance
[ ] quality model
[ ] reference-frame model
[ ] interfaces
[ ] failure behaviour
[ ] safety allocation
[ ] Planning / Dynamic Return allocation
[ ] HMI allocation
[ ] verification mapping
[ ] regulatory applicability
[ ] requirement traceability
[ ] open items resolved or formally accepted
```

До закрытия применимых пунктов статус остаётся `DRAFT_FOR_AGREEMENT`.

## 22. Status

**DRAFT_FOR_AGREEMENT**

Новых SYS-REQ данным документом не создаётся.

Следующий модуль Navigation определяется только после завершения текущего reconciliation-pass, а не автоматически по факту создания данного файла.
