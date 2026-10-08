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
