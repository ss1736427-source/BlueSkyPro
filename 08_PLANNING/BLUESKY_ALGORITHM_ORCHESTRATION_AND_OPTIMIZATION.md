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
## 38. MT-01 / MT-02 Requirement Traceability — Current Baseline Allocation

The current requirement baseline and traceability master use the identifiers below. This section allocates only requirements directly supported by the present MT-01/MT-02 contracts. It does not invent or renumber requirements.

| Requirement | MT-01 | MT-02 | Allocation |
|---|---|---|---|
| SYS-003 | ✓ | ✓ | Direct: planning core remains independent of vendor protocols |
| SYS-004 | ✓ | ✓ | Direct: planner consumes capability-driven UAV/payload models |
| SYS-005 | ✓ | ✓ | Direct: provenance, versioning, candidate evidence and replay contract |
| SYS-VP-001 | ✓ | ✓ | Direct dependency: versioned UAV profile |
| SYS-VP-002 | ✓ | ✓ | Direct dependency: versioned payload profile |
| SYS-VP-004 | ✓ | ✓ | Direct hard gate: UAV/payload compatibility |
| EXT-AIR-001 | ✓ | ✓ | Direct hard dependency: airspace/restriction domain |
| EXT-AIR-002 | ✓ | ✓ | Direct hard dependency where applicable: NOTAM/aeronautical data |
| EXT-WX-001 | ✓ | ✓ | Direct dependency: wind snapshot and freshness/quality |
| EXT-GIS-001 | ✓ | ✓ | Direct dependency: map/DEM/terrain |
| MIS-001 | ✓ | ✓ | Direct: canonical mission representation |
| MIS-002 | ✓ | ✓ | Direct: mission-template planning contract |
| MIS-003 | ✓ | ✓ | Direct: feasibility against vehicle/equipment |
| MIS-004 | ✓ | ✓ | Direct: wind/energy/aerodynamic optimization |
| MIS-005 | conditional | ✓ | Multi-UAV extension defined; group allocation remains a shared service |
| AI-001 | conditional | conditional | AI may assist strategy/parameter selection but is not authoritative |
| AI-002 | ✓ | ✓ | Direct: Safety Gate remains outside AI |
| AI-004 | ✓ | ✓ | Direct: explainability/provenance output |
| RDY-001 | downstream | downstream | Planning contributes feasibility evidence; readiness aggregation is external |
| RDY-002 | downstream | downstream | Planning contributes blocking reasons; readiness state is external |
| CNT-004 | downstream | downstream | Energy engine supplies prediction/gate inputs; contingency execution is external |
| VAL-001 | ✓ | ✓ | Direct: unit/integration verification target |
| VAL-002 | ✓ | ✓ | Direct: simulation datasets are defined |
| VAL-006 | ✓ | ✓ | Direct: deterministic dependency/invalidation model supports impact analysis |

### 38.1. Allocation rules

**Direct** means the requirement creates a rule or interface explicitly implemented by the template planning pipeline.

**Conditional** means the template consumes or exposes the capability, while the authoritative implementation belongs to a shared service or another lifecycle stage.

**Downstream** means the template provides planning evidence, but does not own the requirement's final lifecycle state.

**OPEN ALLOCATION** shall be used whenever the existing requirement set is too broad to determine an exact template-level allocation without architectural invention.

### 38.2. Traceability closure status

The requirement chain is now structurally defined:

Requirement → MT-01 / MT-02 algorithm rule → logical implementation module → verification dataset/scenario → evidence package.

Requirement closure is not claimed. The current master baseline contains requirements with IN PROGRESS and NOT DONE implementation/evidence status. Therefore:

- **SPECIFICATION TRACEABILITY:** defined;
- **IMPLEMENTATION TRACEABILITY:** target modules defined;
- **VERIFICATION TRACEABILITY:** test scenarios defined;
- **EVIDENCE CLOSURE:** not yet achieved.

This distinction is mandatory for certification integrity.

## 39. Final Gate Before MT-03

MT-03 remains blocked until the following review is completed for both MT-01 and MT-02:

1. algorithm contract review;
2. data-contract review;
3. numerical-policy review;
4. implementation-module review;
5. requirement allocation review;
6. controlled test-data review;
7. deterministic replay design review;
8. explicit acceptance of all remaining OPEN ALLOCATION items.

After that review, the next phase is implementation/test preparation for MT-01 and MT-02, not expansion of the template family by default.
