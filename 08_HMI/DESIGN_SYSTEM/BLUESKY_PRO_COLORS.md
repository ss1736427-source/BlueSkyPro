---
id: HMI-COLOR-SYSTEM-001
type: hmi_visual_design
status: draft_for_agreement
system: BlueSky PRO
---

# BlueSky PRO — Color System

## Source

Рабочая палитра взята из существующего проектного Concept Document. Значения не переопределяются в процессе трассировки без отдельного решения.

| Назначение | HEX |
|---|---|
| Основной фон | `#050A12` |
| Панели | `#08111D` |
| Карточки | `#0C1725` |
| Selected / Hover | `#111F30` |
| BlueSky Blue | `#168BFF` |
| Cyan | `#35B9FF` |
| Normal | `#39D98A` |
| Warning | `#FFC857` |
| Critical | `#FF5C6C` |

## Пропорция

Ориентир визуального распределения: около 90% тёмные нейтральные поверхности, 8% blue/cyan, 2% status colors.

## Правила

- Основные поверхности не должны конкурировать с operational overlays.
- Blue/Cyan используются для навигации, активных элементов, маршрута и выделения.
- Green — штатное/готовое состояние.
- Yellow — предупреждение/внимание.
- Red — critical / опасное состояние.
- Цвета operational layer карты должны быть различимы на выбранной base map.

## Map operational layer

Визуально отдельно контролируются:

- маршрут;
- БПЛА;
- waypoint;
- границы;
- зоны;
- NOTAM;
- weather;
- corrections.

Для route/WP предусмотрены настройки цвета, толщины и прозрачности.

## Status

DRAFT_FOR_AGREEMENT. Цвета являются текущей проектной baseline и могут быть подтверждены пользователем отдельно.
