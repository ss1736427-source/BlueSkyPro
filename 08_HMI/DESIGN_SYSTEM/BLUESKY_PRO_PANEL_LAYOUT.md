---
id: HMI-PANEL-LAYOUT-001
type: hmi_layout
status: draft_for_agreement
system: BlueSky PRO
---

# BlueSky PRO — Panel Layout and Ergonomics

## 1. Goal

Создать полноценный рабочий вид без визуальной перегрузки. Текущая трассировка является starting point, а не фиксированной компоновкой.

## 2. Panel classes

### Persistent

- Flight Chart / карта;
- ключевой mission state;
- critical warnings;
- C2/link state;
- current UAV state.

### Compact persistent

- GNSS;
- Battery;
- Data/Video link;
- weather summary;
- ETA/ETD и ключевые flight values.

### Context / expandable

- Mission Analysis;
- Instruments;
- ATC details;
- extended diagnostics;
- detailed parameters.

### Secondary

- расширенные настройки;
- редкие операции;
- исторические/архивные данные.

## 3. Layout flexibility

Панели должны быть независимыми компонентами. Поддерживаются layout-варианты:

- Desktop — широкая центральная Flight Chart;
- Tablet — уменьшенная постоянная периферия и context overlays;
- Expert — расширенный набор параметров;
- Pilot — только данные и действия, необходимые пилоту.

## 4. Map priority

Flight Chart является главным рабочим пространством. Operational layer карты включает маршрут, БПЛА, waypoint, границы, зоны, NOTAM, weather и corrections.

Поддерживаемые действия карты из существующей наработки:

- zoom;
- pan;
- rotate;
- tilt;
- select;
- draw polygon;
- edit route;
- edit altitude.

## 5. Map states

`Planning` → `Validating` → `Ready` → `Flying` → `Paused` → `Emergency` → `Completed`.

## 6. UAV panels

Перечень отображаемых параметров должен быть настраиваемым. Система должна эргономично размещать выбранную информацию и адаптировать компоновку при изменении набора параметров.

Функция «Применить для всех» применяется только к панелям БВС. Настройки верхней, нижней, левой, правой панели и карты не объединяются этой функцией.

Настройки представления сохраняются в текущей конфигурации интерфейса и не изменяют исходные данные миссии, маршрут или состояние БВС.

## 7. Design rule

Не удалять данные из системы ради упрощения HMI. Упрощать представление: hide, collapse, context panel, drill-down.

## Status

DRAFT_FOR_AGREEMENT.
