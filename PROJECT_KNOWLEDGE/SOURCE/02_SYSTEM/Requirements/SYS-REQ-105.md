---
id: SYS-REQ-105
type: system_requirement
title: Annual Upgrade Package
status: draft
criticality: high
verification_method: analysis
derives_from:
  - "[[ARCH-029]]"
---

# SYS-REQ-105 — Annual Upgrade Package

## Requirement

Система должна поддерживать формирование периодического, включая годовой, upgrade package на основе накопленного опыта.

ри формировании upgrade package должны анализироваться как минимум:
- накопленные улучшения
- неудачные улучшения
- rollback
- повторяющиеся проблемы
- performance trends
- resource trends
- fleet trends
- operator feedback
- reliability trends
- выявленные новые возможности

аждое предлагаемое изменение в upgrade package должно иметь обоснование и результаты соответствующей проверки.

Upgrade package должен поддерживать версионирование и возможность установить состав включённых изменений.

## Traceability
- [[ARCH-029]]



