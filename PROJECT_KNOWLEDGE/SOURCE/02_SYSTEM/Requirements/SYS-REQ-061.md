---
id: SYS-REQ-061
type: system_requirement
title: HUB Automatic Failover
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-025]]"
---

# SYS-REQ-061 — HUB Automatic Failover

## Requirement

Система должна автоматически обнаруживать подтверждённый отказ активного HUB и выполнять переключение на резервный экземпляр.

ереключение не должно требовать ручного переподключения функциональных модулей.

осле переключения система должна продолжить работу с сохранением критического состояния.

## Traceability
- [[ARCH-025]]



