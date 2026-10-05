---
id: SYS-REQ-062
type: system_requirement
title: HUB State Synchronization
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-025]]"
---

# SYS-REQ-062 — HUB State Synchronization

## Requirement

езервный HUB должен поддерживать синхронизированное состояние критических функций.

Синхронизации подлежат как минимум:
- состояние миссии;
- состояние ;
- состояние флота;
- нормативное состояние;
- критические события;
- конфигурация;
- критические очереди сообщений.

осле takeover резервный экземпляр должен продолжить работу с актуальным состоянием в пределах установленного RPO.

## Traceability
- [[ARCH-025]]



