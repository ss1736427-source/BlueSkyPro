---
id: SW-REQ-001
type: software_requirement
title: Планирование маршрута полета
status: draft
criticality: high
derives_from:
allocated_to:
implements:
verification_method:
verified_by:
baseline:
---
---
---
---
id: SW-REQ-001
type: software_requirement
title: Планирование маршрута полета
status: draft
criticality: high
derives_from:
allocated_to:
implements:
verification_method: test
verified_by:
baseline:
---

# SW-REQ-001 — Планирование маршрута полета

## 1. Требование

Система должна обеспечивать создание маршрута полета БПЛА
с заданной последовательностью путевых точек и параметрами маршрута.

## 2. Назначение

Обеспечить формирование проверяемого плана полета.

## 3. Входные данные

- координаты путевых точек;
- высоты;
- параметры маршрута;
- ограничения маршрута.

## 4. Выходные данные

- сформированный маршрут;
- последовательность путевых точек;
- параметры маршрута.

## 5. Ограничения

Маршрут должен соответствовать установленным системным
и эксплуатационным ограничениям.

## 6. Связи

### Родительское требование

derives_from:

[[SYS-REQ-001]]

### Архитектурный элемент

allocated_to:

[[ARCH-001]]

### Реализация

implements:

[[DES-001]]

### Верификация

verification_method:

test

verified_by:

[[VER-001]]

### Конфигурация

baseline:

[[BASELINE-001]]


