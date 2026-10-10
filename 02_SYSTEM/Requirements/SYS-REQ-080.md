---
id: SYS-REQ-080
type: system_requirement
title: Dynamic Task Reallocation
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-027]]"
---

# SYS-REQ-080 — Dynamic Task Reallocation

## Requirement

ри изменении состояния группы  система должна иметь возможность динамически перераспределять незавершённые задачи между доступными исполнителями.

ерераспределение должно учитывать capability, состояние, энергию, связь, риски и приоритет задачи.

ри потере одного  система должна определить, какие задачи могут быть переданы другим .

## Traceability
- [[ARCH-DEC-008]]
- [[ARCH-DEC-039]]
- [[ARCH-DEC-012]]
- [[ARCH-027]]



