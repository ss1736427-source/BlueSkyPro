---
id: SYS-REQ-084
type: system_requirement
title: Resource Reservation
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-028]]"
---

# SYS-REQ-084 — Resource Reservation

## Requirement

HUB должен резервировать минимально необходимые вычислительные, коммуникационные и системные ресурсы для P0 и P1 функций.

арезервированные ресурсы не должны быть доступны для произвольного использования функциями более низкого приоритета.

езерв должен учитывать текущий operational profile и состояние системы.

## Traceability
- [[ARCH-DEC-037]]
- [[ARCH-DEC-036]]
- [[ARCH-028]]



