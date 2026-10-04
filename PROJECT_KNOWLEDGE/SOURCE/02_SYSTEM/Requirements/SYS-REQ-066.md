---
id: SYS-REQ-066
type: system_requirement
title: HUB Module Independence
status: draft
criticality: high
verification_method: inspection
derives_from:
  - "[[ARCH-025]]"
---

# SYS-REQ-066 — HUB Module Independence

## Requirement

Функциональные модули должны взаимодействовать с HUB через определённые интерфейсы и сервисные контракты.

амена или модернизация отдельного модуля не должна требовать изменения HUB Core при сохранении совместимого интерфейса.

Физический экземпляр HUB не должен быть частью интерфейсного контракта прикладного модуля.

одуль должен обращаться к логическому HUB Service, а не непосредственно к HUB-A или HUB-B.

## Traceability
- [[ARCH-025]]



