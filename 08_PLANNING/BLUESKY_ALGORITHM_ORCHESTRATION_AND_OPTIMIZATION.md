# BlueSky PRO — Algorithm Orchestration & Mission Optimization

**Status:** ACCEPTED — архитектурное решение

## 1. Назначение

BlueSky PRO использует не один универсальный алгоритм планирования, а набор специализированных алгоритмов, выбираемых и комбинируемых автоматически в зависимости от структуры задачи, характеристик конкретного БПЛА, полезной нагрузки, условий среды и требований к результату.

Пользователь ставит операционную задачу. Он не обязан выбирать алгоритм, метод поиска или параметры оптимизатора. BlueSky самостоятельно формирует способ решения, рассчитывает и сравнивает допустимые варианты и выбирает наиболее подходящий.

Если выбранный маршрут неочевиден для пилота, система кратко объясняет ключевую причину выбора. Объяснение должно быть тезисным и не перегружать оператора внутренней математикой или названиями алгоритмов.

## 2. Основной принцип

```text
USER TASK
   ↓
TASK ANALYSIS
   ↓
MISSION / VEHICLE / PAYLOAD / ENVIRONMENT MODEL
   ↓
ALGORITHM ORCHESTRATOR
   ↓
SPECIALIZED PLANNERS + OPTIMIZERS
   ↓
CANDIDATE SOLUTIONS
   ↓
HARD SAFETY / FEASIBILITY VALIDATION
   ↓
ENERGY / BATTERY / ENGINE RESOURCE EVALUATION
   ↓
MISSION QUALITY EVALUATION
   ↓
BEST VALID SOLUTION
   ↓
FINAL VALIDATION
   ↓
AUTOPILOT / EXECUTION
```

Оркестратор является диспетчером вычислений, а не дополнительным последовательным этапом, который должен замедлять подготовку миссии.

## 3. Иерархия принятия решения

В BlueSky критерии разделяются на обязательные ограничения и оптимизационные цели.

### 3.1. Hard constraints

Нарушение обязательного ограничения автоматически исключает решение из числа допустимых. Оптимизация не имеет права обходить Safety Gate.

Примеры:
- воздушное пространство;
- геозоны;
- высотные ограничения;
- рельеф и препятствия;
- минимальное безопасное разделение;
- технические ограничения БПЛА;
- C2;
- аварийные процедуры;
- требуемый энергетический резерв;
- регуляторные ограничения.

### 3.2. Mission objectives

После прохождения обязательных ограничений сравниваются варианты по требованиям конкретной задачи.

Общий принцип приоритета:

```text
SAFETY / FEASIBILITY
        ↓
ENERGY RESERVE
        ↓
ENGINE / PROPULSION RESOURCE
        ↓
MISSION QUALITY
        ↓
TIME / ETA
```

Эта последовательность является базовой, но конкретная задача может изменять относительный приоритет качества, времени и ресурса. Обязательные ограничения безопасности не изменяются профилем задачи.

## 4. Mission Objective Profiles

BlueSky не требует от пилота вручную задавать математические веса. Для каждого типа операции система использует соответствующий профиль целей и может автоматически адаптировать его по контексту.

### 4.1. Аэрофотосъёмка / картография

Главный критерий — качество и полнота требуемых данных.

Учитываются:
- требуемое покрытие;
- GSD;
- продольное и поперечное перекрытие;
- скорость съёмки;
- высота;
- положение и ориентация сенсора;
- возможности камеры;
- освещённость и погодные условия;
- энергетический резерв.

### 4.2. 3D / объёмная съёмка

Главный критерий — геометрическое качество реконструкции.

Учитываются:
- полнота покрытия поверхности;
- требуемые ракурсы;
- углы наблюдения;
- перекрытие;
- высота и профиль;
- ограничения камеры/подвеса;
- энергетический резерв;
- время выполнения.

### 4.3. Поиск / разведка

Главный критерий — полнота и вероятность обнаружения объекта/события.

Учитываются:
- покрытие зоны;
- модель обнаружения;
- высота и скорость;
- характеристики сенсора;
- вероятность пропуска;
- возможность подтверждения другим БПЛА;
- C2;
- энергия и резерв.

### 4.4. Инспекция

Главный критерий — получение требуемого набора инспекционных данных.

Учитываются:
- точки/секторы осмотра;
- требуемая детализация;
- ракурсы;
- дистанция до объекта;
- ограничения полезной нагрузки;
- стабильность наблюдения;
- энергия;
- время.

### 4.5. Доставка груза

Главный критерий — гарантированное выполнение доставки.

Учитываются:
- масса и габариты груза;
- совместимость с UAV;
- энергетический баланс;
- резерв на возврат/contingency;
- безопасная зона доставки;
- время/ETA;
- ресурс силовой установки.

### 4.6. Длинная BVLOS-миссия

Главный критерий — надёжное выполнение всей операции.

Учитываются:
- энергетический резерв;
- дальность;
- C2 coverage и resilience;
- weather;
- резервирование;
- ресурс UAV;
- contingency;
- возможность динамического replanning.

### 4.7. Групповая миссия

Главный критерий — совместное выполнение единой задачи всей группой.

Учитываются:
- capability matching;
- распределение задач;
- роли UAV;
- пространственное и временное разделение;
- высоты;
- C2 и relay;
- батарея каждого UAV;
- payload;
- зависимости между задачами;
- резервные аппараты;
- data fusion.

## 5. Оркестр алгоритмов

BlueSky должен иметь расширяемый набор расчётных модулей. Добавление нового алгоритма не должно требовать переделки Mission Core.

### 5.1. Global graph planning

**Dijkstra** — базовый детерминированный алгоритм поиска по графу. Используется как reference/global planner там, где пространство удобно представить графом и важны предсказуемость и трассируемость.

**A\*** — ускоренный глобальный поиск с эвристикой. Предпочтителен, когда требуется быстро найти путь к известной цели в большом графе.

### 5.2. Dynamic replanning

**D\* / D\*-Lite** — специализированный механизм перестройки маршрута при изменении стоимости или доступности участков пространства. Используется прежде всего при динамических изменениях во время выполнения.

### 5.3. Coverage planning

Отдельный класс алгоритмов для покрытия площади, линии, объекта или объёмной поверхности:
- lawnmower / boustrophedon;
- cellular decomposition;
- sweep-based planning;
- специализированные 2D/3D coverage methods.

Coverage Planner не заменяется Dijkstra: он решает другую математическую задачу. Графовые алгоритмы могут использоваться внутри него для переходов между рабочими участками.

### 5.4. Multi-UAV task allocation

Для распределения работы между аппаратами могут использоваться:
- Hungarian assignment;
- MILP;
- auction / market-based methods;
- greedy / heuristic methods;
- metaheuristics для сложных больших задач.

Выбор метода зависит от размера флота, связности задач, количества ограничений и времени расчёта.

### 5.5. Sampling-based planning

**RRT / RRT\*** используются для непрерывного и сложного 3D-пространства, когда графовая дискретизация неудобна или необходимо учитывать сложные кинематические ограничения.

### 5.6. Multi-objective optimization

Для конфликтующих целей могут применяться Pareto-based методы, включая NSGA-II и другие подходящие оптимизаторы.

Их роль — искать компромисс между несколькими допустимыми решениями, а не заменять обязательный Safety Gate.

### 5.7. Trajectory optimization / MPC

Используется на уровне траектории и динамического выполнения, особенно когда необходимо учитывать:
- ветер;
- динамику UAV;
- энергетические ограничения;
- ограничения управления;
- прогноз состояния;
- изменения среды.

## 6. Алгоритм выбирается по задаче, а не по названию UAV

Для каждого конкретного UAV BlueSky оценивает:
- тип платформы;
- аэродинамику;
- массу;
- полезную нагрузку;
- двигатель/силовую установку;
- батарею;
- degradation coefficient;
- доступную энергию;
- ресурс двигателя;
- ограничения скорости/высоты;
- C2;
- текущий health/state.

Один и тот же mission может использовать разные алгоритмы для разных UAV.

Пример:

```text
MISSION
   ↓
UAV-01 → A* + energy optimization
UAV-02 → Dijkstra + coverage optimization
UAV-03 → RRT* + trajectory optimization
   ↓
GROUP COORDINATION
   ↓
GLOBAL VALIDATION
```

## 7. Ветер — второй этап перерасчёта

Базовая задача сначала решается с использованием геометрии, ограничений, возможностей UAV и требований к покрытию/результату.

После формирования рабочего варианта к нему применяются актуальные метеоданные.

```text
TASK
 ↓
COVERAGE / ALLOCATION / BASE ROUTE
 ↓
CURRENT WIND FIELD
 ↓
WIND-AWARE RECALCULATION
 ↓
GROUND SPEED
 ↓
TIME / ETA
 ↓
POWER / ENERGY
 ↓
BATTERY RESERVE
 ↓
ENGINE RESOURCE
 ↓
RE-OPTIMIZATION / CORRECTION
```

Ветер учитывается не одним глобальным значением, а по возможности как пространственно-временное поле по маршруту и высоте.

Для каждого сегмента оцениваются как минимум:
- направление и скорость ветра;
- воздушная скорость;
- путевая скорость;
- время;
- энергетические затраты;
- доступный резерв.

Изменение ветра не должно автоматически заставлять пересчитывать всю миссию с нуля. Неизменившиеся результаты должны переиспользоваться.

## 8. Энергия батареи — критический критерий

Энергетический резерв является не просто целью оптимизации, а частью проверки выполнимости.

```text
AVAILABLE ENERGY
      >
MISSION ENERGY
 + REQUIRED RESERVE
```

Расчёт должен учитывать, где применимо:
- текущий SOC;
- battery health;
- degradation coefficient;
- температуру;
- массу;
- payload;
- профиль скорости;
- набор/снижение высоты;
- ветер;
- силовую установку;
- contingency;
- требуемый резерв.

Маршрут с недостаточным резервом не может быть выбран как лучший экономический вариант.

## 9. Ресурс двигателя

Система должна учитывать не только текущий расход энергии, но и влияние выбранного режима полёта на ресурс силовой установки.

Сравниваются допустимые варианты по:
- длительности работы;
- режимам мощности;
- нагрузке;
- характеру изменений тяги;
- количеству/характеру манёвров, где это применимо;
- установленному оборудованию;
- модели ресурса конкретного UAV.

При прочих равных BlueSky предпочитает вариант, уменьшающий ресурсное потребление.

## 10. Быстродействие оркестратора

BAO не должен последовательно запускать все имеющиеся алгоритмы на максимальной точности.

Используется многоуровневый расчёт:

### Tier 1 — Fast Plan

Быстро формируется первичный рабочий вариант.

### Tier 2 — Candidate Generation

Запускаются только релевантные алгоритмы; независимые расчёты могут выполняться параллельно.

### Tier 3 — Accurate Evaluation

Детальная энергетическая, ветровая, ресурсная и качественная оценка применяется только к небольшому числу лучших кандидатов.

### Tier 4 — Final Validation

Финальный вариант проходит полный набор обязательных проверок.

## 11. Cache и incremental planning

Результаты неизменившихся стадий должны сохраняться.

Например:

```text
BASE MISSION PLAN
      ↓
WIND UPDATE
      ↓
не пересчитываем:
- задачу;
- decomposition;
- неизменившееся распределение;
- неизменившуюся геометрию
      ↓
пересчитываем:
- ground speed;
- time;
- energy;
- reserve;
- affected route segments
```

При динамическом изменении среды применяется incremental replanning, а не полный пересчёт без необходимости.

## 12. Algorithm Selection

BAO оценивает задачу по признакам:
- тип операции;
- размер пространства;
- плотность препятствий/ограничений;
- динамичность среды;
- число UAV;
- неоднородность флота;
- требования покрытия;
- требования времени;
- требования качества;
- энергетические ограничения;
- требования C2;
- вычислительный бюджет.

На основании этих признаков выбираются один или несколько подходящих методов.

AI может помогать выбирать стратегию, параметры и кандидатов на основе накопленной статистики, но не имеет права обходить Safety Engine, Mission Validation или Mission Readiness.

## 13. Критерий выбора лучшего решения

BlueSky не выбирает «самый короткий маршрут» как универсальную цель.

Выбирается **лучшее допустимое решение для данной задачи**.

Логика:

```text
ALL CANDIDATES
      ↓
SAFETY / REGULATORY / TECHNICAL GATE
      ↓
REMOVE INVALID
      ↓
ENERGY / BATTERY RESERVE
      ↓
REMOVE INSUFFICIENT
      ↓
ENGINE RESOURCE
      ↓
MISSION QUALITY
      ↓
TIME / ETA
      ↓
BEST VALID PLAN
```

При конфликте целей используется соответствующий Mission Objective Profile и Pareto/многоцелевой оптимизатор, если это оправдано.

## 14. Объяснение решения пилоту

BlueSky не обязан показывать оператору название применённого алгоритма.

Если маршрут очевиден — дополнительное объяснение не требуется.

Если маршрут неочевиден, система показывает краткие тезисы:

```text
Почему такой маршрут?

• Сильный встречный ветер на прямом участке.
• Обход снижает расход энергии.
• Резерв батареи увеличен.
• Ограничения воздушного пространства соблюдены.
• Требуемое качество съёмки сохранено.
```

Это является объяснением **решения**, а не внутренней математики алгоритма.

## 15. Dynamic orchestration во время полёта

При изменении условий:

```text
MONITOR
 ↓
DETECT CHANGE
 ↓
REVALIDATE
 ↓
ORCHESTRATOR
 ↓
SELECT / COMBINE ALGORITHM
 ↓
REPLAN / ADAPT
 ↓
VALIDATE
 ↓
AUTOPILOT
```

Примеры изменений:
- ветер;
- батарея;
- состояние UAV;
- потеря/ухудшение C2;
- новое препятствие;
- изменение airspace/geofence;
- отказ UAV;
- изменение состава флота.

Оркестратор может заменить алгоритм или перераспределить задачу, если прежний метод больше не является подходящим.

## 16. Multi-UAV orchestration

Группа рассматривается как единая система выполнения задачи.

Решение должно одновременно учитывать:
- task allocation;
- role assignment;
- route;
- timing;
- altitude;
- spatial separation;
- C2;
- relay;
- energy balance;
- payload;
- dependencies;
- reserve UAV.

После индивидуальной оптимизации каждого UAV проводится глобальная проверка группы.

## 17. Traceability

Для каждого принятого решения должна сохраняться связь:

```text
MISSION
 → MISSION REVISION
 → INPUT DATA / VERSIONS
 → ALGORITHMS USED
 → PARAMETERS
 → CANDIDATES
 → VALIDATION RESULTS
 → SELECTED PLAN
 → EXPLANATION
 → EXECUTION RESULT
```

Это необходимо для анализа, повторного расчёта, simulation/replay, диагностики и доказуемости результата.

## 18. Simulation / Digital Twin

Оркестрация должна быть полностью воспроизводима в Simulation/Digital Twin.

Должны проверяться:
- разные алгоритмы;
- разные UAV;
- ветер;
- энергия;
- двигатель;
- C2;
- multi-UAV coordination;
- contingencies;
- dynamic replanning.

Целевой цикл:

```text
SIMULATE
 → EVALUATE
 → OPTIMIZE
 → APPROVE
 → EXECUTE
```

## 19. Архитектурное решение

BlueSky PRO принимает следующие решения:

1. **Algorithm Portfolio** — в системе предусматривается набор специализированных алгоритмов.
2. **Algorithm Orchestrator** — BAO автоматически выбирает и комбинирует алгоритмы по назначению.
3. **No manual algorithm selection required** — пилот ставит задачу, а не выбирает математический метод.
4. **Task-specific optimization** — приоритеты зависят от типа миссии и требуемого результата.
5. **Safety first** — обязательные safety/regulatory/technical constraints не могут быть отменены оптимизацией.
6. **Energy first among optimization-critical resources** — достаточность батареи и требуемый резерв являются обязательной проверкой.
7. **Engine resource awareness** — ресурс силовой установки учитывается при сравнении допустимых вариантов.
8. **Quality is mission-dependent** — качество результата имеет приоритет там, где оно определяет успех задачи: съёмка, 3D, поиск, инспекция и т. п.
9. **Wind-aware refinement** — актуальный ветер применяется после построения базового решения и вызывает необходимый перерасчёт.
10. **Different algorithms for different UAVs** — один mission может использовать разные методы для разных аппаратов.
11. **Parallel and progressive computation** — оркестрация не должна создавать ненужную задержку подготовки миссии.
12. **Caching/incremental replanning** — неизменившиеся результаты не пересчитываются.
13. **Explainable result** — неочевидные решения объясняются оператору кратко и по существу.
14. **AI subordinate to safety** — AI может помогать выбирать стратегию и параметры, но не может обходить Safety Engine, Mission Validation и Mission Readiness.
15. **Full traceability** — каждое существенное расчётное решение должно быть воспроизводимо и связано с входными данными и версией миссии.

## 20. Итоговая модель

```text
                         BLUE SKY PRO
                              │
                        USER MISSION
                              │
                              ▼
                       TASK ANALYZER
                              │
                              ▼
                   MISSION OBJECTIVE MODEL
                              │
                              ▼
                    ALGORITHM ORCHESTRATOR
                              │
          ┌───────────────────┼───────────────────┐
          ▼                   ▼                   ▼
      COVERAGE             ROUTING            ALLOCATION
      PLANNERS             PLANNERS              │
          │             Dijkstra / A*            │
          │             D* / D*-Lite              │
          │             RRT*                      │
          └───────────────────┼───────────────────┘
                              ▼
                   WIND / PERFORMANCE MODEL
                              │
                              ▼
                  ENERGY / BATTERY / ENGINE
                              │
                              ▼
                    MULTI-OBJECTIVE OPTIMIZER
                              │
                              ▼
                     SAFETY / VALIDATION
                              │
                              ▼
                        BEST VALID PLAN
                              │
                              ▼
                     EXPLAIN TO OPERATOR
                              │
                              ▼
                           AUTOPILOT
                              │
                              ▼
                         EXECUTION
                              │
                              ▼
                       MONITOR / REPLAN
                              │
                              └──────→ ORCHESTRATOR
```

**Решение:** BlueSky PRO строится как система оркестрации специализированных алгоритмов, а не как система, привязанная к одному алгоритму. Оркестратор автоматически выбирает наиболее подходящий метод или их комбинацию для конкретной задачи и конкретного UAV, при этом безопасность, выполнимость и энергетический резерв имеют обязательный приоритет, а качество результата определяется типом миссии.


## 21. Mission Template Algorithm Profiles

**Status:** WORKING ALGORITHM BASELINE — 2026-10-09  
**Scope:** связывает утверждённые пользовательские mission templates с внутренними классами планировщиков, оптимизаторов и обязательными этапами валидации.

Этот раздел не превращает шаблон в фиксированный алгоритм. Шаблон задаёт **операционную задачу**, после чего BAO выбирает конкретную реализацию по геометрии, UAV, payload, среде, ограничениям и вычислительному бюджету.

### 21.1. Общий контракт

Для каждого шаблона применяется базовая цепочка:

```text
TASK
 ↓
TEMPLATE / OBJECTIVE PROFILE
 ↓
TASK GEOMETRY + CONSTRAINTS
 ↓
TASK-SPECIFIC PLANNER
 ↓
ROUTE / COVERAGE / ALLOCATION CANDIDATES
 ↓
WIND + VEHICLE PERFORMANCE
 ↓
ENERGY / RESERVE / RESOURCE
 ↓
TRAJECTORY
 ↓
SAFETY / REGULATORY / C2 / QUALITY VALIDATION
 ↓
BEST VALID PLAN
```

Для multi-UAV задач между генерацией кандидатов и wind/performance добавляются:

```text
TASK DECOMPOSITION
 ↓
ZONE / TASK ALLOCATION
 ↓
ROUTE-IN-ZONE
 ↓
4D TRAJECTORY
 ↓
4D CONFLICT VERIFICATION
```

### 21.2. Матрица 13 шаблонов

| ID | Шаблон | Основной планировщик | Дополнительные методы | Главный критерий |
|---|---|---|---|---|
| MT-01 | Картографирование территории | Coverage Planner: cellular decomposition / lawnmower | Dijkstra/A* для переходов; wind/energy optimization | полнота покрытия + качество данных |
| MT-02 | 3D-картография / реконструкция | 3D coverage / viewpoint planning | sweep; graph transitions; RRT* при сложной 3D-геометрии; trajectory optimization | геометрическая полнота и качество реконструкции |
| MT-03 | Инспекция объектов и инфраструктуры | Viewpoint / inspection-path planning | graph planning; RRT* для сложных пространственных подходов; trajectory optimization | получение требуемых инспекционных ракурсов и данных |
| MT-04 | Мониторинг строительства | Repeatable coverage / corridor planning | cellular decomposition; graph transitions; temporal comparison; incremental recalculation | сопоставимая полнота наблюдения во времени |
| MT-05 | Мониторинг территории и периметра | Patrol / corridor planning | graph shortest path; coverage; revisit scheduling; multi-UAV allocation | непрерывность/полнота наблюдения |
| MT-06 | Поиск и спасение | Search Coverage Planner | cellular decomposition; detection-oriented candidate comparison; D*/D*-Lite для изменения обстановки; multi-UAV allocation | вероятность и полнота обнаружения |
| MT-07 | Пожарный мониторинг и ЧС | Adaptive Coverage Planner | search/recon coverage; dynamic replanning; multi-UAV allocation; wind/performance | актуальная полнота наблюдения при изменяющейся обстановке |
| MT-08 | Экологический и природный мониторинг | Survey / sampling coverage | transect/grid planning; revisit scheduling; graph transitions | полнота и репрезентативность наблюдений |
| MT-09 | Сельское хозяйство | Precision Coverage Planner | grid/lawnmower; cellular decomposition; repeatable route generation; payload-aware optimization | полнота и повторяемость полевого покрытия |
| MT-10 | Доставка грузов | Point-to-point constrained route planning | Dijkstra/A*; wind/energy optimization; trajectory optimization | гарантированная доставка + энергетический резерв |
| MT-11 | Ретрансляция связи | Connectivity-aware mission planning | multi-UAV allocation; graph planning; trajectory optimization | обеспечение требуемой связности C2/relay |
| MT-12 | Аэрофотосъёмка и медиапроизводство | Viewpoint / shot-sequence planning | graph transitions; trajectory smoothing/optimization; payload/gimbal constraints | выполнение набора требуемых кадров/ракурсов |
| MT-13 | C-UAS — обнаружение БПЛА | Search / reconnaissance planning | detection-area coverage; multi-UAV allocation; dynamic replanning; tracking/analytics integration | обнаружение, классификация и сопровождение в пределах разрешённой задачи |

### 21.3. Правило выбора алгоритма внутри шаблона

Один шаблон не означает один алгоритм.

BAO выбирает комбинацию по следующим признакам:

1. геометрия задачи;
2. 2D/3D характер пространства;
3. плотность препятствий и ограничений;
4. необходимость полного покрытия или выборочных наблюдений;
5. количество UAV;
6. однородность/неоднородность флота;
7. характеристики payload;
8. динамичность среды;
9. wind field;
10. energy/reserve;
11. C2;
12. требования качества;
13. требуемое время расчёта.

### 21.4. Типовые цепочки

#### Coverage-класс

Для MT-01, MT-02, MT-04, MT-08, MT-09:

```text
MISSION AREA
 ↓
CONSTRAINED OPEN SPACE
 ↓
DECOMPOSITION
 ↓
COVERAGE CELLS / TRACKS
 ↓
CELL TRANSITION ROUTING
 ↓
WIND + PERFORMANCE
 ↓
ENERGY / QUALITY
 ↓
TRAJECTORY
 ↓
VALIDATION
```

#### Inspection / viewpoint-класс

Для MT-03 и MT-12:

```text
TARGET / SURFACE / SHOT REQUIREMENTS
 ↓
VIEWPOINT GENERATION
 ↓
FEASIBLE VIEWPOINT FILTER
 ↓
VIEWPOINT SEQUENCING
 ↓
TRANSITION ROUTING
 ↓
WIND + PERFORMANCE
 ↓
PAYLOAD / QUALITY CHECK
 ↓
TRAJECTORY
 ↓
VALIDATION
```

#### Search / reconnaissance-класс

Для MT-06, MT-07 и MT-13:

```text
SEARCH / OBSERVATION AREA
 ↓
CONSTRAINED SEARCH SPACE
 ↓
COVERAGE / OBSERVATION CANDIDATES
 ↓
DETECTION / SENSOR EVALUATION
 ↓
WIND + PERFORMANCE
 ↓
ENERGY / C2
 ↓
TRAJECTORY
 ↓
VALIDATION
```

Для MT-07 и MT-13 динамические изменения среды/объектов могут инициировать incremental replanning; для MT-06 это также применяется при изменении поисковой обстановки.

#### Point-to-point delivery-класс

Для MT-10:

```text
ORIGIN / DESTINATION
 ↓
CONSTRAINED OPEN SPACE
 ↓
Dijkstra / A* CANDIDATES
 ↓
UAV + PAYLOAD FEASIBILITY
 ↓
WIND + PERFORMANCE
 ↓
ENERGY + RETURN / CONTINGENCY RESERVE
 ↓
TRAJECTORY
 ↓
DELIVERY / RECOVERY VALIDATION
```

#### Multi-UAV coordination

Для любого шаблона при участии нескольких UAV:

```text
PARENT TASK
 ↓
TASK DECOMPOSITION
 ↓
ZONE / TASK PARTITION
 ↓
UAV ↔ TASK / ZONE ASSIGNMENT
 ↓
ROUTE-IN-ZONE
 ↓
WIND + PERFORMANCE
 ↓
4D TRAJECTORY
 ↓
4D CONFLICT VERIFY
 ↓
GROUND CONFLICT RESOLUTION IF REQUIRED
 ↓
4D RE-VERIFY
 ↓
FINAL VALIDATION
```

Зональная пространственная деконфликтность является предпочтительным механизмом. Start-delay и вертикальная коррекция остаются fallback-механизмами в соответствии с действующей Multi-UAV спецификацией.

### 21.5. Специальные замечания

**MT-01 / MT-09.** Повторяемая геометрия является самостоятельным преимуществом: при неизменных входах сохраняются decomposition и route geometry; пересчитываются только затронутые зависимости.

**MT-02 / MT-03 / MT-12.** Payload и требуемая геометрия наблюдения являются частью допустимости кандидата, а не только параметрами последующей оценки.

**MT-06 / MT-07 / MT-13.** Detection/tracking analytics не становятся частью Safety Authority. Аналитика формирует данные/кандидаты, а planning и readiness проходят независимые обязательные проверки.

**MT-10.** Энергетический резерв на завершение операции и предусмотренное восстановление/contingency является hard feasibility condition.

**MT-11.** Для relay-миссии конкретная модель требуемой C2/connectivity objective ещё требует отдельного формального verification contract; настоящий раздел фиксирует только архитектурную привязку, не утверждая численные пороги.

### 21.6. Граница текущей проработки

На этом этапе зафиксированы:

- связь всех 13 утверждённых шаблонов с классами планирования;
- типовые цепочки расчёта;
- граница автоматического выбора алгоритмов;
- связь шаблонов с Objective Profiles;
- применение wind/performance/energy/trajectory stages;
- multi-UAV orchestration boundary.

Не зафиксированы как окончательные:

- конкретные численные коэффициенты оптимизации;
- calibration values;
- sensor-specific detection models;
- UAV-specific performance curves;
- нормативные separation values;
- production implementation details.

Они должны появляться только из соответствующих controlled requirements, vehicle/payload data, verification evidence или отдельных утверждённых design decisions.

### 21.7. Следующий детерминированный этап

После фиксации матрицы следующим уровнем является разработка **Template Algorithm Contracts**: для каждого MT-01…MT-13 определить входы, выходы, hard constraints, quality metrics, invalidation dependencies и минимальный позитивный/негативный verification scenario.

Первая реализационная цепочка для проверки должна оставаться детерминированной и малой: **MT-01 Картографирование территории → один UAV → одна зона → coverage → route → wind/performance → energy → trajectory → final validation**.


## 22. Template Algorithm Contract — MT-01

**Status:** WORKING CONTRACT — first template implementation slice  
**Template:** MT-01 — Картографирование территории

### 22.1. Назначение

MT-01 формирует маршрут для получения требуемого картографического покрытия заданной территории с сохранением допустимой геометрии съёмки, энергетического резерва и обязательных safety/regulatory constraints.

Оператор задаёт задачу и требования к результату. Выбор конкретного coverage/search/routing backend выполняет BAO.

### 22.2. Входной контракт

Минимальный набор входов:

- mission ID/version;
- operational area geometry;
- mandatory/prohibited geometry;
- terrain/elevation and obstacle state;
- airspace/restriction/authorization-qualified state;
- operational time window;
- UAV configuration;
- payload/camera configuration;
- battery/SOC/health/degradation state;
- performance envelope;
- C2 constraints;
- required mapping quality;
- applicable objective profile;
- current environmental/wind snapshot.

Каждый вход, влияющий на расчёт, должен иметь идентификатор версии/снимка либо эквивалентную provenance-ссылку.

### 22.3. Расчётный контракт

~~~text
MISSION AREA
 ↓
CONSTRAINED OPEN SPACE
 ↓
COVERAGE DECOMPOSITION
 ↓
COVERAGE TRACK GENERATION
 ↓
CELL TRANSITION ROUTING
 ↓
ROUTE FEASIBILITY
 ↓
WIND + VEHICLE PERFORMANCE
 ↓
ENERGY / RESERVE
 ↓
TRAJECTORY
 ↓
MAPPING QUALITY
 ↓
FINAL VALIDATION
~~~

Для покрытия территории базовым reference-классом остаётся deterministic coverage planning; графовый planner используется для допустимых переходов между рабочими участками, а не как замена coverage planner.

### 22.4. Hard constraints

Кандидат MT-01 отклоняется, если нарушается применимое обязательное ограничение, включая:

- regulatory/airspace restriction;
- отсутствие требуемой authorization в её области действия;
- terrain/obstacle clearance;
- UAV operating envelope;
- payload/camera operating limits;
- C2 requirement;
- minimum energy reserve;
- applicable safety/separation constraint;
- mandatory mission geometry;
- физическая невозможность выполнить требуемое покрытие.

Ни один improvement по времени, расстоянию или качеству не может сделать такой кандидат допустимым.

### 22.5. Критерии качества

После прохождения hard gates кандидаты сравниваются по:

1. coverage completeness;
2. соответствию требуемому качеству картографических данных;
3. требованиям GSD/overlap, если они заданы mission profile и payload capability;
4. energy efficiency при сохранении требуемого резерва;
5. propulsion/resource consumption;
6. flight time / ETA;
7. route complexity/smoothness как tie-breaker.

Конкретные численные значения не фиксируются этим контрактом без соответствующего controlled requirement или payload/UAV data source.

### 22.6. Выходной контракт

Успешный расчёт создаёт versioned результат, содержащий как минимум:

- mission/version reference;
- selected UAV/payload;
- coverage decomposition;
- ordered route/waypoints;
- altitude/profile information;
- spatial feasibility result;
- wind/performance result;
- energy estimate and reserve result;
- trajectory;
- mapping-quality result;
- source/dependency versions;
- algorithm/backend versions;
- validation result;
- provenance.

Результат должен быть пригоден для downstream Flight Profile, Mission Package и final validation без повторного независимого расчёта тех же величин.

### 22.7. Invalidation dependencies

Изменение:

| Изменение входа | Инвалидируемые результаты |
|---|---|
| mission area / geometry | decomposition → coverage → route → performance → energy → trajectory → quality → final validation |
| restriction / authorization scope | affected constrained space → coverage → route → downstream stages |
| terrain / obstacle data | affected spatial cells/routes → downstream stages |
| UAV configuration | assignment/feasibility → performance → energy → trajectory → quality → final validation |
| payload/camera configuration | coverage geometry/quality → performance → energy → trajectory → quality |
| wind | performance → energy → trajectory → affected quality/time results |
| battery/SOC/health/degradation | energy → feasibility → selected candidate → final validation |
| mapping-quality requirement | candidate comparison / coverage parameters → quality → selected plan |
| objective priority | candidate comparison → selected plan |

Неизменившиеся upstream results должны переиспользоваться.

### 22.8. Минимальные verification scenarios

**Positive baseline**

~~~text
1 UAV
+ valid mapping area
+ compatible payload
+ valid environment
+ sufficient battery reserve
+ no blocking restrictions
→ coverage generated
→ route feasible
→ energy sufficient
→ trajectory valid
→ quality valid
→ FINAL VALIDATION PASS
~~~

**Negative — insufficient energy**

~~~text
same baseline
+ insufficient available energy for mission + required reserve
→ candidate rejected
→ no READY / RELEASE_ELIGIBLE
~~~

**Negative — blocked spatial constraint**

~~~text
same baseline
+ route/cell requires prohibited or unauthorized space
→ affected candidate rejected or regenerated
→ no READY until a valid alternative exists
~~~

**Incremental recalculation**

~~~text
valid baseline
→ wind update only
→ coverage/decomposition unchanged
→ recompute performance + energy + trajectory
→ revalidate
~~~

### 22.9. Current maturity

This contract is a **controlled working specification**, not implementation evidence.

The following remain UNVERIFIED until executable tests and corresponding CI/evidence exist:

- numerical coverage-quality thresholds;
- camera-specific capture model;
- UAV-specific performance/energy curves;
- exact optimizer selection policy;
- production benchmark values;
- real-flight performance.

### 22.10. Next deterministic contract

After MT-01, the next contract is **MT-02 — 3D-картография / реконструкция**, because it reuses the established constrained-space, coverage, performance, energy and trajectory foundations while adding explicit 3D/viewpoint requirements.


## 23. Template Algorithm Contract — MT-02

**Status:** WORKING CONTRACT — second template implementation slice  
**Template:** MT-02 — 3D-картография / реконструкция

### 23.1. Назначение

MT-02 формирует план наблюдения поверхности/объекта для получения данных, достаточных для 3D-реконструкции, point-cloud или volumetric product, при соблюдении обязательных пространственных, payload, energy и safety constraints.

### 23.2. Входной контракт

- mission ID/version;
- target surface/volume geometry;
- terrain/obstacle state;
- restricted/authorized airspace state;
- required reconstruction quality;
- required observation directions/viewpoints where specified;
- UAV configuration and performance envelope;
- camera/LiDAR/payload configuration;
- battery/SOC/health/degradation state;
- C2 constraints;
- environmental/wind snapshot;
- applicable objective profile.

### 23.3. Расчётная цепочка

~~~text
TARGET / SURFACE MODEL
 ↓
CONSTRAINED OPEN SPACE
 ↓
3D COVERAGE / VIEWPOINT GENERATION
 ↓
VIEWPOINT FEASIBILITY FILTER
 ↓
VIEWPOINT / TRACK SEQUENCING
 ↓
TRANSITION ROUTING
 ↓
WIND + VEHICLE PERFORMANCE
 ↓
ENERGY / RESERVE
 ↓
TRAJECTORY
 ↓
RECONSTRUCTION QUALITY
 ↓
FINAL VALIDATION
~~~

Для сложной 3D-геометрии RRT* может использоваться как planning backend; он не отменяет coverage/viewpoint requirements и обязательные feasibility gates.

### 23.4. Hard constraints

Кандидат отклоняется при нарушении:

- airspace/regulatory constraints;
- authorization scope;
- terrain/obstacle clearance;
- UAV flight envelope;
- payload/sensor operating envelope;
- required observation geometry;
- C2 constraints;
- minimum energy reserve;
- mandatory mission geometry;
- physical inability to obtain the required surface/volume observations.

### 23.5. Quality objectives

После hard gates сравниваются:

1. reconstruction completeness;
2. observation geometry adequacy;
3. required overlap where applicable;
4. occlusion reduction;
5. sensor-specific quality requirements;
6. energy efficiency with required reserve;
7. flight time/resource use.

Численные thresholds для camera/LiDAR quality остаются external controlled inputs и не задаются этим контрактом.

### 23.6. Output

Versioned result shall include:

- selected UAV/payload;
- generated viewpoints/tracks;
- ordered route;
- altitude/orientation profile;
- spatial feasibility;
- wind/performance;
- energy/reserve;
- trajectory;
- reconstruction-quality evaluation;
- dependency/source versions;
- algorithm/backend versions;
- validation/provenance.

### 23.7. Invalidation

Изменение target geometry, required viewpoints, obstacle/airspace state, UAV/payload configuration или quality requirements инвалидирует только затронутые viewpoint/coverage and downstream dependencies.

Изменение только wind invalidates performance → energy → trajectory → affected quality/time results.

Неизменившиеся spatial/viewpoint results должны переиспользоваться.

### 23.8. Minimum verification scenarios

**Positive:** valid 3D target + compatible sensor + feasible viewpoints + sufficient energy → valid trajectory → required quality → FINAL VALIDATION PASS.

**Negative — viewpoint infeasible:** required viewpoint violates obstacle/UAV/airspace constraint → viewpoint rejected or regenerated; mission remains blocked if required coverage cannot be satisfied.

**Negative — quality infeasible:** required reconstruction quality cannot be achieved with selected payload/UAV under constraints → candidate rejected; no lower-quality substitute is silently accepted.

**Incremental:** wind-only change → retain target decomposition/viewpoints → recalculate affected performance/energy/trajectory → revalidate.

### 23.9. Maturity

Working specification only. Numerical reconstruction thresholds, sensor models, calibration and real-flight evidence remain UNVERIFIED until supplied by controlled sources and executable verification.

### 23.10. Next deterministic contract

Next: **MT-03 — Инспекция объектов и инфраструктуры**.

## 24. Detailed Algorithm Specification — MT-01 Survey / Mapping

**Status:** DETAILED WORKING ALGORITHM BASELINE — 2026-10-09  
**Maturity:** engineering specification; numerical aircraft/sensor coefficients remain controlled inputs.

### 24.1. Operational objective

MT-01 converts a mapping task into one or more executable UAV acquisition routes whose combined sensor footprint satisfies the required spatial coverage and data-quality contract while all hard constraints and protected energy reserve remain satisfied.

The algorithm does **not** optimize distance first and add coverage afterwards. Coverage geometry is part of the mission definition and is generated before route optimization.

### 24.2. Canonical input object

The planner shall receive a MappingPlanningContext containing:

~~~text
MissionIdentity
  missionId
  missionVersion

TaskGeometry
  AOI polygon(s)
  exclusion polygon(s)
  mandatory corridor/area(s)
  optional buffer
  home / launch / recovery geometry
  required boundary margin

EnvironmentSnapshot
  terrain model
  obstacle model
  airspace / NOTAM / restriction model
  altitude limits
  wind field
  temperature / pressure where performance model requires them
  data timestamp / validity / version

VehicleCapability
  UAV configuration
  mass / payload state
  speed envelope
  altitude envelope
  climb/descent limits
  turning limits
  propulsion/energy model
  battery SOC/SOH/degradation
  C2 envelope

PayloadCapability
  sensor type
  image dimensions
  sensor dimensions
  focal length
  resolution
  field of view
  trigger / frame-rate limits
  shutter / exposure constraints
  stabilization/gimbal limits
  RTK/PPK capability
  LiDAR FOV / scan characteristics where applicable

QualityRequirement
  target GSD
  frontal overlap requirement
  side overlap requirement
  coverage completeness
  optional oblique / cross-grid requirement
  optional GCP/checkpoint requirement
  optional sensor-specific quality requirements

ObjectiveProfile
  primary objective
  secondary objectives
  tie-breakers
  admissibility policy

OperationalPolicy
  protected energy reserve
  C2 policy
  safety policy
  regulatory policy
  mission time window
~~~

Every material input is versioned or snapshot-addressable.

### 24.3. Stage 0 — input integrity

Before geometric planning:

1. verify that every mandatory input exists;
2. verify coordinate reference systems;
3. transform all planning geometry into a common metric working frame;
4. validate polygon topology;
5. remove/flag self-intersections and invalid rings;
6. verify terrain coverage over the AOI;
7. verify payload-camera calibration/configuration completeness;
8. verify UAV operating envelope;
9. verify energy state freshness;
10. verify environmental data validity;
11. verify restriction/authorization applicability in time and altitude.

If a mandatory input is missing or stale, the planner returns BLOCKED_INPUT rather than inventing a default.

### 24.4. Stage 1 — constrained open-space construction

The planner constructs the spatial domain in which acquisition tracks may legally and physically exist.

~~~text
AOI
 ↓
AIRSPACE / NOTAM FILTER
 ↓
ALTITUDE-BAND FILTER
 ↓
TERRAIN / OBSTACLE CLEARANCE
 ↓
UAV ENVELOPE
 ↓
PAYLOAD / OBSERVATION FEASIBILITY
 ↓
BOUNDARY / SAFETY BUFFER
 ↓
CONSTRAINED OPEN SPACE
~~~

Hard exclusion regions are removed before coverage-track generation.

The result is a versioned ConstrainedMappingDomain.

A route is never intentionally generated through a known prohibited region and merely rejected afterwards.

### 24.5. Stage 2 — acquisition geometry calculation

For an optical camera, the planner derives the ground footprint from the camera model and planned camera-to-ground distance.

A simplified nadir relationship is:

GSD ≈ H × SW / (F × ImW)

where:

- H = camera-to-ground distance;
- SW = sensor dimension corresponding to image width;
- F = focal length;
- ImW = image width in pixels.

The exact camera model used in production shall be the payload-calibration model.

From the footprint and requested overlap:

track_spacing = footprint_cross_track × (1 - side_overlap)

and, for the usual camera orientation:

image_spacing = footprint_along_track × (1 - frontal_overlap).

Trigger/frame-rate requirements are then derived from image spacing and ground speed.

The planner shall distinguish:

- desired GSD;
- achievable GSD;
- requested overlap;
- achievable overlap at the selected speed/altitude;
- sensor trigger-rate limit.

If the requested acquisition geometry is infeasible, the planner must either generate a different admissible altitude/speed configuration or return a quality infeasibility result. It shall not silently weaken the quality requirement.

### 24.6. Stage 3 — terrain-following altitude

For terrain with material elevation variation, the planner shall prefer maintaining a controlled camera-to-ground distance when the UAV and regulatory envelope permit it.

The flight profile therefore derives from:

H_camera_ground(s) = H_target

subject to:

- terrain;
- obstacle clearance;
- altitude floor/ceiling;
- climb/descent rate;
- aircraft attitude;
- payload limits;
- C2 and regulatory limits.

Terrain-following is not permitted to violate an absolute altitude limit or obstacle/safety constraint.

Where terrain-following is impossible, the planner may divide the mission into altitude-compatible subareas and recalculate acquisition geometry for each subarea.

### 24.7. Stage 4 — coverage orientation

The planner evaluates candidate sweep orientations rather than blindly using north/south.

For each candidate orientation θ:

1. rotate the constrained domain;
2. determine projected width;
3. estimate number of coverage tracks;
4. estimate turn count;
5. estimate transition distance;
6. estimate terrain-following complexity;
7. estimate wind exposure;
8. reject orientations that create infeasible cells;
9. retain a bounded candidate set.

The initial geometric heuristic is to favour orientations that reduce the number of tracks and turns. The final selection remains objective-profile dependent because wind and energy can outweigh geometric distance.

### 24.8. Stage 5 — cellular decomposition

For non-convex or obstacle-fragmented AOIs, the planner decomposes the valid domain into cells.

Reference approach:

~~~text
CONSTRAINED DOMAIN
 ↓
DECOMPOSE
 ↓
CELLS
 ↓
CELL COVERAGE TRACKS
 ↓
CELL TRANSITION GRAPH
~~~

A simple convex polygon may remain a single cell.

Complex regions are decomposed so that each cell has a locally feasible sweep pattern.

Each cell stores:

- geometry;
- acquisition altitude band;
- track spacing;
- sweep orientation;
- expected coverage;
- entry candidates;
- exit candidates;
- terrain complexity;
- obstacle margin;
- wind/performance metadata.

### 24.9. Stage 6 — coverage track generation

For each cell:

1. offset the cell boundary by the required acquisition/safety margin;
2. generate parallel sweep lines at the calculated track spacing;
3. clip each sweep line to the valid cell;
4. remove segments below the minimum useful acquisition length;
5. create entry and exit candidates;
6. calculate endpoint turn feasibility;
7. extend/trim tracks only within the admissible acquisition geometry;
8. calculate expected sensor footprint coverage;
9. mark uncovered boundary fragments.

The default pattern is boustrophedon/lawnmower because it provides deterministic and computationally efficient full-area coverage in suitable domains. Cellular decomposition is used when the AOI geometry requires it.

### 24.10. Stage 7 — boundary and edge coverage

Boundary coverage is checked independently.

The planner shall identify:

- uncovered boundary strips;
- excessive edge distance;
- sensor footprint clipping;
- corner gaps;
- gaps introduced by obstacle buffers.

If edge gaps exceed the mission quality tolerance, the planner generates an additional edge pass or adjusts the track placement.

No candidate is accepted solely because the centreline tracks intersect the AOI.

### 24.11. Stage 8 — transition graph

Every cell and track endpoint becomes a node in a transition graph.

An edge is valid only if the connecting route remains inside constrained open space and satisfies:

- altitude;
- obstacle clearance;
- turning constraints;
- UAV performance;
- C2;
- regulatory constraints.

Edge cost may include:

C_edge = distance + turn_cost + energy_cost + wind_penalty + risk_penalty

but hard violations remove the edge completely.

The graph is used to order cells and connect coverage tracks. This separates the coverage problem from the transition-routing problem.

### 24.12. Stage 9 — route candidate generation

The planner creates multiple bounded candidates rather than one greedy route.

Candidate dimensions may include:

- sweep orientation;
- cell visitation order;
- track direction;
- entry/exit endpoint;
- altitude profile;
- speed profile;
- optional cross-grid;
- optional edge pass.

For small transition graphs, deterministic shortest-path methods may be used. For larger route-ordering problems, the orchestrator may use graph search or bounded combinatorial optimization.

Candidate generation is bounded to preserve predictable computation time.

### 24.13. Stage 10 — wind-aware performance calculation

Wind is applied after geometric candidates exist but before final candidate comparison.

For each route segment:

~~~text
route ground vector
        +
wind vector
        ↓
required air-relative velocity
        ↓
feasible speed / attitude
        ↓
segment time
        ↓
segment energy
~~~

The performance model must determine whether the UAV can maintain the required ground track under the wind field.

A candidate becomes infeasible if required airspeed, bank/attitude, propulsion or other aircraft limits are exceeded.

Wind may therefore change the preferred route orientation, speed and candidate selection without requiring unnecessary regeneration of unchanged coverage geometry.

### 24.14. Stage 11 — energy calculation

Energy is calculated for the complete operational sequence, not only the acquisition tracks.

At minimum:

~~~text
departure
+ transit to AOI
+ acquisition
+ transitions
+ return/recovery
+ contingency allowance
+ protected reserve
≤ available energy
~~~

The energy model consumes the authoritative vehicle/payload performance model and battery state.

A candidate that improves nominal efficiency by consuming protected reserve is rejected.

### 24.15. Stage 12 — trajectory generation

The selected route geometry is converted into a feasible trajectory.

Trajectory generation must enforce:

- waypoint continuity;
- altitude profile;
- climb/descent limits;
- speed limits;
- turn radius;
- acceleration/jerk limits where modelled;
- camera acquisition timing;
- payload orientation/gimbal constraints;
- obstacle clearance;
- C2 constraints.

The output is a time-parameterized trajectory, not merely a polyline.

### 24.16. Stage 13 — acquisition-event validation

Every planned image/LiDAR acquisition event is evaluated against:

- position;
- camera-to-ground distance;
- orientation;
- sensor FOV;
- expected footprint;
- GSD;
- frontal overlap;
- side overlap;
- exposure/trigger feasibility;
- terrain/obstacle occlusion where modelled.

For photogrammetry, image network quality also depends on geometry and connectivity, not merely nominal percentage overlap. The planner therefore records the acquisition graph and detects weakly connected areas.

### 24.17. Stage 14 — mapping quality validation

Quality validation produces at minimum:

- area coverage percentage;
- uncovered-area geometry;
- target GSD compliance;
- overlap compliance;
- image/network connectivity;
- sensor operating compliance;
- optional GCP/checkpoint requirements;
- quality warnings.

The final quality verdict is:

PASS, PASS_WITH_WARNING, BLOCKED_QUALITY, or NOT_EVALUABLE.

### 24.18. Stage 15 — candidate scoring

Candidates are compared only after hard gates.

Hierarchical selection:

~~~text
1. HARD ADMISSIBILITY
   ↓
2. REQUIRED COVERAGE / QUALITY
   ↓
3. PROTECTED ENERGY RESERVE
   ↓
4. PRIMARY OBJECTIVE
   ↓
5. SECONDARY OBJECTIVES
   ↓
6. TIE-BREAKERS
~~~

This prevents a shorter route from defeating a candidate with better coverage or required reserve.

### 24.19. Stage 16 — final integrity validation

Before READY:

1. verify all source versions;
2. verify no relevant input changed;
3. verify candidate dependencies;
4. verify route remains in constrained open space;
5. verify quality;
6. verify energy reserve;
7. verify trajectory;
8. verify payload events;
9. verify mission version;
10. generate provenance.

The final validator checks integrity and consistency; it does not silently perform an independent alternative planning calculation.

### 24.20. Incremental recalculation graph

~~~text
AOI / RESTRICTIONS / TERRAIN
        ↓
CONSTRAINED DOMAIN
        ↓
DECOMPOSITION
        ↓
COVERAGE TRACKS
        ↓
TRANSITION ROUTES
        ↓
WIND + PERFORMANCE
        ↓
ENERGY
        ↓
TRAJECTORY
        ↓
ACQUISITION QUALITY
        ↓
CANDIDATE SELECTION
        ↓
FINAL VALIDATION
~~~

Examples:

- wind-only change → performance → energy → trajectory → quality/time → candidate comparison;
- battery/SOH change → energy → feasibility → candidate comparison;
- restriction change → affected domain cells → affected coverage/transitions → downstream stages;
- camera change → acquisition geometry → tracks → downstream stages;
- objective-priority change → candidate comparison only, provided feasibility inputs remain unchanged.

### 24.21. Failure states

The planner shall use explicit machine-readable failure classes:

- BLOCKED_INPUT;
- BLOCKED_AUTHORIZATION;
- BLOCKED_AIRSPACE;
- BLOCKED_GEOMETRY;
- BLOCKED_TERRAIN_OBSTACLE;
- BLOCKED_UAV_ENVELOPE;
- BLOCKED_PAYLOAD;
- BLOCKED_C2;
- BLOCKED_ENERGY;
- BLOCKED_QUALITY;
- BLOCKED_TRAJECTORY;
- NO_FEASIBLE_CANDIDATE.

The UI may translate these into pilot-facing language, but the planning core retains the structured reason.

### 24.22. Reference verification set

**V-M01-01 — simple convex AOI:** one UAV, flat terrain, no obstacles.

**V-M01-02 — concave AOI:** cellular decomposition and complete coverage.

**V-M01-03 — internal exclusion:** coverage regenerated around a prohibited polygon.

**V-M01-04 — terrain variation:** terrain-following profile and GSD consistency.

**V-M01-05 — wind shift:** geometry retained, performance/energy/trajectory recalculated.

**V-M01-06 — insufficient reserve:** all otherwise-valid candidates rejected.

**V-M01-07 — payload infeasibility:** requested GSD/overlap cannot be achieved within payload/UAV limits.

**V-M01-08 — edge coverage:** boundary strips detected and corrected.

**V-M01-09 — deterministic replay:** identical versioned inputs produce identical planning result within defined numerical tolerance.

**V-M01-10 — incremental recalculation:** unchanged upstream results are reused after a wind-only change.

### 24.23. Engineering rule

MT-01 is considered algorithmically complete only when the above stages have:

1. defined inputs;
2. defined outputs;
3. explicit dependency ownership;
4. hard/soft classification;
5. failure states;
6. deterministic verification cases;
7. provenance;
8. implementation mapping.

A prose description of lawnmower coverage alone is not considered an implemented algorithm.

## 25. Detailed Algorithm Specification — MT-02 3D Mapping / Reconstruction

**Status:** DETAILED WORKING ALGORITHM BASELINE — 2026-10-09  
**Maturity:** engineering specification; reconstruction/sensor coefficients remain controlled inputs.

### 25.1. Operational objective

MT-02 generates an acquisition trajectory that observes the required surfaces/volumes from sufficiently informative viewpoints to support the requested 3D reconstruction product.

The central planning problem is not merely area coverage. It is **coverage of geometry with useful observation relationships**.

### 25.2. Input extension over MT-01

MT-02 inherits all common planning inputs from MT-01 and adds:

- target surface/mesh/point-cloud/volume model when available;
- target semantic regions;
- required reconstruction product;
- required surface completeness;
- required detail/GSD;
- viewpoint angle constraints;
- minimum/maximum observation distance;
- multi-view requirements;
- overlap/connectivity requirements;
- occlusion constraints;
- optional oblique-camera requirements;
- optional facade/vertical-surface requirements;
- sensor-specific reconstruction model.

### 25.3. Reconstruction target representation

The target is represented as one or more of:

2.5D terrain, surface mesh, voxel volume, point cloud, or semantic object surfaces.

Each target element stores:

- position;
- normal estimate where available;
- importance;
- desired observation distance;
- acceptable incidence angle;
- required observation count;
- current observation state.

The planner must support an initially incomplete target model. Unknown geometry is represented explicitly rather than treated as already observed.

### 25.4. Stage 0 — target validation

Check:

- target geometry validity;
- coordinate reference;
- surface normals where required;
- terrain/obstacle consistency;
- target extent;
- sensor compatibility;
- required reconstruction quality;
- known/unknown regions.

If a 3D model is supplied, its provenance and age are retained because outdated geometry can create invalid viewpoints.

### 25.5. Stage 1 — observation-space construction

Instead of generating routes directly around the target, MT-02 first constructs a feasible observation space.

For each target element:

~~~text
target point/surface
 ↓
desired viewing direction
 ↓
candidate camera positions
 ↓
distance constraint
 ↓
incidence-angle constraint
 ↓
obstacle/airspace clearance
 ↓
UAV envelope
 ↓
payload/gimbal feasibility
 ↓
FEASIBLE VIEWPOINT SET
~~~

A viewpoint is admissible only when both the UAV position and sensor observation geometry are feasible.

### 25.6. Stage 2 — viewpoint generation

Candidate viewpoints are generated using one or more strategies:

- structured rings/orbits;
- grid viewpoints;
- surface-normal offsets;
- multi-altitude layers;
- facade-specific viewpoints;
- terrain-following strips;
- sampling around weakly observed surfaces.

The orchestrator chooses the strategy based on target geometry.

For simple terrain, MT-01-style coverage remains the primary mechanism.

For complex objects, viewpoint sampling is preferred over forcing a 2D lawnmower pattern onto a 3D surface.

### 25.7. Stage 3 — viewpoint scoring

Each candidate viewpoint receives a score containing, where applicable:

- visible target area;
- observation distance quality;
- frontality / incidence angle;
- expected GSD;
- expected overlap with existing views;
- parallax contribution;
- occlusion;
- sensor FOV utilisation;
- flight cost to/from viewpoint;
- energy cost;
- C2 feasibility.

Hard violations remove the viewpoint.

A high score does not guarantee selection: viewpoint redundancy and global network connectivity are evaluated later.

### 25.8. Stage 4 — visibility model

The planner evaluates line-of-sight between candidate viewpoint and target elements.

Visibility must account for:

- terrain;
- buildings/structures;
- target self-occlusion;
- sensor FOV;
- camera orientation;
- minimum/maximum range.

The result is a viewpoint-to-surface visibility matrix:

V[i,j] = 1 if viewpoint i provides admissible observation of target element j.

This matrix becomes a core planning dependency.

### 25.9. Stage 5 — observation coverage problem

The planner selects a subset of viewpoints such that required target elements receive sufficient observations.

A target element may require:

- one observation for simple mapping;
- multiple observations from different directions;
- minimum angular diversity;
- overlap with neighbouring image sets;
- special views for weakly observed geometry.

This is therefore closer to a constrained set-cover / viewpoint-selection problem than ordinary 2D coverage.

### 25.10. Stage 6 — initial global coverage

The first pass seeks a globally efficient set of viewpoints covering the required target.

The planner shall prefer a bounded deterministic or reproducible heuristic:

~~~text
uncovered target elements
 ↓
candidate viewpoints
 ↓
marginal information / coverage gain
 ↓
feasibility filter
 ↓
select best admissible viewpoint
 ↓
update uncovered set
 ↓
repeat
~~~

The algorithm stops when:

- required coverage is achieved;
- no admissible viewpoint can add required coverage;
- or the mission becomes infeasible.

If the latter occurs, the result is BLOCKED_QUALITY rather than silently reducing the requirement.

### 25.11. Stage 7 — weak-area detection

After the first global solution, the planner searches for:

- uncovered surfaces;
- surfaces with too few observations;
- weak camera-network connectivity;
- excessive incidence angle;
- poor GSD;
- occluded areas;
- insufficient parallax;
- isolated image groups.

These areas become weak regions.

### 25.12. Stage 8 — local refinement

For each weak region:

1. generate additional local viewpoints;
2. apply finer viewpoint sampling;
3. evaluate geometry and visibility;
4. remove redundant candidates;
5. add only viewpoints that materially improve the weak-region score.

This creates the two-level strategy:

~~~text
GLOBAL COVERAGE
      ↓
WEAK-AREA ANALYSIS
      ↓
LOCAL REFINEMENT
      ↓
GLOBAL RECHECK
~~~

This structure is consistent with current UAV 3D-reconstruction research, which increasingly treats global coverage and local viewpoint refinement as separate planning layers.

### 25.13. Stage 9 — viewpoint sequencing

Selected viewpoints become nodes in a route graph.

Edge feasibility checks:

- collision/clearance;
- airspace;
- altitude;
- UAV dynamics;
- C2;
- wind;
- payload orientation;
- transition energy.

Edge cost includes time/energy/distance and, where useful, observation-loss penalties.

For a small set, deterministic graph search is sufficient.

For a large set, bounded route-ordering optimisation may be applied.

### 25.14. Stage 10 — trajectory-aware viewpoint adjustment

A viewpoint that is individually feasible may become infeasible when connected into a real trajectory.

Therefore:

~~~text
VIEWPOINT SET
 ↓
SEQUENCING
 ↓
TRAJECTORY GENERATION
 ↓
DYNAMIC FEASIBILITY
 ↓
VIEWPOINT RECHECK
 ↓
LOCAL REPAIR IF REQUIRED
~~~

The system must not accept a viewpoint set solely because every viewpoint is individually valid.

### 25.15. Stage 11 — camera orientation

The trajectory contains camera orientation as an explicit variable.

Depending on the mission:

- nadir;
- oblique;
- side-looking;
- target-normal;
- gimbal-tracked;
- mixed orientation

may be selected.

Orientation must remain inside the verified payload/gimbal envelope.

For reconstruction, camera pose is part of the acquisition network and therefore part of the quality calculation.

### 25.16. Stage 12 — image-network quality

The planner evaluates:

- image overlap;
- spatial distribution;
- viewpoint diversity;
- graph connectivity;
- weakly connected components;
- expected parallax;
- repeated observations;
- exposure geometry.

Nominal overlap alone is insufficient to certify reconstruction quality.

Current photogrammetry guidance recommends regular-grid acquisition with at least 75% frontal and 60% side overlap for the general case, while more difficult scenes can require higher values; these are reference acquisition practices, not BlueSky hard-coded universal constants.

### 25.17. Stage 13 — GSD and distance consistency

For camera-based reconstruction, GSD is evaluated from the actual camera-to-surface distance and calibrated camera parameters.

If terrain/object height changes materially, the planner must account for the resulting GSD variation rather than assuming a constant nominal altitude.

Terrain-following or segmented altitude planning may therefore be required to maintain the target GSD.

### 25.18. Stage 14 — LiDAR branch

For LiDAR payloads, the equivalent quality model is based on:

- sensor FOV;
- swath/footprint;
- point density;
- scan angle;
- altitude;
- ground speed;
- overlap between swaths;
- target visibility;
- required point density.

The planner therefore replaces image-overlap equations with a sensor-footprint/point-density model while retaining the same higher-level stages:

~~~text
target
 ↓
sensor coverage
 ↓
viewpoint/track generation
 ↓
transition routing
 ↓
performance
 ↓
energy
 ↓
trajectory
 ↓
quality
~~~

Recent UAV-LiDAR CPP research evaluates lawnmower, boustrophedon and spiral strategies against coverage percentage, trajectory length and waypoint density using a sensor-footprint model, supporting this separation between route pattern and sensor-quality model.

### 25.19. Stage 15 — wind and performance

Wind affects:

- achievable ground speed;
- camera exposure timing;
- image spacing;
- trajectory feasibility;
- energy;
- stability;
- acquisition quality.

Therefore wind is not merely a post-processing penalty.

If wind changes the actual acquisition spacing beyond the quality tolerance, the planner must regenerate or locally adjust acquisition events.

### 25.20. Stage 16 — energy and reserve

The complete 3D mission energy includes:

- departure;
- transit;
- global coverage;
- local refinement;
- transitions;
- return/recovery;
- contingency;
- protected reserve.

If the full-quality solution exceeds available energy, the planner may attempt:

1. better viewpoint ordering;
2. more efficient transition routing;
3. altitude/speed adjustment within quality constraints;
4. task decomposition;
5. additional sortie/UAV where permitted.

It may not silently reduce the reconstruction quality requirement.

### 25.21. Stage 17 — multi-UAV 3D decomposition

For multiple UAVs:

~~~text
3D TARGET
 ↓
SURFACE / VOLUME PARTITION
 ↓
UAV CAPABILITY MATCHING
 ↓
VIEWPOINT / REGION ALLOCATION
 ↓
INDIVIDUAL ROUTES
 ↓
4D TRAJECTORIES
 ↓
CONFLICT CHECK
 ↓
COORDINATED MISSION
~~~

Partitioning should avoid assigning a surface region to a UAV whose payload or geometry cannot satisfy that region's quality requirements.

### 25.22. Stage 18 — final reconstruction-quality verdict

The final verdict shall distinguish:

- PASS;
- PASS_WITH_WARNING;
- BLOCKED_QUALITY;
- NOT_EVALUABLE.

It shall report at minimum:

- target coverage;
- weak/unobserved regions;
- observation count distribution;
- GSD distribution where applicable;
- overlap/connectivity;
- visibility/occlusion findings;
- sensor-specific metrics;
- unresolved quality risks.

### 25.23. Stage 19 — incremental recalculation

Dependency graph:

~~~text
TARGET GEOMETRY
      ↓
OBSERVATION SPACE
      ↓
VIEWPOINTS
      ↓
VISIBILITY
      ↓
VIEWPOINT SELECTION
      ↓
SEQUENCING
      ↓
WIND + PERFORMANCE
      ↓
ENERGY
      ↓
TRAJECTORY
      ↓
RECONSTRUCTION QUALITY
      ↓
FINAL VALIDATION
~~~

Examples:

- wind-only change → performance → sequencing/trajectory → energy → quality;
- obstacle change → affected visibility/viewpoints → sequencing → downstream;
- target geometry update → observation space → visibility → viewpoint selection → downstream;
- camera focal length change → GSD/FOV → viewpoint feasibility/selection → downstream;
- reconstruction quality requirement change → candidate selection/quality evaluation, with geometry reused where valid;
- battery degradation → energy → candidate feasibility/comparison; viewpoint geometry remains reusable if unchanged.

### 25.24. Failure states

Use explicit machine-readable states:

- BLOCKED_INPUT;
- BLOCKED_TARGET_MODEL;
- BLOCKED_AIRSPACE;
- BLOCKED_VIEWPOINT;
- BLOCKED_VISIBILITY;
- BLOCKED_SENSOR;
- BLOCKED_UAV_ENVELOPE;
- BLOCKED_C2;
- BLOCKED_ENERGY;
- BLOCKED_RECONSTRUCTION_QUALITY;
- BLOCKED_TRAJECTORY;
- NO_FEASIBLE_3D_PLAN.

### 25.25. Reference verification set

**V-M02-01 — flat terrain:** nadir photogrammetry, known GSD and overlap.

**V-M02-02 — terrain relief:** terrain-following / segmented altitude and GSD verification.

**V-M02-03 — vertical structure:** oblique viewpoints and facade coverage.

**V-M02-04 — occlusion:** hidden surface detected as weak/unobserved and local viewpoints generated.

**V-M02-05 — insufficient overlap:** quality blocker.

**V-M02-06 — viewpoint obstacle conflict:** viewpoint rejected and alternative generated.

**V-M02-07 — camera change:** viewpoint/GSD dependencies invalidated; unaffected target geometry reused.

**V-M02-08 — wind change:** trajectory/performance recalculated without rebuilding unchanged target visibility.

**V-M02-09 — insufficient energy:** candidate rejected or task partition/sortie alternative generated if permitted.

**V-M02-10 — multi-UAV:** target partition, independent trajectories and 4D conflict validation.

**V-M02-11 — deterministic replay:** identical versioned inputs produce reproducible viewpoint set and route within defined numerical tolerance.

### 25.26. Research boundary

Current research supports a layered interpretation of UAV 3D planning: global coverage, local viewpoint refinement and trajectory smoothing are distinct but coupled planning problems.

For BlueSky, research algorithms are therefore treated as interchangeable planning backends under the Algorithm Orchestrator rather than as fixed product behaviour.

### 25.27. Engineering completion criterion

MT-02 is complete at the algorithm-specification level only when every stage above has:

1. defined input contract;
2. defined output contract;
3. explicit feasibility conditions;
4. explicit quality metrics;
5. invalidation dependencies;
6. failure states;
7. deterministic verification scenarios;
8. provenance;
9. implementation mapping;
10. controlled sensor/UAV model references.

### 25.28. Relationship between MT-01 and MT-02

MT-02 is not a separate planning universe.

Shared services:

- Mission Model;
- constrained open space;
- terrain/obstacle processing;
- regulatory filtering;
- UAV performance;
- wind;
- energy;
- trajectory;
- C2;
- final validation;
- provenance;
- incremental recalculation.

MT-02 replaces only the task-specific acquisition planner:

~~~text
MT-01
AOI → decomposition → coverage tracks → transitions

MT-02
target geometry → observation space → viewpoints → visibility → viewpoint network
~~~

The downstream planning and validation architecture remains common.

### 25.29. Explicit design decision

**Decision:** Complete MT-01 and MT-02 to implementation-ready algorithm contracts before expanding the template family to MT-03.

**Rationale:**

- MT-01 establishes the canonical 2D coverage pipeline.
- MT-02 extends that pipeline into 3D observation/viewpoint planning.
- Together they establish the reusable foundation for inspection, monitoring, search and other templates.
- Later templates should reuse these engines rather than introduce parallel planning architectures.

## 26. Deterministic Planning State Machine for MT-01 / MT-02

The common state machine is:

~~~text
INPUT_RECEIVED
 → INPUT_VALIDATION
 → ENVIRONMENT_NORMALIZED
 → CONSTRAINED_DOMAIN_READY
 → CANDIDATE_GENERATION
 → CANDIDATE_FEASIBILITY
 → PERFORMANCE_CALCULATION
 → ENERGY_CALCULATION
 → TRAJECTORY_GENERATION
 → TASK_QUALITY_EVALUATION
 → CANDIDATE_COMPARISON
 → FINAL_INTEGRITY_VALIDATION
 → PLAN_READY
~~~

Any failed mandatory gate produces an explicit BLOCKED_* state. No candidate moves directly from generation to READY.

### 26.1. Candidate lifecycle

Normal lifecycle:

GENERATED → FEASIBLE → PERFORMANCE_VALID → ENERGY_VALID → TRAJECTORY_VALID → QUALITY_VALID → SELECTABLE → SELECTED

Terminal rejection states:

REJECTED_HARD_CONSTRAINT, REJECTED_ENERGY, REJECTED_QUALITY, REJECTED_TRAJECTORY, REJECTED_STALE.

The rejection reason is retained as evidence.

### 26.2. Candidate identity

A candidate identity contains:

- mission version;
- candidate-generation version;
- task-specific planner version;
- geometry hash;
- environment snapshot hash;
- UAV/payload configuration hash;
- objective-profile hash.

A result calculated for another context cannot be silently reused.

### 26.3. Authoritative calculation rule

Each result has one authoritative producer.

- constrained domain → Spatial Constraint Engine;
- coverage tracks → Coverage Planner;
- viewpoint visibility → Visibility Engine;
- wind-adjusted performance → Performance Engine;
- energy → Energy Engine;
- trajectory → Trajectory Engine;
- task quality → Task Quality Engine;
- candidate selection → Objective Orchestrator.

Downstream modules consume stored results rather than recreating them.

## 27. MT-01 Exact Planning Procedure

### 27.1. Reference pseudocode

~~~text
PLAN_MT01(context):

1. validate(context)
2. normalize_coordinates(context)
3. build_constrained_domain(context)
4. derive_acquisition_geometry(context.payload, context.quality)

5. orientations = generate_orientation_candidates(domain, acquisition_geometry)

6. FOR each orientation:
      cells = decompose(domain, orientation)
      tracks = generate_tracks(cells, acquisition_geometry)
      edge_graph = build_transition_graph(cells, tracks, domain)
      route_candidates = generate_bounded_routes(edge_graph)

      FOR each route:
          IF not spatially_feasible(route):
              reject(HARD_SPATIAL)
              CONTINUE

          performance = calculate_wind_performance(route, context)
          IF not performance.feasible:
              reject(UAV_OR_ENVIRONMENT)
              CONTINUE

          trajectory = generate_trajectory(route, performance, context)
          IF not trajectory.feasible:
              reject(TRAJECTORY)
              CONTINUE

          acquisition = calculate_acquisition_events(
              trajectory, context.payload,
              context.quality, context.terrain
          )

          quality = evaluate_mapping_quality(acquisition, context.quality)
          IF not quality.acceptable:
              reject(QUALITY)
              CONTINUE

          energy = calculate_total_energy(
              trajectory, context.vehicle,
              context.payload, context.environment,
              context.operational_policy
          )

          IF not energy.reserve_satisfied:
              reject(ENERGY)
              CONTINUE

          retain(candidate)

7. IF no selectable candidate:
      return NO_FEASIBLE_CANDIDATE

8. selected = compare_candidates(
       candidates, context.objective_profile
   )

9. final_validate(selected, context)

10. return versioned_plan(selected)
~~~

### 27.2. Orientation search

The orientation search shall be bounded and reproducible.

Reference policy:

1. calculate principal polygon orientations;
2. include the minimum-track orientation;
3. include nearby orientations;
4. include materially wind-favourable orientations;
5. include an operator-requested orientation when permitted;
6. deduplicate equivalent orientations;
7. retain a configured maximum number.

The maximum is an algorithm parameter, not a mission requirement.

### 27.3. Track generation

For each sweep line:

1. intersect the line with the valid cell;
2. clip to the cell;
3. apply acquisition/safety margin;
4. reject segments below minimum useful length;
5. calculate entry and exit headings;
6. validate turn feasibility;
7. create the track object.

Track object:

- start/end;
- length;
- heading;
- altitude reference;
- acquisition interval;
- expected event count;
- coverage polygon;
- feasibility state.

### 27.4. Coverage completeness

Coverage shall be calculated from sensor footprint geometry, not waypoint points.

Reference metric:

coverage_ratio = area(union(sensor_footprints ∩ required_AOI)) / area(required_AOI)

The implementation shall define numerical tolerance for small geometry slivers.

Uncovered geometry is retained for diagnostics and local repair.

### 27.5. Local repair

When a candidate has localized coverage gaps:

1. identify connected uncovered regions;
2. determine possible local passes;
3. calculate incremental route cost;
4. recalculate performance and energy;
5. accept only if all hard constraints remain valid and the repair improves the mission result.

### 27.6. Cross-grid

A perpendicular second grid is not universally mandatory.

It may be introduced when:

- the objective profile requests it;
- the sensor/processing contract requires stronger image geometry;
- scene characteristics create weak reconstruction geometry;
- quality validation identifies a material deficiency.

The second grid remains a candidate requiring full energy/time validation.

## 28. MT-02 Exact Planning Procedure

### 28.1. Reference pseudocode

~~~text
PLAN_MT02(context):

1. validate(context)
2. normalize_target_geometry(context)
3. build_constrained_observation_domain(context)

4. target_elements = discretize_target(
       geometry,
       required_resolution,
       semantic_importance
   )

5. viewpoints = generate_initial_viewpoints(
       target_elements, sensor_model, UAV_model
   )

6. viewpoints = feasibility_filter(viewpoints)

7. visibility = calculate_visibility(
       viewpoints, target_elements, environment
   )

8. selected = select_global_viewpoints(
       viewpoints, visibility, quality_requirements
   )

9. weak_regions = detect_weak_regions(
       selected, target_elements,
       visibility, quality_requirements
   )

10. WHILE weak_regions remain and refinement budget exists:
        local_candidates = generate_local_viewpoints(weak_regions)
        local_candidates = feasibility_filter(local_candidates)
        update_visibility(local_candidates)
        selected = improve_viewpoint_set(
            selected, local_candidates
        )
        weak_regions = re_evaluate_weak_regions()

11. route_graph = build_viewpoint_transition_graph(
        selected, environment, UAV_model, payload_model
    )

12. route_candidates = generate_bounded_sequences(route_graph)

13. FOR each sequence:
        trajectory = generate_3D_trajectory(sequence, context)

        IF not trajectory.feasible:
            reject(TRAJECTORY)
            CONTINUE

        acquisition = generate_sensor_events(
            trajectory, sensor_model, target_geometry
        )

        performance = calculate_wind_performance(
            trajectory, context
        )

        IF not performance.feasible:
            reject(PERFORMANCE)
            CONTINUE

        energy = calculate_total_energy(trajectory, context)

        IF not energy.reserve_satisfied:
            reject(ENERGY)
            CONTINUE

        quality = evaluate_3D_reconstruction_quality(
            acquisition, visibility,
            target_elements, quality_requirements
        )

        IF not quality.acceptable:
            reject(QUALITY)
            CONTINUE

        retain(sequence, trajectory, energy, quality)

14. IF no selectable candidate:
       return NO_FEASIBLE_3D_PLAN

15. selected_plan = compare_candidates(
       candidates, objective_profile
   )

16. final_validate(selected_plan, context)

17. return versioned_plan(selected_plan)
~~~

### 28.2. Target discretisation

Resolution shall be driven by the required product.

Possible representations:

- regular surface samples;
- adaptive mesh vertices;
- voxel centres;
- semantic surface patches.

Adaptive discretisation is preferred for heterogeneous geometry:

~~~text
simple surface → coarse sampling
high-curvature / critical surface → fine sampling
weakly observed surface → fine sampling
~~~

Stable target-element IDs permit incremental recalculation.

### 28.3. Viewpoint generation

For a target element with surface normal n, a nominal camera position can be generated as:

p_view = p_target + d × n

where d is an admissible observation distance.

The candidate is then perturbed within a bounded angular/distance neighbourhood.

This is seed generation only; visibility and trajectory feasibility are separate gates.

### 28.4. Viewpoint feasibility

A viewpoint is admissible only when all applicable conditions hold:

~~~text
allowed airspace
AND
terrain/obstacle clearance
AND
UAV altitude envelope
AND
UAV dynamics
AND
camera range
AND
camera/gimbal orientation
AND
C2 requirement
AND
required target visibility
~~~

Hard failures remove the viewpoint before global selection.

### 28.5. Viewpoint utility

A reference utility may combine:

- coverage gain;
- geometry quality;
- parallax gain;
- visibility gain;
- energy cost;
- time cost;
- occlusion penalty.

The utility weights are versioned algorithm parameters. Hard constraints remain outside the utility.

### 28.6. Redundancy control

A viewpoint may be rejected when:

- its visible target set is nearly identical to selected viewpoints;
- it adds negligible parallax;
- it adds negligible network connectivity;
- its incremental quality gain is not worth its cost.

The redundancy threshold is a controlled parameter.

### 28.7. Weak-region detection

A region becomes weak when one or more apply:

- observation count below requirement;
- angular diversity below requirement;
- GSD above limit;
- overlap below requirement;
- visibility too low;
- image graph connectivity insufficient;
- occlusion risk too high.

Weak regions are spatial objects and therefore support local replanning.

### 28.8. Global/local refinement loop

~~~text
GLOBAL PLAN
   ↓
QUALITY ANALYSIS
   ↓
WEAK REGION EXTRACTION
   ↓
LOCAL VIEWPOINT GENERATION
   ↓
LOCAL REPAIR
   ↓
GLOBAL QUALITY RECHECK
~~~

The loop terminates when quality passes, no useful refinement remains, or the refinement budget is exhausted. Exhaustion without quality compliance is a blocked result.

### 28.9. Viewpoint graph and local motion planning

A viewpoint graph edge is valid only when a feasible trajectory exists between viewpoints.

For open geometry, a direct feasibility check is preferred.

For complex geometry, graph search or a sampling-based local planner such as RRT* may be used.

RRT* is therefore a local motion-planning backend, not the primary 3D acquisition algorithm.

### 28.10. Reconstruction-quality decomposition

The initial quality model is:

~~~text
GEOMETRIC COVERAGE
+
OBSERVATION QUALITY
+
SENSOR NETWORK CONNECTIVITY
+
MULTI-VIEW GEOMETRY
+
GSD / SENSOR RESOLUTION
+
OCCLUSION
=
RECONSTRUCTION QUALITY
~~~

The production metric must eventually be tied to the selected reconstruction pipeline and verified on representative datasets.

### 28.11. Multi-view geometry

The planner shall retain, where supported:

- baseline;
- viewing-angle difference;
- overlap;
- common visible surface;
- temporal separation;
- pose uncertainty.

This permits later quality engines to evaluate image-pair geometry without rebuilding the route.

### 28.12. Camera branch

~~~text
camera calibration
→ footprint
→ GSD
→ overlap
→ viewpoint geometry
→ image events
→ image network
~~~

Megapixel count alone is never treated as sufficient evidence of GSD or reconstruction quality.

### 28.13. LiDAR branch

~~~text
sensor calibration
→ beam/FOV model
→ swath
→ point density
→ incidence/visibility
→ scan overlap
→ trajectory
~~~

The sensor-specific quality model remains separate from the shared planning architecture.

### 28.14. Final plan package

Both templates output:

~~~text
MissionPlan
 ├─ mission/version
 ├─ planner/version
 ├─ selected UAV/payload
 ├─ constrained-domain reference
 ├─ task-specific geometry
 ├─ route/waypoints
 ├─ time-parameterized trajectory
 ├─ payload acquisition events
 ├─ performance result
 ├─ energy/reserve result
 ├─ quality result
 ├─ validation result
 └─ provenance/dependency manifest
~~~

### 28.15. Reproducibility

For identical mission version, environment snapshot, UAV/payload configuration, algorithm versions, objective profile and numerical configuration, the result shall be reproducible within explicitly defined numerical tolerances.

Any stochastic backend must use a controlled seed or retain sufficient random-state information for replay.

### 28.16. Performance accounting

Each planner shall report:

- calculation wall time;
- candidate count;
- rejection count by gate;
- geometry-operation count;
- visibility-test count;
- trajectory-evaluation count;
- cache/reuse ratio.

This permits kernel tuning without changing mission semantics.

### 28.17. Explainability

The planner shall produce machine-readable reasons such as:

~~~text
PRIMARY_REASON: wind_favourable_orientation
SECONDARY_REASON: protected_energy_reserve
QUALITY_REASON: required_coverage_maintained
RECALCULATION_REASON: wind_snapshot_changed
~~~

The pilot-facing UI may reduce this to one concise operational explanation.

### 28.18. Parameter classes

**MISSION INPUT**
- task area;
- target GSD;
- required overlap;
- required product quality.

**VEHICLE/PAYLOAD DATA**
- camera;
- FOV;
- focal length;
- speed limits;
- energy model.

**ENVIRONMENT**
- terrain;
- obstacles;
- restrictions;
- wind.

**ALGORITHM PARAMETERS**
- sampling resolution;
- candidate limits;
- refinement thresholds;
- graph-search limits.

**POLICY / REQUIREMENT**
- protected reserve;
- mandatory safety constraints;
- authorization rules.

The planner shall not convert an algorithm parameter into a mission requirement.

### 28.19. Numerical-policy boundary

The following remain external controlled values until formally approved:

- protected energy reserve;
- safety margins;
- obstacle clearance;
- GSD thresholds by mission class;
- overlap defaults by sensor/product;
- viewpoint angular tolerances;
- visibility thresholds;
- candidate/refinement budgets.

The present specification defines structure, not qualification values.

## 29. Completion Gate for MT-01 and MT-02

Before opening MT-03, each of the two templates shall have:

1. algorithm specification;
2. data/input schema;
3. dependency graph;
4. candidate lifecycle;
5. hard/soft constraints;
6. quality model;
7. failure taxonomy;
8. pseudocode;
9. parameter classification;
10. verification scenarios;
11. reproducibility rule;
12. performance metrics;
13. provenance contract;
14. implementation-module mapping;
15. traceability to requirements;
16. test-data definition.

Only after this gate is satisfied should the template family advance.


## 30. Implementation Data Contracts for MT-01 / MT-02

The algorithm specification is not implementation-ready until the calculation boundary is represented by explicit typed data contracts. The following logical contracts are normative; the concrete C++/QML transport representation may differ, but field semantics and units shall not.

### 30.1. Common identifiers

```text
MissionId            opaque stable identifier
MissionVersion      monotonically versioned mission definition
PlanId               immutable selected-plan identifier
PlannerVersion       semantic/versioned planner implementation
AlgorithmConfigId   versioned numerical configuration
EnvironmentSnapshotId immutable environment snapshot
UavConfigId          versioned vehicle configuration
PayloadConfigId      versioned payload configuration
ObjectiveProfileId   versioned mission objective profile
GeometryRevision     geometry revision identifier
```

### 30.2. MappingPlanningContext

```text
MappingPlanningContext
 ├─ mission
 │   ├─ mission_id
 │   ├─ mission_version
 │   ├─ AOI geometry + CRS
 │   ├─ mandatory / excluded geometry
 │   ├─ operational window
 │   └─ objective_profile
 ├─ environment
 │   ├─ terrain_model
 │   ├─ obstacle_set
 │   ├─ airspace / restrictions
 │   ├─ authorization_state
 │   └─ wind_snapshot
 ├─ vehicle
 │   ├─ UAV configuration
 │   ├─ performance envelope
 │   ├─ mass / payload limits
 │   ├─ battery state + degradation
 │   ├─ C2 model
 │   └─ safety configuration
 ├─ payload
 │   ├─ sensor configuration
 │   ├─ calibration
 │   ├─ FOV / focal parameters
 │   ├─ acquisition mode
 │   └─ trigger / exposure model
 ├─ quality
 │   ├─ target GSD
 │   ├─ overlap requirements
 │   ├─ coverage requirement
 │   └─ product-quality requirements
 └─ algorithm
     ├─ algorithm_config_id
     ├─ candidate budgets
     ├─ sampling configuration
     ├─ numerical tolerances
     └─ deterministic seed
```

### 30.3. ReconstructionPlanningContext

MT-02 extends the common context:

```text
ReconstructionPlanningContext
 ├─ MappingPlanningContext
 └─ reconstruction
     ├─ target representation
     ├─ target elements / stable IDs
     ├─ required observation count
     ├─ desired observation distance
     ├─ incidence-angle requirements
     ├─ parallax requirements
     ├─ occlusion policy
     ├─ camera/gimbal limits
     ├─ reconstruction product type
     └─ sensor-specific quality model
```

### 30.4. CandidatePlan

Every candidate shall be self-describing enough for deterministic comparison and post-flight audit:

```text
CandidatePlan
 ├─ candidate_id
 ├─ context_identity
 ├─ geometry_revision
 ├─ route / trajectory
 ├─ acquisition_events
 ├─ performance_result
 ├─ energy_result
 ├─ quality_result
 ├─ hard_constraint_result
 ├─ objective_result
 ├─ rejection_state / selection_state
 ├─ rejection_reasons[]
 └─ provenance
```

A candidate without complete mandatory result objects is not selectable.

## 31. Calculation-Module Interfaces

The implementation shall preserve the separation between task-specific planning and shared calculation services.

### 31.1. Spatial Constraint Engine

Input:

```text
Mission geometry
+ airspace/restrictions
+ terrain
+ obstacles
+ UAV envelope
+ safety policy
```

Output:

```text
ConstrainedDomain
 ├─ allowed volumes / surfaces
 ├─ excluded regions
 ├─ boundary buffers
 ├─ altitude bands
 └─ provenance
```

Invariant: a downstream planner shall never treat an excluded region as traversable.

### 31.2. Coverage Planner

MT-01 input:

```text
ConstrainedDomain
+ acquisition geometry
+ orientation candidate
+ coverage policy
```

Output:

```text
CoveragePlan
 ├─ cells
 ├─ tracks
 ├─ coverage footprints
 ├─ transition graph
 └─ uncovered regions
```

The Coverage Planner shall not calculate final battery feasibility. It supplies geometry to the Performance and Energy Engines.

### 31.3. Visibility Engine

MT-02 input:

```text
candidate viewpoints
+ target elements
+ terrain / obstacle model
+ sensor model
```

Output:

```text
VisibilityResult
 ├─ viewpoint → target visibility
 ├─ occlusion state
 ├─ incidence / observation geometry
 ├─ effective footprint
 └─ visibility provenance
```

Visibility calculations shall be reusable between candidate route evaluations when their dependencies are unchanged.

### 31.4. Performance Engine

Input:

```text
trajectory
+ UAV performance model
+ payload configuration
+ wind snapshot
+ environmental conditions
```

Output:

```text
PerformanceResult
 ├─ airspeed
 ├─ ground speed
 ├─ segment time
 ├─ operating state
 ├─ feasibility
 └─ limiting conditions
```

### 31.5. Energy Engine

Input:

```text
time-parameterized trajectory
+ UAV propulsion model
+ battery state
+ degradation model
+ payload
+ environment
+ reserve policy
```

Output:

```text
EnergyResult
 ├─ energy by segment
 ├─ total mission energy
 ├─ contingency energy
 ├─ protected reserve
 ├─ predicted terminal state
 ├─ reserve margin
 └─ feasibility
```

Energy feasibility is a gate, not merely a ranking score.

### 31.6. Trajectory Engine

Input:

```text
route / viewpoint sequence
+ performance result
+ UAV dynamics
+ altitude policy
+ safety constraints
```

Output:

```text
Trajectory
 ├─ time-tagged states
 ├─ position
 ├─ altitude
 ├─ velocity
 ├─ heading
 ├─ attitude / gimbal where modeled
 ├─ acquisition events
 └─ feasibility
```

### 31.7. Task Quality Engine

MT-01 evaluates mapping quality; MT-02 evaluates reconstruction quality.

The engine shall return both:

1. aggregate acceptance state;
2. decomposed quality metrics and deficiencies.

A single scalar score is insufficient for auditability.

### 31.8. Objective Orchestrator

The Objective Orchestrator shall:

1. remove candidates failing hard gates;
2. remove candidates failing mandatory quality/reserve requirements;
3. compare remaining candidates using the active objective profile;
4. apply deterministic tie-breaking;
5. emit the selected candidate and machine-readable selection reasons.

## 32. Units, Numerical Policy and Tolerances

All persisted numerical fields shall have an explicit unit.

Normative internal units:

| Quantity | Internal unit |
|---|---|
| horizontal/vertical distance | m |
| altitude | m |
| speed | m/s |
| acceleration | m/s² |
| time | s |
| angle | rad |
| area | m² |
| energy | Wh or J, selected consistently per subsystem |
| power | W |
| mass | kg |
| battery SOC | dimensionless [0,1] |
| overlap | dimensionless [0,1] |
| coverage ratio | dimensionless [0,1] |
| GSD | m/pixel |

External aviation/navigation formats may be converted at the boundary. Mixed-unit arithmetic inside a calculation module is prohibited.

### 32.1. Numerical comparison policy

Every geometric or numerical gate shall distinguish:

- exact domain condition;
- engineering tolerance;
- display rounding.

Display rounding shall never affect feasibility.

Tolerance values are controlled configuration, not hidden constants.

### 32.2. Geometry policy

For polygon/mesh operations:

- CRS shall be normalized before metric operations;
- invalid topology shall block planning or be explicitly repaired and recorded;
- small numerical slivers shall be handled by a controlled tolerance;
- area and distance calculations shall use the appropriate metric representation;
- geometry repair shall never silently change mission intent.

### 32.3. Deterministic ordering

Where values compare equal within tolerance, the implementation shall use a deterministic secondary key:

```text
primary objective
→ secondary objective
→ candidate generation index
→ stable candidate ID
```

Hash-map iteration order shall never determine the selected plan.

## 33. Hard Gates, Soft Objectives and Decision Order

The following classification is normative for MT-01 and MT-02.

### 33.1. Hard gates

- authorization and applicable airspace;
- prohibited geometry;
- terrain/obstacle clearance;
- UAV operating envelope;
- payload operating envelope;
- C2 requirements;
- mandatory separation;
- trajectory feasibility;
- mandatory acquisition geometry;
- protected energy reserve;
- mandatory product-quality thresholds.

### 33.2. Soft objectives

Depending on the active mission profile:

- total energy above the protected reserve;
- engine/propulsion resource consumption;
- flight time;
- route length;
- number of turns;
- transition distance;
- additional observations;
- additional redundancy;
- computational cost.

A soft objective can never compensate for a hard-gate failure.

### 33.3. Deterministic selection

If multiple candidates remain equivalent after the active objective profile:

```text
1. higher safety margin
2. higher protected energy margin
3. higher required-quality margin
4. lower energy
5. lower time
6. lower route complexity
7. stable candidate ID
```

The exact mission profile may alter the order of soft objectives, but the deterministic final tie-break remains mandatory.

## 34. Incremental-Recalculation Dependency Contracts

The dependency graph shall be explicit rather than inferred at runtime.

### 34.1. MT-01

```text
AOI / restriction change
 → constrained domain
 → decomposition
 → tracks
 → transitions
 → trajectory
 → performance
 → energy
 → quality
 → final validation

Wind snapshot change
 → performance
 → trajectory timing
 → energy
 → affected quality
 → final validation

Battery state / degradation change
 → energy
 → feasibility / selection
 → final validation

Camera calibration / FOV change
 → acquisition geometry
 → tracks
 → acquisition events
 → quality
 → energy / time
 → final validation
```

### 34.2. MT-02

```text
Target geometry change
 → target discretisation
 → viewpoints
 → visibility
 → selection/refinement
 → viewpoint graph
 → trajectory
 → performance
 → energy
 → quality

Wind snapshot change
 → performance
 → trajectory timing
 → energy
 → affected acquisition timing
 → final validation

Camera / gimbal change
 → viewpoint feasibility
 → visibility
 → selection/refinement
 → acquisition events
 → quality
 → trajectory/performance/energy where affected

Battery state / degradation change
 → energy
 → candidate selection
 → final validation
```

A cache entry shall carry the dependency identities required to prove that reuse is valid.

## 35. Implementation Mapping and Test-Data Contract

The first implementation shall map the algorithm specification to explicit backend modules. Names below are logical component names and may be refined during architecture implementation.

| Algorithm responsibility | Logical implementation module |
|---|---|
| mission/context validation | MissionContextValidator |
| coordinate/geometry normalization | GeometryNormalizer |
| airspace/restriction filtering | SpatialConstraintEngine |
| MT-01 acquisition geometry | AcquisitionGeometryEngine |
| MT-01 decomposition | CoverageDecompositionEngine |
| MT-01 track generation | CoverageTrackGenerator |
| MT-02 target discretisation | TargetDiscretizationEngine |
| MT-02 viewpoint generation | ViewpointGenerator |
| MT-02 visibility | VisibilityEngine |
| transition routing | RouteGraphEngine |
| UAV/wind performance | PerformanceEngine |
| battery/energy | EnergyEngine |
| trajectory | TrajectoryEngine |
| mapping/reconstruction quality | TaskQualityEngine |
| candidate lifecycle | CandidateRegistry |
| objective comparison | ObjectiveOrchestrator |
| provenance/versioning | PlanningProvenanceService |
| incremental invalidation | PlanningDependencyGraph |
| deterministic replay | PlanningReplayService |

### 35.1. Minimum controlled test datasets

MT-01 shall have at least:

- T01 simple convex AOI;
- T02 concave AOI;
- T03 AOI with exclusion zone;
- T04 terrain relief;
- T05 obstacle corridor;
- T06 wind-shift scenario;
- T07 insufficient-energy scenario;
- T08 edge-coverage scenario;
- T09 deterministic replay pair.

MT-02 shall have at least:

- T21 flat surface;
- T22 terrain relief;
- T23 vertical structure;
- T24 occluded surface;
- T25 insufficient-overlap surface;
- T26 viewpoint-obstacle conflict;
- T27 camera/gimbal change;
- T28 wind change;
- T29 insufficient-energy scenario;
- T30 multi-UAV reconstruction scenario;
- T31 deterministic replay pair.

Each controlled dataset shall define:

```text
dataset_id
version
CRS
geometry
terrain/obstacles
airspace/restrictions
UAV configuration
payload configuration
environment snapshot
mission requirements
expected hard-gate outcomes
expected quality outcomes
expected invalidation scope
expected deterministic properties
```

### 35.2. Acceptance evidence

For each test dataset, evidence shall retain:

- input snapshot identifiers;
- planner and algorithm versions;
- selected parameters;
- candidate counts;
- rejection reasons;
- selected plan;
- quality metrics;
- energy/reserve result;
- final validation result;
- calculation time;
- replay result.

## 36. Traceability Closure for MT-01 / MT-02

The two templates shall not be declared implementation-ready merely because pseudocode exists. Each requirement affecting their planning behaviour shall trace to:

```text
REQUIREMENT
 ↓
ALGORITHM RULE
 ↓
IMPLEMENTATION MODULE
 ↓
VERIFICATION CASE
 ↓
EVIDENCE
```

The current repository requirement catalogue shall be used as the authoritative source for requirement IDs. Where a requirement cannot yet be allocated directly to MT-01 or MT-02, the trace shall be marked **OPEN ALLOCATION** rather than guessed.

The completion gate therefore requires explicit closure of:

- algorithm-to-requirement links;
- requirement-to-module links;
- requirement-to-verification links;
- verification-to-evidence definitions.

No requirement ID shall be invented solely to make the matrix appear complete.

## 37. Implementation-Readiness Status

The addition of Sections 30–36 closes the previously missing structural layer:

- typed logical data contracts — defined;
- calculation-module boundaries — defined;
- units and numerical policy — defined;
- hard/soft decision boundary — defined;
- deterministic comparison — defined;
- incremental dependency contracts — defined;
- implementation-module mapping — defined;
- controlled test-data contract — defined;
- traceability closure rule — defined.

**Status:** MT-01 and MT-02 are now at **implementation-ready specification structure**. They are not yet certified, qualified, or empirically validated. Controlled numerical values, UAV/sensor models, requirement allocation and executable verification evidence remain implementation/test work.

MT-03 remains blocked by the project sequencing decision until the implementation-readiness gate for MT-01/MT-02 is reviewed and accepted.
## 38. MT-01 / MT-02 Requirement Traceability — Controlled Allocation Review

The repository contains more than one requirement-facing document. The authoritative requirement identity is controlled by `01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md`, which explicitly requires preservation of existing `SYS-REQ-*` identities. The separate `BLUESKY_SYSTEM_REQUIREMENTS_BASELINE.md` uses a different `SYS-*` naming convention and shall not be treated as a replacement identity system.

Therefore MT-01/MT-02 traceability shall distinguish:

1. **authoritative requirement ID** — existing `SYS-REQ-*` / `SAF-REQ-*` records;
2. **working baseline capability** — statements from the system baseline;
3. **candidate derived requirements** — e.g. `MIS-REQ-*`, `NAV-REQ-*`, etc., which remain candidates until consolidation.

No crosswalk between these namespaces is inferred without exact controlled wording.

### 38.1. Confirmed existing requirement relationships

The following existing requirements are supported by repository traceability records and are relevant to MT-01/MT-02:

| Existing requirement | MT-01 | MT-02 | Current relationship |
|---|---|---|---|
| SYS-REQ-080 | supporting | supporting | Dynamic task/reallocation dependency where multi-UAV planning is active |
| SYS-REQ-081 | supporting | supporting | UAV-failure tolerance consumes valid planning/navigation/resource state |
| SYS-REQ-082 | direct dependency | direct dependency | Safe mission completion depends on valid feasible planning state |
| SYS-REQ-083 | direct | direct | Platform independence / canonical planning model |
| SYS-REQ-084 | direct dependency | direct dependency | Resource reservation affects planning feasibility |
| SYS-REQ-085 | supporting | supporting | Safety-critical priority protects planning dependencies |
| SYS-REQ-086 | supporting | supporting | Controlled degradation boundary |
| SYS-REQ-091 | supporting | supporting | Critical-latency dependency; numerical latency thresholds remain to be allocated |
| SYS-REQ-093 | supporting | supporting | Controlled recovery dependency |
| SYS-REQ-110 | open allocation | open allocation | Existing project requirement; exact controlled wording must be inspected before allocation |
| SYS-REQ-111 | open allocation | open allocation | Existing project requirement; exact controlled wording must be inspected before allocation |
| SYS-REQ-112 | open allocation | open allocation | Existing project requirement; exact controlled wording must be inspected before allocation |

The above does **not** mean these requirements are verified by MT-01/MT-02. It means the planning algorithms have a documented dependency relationship with them.

### 38.2. Candidate requirement families

The following candidate families remain outside the authoritative identity set until exact consolidation:

- `MIS-REQ-*`
- `NAV-REQ-*`
- `RTE-REQ-*`
- `WP-REQ-*`
- `MUL-REQ-*`
- `RDY-REQ-*`
- other SRS-derived families.

For MT-01/MT-02, the disposition is:

```text
CANDIDATE
→ compare exact wording against existing SYS-REQ / SAF-REQ
→ identify parent/child/derived relationship
→ allocate only after controlled reconciliation
```

### 38.3. Requirements for which allocation is not yet proven

The following planning dependencies are real at the algorithm level but must **not** be assigned invented requirement IDs:

- airspace/restriction ingestion;
- NOTAM/aeronautical data;
- weather/wind;
- map/DEM/terrain;
- GNSS/RTK/PPK/NTRIP;
- payload compatibility;
- readiness aggregation;
- contingency execution;
- AI recommendation authority.

Until exact authoritative records are extracted and compared, their status is:

**OPEN ALLOCATION — NO ID INVENTED.**

### 38.4. Traceability chain

The controlled chain is:

```text
AUTHORITATIVE REQUIREMENT
        ↓
MT-01 / MT-02 ALGORITHM RULE
        ↓
LOGICAL IMPLEMENTATION MODULE
        ↓
VERIFICATION DATASET / CASE
        ↓
EXECUTION RESULT
        ↓
CONTROLLED EVIDENCE
```

The present project state supports the first four structural links for MT-01/MT-02. Executed evidence is not established by this documentation work.

### 38.5. Correction to the previous allocation

The previous version of this section incorrectly treated identifiers from `BLUESKY_SYSTEM_REQUIREMENTS_BASELINE.md` such as `SYS-003`, `MIS-001`, `EXT-AIR-001`, etc. as though they were already authoritative `SYS-REQ-*` identities.

**That interpretation is withdrawn.**

Those identifiers remain source-document terminology only until controlled consolidation establishes an explicit relationship to the authoritative register.

No requirement ID is created or renumbered by this correction.

## 39. MT-01 / MT-02 Review Gate Before MT-03

MT-03 remains blocked. Before it can be considered, the following review must be completed for both MT-01 and MT-02:

1. algorithm contract review;
2. data-contract review;
3. numerical-policy review;
4. implementation-module review;
5. authoritative requirement allocation review;
6. candidate-requirement consolidation review;
7. controlled test-data review;
8. deterministic replay design review;
9. explicit resolution of all OPEN ALLOCATION items;
10. review of remaining architecture/safety dependencies.

The gate does **not** require every related system requirement to be implemented before MT-01/MT-02 can proceed. It requires the planning algorithms to have an authoritative, non-invented allocation and a defined verification path.

### 39.1. Current gate status

| Gate item | Status |
|---|---|
| Algorithm contract | DEFINED |
| Data contracts | DEFINED |
| Numerical policy | DEFINED |
| Module boundaries | DEFINED |
| Candidate lifecycle | DEFINED |
| Test-data contract | DEFINED |
| Deterministic replay design | DEFINED |
| Authoritative requirement allocation | PARTIAL |
| Candidate SRS consolidation | OPEN |
| Executed verification evidence | NOT DONE |
| Certification closure | NOT DONE |

**Decision:** MT-01/MT-02 remain in controlled specification/review stage. MT-03 remains blocked.

## 40. Formal Quality Metrics for MT-01

All MT-01 quality metrics shall be calculated from the actual planned acquisition geometry and not from nominal route length alone.

### 40.1. Area coverage

Let A_AOI be the valid mission area after mandatory exclusions and A_covered the union of valid sensor footprints projected onto the acquisition surface.

coverage_ratio = area(A_covered ∩ A_AOI) / area(A_AOI)

Any mandatory sub-area shall have its own coverage gate. A high global coverage ratio shall not compensate for an uncovered mandatory region.

### 40.2. Uncovered-area representation

uncovered_geometry = AOI − covered_geometry

Classify it at minimum as boundary gap, exclusion-induced gap, terrain/obstacle-induced gap, trajectory infeasibility, sensor/acquisition infeasibility, or intentional non-required region. Preserve the reason in candidate provenance.

### 40.3. GSD

For a nadir-oriented pinhole-camera approximation:

GSD ≈ H · sensor_width / (focal_length · image_width_pixels)

where H is camera-to-ground distance; sensor_width and focal_length use the same physical length unit; image_width_pixels is the image dimension across sensor_width.

The implementation shall use the payload calibration model when available. This approximation is the fallback engineering model.

### 40.4. Overlap

frontal_overlap = 1 − image_spacing / footprint_along_track
side_overlap = 1 − track_spacing / footprint_cross_track

Values are evaluated from the actual footprint at the acquisition event. A nominal route spacing calculation alone is insufficient for acceptance.

### 40.5. Acquisition-event quality

Each acquisition event shall expose: event_id, position, camera_ground_distance, orientation, footprint, GSD, frontal_overlap, side_overlap, sensor_state, trigger_state, quality_state.

An event is mandatory-quality-valid only when every applicable hard quality criterion is satisfied.

### 40.6. Mapping-quality result

MT-01 shall return: coverage_ratio, mandatory_area_coverage, uncovered_geometry, GSD_min, GSD_max, GSD_target_deviation, frontal_overlap_min, side_overlap_min, acquisition_event_valid_ratio, terrain_following_compliance, sensor_compliance, quality_gate, quality_deficiencies[].

No aggregate score may hide a failed mandatory component.

## 41. Formal Quality Metrics for MT-02

MT-02 quality is based on observation geometry and reconstruction sufficiency, not simply percentage of target area visited.

### 41.1. Target-element model

Each target element j shall carry, where applicable: target_id, position, normal, importance_weight, required_observation_count, desired_distance, allowed_distance_range, allowed_incidence_range, minimum_parallax, required_sensor_mode, state.

### 41.2. Observation coverage

observation_count(j) = number of valid acquisition events observing j

A target element passes its observation-count gate when observation_count(j) >= required_observation_count(j).

weighted_target_coverage = Σ importance_weight(j) for valid elements / Σ importance_weight(j) for required elements

Mandatory target elements remain individual hard gates.

### 41.3. Viewing geometry

For target normal n, target point p_t, camera position p_c, and normalized viewing direction v:

incidence_cosine = dot(n, v)
distance_error = |d_actual − d_desired|

The exact sign convention shall be fixed by the sensor model and persisted in payload configuration. The planner shall never infer it from display orientation.

An observation passes when actual distance and incidence angle satisfy the applicable payload/mission bounds.

### 41.4. Parallax

For two valid observations i and k of the same target element:

parallax_angle = angle(view_vector_i, view_vector_k)

The quality engine shall retain the distribution of usable parallax, not only its maximum. A candidate may fail even when maximum parallax is sufficient if required target regions remain observed from insufficiently diverse viewpoints.

### 41.5. Visibility and occlusion

Classify each target-element/observation pair as VISIBLE, PARTIAL, OCCLUDED, OUT_OF_FOV, OUT_OF_RANGE, BLOCKED, or INVALID_SENSOR_STATE.

Only states accepted by the active quality model contribute to valid observation coverage.

### 41.6. Observation-network connectivity

Construct a bipartite graph of VIEWPOINT/IMAGE nodes and TARGET/OBSERVATION nodes. Detect isolated required target elements and disconnected reconstruction-critical components.

A candidate with adequate raw image count but disconnected reconstruction-critical components shall fail the corresponding quality gate.

### 41.7. MT-02 quality result

Return: weighted_target_coverage, mandatory_target_coverage, observation_count_distribution, distance_error_distribution, incidence_distribution, usable_parallax_distribution, visibility_statistics, occlusion_statistics, observation_network_components, sensor_resolution_metrics, GSD_or_point_density_metrics, quality_gate, quality_deficiencies[].

### 41.8. LiDAR branch

For LiDAR, image overlap is not the primary quality criterion. Evaluate applicable sensor quantities such as swath, swath_overlap, point_density, scan_angle, incidence, coverage, occlusion, and trajectory/sensor stability. The active sensor model determines which are mandatory.

## 42. Parameter Registry — No Hidden Constants

Every numerical planning parameter shall belong to one class: REQUIREMENT, VEHICLE, PAYLOAD, ENVIRONMENT, SAFETY_POLICY, ALGORITHM, NUMERICAL_TOLERANCE, OBJECTIVE_PROFILE, or DERIVED.

| Class | Meaning | Examples |
|---|---|---|
| REQUIREMENT | externally imposed or approved mission requirement | target GSD, mandatory overlap |
| VEHICLE | UAV capability/model value | max speed, climb rate |
| PAYLOAD | sensor/calibration value | focal length, FOV, trigger limits |
| ENVIRONMENT | measured/ingested state | wind, terrain, obstacles |
| SAFETY_POLICY | controlled safety value | clearance, separation, reserve |
| ALGORITHM | bounded computational setting | candidate budget, sampling density |
| NUMERICAL_TOLERANCE | engineering comparison tolerance | geometric equality tolerance |
| OBJECTIVE_PROFILE | task preference | time vs energy priority |
| DERIVED | calculated value | footprint, track spacing, energy |

### 42.1. Parameter identity

Each controlled parameter shall carry parameter_id, value, unit, source, version, validity, scope, class, and approved_state.

### 42.2. Parameter precedence

authoritative safety/requirement value > validated vehicle/payload configuration > validated external environment > approved mission override > algorithm default

An algorithm default may never silently override an authoritative value.

### 42.3. Missing parameter policy

If a mandatory parameter is unavailable, use a known safe fallback and record provenance; otherwise return BLOCKED_INPUT. The planner shall not manufacture a plausible numerical value.

### 42.4. Parameter versioning

Changing a parameter that affects candidate geometry, feasibility, energy, or quality shall invalidate every dependent result identified in the dependency graph.

## 43. Acceptance Criteria for MT-01 / MT-02 Specification Review

An item may be marked DEFINED only when purpose, inputs, outputs, units, dependencies, hard constraints, soft objectives, failure states, deterministic ordering, recalculation scope, provenance, and verification method are explicit.

### 43.1. MT-01 review checklist

- [x] input context defined;
- [x] constrained planning domain defined;
- [x] acquisition geometry defined;
- [x] coverage/decomposition defined;
- [x] track generation defined;
- [x] transition routing defined;
- [x] wind/performance dependency defined;
- [x] energy gate defined;
- [x] trajectory generation defined;
- [x] acquisition-event validation defined;
- [x] formal coverage/GSD/overlap metrics defined;
- [x] candidate selection defined;
- [x] failure taxonomy defined;
- [x] deterministic replay rule defined;
- [x] test-data contract defined;
- [ ] exact authoritative requirement allocation;
- [ ] controlled numerical parameter values;
- [ ] executed verification evidence.

### 43.2. MT-02 review checklist

- [x] target representation defined;
- [x] observation-space model defined;
- [x] viewpoint generation defined;
- [x] feasibility filtering defined;
- [x] visibility model defined;
- [x] global selection defined;
- [x] weak-region refinement defined;
- [x] transition graph defined;
- [x] camera orientation model defined;
- [x] reconstruction-network quality model defined;
- [x] LiDAR branch defined;
- [x] formal observation/parallax/visibility metrics defined;
- [x] candidate selection defined;
- [x] failure taxonomy defined;
- [x] deterministic replay rule defined;
- [x] test-data contract defined;
- [ ] exact authoritative requirement allocation;
- [ ] controlled numerical parameter values;
- [ ] executed verification evidence.

Conclusion: the algorithmic specification is sufficiently formal to begin implementation design, but MT-01/MT-02 are not yet verification-closed or certification-closed.


## 44. MT-01 / MT-02 Authoritative Requirement Allocation — Controlled Review Result

This section records the next controlled reconciliation step using the actual requirement records and existing traceability material. It does not promote candidate SRS identifiers to authoritative requirements and does not create new requirement IDs.

### 44.1 Allocation rule

Only the following dispositions are permitted:

- **DIRECT DEPENDENCY** — the requirement explicitly constrains or is consumed by planning behaviour;
- **SUPPORTING** — the requirement affects planning indirectly or through a shared system mechanism;
- **CROSS-CUTTING** — the requirement constrains the planning implementation/runtime but is not a planning functional requirement;
- **OPEN ALLOCATION** — the requirement identity or exact scope is not sufficiently established;
- **NOT ALLOCATED** — no defensible MT-01/MT-02 relationship has been established.

Candidate families such as `NAV-REQ-*`, `RTE-REQ-*`, `WP-REQ-*`, `MIS-REQ-*`, `MUL-REQ-*`, `RDY-REQ-*`, `C2-REQ-*` remain candidate records and are not treated as authoritative identities.

### 44.2 Confirmed allocation

| Requirement | MT-01 | MT-02 | Allocation basis |
|---|---|---|---|
| SYS-REQ-080 | SUPPORTING | SUPPORTING | Dynamic task/reallocation affects multi-UAV planning and task redistribution. |
| SYS-REQ-081 | SUPPORTING | SUPPORTING | UAV failure tolerance consumes valid planning/resource state and can require continuation or reassignment. |
| SYS-REQ-082 | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Safe completion explicitly depends on UAV state, resources, energy, communications, risks and safe return/landing/emergency behaviour. |
| SYS-REQ-083 | DIRECT | DIRECT | Mission/planning logic is required to remain platform-independent and use adapter/capability boundaries. |
| SYS-REQ-084 | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Planning requires reservation/availability of relevant compute, communication and system resources. |
| SYS-REQ-085 | CROSS-CUTTING | CROSS-CUTTING | Safety-critical functions, including navigation/C2, have priority over optimisation and lower-priority workloads. |
| SYS-REQ-086 | SUPPORTING | SUPPORTING | Controlled degradation/replanning/resource reallocation constrains behaviour when planning dependencies degrade. |
| SYS-REQ-091 | SUPPORTING | SUPPORTING | Critical latency affects validity/usability of safety-significant planning inputs and services; exact planning-specific limits remain lower-level allocation work. |
| SYS-REQ-093 | SUPPORTING | SUPPORTING | Controlled recovery constrains recovery/replanning and protection of critical functions after resource degradation. |

The allocation above is a **dependency allocation**, not a verification result. Existing architecture coverage being marked PARTIAL remains PARTIAL and is not upgraded by this section.

### 44.3 Requirement-specific interpretation

**SYS-REQ-082 — Safe Mission Completion**

The controlled requirement text states that safe completion considers UAV state, task criticality, available resources, energy, communications and risks, and may require return, landing, reserve mode, result preservation and unfinished-task recording. Therefore MT-01/MT-02 planning must expose sufficient validated state and plan outputs to support the safe-completion chain. The planning algorithm does not itself become the authority for emergency execution.

**SYS-REQ-083 — Platform Independence**

The requirement explicitly requires mission-management logic to be independent of a particular manufacturer, autopilot or physical platform, with platform integration through adapters and capability profiles. Therefore MT-01/MT-02 data contracts and planner modules must consume canonical UAV/payload capability models rather than platform-specific assumptions.

**SYS-REQ-084 — Resource Reservation**

The architecture traceability establishes resource reservation and controlled temporal scheduling. Therefore planning computation and mission resource requirements must be compatible with the reservation lifecycle; a planner may not assume unreserved resources are available merely because they are technically present.

**SYS-REQ-085 — Safety-Critical Priority**

The requirement explicitly protects P0/P1 functions, including safety, flight control, navigation and C2. Planning optimisation therefore remains subordinate to safety-critical resource and authority handling.

**SYS-REQ-086 — Graceful Degradation**

The existing architecture allocates controlled continuation, adaptation, resource reallocation, UAV role change, mission-scope reduction, replanning and abort to degradation handling. MT-01/MT-02 incremental recalculation must therefore distinguish a controlled degraded/replanning path from an invalid plan.

**SYS-REQ-091 — Critical Latency**

The requirement concerns controlled latency for critical messages and protective action on threshold exceedance. This establishes a cross-cutting planning dependency, but it does not by itself establish a numerical planner latency target.

**SYS-REQ-093 — Controlled Resource Recovery**

Recovery must be controlled, must not create secondary overload, and must restore critical functions before lower-priority functions. Planning/replanning services therefore remain subject to the recovery state rather than treating recovery as an invisible background event.

### 44.4 SYS-REQ-110 / 111 / 112

The repository confirms these IDs and their titles:

- SYS-REQ-110 — Multi-Agent AI Orchestration;
- SYS-REQ-111 — AI Agent Authority and Proposal Control;
- SYS-REQ-112 — Offline AI Operational Continuity.

For MT-01/MT-02 these remain **OPEN ALLOCATION** at requirement level in this review. The algorithm document can define AI as a bounded strategy/parameter/candidate aid, but exact allocation to these authoritative records requires inspection of their controlled requirement text and existing capability traceability before a direct relationship is asserted.

No AI requirement is created by this section.

### 44.5 Candidate SRS families

The following remain unbaselined candidate families:

- NAV-REQ-001..009;
- RTE-REQ-001..004;
- WP-REQ-001..003;
- MIS-REQ-001..003;
- RET-REQ-001..004;
- COL-REQ-001..004;
- C2-REQ-001..003;
- MUL-REQ-001..003;
- RDY-REQ-001..002.

They are not silently mapped to MT-01/MT-02 as authoritative requirements.

Their role in the algorithm specification is currently **candidate derived allocation** pending exact wording comparison against existing authoritative records.

### 44.6 Allocation status after this review

| Allocation area | Status |
|---|---|
| Existing SYS-REQ-080..086 | ALLOCATED / dependency relationship defined |
| SYS-REQ-091 | ALLOCATED / supporting dependency |
| SYS-REQ-093 | ALLOCATED / supporting dependency |
| SYS-REQ-110..112 | OPEN ALLOCATION |
| Candidate NAV/RTE/WP/MIS/RET/COL/C2/MUL/RDY families | CANDIDATE / NOT BASELINED |
| Airspace / NOTAM | OPEN ALLOCATION |
| Weather / wind | OPEN ALLOCATION |
| Terrain / DEM | OPEN ALLOCATION |
| GNSS / RTK / PPK / NTRIP | OPEN ALLOCATION |
| Payload compatibility | OPEN ALLOCATION |
| Readiness aggregation | OPEN ALLOCATION |
| Contingency execution authority | OPEN ALLOCATION |
| AI authority | OPEN ALLOCATION |
| Executed verification evidence | NOT DONE |

### 44.7 Gate consequence

This review closes the **structural allocation** for the confirmed existing SYS-REQ dependencies above, but it does not close the overall MT-01/MT-02 gate.

The remaining blockers are:

1. exact allocation of SYS-REQ-110..112;
2. controlled consolidation of candidate SRS families;
3. allocation of external/environmental dependencies;
4. controlled numerical parameter values;
5. executable verification and evidence.

MT-03 remains blocked.


## 45. MT-01 / MT-02 Allocation — SYS-REQ-110..112 Controlled Resolution

The previous Section 44 disposition of SYS-REQ-110..112 as wholly OPEN ALLOCATION is superseded by this controlled review result.

Repository traceability explicitly links:

`ARCH-DEC-046 → SYS-REQ-110 → capability → agent/orchestrator → proposal → validation → Safety Gate → C++ Core → execution`

and the reverse chain explicitly links ARCH-DEC-046 to SYS-REQ-110, SYS-REQ-111 and SYS-REQ-112.

### 45.1 SYS-REQ-110 — Multi-Agent AI Orchestration

**Allocation: SUPPORTING / DIRECT DEPENDENCY when AI-assisted planning is active.**

Basis:
- SYS-REQ-110 explicitly covers AI orchestration for analysis and planning;
- required capabilities include task assignment, controlled context, result aggregation, conflict handling and traceability;
- ARCH-DEC-046 defines the corresponding orchestration architecture;
- MT-01/MT-02 already define AI as an optional bounded strategy/parameter/candidate aid rather than an authority.

Planning consequence:

`AI ANALYSIS / PROPOSAL → PLANNING VALIDATION → SAFETY / AUTHORITY GATES`

AI orchestration may assist generation or comparison of candidates, but it does not replace the deterministic planner, hard-constraint gates or final validation.

### 45.2 SYS-REQ-111 — AI Agent Authority and Proposal Control

**Allocation: DIRECT DEPENDENCY.**

This requirement is directly relevant to MT-01/MT-02 because the planning architecture permits AI-assisted strategy, parameter and candidate selection.

The authoritative execution chain remains:

`AI AGENT → PROPOSAL → VALIDATION → SAFETY GATE → AUTHORIZATION → C++ CORE → EXECUTION`

Therefore:
- AI cannot modify authoritative mission state directly;
- AI cannot bypass planner validation;
- AI cannot weaken hard constraints;
- AI cannot convert a rejected candidate into an executable plan;
- AI-generated planning recommendations require the same mandatory validation boundary as non-AI planning proposals.

### 45.3 SYS-REQ-112 — Offline AI Operational Continuity

**Allocation: SUPPORTING / CONDITIONAL DEPENDENCY.**

This requirement applies when AI assistance is enabled and the planning system operates without external AI/network services.

The requirement requires local/authorised AI capability and preservation of:
- Mission Validation;
- Safety Engine;
- Safety Gate;
- mandatory safety constraints;
- operator approval;
- C++ Core execution authority.

Planning consequence:

Loss of external AI capability may reduce AI-assisted optimisation/recommendation capability, but it shall not invalidate the deterministic planning core merely because an external AI service is unavailable. If a required local planning dependency is genuinely unavailable, the planner follows the existing BLOCKED_INPUT / controlled-degradation rules and records provenance.

### 45.4 Controlled allocation table

| Requirement | MT-01 | MT-02 | Status |
|---|---|---|---|
| SYS-REQ-110 | SUPPORTING / conditional direct dependency | SUPPORTING / conditional direct dependency | ALLOCATED |
| SYS-REQ-111 | DIRECT DEPENDENCY | DIRECT DEPENDENCY | ALLOCATED |
| SYS-REQ-112 | SUPPORTING / conditional dependency | SUPPORTING / conditional dependency | ALLOCATED |

This allocation does not claim that the requirements themselves are baselined or verified. It establishes their relationship to MT-01/MT-02 using the existing controlled requirement and architecture records.

### 45.5 Remaining requirement-allocation blockers

After this resolution, the remaining allocation work is:

1. candidate SRS-family reconciliation against authoritative SYS-REQ/SAF-REQ records;
2. exact allocation of airspace/NOTAM, weather, terrain/DEM, GNSS/RTK/PPK/NTRIP, payload compatibility and readiness dependencies;
3. controlled numerical parameter approval;
4. executable verification/evidence.

SYS-REQ-110..112 are no longer treated as unresolved merely because their detailed lower-level verification is pending.

MT-03 remains blocked.

## 46. Candidate SRS Family Reconciliation — Controlled Result

This section records the family-level reconciliation of candidate SRS identifiers against the authoritative requirement records actually present in the repository. It does not promote candidate IDs to authoritative identities and does not create new IDs.

### 46.1 Reconciliation rule

- **CONSOLIDATE** — substantial overlap with existing authoritative requirements; use existing IDs plus lower-level derived allocation.
- **DERIVED** — lower-level functional decomposition of existing requirements.
- **CLARIFYING** — clarification of an existing safety/authority requirement; not an independent baseline.
- **ENGINEERING** — engineering-level candidate, not automatically a system certification requirement.
- **OPEN ALLOCATION** — controlled wording/source comparison is still insufficient for one-to-one allocation.

This is a reconciliation disposition, not a baseline or verification result.

### 46.2 Controlled family reconciliation

| Candidate family | Disposition | Existing authoritative overlap / basis |
|---|---|---|
| NAV-REQ-001..009 | DERIVED / OPEN ALLOCATION | Navigation-specific decomposition is not represented by an independent authoritative family. SYS-REQ-016, SYS-REQ-076, SYS-REQ-083 and SYS-REQ-085 constrain navigation-related planning inputs/capabilities. |
| RTE-REQ-001..004 | DERIVED | Overlaps mission planning/compiler through SYS-REQ-004, SYS-REQ-005, SYS-REQ-010, SYS-REQ-022 and SYS-REQ-023. |
| WP-REQ-001..003 | DERIVED | Waypoint concepts are lower-level manifestations of SYS-REQ-004, SYS-REQ-022 and SYS-REQ-023. |
| MIS-REQ-001..003 | CONSOLIDATE | Strong overlap with SYS-REQ-001, SYS-REQ-002, SYS-REQ-004, SYS-REQ-005, SYS-REQ-022, SYS-REQ-023 and SYS-REQ-035. |
| RET-REQ-001..004 | CONSOLIDATE / DERIVED | Return and safe completion overlap SYS-REQ-012, SYS-REQ-081, SYS-REQ-082, SYS-REQ-086 and SYS-REQ-093. |
| COL-REQ-001..004 | CONSOLIDATE / DERIVED | Conflict resolution is SYS-REQ-009; multi-UAV execution/reallocation is covered by SYS-REQ-075..082. |
| C2-REQ-001..003 | CONSOLIDATE / DERIVED | Strong overlap with SYS-REQ-016, SYS-REQ-067..074, SYS-REQ-085 and SYS-REQ-091. |
| MUL-REQ-001..003 | CONSOLIDATE | Strong overlap with SYS-REQ-032 and SYS-REQ-075..078, plus SYS-REQ-080..082. |
| RDY-REQ-001..002 | CONSOLIDATE | SYS-REQ-008 already defines Mission Readiness; Readiness remains distinct from Safety Gate and operator approval. |
| SAF-REQ-019..020 | CLARIFYING | Master Register explicitly keeps these as candidate derived/clarifying records pending comparison with SYS-REQ-082, SYS-REQ-085 and ARCH-DEC-007/016/017. |
| AUTH-REQ-001..002 | CLARIFYING | Existing authority chain and SYS-REQ-007, SYS-REQ-015, SYS-REQ-082 and SYS-REQ-085 already define the authority boundary. |
| HMI-REQ-001..002 | DERIVED | SYS-REQ-026 defines UI architecture; HMI cannot bypass validation, safety or approval. |
| AI-REQ-001..003 | CONSOLIDATE / DERIVED | Existing coverage spans SYS-REQ-005, 013, 014, 017..019, 025, 087, 088, 094..104, 107, 108 and 110..112. |
| DATA-REQ-001..002 | DERIVED | Data/technical-data coverage exists in SYS-REQ-022, SYS-REQ-029, SYS-REQ-030, SYS-REQ-094 and SYS-REQ-104. |
| CFG-REQ-001..003 | DERIVED | SYS-REQ-028 is the existing Configuration Management requirement; SYS-REQ-076 also constrains machine-readable capability profiles. |
| SW-REQ-001..003 | ENGINEERING | SRS explicitly identifies these as preliminary engineering requirements, not automatically software certification assurance requirements. |
| HW-REQ-001..002 | ENGINEERING | SRS explicitly identifies these as preliminary engineering requirements, not automatically hardware certification assurance requirements. |

### 46.3 Controlled conclusion

The candidate SRS is **not a second independent requirement baseline**.

The strongest overlap groups are MIS, RET, COL, C2, MUL, RDY and AI. NAV, RTE, WP, HMI, DATA and CFG are more appropriately treated as derived lower-level decomposition, subject to exact wording/source allocation before agreement. SAF-REQ-019..020 and AUTH-REQ-001..002 remain clarifying candidates and must not establish a competing authority chain. SW-REQ-* and HW-REQ-* remain engineering candidates.

### 46.4 Gate status after reconciliation

| Gate | Status |
|---|---|
| Candidate SRS family-level overlap analysis | **COMPLETED** |
| Candidate IDs promoted to authoritative baseline | **NO** |
| Existing SYS-REQ / SAF-REQ identities preserved | **YES** |
| Exact individual candidate wording consolidation | **OPEN** |
| External/environmental dependency allocation | **OPEN** |
| Controlled numerical parameter approval | **OPEN** |
| Executable verification/evidence | **NOT DONE** |

The family-level reconciliation blocker is therefore closed. MT-01/MT-02 remain incomplete because exact individual candidate wording, external/environmental dependencies, numerical parameters and executable evidence are still open.

MT-03 remains blocked.

## 47. External / Environmental Planning Dependency Allocation — Controlled Review

The next allocation gate was reviewed against the current Master Requirements Register, the derived SRS, and the existing authoritative SYS-REQ records. No new requirement IDs are created.

### 47.1 Current authoritative coverage

| Dependency | Current controlled evidence | MT-01 / MT-02 status |
|---|---|---|
| Airspace / NOTAM / operational restrictions | The algorithm contract requires airspace/authorization/restriction handling, but the current Master Register does not identify a dedicated authoritative requirement ID for this planning dependency. | **OPEN ALLOCATION** |
| Weather / wind | Wind is an explicit algorithm input and performance dependency; no dedicated authoritative requirement ID was established by the current controlled register review. | **OPEN ALLOCATION** |
| Terrain / DEM / obstacles | Terrain and obstacle constraints are explicit algorithm inputs and hard feasibility gates; a dedicated authoritative requirement ID was not established by the current controlled register review. | **OPEN ALLOCATION** |
| GNSS / RTK / PPK / NTRIP | These are identified as navigation/data dependencies, but the current controlled register/SRS does not provide a verified one-to-one authoritative allocation. | **OPEN ALLOCATION** |
| Payload / sensor compatibility | SYS-REQ-035 establishes task-to-capability mapping and SYS-REQ-076 provides machine-readable capability-profile linkage; exact payload-planning allocation remains lower-level. | **PARTIAL / OPEN ALLOCATION** |
| Mission readiness aggregation | SYS-REQ-008 is authoritative for Mission Readiness; exact allocation of the planner's readiness inputs/state aggregation remains lower-level. | **ALLOCATED at system level / OPEN at planning detail** |
| Contingency execution authority | Existing authority chain and SYS-REQ-082/085 constrain execution; planning may prepare alternatives but does not own emergency execution authority. | **ALLOCATED at authority level / OPEN at planning interface detail** |

### 47.2 Controlled interpretation

The absence of a dedicated ID is not evidence that the dependency is missing from the system. It means only that the present controlled register does not establish a unique one-to-one requirement identity for the specific planning dependency.

Therefore the algorithm specification shall continue to model these dependencies as mandatory inputs/constraints, while traceability records use OPEN ALLOCATION until an exact authoritative source record is identified.

No numerical value is introduced by this review.

In particular, this review does not establish:
- a specific NOTAM/airspace rule;
- a specific wind threshold;
- a terrain/obstacle clearance value;
- GNSS/RTK/PPK accuracy thresholds;
- payload compatibility thresholds;
- readiness timing thresholds;
- contingency execution authority values.

Such values require controlled source, engineering basis, or approved requirement allocation.

### 47.3 Gate status

| Gate | Status |
|---|---|
| External/environmental dependency identification | **COMPLETED** |
| Exact authoritative ID allocation | **OPEN** |
| Numerical parameter approval | **OPEN** |
| Regulatory/source clause mapping | **OPEN** |
| Executable verification/evidence | **NOT DONE** |

This review closes the identification sub-step but deliberately does not close the allocation sub-step.

MT-03 remains blocked.


## 48. External / Environmental Dependency Allocation — Controlled Resolution

Section 47 identified the dependency classes but intentionally stopped short of allocation. This section records the exact existing project records and external source clauses that can now be allocated without inventing new requirement IDs.

### 48.1 Airspace / NOTAM / operational restrictions

The repository contains existing authoritative system-level records that explicitly cover the required information flow and readiness/validation boundary:

- **SYS-REQ-008 — Mission Readiness**: Airspace is an explicit readiness area.
- **SYS-REQ-067 — HUB to PILOT Interface**: HUB provides PILOT with Airspace and Regulatory Constraints.
- **SYS-REQ-068 — HUB to PRO Interface**: HUB provides PRO with Airspace, NOTAM and Regulatory State.
- **ARCH-DEC-007 — Mission Validation and Safety Gate**: readiness includes Airspace and execution is blocked by prohibited airspace/geofence conditions.
- **ARCH-026** provides the corresponding HUB interface contract.

Therefore the dependency is no longer correctly described as having *no existing authoritative coverage*. The controlled allocation is:

| Dependency | MT-01 | MT-02 | Allocation |
|---|---|---|---|
| Airspace / restrictions | DIRECT DEPENDENCY | DIRECT DEPENDENCY | SYS-REQ-008 + ARCH-DEC-007 |
| Airspace / NOTAM data interface | SUPPORTING INPUT | SUPPORTING INPUT | SYS-REQ-067 / SYS-REQ-068 + ARCH-026 |
| Authorization state | DIRECT DEPENDENCY | DIRECT DEPENDENCY | ARCH-DEC-007 authority/readiness boundary; exact lower-level requirement wording remains OPEN |

The project records do **not** yet establish a unique lower-level requirement ID specifically for the planner's airspace-data schema, NOTAM freshness model, authorization-data validity model, or exact restriction-resolution algorithm. Those remain **OPEN ALLOCATION at planning/interface detail**, not at system-level dependency existence.

External regulatory basis requiring controlled applicability mapping includes the current Federal Rules for Use of Airspace, approved by Government Resolution No. 138 of 11 March 2010. The current text contains UAV-specific provisions including flight-plan/airspace-use permission conditions and publication of UAV-route information in aeronautical information documents. The rules were amended by Government Resolution No. 1253 of 29 September 2026; the project must use the applicable current revision when the regulatory trace is baselined.

**Regulatory clause mapping status: OPEN.** No clause is promoted into a BlueSky requirement until applicability, exact wording and compliance method are controlled in the requirements register.

### 48.2 Weather / wind

Existing project coverage is also identifiable:

- **SYS-REQ-008 — Mission Readiness** explicitly includes Weather.
- **SYS-REQ-016 — Communication and C2** does not define weather, so it is not used as the weather requirement allocation.
- **SYS-REQ-067 / SYS-REQ-068** provide Weather through the HUB interfaces.
- **ARCH-DEC-007** requires Weather in validation/readiness and identifies changes in weather as conditions that may invalidate prior validation/readiness.

For MT-01/MT-02:

| Dependency | MT-01 | MT-02 | Allocation |
|---|---|---|---|
| Weather readiness state | DIRECT DEPENDENCY | DIRECT DEPENDENCY | SYS-REQ-008 + ARCH-DEC-007 |
| Weather data availability/interface | SUPPORTING INPUT | SUPPORTING INPUT | SYS-REQ-067 / SYS-REQ-068 |
| Wind as planning/performance input | DIRECT ALGORITHM INPUT | DIRECT ALGORITHM INPUT | Existing algorithm contract; lower-level numerical/source allocation remains OPEN |

A current external source is **Order of the Ministry of Transport of Russia No. 49 dated 05.02.2026**, establishing the Federal Aviation Rules for provision of meteorological information for aircraft operations. The rules explicitly provide meteorological information to operators of unmanned aviation systems and external pilots, including METAR/SPECI, TAF, GAMET/AIRMET, SIGMET, SIGWX and upper-level wind/temperature forecasts.

This source supports the existence and categories of meteorological information required for the planning data chain. It does **not** by itself establish BlueSky-specific wind limits, UAV performance thresholds, mission cancellation criteria or energy penalties.

**Regulatory/source-clause mapping status: OPEN.** Exact applicability to each MT-01/MT-02 data field and compliance method must be controlled before baseline.

### 48.3 Terrain / DEM / obstacles

The repository provides system-level allocation through:

- **SYS-REQ-007 — Mission Validation Engine**: mission validation covers mandatory operational and safety constraints and includes UAV/altitude/geofence/emergency-recovery validation.
- **SYS-REQ-008 — Mission Readiness**: Terrain is an explicit readiness area.
- **SYS-REQ-011 — Simulation / Digital Twin**: Terrain is an explicit simulation input.
- **ARCH-DEC-007**: Mission Validation/Readiness includes Terrain; unacceptable terrain conditions can block execution.
- Existing capability traceability identifies obstacle-avoidance capability as a supporting capability where applicable.

For MT-01/MT-02 the controlled allocation is:

| Dependency | MT-01 | MT-02 | Allocation |
|---|---|---|---|
| Terrain / elevation state | DIRECT DEPENDENCY | DIRECT DEPENDENCY | SYS-REQ-008 + ARCH-DEC-007 |
| Terrain in validation | DIRECT DEPENDENCY | DIRECT DEPENDENCY | SYS-REQ-007 + ARCH-DEC-007 |
| Terrain in simulation/replay | SUPPORTING | SUPPORTING | SYS-REQ-011 |
| Obstacles / obstacle state | DIRECT ALGORITHM INPUT | DIRECT ALGORITHM INPUT | Algorithm contract; exact system-level requirement identity remains OPEN |
| Obstacle-avoidance capability | SUPPORTING | SUPPORTING | Existing capability traceability; not promoted to a new requirement |

The controlled records therefore establish Terrain as a system-level dependency, but do not yet establish a dedicated authoritative requirement for the **source, resolution, vertical datum, freshness, integrity, obstacle classification or update procedure** of DEM/obstacle data.

Those lower-level data-contract properties remain **OPEN ALLOCATION** and must not be filled with assumed values.

### 48.4 Result of the controlled resolution

| Dependency class | Previous Section 47 status | Current status |
|---|---|---|
| Airspace / restrictions | OPEN ALLOCATION | **ALLOCATED at system level; planning-data detail OPEN** |
| NOTAM / regulatory state | OPEN ALLOCATION | **ALLOCATED as interface/supporting input; exact planner schema/validity OPEN** |
| Weather | OPEN ALLOCATION | **ALLOCATED at system/readiness/interface level; planning detail OPEN** |
| Wind | OPEN ALLOCATION | **ALLOCATED as explicit algorithm input; numerical limits/source mapping OPEN** |
| Terrain / DEM | OPEN ALLOCATION | **ALLOCATED at system/readiness/validation level; data-contract detail OPEN** |
| Obstacles | OPEN ALLOCATION | **Algorithm dependency confirmed; authoritative lower-level requirement OPEN** |

### 48.5 Controlled boundary

This resolution does **not** create new requirement IDs and does not claim regulatory compliance.

It establishes the following controlled rule:

`EXISTING SYSTEM REQUIREMENT / ARCHITECTURE`
→ `MT-01 / MT-02 ALGORITHM DEPENDENCY`
→ `LOWER-LEVEL DATA / INTERFACE REQUIREMENT`
→ `VERIFICATION`
→ `EVIDENCE`

The lower-level requirement/data-contract layer remains open where the repository does not yet contain a unique authoritative record.

### 48.6 Gate status after Section 48

| Gate | Status |
|---|---|
| Existing system-level allocation for airspace / weather / terrain | **RESOLVED** |
| Existing interface allocation for Airspace / NOTAM / Weather | **RESOLVED** |
| Dedicated planner data-contract IDs | **OPEN** |
| Obstacles dedicated requirement identity | **OPEN** |
| Regulatory clause applicability mapping | **OPEN** |
| Numerical parameter approval | **OPEN** |
| Executable verification/evidence | **NOT DONE** |

**MT-03 remains blocked.**

The next deterministic allocation class is **GNSS / RTK / PPK / NTRIP and navigation-data provenance**, followed by payload/sensor compatibility and readiness/contingency lower-level interfaces.

## 49. GNSS / RTK / PPK / NTRIP and Navigation-Data Provenance — Controlled Resolution

Section 48 identified GNSS/RTK/PPK/NTRIP as the next allocation class. The repository review found that Navigation already has a substantial controlled engineering and verification chain. The correct action is therefore allocation and reconciliation, not creation of a parallel navigation requirement database.

### 49.1 Existing controlled navigation basis

The current project records establish the following:
- **CAP-001 — Navigation** is the reusable navigation capability. It consumes position, velocity, heading, altitude, route, waypoints, spatial/environmental information and UAV state, and produces navigation state, estimated position/motion, route-relative state and navigation validity.
- **Navigation State Model** separates PLANNED, ACTUAL, DERIVED and QUALITY layers.
- **Navigation Knowledge Map** requires provenance for critical values: source, timestamp, freshness, validity, confidence.
- **Navigation Algorithm** requires source validation and classifies critical navigation data as VALID, DEGRADED, STALE, INVALID or UNAVAILABLE.
- **Navigation Verification Model** verifies source/quality, reference frame, timestamp, freshness, validity and confidence before calculation and safety decision.
- **NAVIGATION_REQUIREMENT_ALLOCATION_001** already allocates existing SYS-REQ records to Navigation without treating them as new Navigation requirements.
- **NAVIGATION_SYSREQ_CONTENT_RECONCILIATION_PASS_003** explicitly retains SYS-REQ-081/082/085/086/091/093 and allocates them rather than replacing them with NAV-REQ records.
- The repository also contains an evidence chain for Navigation / GNSS / RTK / NTRIP (EC-03) and a source category GNSS_RTK_NTRIP.

### 49.2 Authoritative system-level allocation

| Existing requirement | Navigation relationship | MT-01 / MT-02 consequence |
|---|---|---|
| SYS-REQ-081 | SUPPORTING — navigation state/quality contributes to failure-tolerant continuation | Navigation validity is an input to candidate feasibility and safe continuation |
| SYS-REQ-082 | DIRECT DEPENDENCY / SUPPORTING at system level — safe completion consumes validated navigation state | Return/recovery feasibility cannot use unvalidated navigation state |
| SYS-REQ-085 | SUPPORTING / CROSS-CUTTING — Navigation is explicitly P0/P1 | Navigation-related processing cannot be displaced by lower-priority workload |
| SYS-REQ-086 | DIRECT | Navigation degradation states must remain within controlled degradation behaviour |
| SYS-REQ-091 | SUPPORTING / INDIRECT | Navigation freshness/latency matters; exact navigation thresholds remain lower-level |
| SYS-REQ-093 | PENDING_WORDING for Navigation-specific ownership | Do not allocate more strongly without full requirement wording review |

This allocation is already represented by the project's Navigation traceability/verification records and is not being duplicated here.

### 49.3 GNSS / RTK / PPK / NTRIP classification

| Dependency | MT-01 | MT-02 | Current allocation |
|---|---|---|---|
| GNSS position/navigation source | DIRECT INPUT | DIRECT INPUT | CAP-001 Navigation + existing Navigation State/Algorithm |
| RTK correction state | DIRECT INPUT when used by selected navigation configuration | DIRECT INPUT when used | GNSS/RTK/NTRIP evidence/interface chain; exact requirement wording OPEN |
| NTRIP correction stream | SUPPORTING INPUT when RTK/NTRIP configuration is selected | SUPPORTING INPUT when selected | Existing GNSS_RTK_NTRIP source/interface records; exact planner dependency OPEN |
| PPK status/data | SUPPORTING INPUT where post-processed navigation is part of dataset/planning provenance | SUPPORTING INPUT where applicable | Existing dataset/provenance model; exact planner requirement OPEN |
| Navigation quality | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Existing Navigation State/Algorithm/Verification chain |
| Freshness / timestamp | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Existing Navigation State/Verification chain; quantitative thresholds OPEN |
| Reference frame / coordinate semantics | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Navigation State/Rules/Algorithm; unresolved conventions remain OPEN |
| Source conflict / source selection | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Navigation Algorithm/Verification; source hierarchy/fusion rule remains OPEN |

### 49.4 Critical boundary: RTK is not automatically a planning requirement

The presence of RTK/NTRIP in the architecture or evidence chain does **not** mean that every MT-01/MT-02 mission requires RTK.

The planning dependency is conditional on the selected UAV/navigation/payload configuration and mission-quality requirements.

MISSION REQUIREMENT / QUALITY PROFILE → REQUIRED NAVIGATION QUALITY → SELECTED NAVIGATION CONFIGURATION → GNSS / RTK / PPK / NTRIP AVAILABILITY → VALIDATION → PLAN CANDIDATE

The planner shall not invent an RTK requirement merely because an RTK-capable configuration exists. Likewise, absence of RTK shall not automatically be classified as failure unless the applicable mission, payload, quality, safety or configuration requirement makes that capability mandatory.

### 49.5 Provenance contract

For navigation data used by MT-01/MT-02, the existing project knowledge chain establishes the minimum provenance concepts:
- source;
- timestamp;
- freshness;
- validity;
- confidence;
- reference frame;
- units;
- configuration context.

For RTK/PPK/NTRIP-specific data, the exact fields and acceptance rules are **not yet baselined**. The project records support the existence of the data category but do not authorize invention of RTK fix thresholds, positional accuracy thresholds, correction age limits, NTRIP latency limits, PPK quality thresholds, GNSS satellite-count thresholds, HDOP/VDOP limits, covariance limits or source-fusion weights.

These remain **OPEN numerical/data-contract parameters** until allocated to an authoritative requirement, approved engineering parameter, or controlled external source.

### 49.6 Candidate NAV-REQ family disposition

NAV-REQ-001..009 remains a candidate/derived family.

The current evidence is sufficient to state:
- do **not** promote NAV-REQ-001..009 to an independent baseline;
- retain the existing SYS-REQ identities;
- use Navigation State / Rules / Algorithm / Verification as the lower-level engineering decomposition;
- use NAV-REQ records only as derived/reconciliation records until exact controlled wording and source allocation are completed.

This preserves the project rule:

ONE REQUIREMENT → ONE STABLE ID → ONE CONTROLLED WORDING → MANY RELATIONSHIPS

### 49.7 Regulatory allocation status

The current project review does not establish a verified one-to-one Russian regulatory clause requiring a specific GNSS/RTK/PPK/NTRIP performance value for MT-01/MT-02.

Therefore no such value is introduced.

The regulatory chain remains:

OFFICIAL SOURCE → APPLICABILITY → REQUIREMENT → NAVIGATION / PLANNING ALLOCATION → VERIFICATION → EVIDENCE

The existing navigation source-review records also explicitly distinguish technical navigation knowledge from regulatory authority. This distinction is retained.

### 49.8 Gate status after Section 49

| Gate | Status |
|---|---|
| Existing Navigation capability allocation | **RESOLVED** |
| Existing SYS-REQ navigation relationships | **RESOLVED / CONTROLLED** |
| GNSS / RTK / NTRIP dependency existence | **RESOLVED** |
| PPK dependency/provenance | **IDENTIFIED; detailed allocation OPEN** |
| Navigation provenance concepts | **RESOLVED at conceptual level** |
| Exact RTK/PPK/NTRIP data contract | **OPEN** |
| Navigation numerical thresholds | **OPEN** |
| Regulatory performance clause mapping | **OPEN** |
| Executable verification/evidence | **NOT DONE** |

**MT-03 remains blocked.**

The next deterministic allocation class is **payload / sensor compatibility**, using the existing capability mapping and payload-related system records before considering any new requirement identity.

## 50. Payload / Sensor Compatibility — Controlled Resolution

Section 49 identified payload / sensor compatibility as the next deterministic allocation class. The repository review confirms that this dependency is already represented by existing system requirements and a controlled vehicle/equipment capability architecture. The correct action is therefore to allocate those existing records to MT-01/MT-02 without introducing a separate Payload requirement identity.

### 50.1 Existing authoritative requirement basis

The controlled requirement records establish the following:

- SYS-REQ-035 — Task to Capability Mapping requires the system to transform a task into the necessary capability set, including task type, required capabilities, suitable vehicle types, required payload, principal algorithms, autonomy, communication requirements and success criteria.
- SYS-REQ-076 — UAV Capability Profile requires each connected UAV to have a machine-readable capability/limitation profile including payload capabilities and sensing capabilities, alongside flight, navigation, communication, energy, health and mission-load state.
- ARCH-027 — Heterogeneous UAV Fleet and Mission Coordination establishes that UAVs are heterogeneous execution resources with different capabilities, limitations, payloads and states; task allocation must consider capability match, current state, energy, range, time, payload, communication, airspace constraints, risks and task priority.
- Vehicle / Equipment Capability Model defines the controlled bridge from mission objective to required capabilities, fleet capability registry, capability matching and a concrete UAV + autopilot + equipment + C2 + battery configuration.
- Canonical Vehicle / Equipment Schema 001 defines Equipment and EquipmentProfile as the canonical domain objects. The schema explicitly states that it contains no separate Payload object; external payload terminology may be retained only as source/protocol metadata when needed for interoperability.
- Vehicle / Payload Integration Architecture defines payloads as independent capability objects associated with a vehicle configuration and requires capability matching before route optimization.
- Equipment Integration Specification defines versioned equipment profiles, aircraft compatibility, configuration/calibration state, capability checks and a pre-flight equipment compatibility gate.

### 50.2 Controlled allocation to MT-01 / MT-02

| Existing record / dependency | MT-01 | MT-02 | Allocation |
|---|---|---|---|
| SYS-REQ-035 — Task to Capability Mapping | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Task requirements must be translated into required payload/sensor capabilities before planning |
| SYS-REQ-076 — UAV Capability Profile | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Active UAV configuration must expose machine-readable payload/sensing capabilities and limits |
| ARCH-027 — heterogeneous capability matching | SUPPORTING / DIRECT for multi-UAV allocation | SUPPORTING / DIRECT for multi-UAV allocation | Capability-based allocation, not platform-ID matching |
| Equipment capability / compatibility state | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Candidate vehicle/equipment combination must be compatible before route/coverage optimization |
| Payload/sensor operating modes | DIRECT ALGORITHM INPUT where mission quality depends on them | DIRECT ALGORITHM INPUT | Sensor mode constrains acquisition geometry and quality evaluation |
| Calibration/configuration state | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Unvalidated or invalidated configuration cannot silently be treated as valid |
| Power / mass / equipment effects | DIRECT INPUT to performance/energy models | DIRECT INPUT | Active equipment configuration affects feasibility and energy/performance |
| Data-output / storage capability | DIRECT for missions requiring recorded products | DIRECT for missions requiring recorded products | Required mission product must be representable and recordable |
| Gimbal / pointing / stabilization capability | CONDITIONAL DIRECT INPUT | DIRECT INPUT where oblique/viewpoint acquisition is required | Planner must use actual supported pointing capability |
| Camera / sensor geometry | DIRECT INPUT | DIRECT INPUT | FOV, resolution/GSD-related parameters and acquisition constraints enter quality/trajectory calculation where applicable |

### 50.3 Canonical terminology boundary

The project currently has two legitimate contexts for the word payload:

1. Mission/integration terminology — payload means mission equipment such as RGB, thermal, multispectral, LiDAR, gimbal or delivery equipment.
2. Canonical domain schema — the normalized object is Equipment / EquipmentProfile, with capability sets and compatibility state.

Therefore MT-01/MT-02 algorithm contracts shall use the normalized capability/equipment representation rather than create a parallel Payload domain object.

Controlled mapping:

MISSION REQUIREMENT → REQUIRED CAPABILITIES → VEHICLE + EQUIPMENT CONFIGURATION → COMPATIBILITY / VERIFICATION → PLANNING INPUT

This is consistent with the existing capability architecture and avoids duplicate domain identity.

### 50.4 Hard capability gate

Payload/sensor compatibility is a pre-planning feasibility gate when the mission requires a mandatory capability.

The controlled sequence is:

TASK → REQUIRED CAPABILITIES → CANDIDATE VEHICLE/EQUIPMENT CONFIGURATION → CAPABILITY CHECK → CONFIGURATION / CALIBRATION CHECK → COMPATIBILITY CHECK → PLANNING CANDIDATE

A candidate is rejected before route optimization when a mandatory capability is absent, unsupported, incompatible, invalid or otherwise not available for the selected configuration.

Soft capability preferences may influence ranking among otherwise admissible configurations.

The optimizer must not compensate for a missing mandatory sensor capability by changing route geometry, energy objective or another unrelated parameter.

### 50.5 MT-01 consequences

For mapping, the payload/equipment layer can directly constrain:

- required GSD and image geometry;
- sensor footprint/FOV;
- frontal and side overlap feasibility;
- acquisition/trigger capability;
- camera orientation and stabilization;
- operating altitude/speed range where controlled by the equipment profile;
- recording/storage capability;
- configuration/calibration validity;
- mass/power effects used by vehicle performance and energy models.

The MT-01 quality gate therefore consumes the validated active equipment configuration, not a nominal camera catalogue entry.

No new threshold is introduced here. Existing algorithm parameters remain controlled through the parameter registry and must be sourced from the applicable payload profile, requirement, approved engineering data or controlled external source.

### 50.6 MT-02 consequences

For 3D reconstruction, the equipment layer additionally constrains:

- required observation geometry;
- FOV and sensor resolution;
- pointing/gimbal capability;
- oblique/side-looking acquisition where supported;
- required multi-view observation;
- acquisition timing;
- sensor-specific reconstruction quality;
- LiDAR swath, scan-angle and point-density parameters where the selected sensor model provides them.

A sensor that cannot provide the required observation mode is not made admissible by changing the viewpoint planner alone.

### 50.7 Configuration and invalidation rule

The existing integration architecture establishes that changes to:

- payload/equipment;
- battery;
- propulsion;
- firmware/autopilot;
- mass/centre-of-gravity;
- equipment installation;
- calibration/configuration;

may alter the performance model or mission feasibility and therefore require recalculation or validation.

For MT-01/MT-02 this maps into the dependency graph as:

EQUIPMENT / CONFIGURATION CHANGE → CAPABILITY VALIDATION → ACQUISITION GEOMETRY / PERFORMANCE → ENERGY → TRAJECTORY → QUALITY → FINAL VALIDATION

If the changed property cannot affect a downstream stage, the existing incremental-planning policy permits reuse of unaffected results. The dependency must be explicit; no blanket full recalculation is required.

### 50.8 Compatibility and verification state

The canonical schema distinguishes:

SUPPORTED ≠ VERIFIED

and:

COMPATIBLE ≠ AUTHORIZED

It also defines compatibility states:

UNKNOWN | COMPATIBLE | NOT_COMPATIBLE | NEEDS_REVIEW

and verification states:

NOT_VERIFIED | VERIFIED | EXPIRED | INVALIDATED

Accordingly, MT-01/MT-02 planning shall not infer operational readiness from capability existence alone.

The algorithm may consume a configuration only when the applicable capability, compatibility and verification conditions required by the mission are satisfied.

### 50.9 Candidate requirement family disposition

The existing candidate SRS requirement families concerning payload/task capability are not promoted to new authoritative IDs by this review.

The controlled requirement chain remains:

SYS-REQ-035 / SYS-REQ-076 → capability/equipment architecture → MT-01/MT-02 algorithm rule → verification case → evidence

Where a future lower-level payload/equipment data contract requires a distinct requirement, its identity must be established through the Master Requirements Register rather than invented in the algorithm document.

### 50.10 Numerical / data-contract boundary

The repository establishes the existence of payload/equipment parameters but does not provide a complete authoritative numerical set for all mission classes.

Therefore this section deliberately does not establish:

- universal camera resolution thresholds;
- universal GSD limits;
- universal overlap limits;
- sensor-specific altitude limits;
- universal minimum/maximum operating speed;
- universal gimbal accuracy;
- universal LiDAR point-density threshold;
- universal storage/data-rate threshold;
- universal power-consumption value;
- universal payload mass limit.

Such values remain controlled parameters whose source must be the active equipment profile, mission requirement/quality profile, validated engineering model or controlled external source.

### 50.11 Gate status after Section 50

| Gate | Status |
|---|---|
| Existing task-to-capability allocation (SYS-REQ-035) | RESOLVED |
| Existing UAV capability-profile allocation (SYS-REQ-076) | RESOLVED |
| Existing heterogeneous capability architecture (ARCH-027) | RESOLVED |
| Canonical equipment/payload terminology boundary | RESOLVED |
| Compatibility as pre-planning feasibility gate | RESOLVED |
| Configuration/calibration invalidation relationship | RESOLVED at architecture level |
| Exact lower-level payload/equipment data-contract requirement IDs | OPEN |
| Equipment-specific numerical parameters | OPEN |
| Regulatory/source-clause mapping for equipment performance | OPEN |
| Executable verification/evidence for MT-01/MT-02 | NOT DONE |

MT-03 remains blocked.

The next deterministic allocation class is readiness and contingency lower-level interfaces, using existing SYS-REQ-008, SYS-REQ-081/082/086/093 and the established Safety Gate/authority architecture before considering any new requirement identity.

## 51. Readiness / Contingency Lower-Level Interfaces — Controlled Resolution

Section 50 closed the payload/sensor allocation gate. The next deterministic review is the lower-level relationship between MT-01/MT-02 planning, Mission Readiness, contingency preparation, Safety Gate and runtime recovery. The repository contains sufficient existing authoritative material to allocate the system-level dependency without creating a parallel readiness or contingency requirement set.

### 51.1 Existing authoritative basis

The controlled records establish:

- SYS-REQ-008 — Mission Readiness: readiness is a consolidated pre-approval assessment based on validation and current conditions. It explicitly includes Airspace, Terrain, Geofence, Weather, C2 Coverage, Fleet Coordination, Energy and Contingency.
- SYS-REQ-081 — UAV Failure Tolerance: loss of an individual UAV must not automatically destroy the mission when remaining resources can continue or safely complete it; critical tasks may be transferred to a reserve executor where defined by the mission profile.
- SYS-REQ-082 — Safe Mission Completion: when full completion becomes impossible, the system must provide safe completion based on UAV state, task criticality, resources, energy, communication and risks. Return, validated landing, emergency behaviour, task transfer and result preservation are possible paths subject to UAV capability and Safety Engine constraints.
- SYS-REQ-086 — Graceful Degradation: resource shortage shall cause controlled degradation by priority; P0/P1 remain protected and P2 is retained to the extent required for safe mission execution.
- SYS-REQ-093 — Controlled Resource Recovery: after overload removal, restricted services recover in a controlled order without secondary overload; degradation/recovery state is recorded.
- ARCH-DEC-007 — Mission Validation and Safety Gate establishes the mandatory path MISSION PLAN → VALIDATION → READINESS → SAFETY GATE → APPROVAL → EXECUTION and states that unresolved contingency, energy, C2 or other critical conditions can block execution.
- ARCH-DEC-016 — Safety Architecture and Execution Gate makes the Safety Gate authoritative for safety-critical execution and explicitly includes contingency, energy, communication, fleet and emergency conditions.
- ARCH-DEC-017 — Error Handling / Recovery / Contingency Architecture defines DETECT → CLASSIFY → ASSESS → RESPOND → REVALIDATE → RECOVER / ADAPT / REPLAN / ABORT, including contingency activation, emergency return/landing, resource replacement, UAV/payload reassignment and Mission Readiness recalculation.
- ARCH-DEC-038 — Mission Execution Orchestration requires current readiness, configuration, resources, capabilities, communication and safety state before execution and requires revalidation after material runtime changes.
- AUTHORIZATION_READINESS_GATE_001 already provides a lower-level deterministic authorization/readiness contract and explicitly states that it does not replace spatial, safety, weather, insurance, technical or other readiness gates.

### 51.2 Controlled allocation to MT-01 / MT-02

| Existing record / dependency | MT-01 | MT-02 | Allocation |
|---|---|---|---|
| SYS-REQ-008 — Mission Readiness | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Planning result must provide the inputs required for consolidated readiness |
| SYS-REQ-081 — UAV Failure Tolerance | SUPPORTING / DIRECT for multi-UAV | SUPPORTING / DIRECT for multi-UAV | Candidate plan must expose task/resource relationships needed for failure impact and possible reassignment |
| SYS-REQ-082 — Safe Mission Completion | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Plan must support safe-completion feasibility inputs; execution authority remains outside planner |
| SYS-REQ-086 — Graceful Degradation | SUPPORTING / CROSS-CUTTING | SUPPORTING / CROSS-CUTTING | Planning/resource decisions must respect priority-based degradation and protected safety functions |
| SYS-REQ-093 — Controlled Resource Recovery | SUPPORTING / CROSS-CUTTING | SUPPORTING / CROSS-CUTTING | Planner consumes current resource/readiness state; recovery authority remains in runtime architecture |
| Safety Gate / Mission Validation | DIRECT GATE | DIRECT GATE | No selected plan becomes executable without validation/readiness/safety/approval path |
| Contingency alternatives | DIRECT PLANNING INPUT/OUTPUT where applicable | DIRECT PLANNING INPUT/OUTPUT where applicable | Planner may prepare alternate routes, landing locations, reserve allocations or fallback mission variants |
| Runtime contingency activation | NOT PLANNER AUTHORITY | NOT PLANNER AUTHORITY | Activation is owned by Safety/Execution architecture |
| Revalidation after material change | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Changed environment/resource/mission state invalidates affected planning/readiness results |
| Authorization readiness | DIRECT DEPENDENCY | DIRECT DEPENDENCY | Valid authorization state is an input to the constrained planning domain where applicable |

### 51.3 Readiness is an input/output boundary, not a planning authority

The controlled architecture establishes a strict separation:

PLANNING → VALIDATION → READINESS → SAFETY GATE → APPROVAL → EXECUTION

Therefore MT-01/MT-02 may calculate feasibility and prepare the information required by readiness, but they shall not set the authoritative mission to READY, approve execution or bypass the Safety Gate.

The planner may report a plan-level feasibility result such as feasible, infeasible, blocked, degraded or contingency-required, but the authoritative system readiness state remains owned by the existing Mission Readiness architecture.

This preserves the distinction already established by SYS-REQ-008 and ARCH-DEC-007.

### 51.4 Contingency planning boundary

The planning layer may prepare contingency candidates when required by the mission profile. Existing architecture explicitly permits reserve UAVs, reserve energy, alternate routes, alternate landing locations, communication relay alternatives, payload alternatives, fallback mission objectives and emergency procedures.

For MT-01/MT-02, these are planning artefacts or feasibility inputs. Their activation is not owned by the planner.

Controlled boundary:

CONTINGENCY CANDIDATE → VALIDATION → READINESS → SAFETY GATE → APPROVAL WHERE REQUIRED → EXECUTION

The planner shall never convert a contingency candidate directly into an execution command.

### 51.5 Failure and resource-loss consequences

For MT-01/MT-02, the dependency graph shall distinguish at least:

UAV LOSS → affected task/candidate identification → capability/resource reassessment → possible reassignment/replanning → validation → readiness → safety gate

ENERGY DEGRADATION → affected trajectory/energy feasibility → candidate invalidation or recovery alternative → validation → readiness → safety gate

C2 DEGRADATION → communication-dependent feasibility assessment → affected candidate invalidation/adaptation → validation → readiness → safety gate

PAYLOAD/CAPABILITY LOSS → affected acquisition/task quality → candidate invalidation or task reassignment → validation → readiness → safety gate

These are dependency rules, not new execution authorities.

### 51.6 Safe completion inputs

SYS-REQ-082 and ARCH-DEC-017 establish that safe completion depends on current UAV state, task criticality, available resources, energy, communication, navigation, environmental/spatial conditions, UAV-specific return/landing/emergency capabilities and Safety Engine decisions.

MT-01/MT-02 shall therefore expose sufficient planning state to evaluate return/recovery feasibility where applicable, but shall not hard-code universal emergency behaviour into the mapping/reconstruction planners.

UAV-specific emergency procedures remain capability/configuration controlled.

### 51.7 Graceful degradation and priority

SYS-REQ-086 establishes priority-based degradation, with P0/P1 protected and P2 retained as required for safe mission execution.

Accordingly, an optimization candidate shall not trade away mandatory safety/C2/navigation capability merely to preserve lower-priority mission quality, time or resource objectives.

Where degradation removes a mission capability, the planning layer may reduce mission scope, reassign tasks, select a lower-complexity candidate, generate a fallback plan or declare the requested mission infeasible; the resulting state must pass the existing validation/readiness/safety path.

### 51.8 Controlled recovery and incremental planning

SYS-REQ-093 governs controlled restoration of restricted services after overload. It does not grant the planner authority to restore runtime services.

For planning purposes:

RESOURCE STATE CHANGE → DEPENDENCY GRAPH → INVALIDATE AFFECTED RESULTS → RECOMPUTE MINIMUM REQUIRED STAGES

Unchanged planning results may be retained when their dependency set remains valid. Recovery of a runtime service is separately governed by the runtime architecture.

### 51.9 Lower-level readiness records

The repository already contains lower-level readiness mechanisms, including the authorization readiness gate and dynamic readiness-action graph. These should be treated as implementation/decomposition artefacts beneath the existing system-level readiness requirement, not as independent replacement requirements.

No new readiness requirement ID is introduced by this section.

The same rule applies to contingency/recovery implementation artefacts: existing architecture and software contracts are lower-level realizations of the authoritative system requirements.

### 51.10 Numerical / authority boundary

This review deliberately does not establish universal readiness thresholds, universal contingency trigger thresholds, universal return-energy thresholds, universal C2 degradation limits, universal recovery timing, universal emergency landing criteria, universal reserve UAV rules or universal degradation percentages.

Those values remain controlled by the applicable requirement, safety policy, UAV configuration, mission profile, validated engineering model or authoritative external source.

### 51.11 Candidate requirement family disposition

Candidate readiness, return/recovery, authorization and degradation families are not promoted to independent authoritative IDs by this review.

The controlled chain remains:

SYS-REQ-008 / SYS-REQ-081 / SYS-REQ-082 / SYS-REQ-086 / SYS-REQ-093 → ARCH-DEC-007 / 016 / 017 / 038 → MT-01 / MT-02 planning rules → verification → evidence

Where a lower-level requirement is genuinely missing, its identity must be established through the Master Requirements Register after controlled gap analysis.

### 51.12 Gate status after Section 51

| Gate | Status |
|---|---|
| Mission Readiness system-level allocation | RESOLVED |
| Safe-completion allocation | RESOLVED |
| UAV failure / reassignment allocation | RESOLVED |
| Graceful degradation allocation | RESOLVED |
| Controlled resource recovery allocation | RESOLVED |
| Safety Gate / authority boundary | RESOLVED |
| Contingency planning boundary | RESOLVED |
| Lower-level readiness implementation relationship | RESOLVED at architecture level |
| Exact lower-level readiness/contingency requirement IDs | OPEN |
| Numerical readiness/contingency parameters | OPEN |
| Regulatory/safety clause mapping for quantitative triggers | OPEN |
| Executable verification/evidence for MT-01/MT-02 | NOT DONE |

MT-03 remains blocked.

The next deterministic allocation class is the verification/evidence linkage for the resolved MT-01/MT-02 dependencies, beginning with existing verification identities and controlled datasets rather than creating new verification IDs.


## 52. MT-01 / MT-02 Verification and Evidence Linkage — Controlled Resolution

Section 51 identified verification/evidence linkage as the next deterministic gate. The repository was first reconciled against the existing Verification Register and verification tree.

### 52.1 Existing verification identity reconciliation

The current branch contains controlled verification identities for Navigation (`NAV-V01..NAV-V20`, `NAV-TV-001..010`) and C2 (`C2-V01..V08`), but no existing `V-M01-*` or `V-M02-*` identity family.

Therefore:

- existing NAV/C2 identities are retained for their existing scopes;
- they are not silently reused as MT planning cases;
- no duplicate legacy MT identity exists to link;
- a real MT-specific verification decomposition gap was confirmed.

### 52.2 Controlled MT verification identities

The confirmed gap is now closed at the **identity/decomposition level** by the controlled case definition:

`05_VERIFICATION/PLANNING/MT01_MT02_VERIFICATION_CASES_001.md`

Controlled identities:

```
MT-01: V-M01-01 … V-M01-10
MT-02: V-M02-01 … V-M02-11
```

The cases are derived directly from the already-defined MT-01/MT-02 verification scenarios and algorithm stages. They do not introduce new requirements.

### 52.3 Controlled test datasets

The case-to-dataset definitions are controlled in:

`05_VERIFICATION/PLANNING/MT01_MT02_TEST_DATASETS_001.md`

Dataset families:

```
MT-01: MT01-T01 … MT01-T09
MT-02: MT02-T21 … MT02-T31
```

The dataset definitions establish scenario intent and required provenance fields. They do not claim that the actual datasets have already been captured.

### 52.4 Requirement / verification linkage

The current controlled requirement allocation established in Sections 44–51 provides the requirement/design basis for the first verification pass.

The principal allocation is:

| Planning dependency | MT-01 / MT-02 verification consequence |
|---|---|
| SYS-REQ-035 / SYS-REQ-076 | capability/equipment feasibility cases |
| SYS-REQ-080 / 081 / 082 / 084 / 085 / 086 / 091 / 093 | resource, safety, navigation, degradation and safe-completion consequences |
| SYS-REQ-110 / 111 / 112 | AI-assisted planning authority/offline continuity cases where AI is active |
| SYS-REQ-008 | readiness-input and blocking-state consequences |
| SYS-REQ-067 / SYS-REQ-068 | external airspace/weather data interface dependencies |
| ARCH-DEC-007 / 016 / 017 / 038 | validation, Safety Gate, contingency and runtime revalidation boundary |
| Navigation State / Algorithm / Verification records | navigation quality, freshness, reference-frame and degradation inputs |

This is a **basis allocation**, not a claim that every case has a one-to-one requirement closure. Lower-level candidate requirement families remain governed by the Master Requirements Register.

### 52.5 Existing verification reuse rule

Existing verification identities shall be linked only where their controlled scope actually covers the MT algorithm objective.

Examples:

- `NAV-V05` may support the wind-change/navigation dependency but does not replace `V-M01-06` or `V-M02-08`.
- `NAV-V08/V09/V12` may support stale/invalid/degraded navigation inputs but do not replace the complete MT planning cases.
- `NAV-V19` may support multi-UAV navigation behavior but does not replace `V-M02-10`.
- C2 cases remain C2-specific and are not relabeled as MT cases.

This prevents verification coverage inflation by semantic overlap.

### 52.6 Evidence state

Current state after controlled decomposition:

| Layer | State |
|---|---|
| Requirement/design basis | CONTROLLED / PARTIAL by allocation class |
| Verification identity | CONTROLLED |
| Verification definition | DEFINED |
| Dataset identity | CONTROLLED |
| Actual dataset package | OPEN |
| Execution configuration | OPEN |
| Execution result | NOT EXECUTED |
| Evidence | OPEN |
| Certification closure | OPEN |

A case cannot become `PASSED` or `VERIFIED` from documentation alone.

### 52.7 Next deterministic verification action

The next step is **not** to create more requirement IDs or more verification IDs.

It is to bind each `V-M01-*` / `V-M02-*` case to:

1. the exact existing authoritative requirement/design basis;
2. the exact dataset version;
3. the execution configuration;
4. objective acceptance criteria from the already-controlled algorithm/parameter registry;
5. the resulting evidence record.

Where a requirement or acceptance criterion is still OPEN, the case remains OPEN rather than receiving an invented value.

### 52.8 Gate after Section 52

| Gate | Status |
|---|---|
| Existing verification identity search | RESOLVED |
| MT-01 verification identity/decomposition | RESOLVED |
| MT-02 verification identity/decomposition | RESOLVED |
| Dataset identity/decomposition | RESOLVED |
| Requirement linkage | PARTIAL / CONTROLLED |
| Exact case-level acceptance criteria | OPEN where controlled parameter is missing |
| Execution configuration | OPEN |
| Actual execution | NOT DONE |
| Evidence | NOT DONE |
| Deterministic replay execution | NOT DONE |
| Certification closure | NOT DONE |

**MT-03 remains BLOCKED.**

The next deterministic work item is case-level requirement/dataset/configuration binding, beginning with `V-M01-01` and `V-M02-01`, without creating additional identities unless a separately proven coverage gap appears.


## 53. MT-01 Transition Graph — First Implementation Slice

The first MT-01 transition-graph implementation is controlled as an isolated planning component:

- input: generated coverage tracks, constrained-environment snapshot, calculation version;
- transition: directed connector from the end of one track to the start of another track;
- admissibility: connector must pass the existing constrained-segment validator at the source track altitude;
- cost policy for this slice: `DISTANCE_ONLY`, therefore `cost_m = distance_m`;
- deterministic dependency identity is retained;
- invalid track input fails closed;
- rejected connectors are counted explicitly.

No unvalidated weights are introduced for turn cost, wind, energy, risk, vehicle dynamics or alternative connector geometry.

The next deterministic MT-01 implementation step is **route candidate refinement/integration**: additional admissible orderings, then wind/vehicle/energy/trajectory evaluation. Full transition-cost refinement remains downstream of the first candidate-generation slice.


### 53.1 Bounded route-candidate refinement

The route-candidate generator now evaluates a bounded deterministic family rather than only one fixed start track:

1. each generated track is considered as a possible starting track;
2. for each start, the same distance-only greedy expansion is applied;
3. incomplete routes are rejected;
4. complete candidates are sorted by total transition cost;
5. equal-cost candidates are ordered lexicographically by their track-ID sequence;
6. `max_candidates` bounds the retained candidate set.

This remains a **candidate-generation** stage. It does not claim global optimality and does not yet include wind, vehicle performance, energy, turn dynamics, trajectory feasibility or risk costs.


### 53.2 Route-candidate → wind/performance integration boundary

Route candidates are now convertible into the existing `Route` model and evaluated through the established `WindPerformanceTrajectory` component.

The integration layer:

- preserves candidate track order and transition geometry;
- creates explicit route segment identities;
- carries candidate lineage into the generated route;
- invokes the existing wind/performance/energy evaluator;
- retains its fail-closed findings, including missing wind, invalid wind, wind tolerance, unavailable ground speed and insufficient reserve.

The integration layer does **not** duplicate wind or energy calculations and does not invent environmental or UAV parameters.

A candidate with missing required wind samples remains `INFEASIBLE` through the existing evaluator; this is an expected controlled outcome, not execution evidence.
