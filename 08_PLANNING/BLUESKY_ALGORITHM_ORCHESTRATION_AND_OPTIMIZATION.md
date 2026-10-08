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
