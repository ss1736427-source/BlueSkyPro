---
id: SYS-REQ-065
type: system_requirement
title: HUB Degraded Operation
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-025]]"
---

# SYS-REQ-065 — HUB Degraded Operation

## Requirement

ри перегрузке, отказе вычислительного ресурса или потере внешнего источника HUB должен обеспечивать деградационный режим с сохранением функций более высокого приоритета.

опускается:
- остановка фоновой аналитики;
- ограничение self-learning;
- снижение некритичной визуализации;
- ограничение некритичного обмена;
- остановка второстепенных сервисов.

е допускается потеря критических функций ради выполнения фоновых задач.

## Traceability
- [[ARCH-025]]



