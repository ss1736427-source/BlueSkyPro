---
id: SYS-REQ-086
type: system_requirement
title: Graceful Degradation
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-028]]"
---

# SYS-REQ-086 — Graceful Degradation

## Requirement

ри недостатке ресурсов HUB должен обеспечивать управляемое снижение функциональности по приоритетам.

 первую очередь должны ограничиваться P5 и P4.

ри необходимости может ограничиваться P3.

P2 должен сохраняться в объёме, необходимом для безопасного выполнения миссии.

P0 и P1 должны сохраняться в требуемом объёме независимо от нагрузки низкоприоритетных функций.

## Traceability
- [[ARCH-DEC-039]]
- [[ARCH-DEC-023]]
- [[ARCH-DEC-017]]
- [[ARCH-028]]



