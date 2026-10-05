---
id: SYS-REQ-081
type: system_requirement
title: UAV Failure Tolerance
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-027]]"
---

# SYS-REQ-081 — UAV Failure Tolerance

## Requirement

тказ или потеря отдельного  не должны автоматически приводить к потере всей миссии, если оставшиеся ресурсы способны выполнить миссию или обеспечить её безопасное завершение.

Система должна определить влияние отказа на текущие задачи и состояние миссии.

ритические задачи должны иметь возможность передачи резервному исполнителю, если такой исполнитель предусмотрен mission profile.

## Traceability
- [[ARCH-DEC-040]]
- [[ARCH-DEC-027]]
- [[ARCH-DEC-017]]
- [[ARCH-027]]



