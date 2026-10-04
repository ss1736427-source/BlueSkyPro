---
id: SYS-REQ-064
type: system_requirement
title: HUB Computational Priority
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-025]]"
---

# SYS-REQ-064 — HUB Computational Priority

## Requirement

HUB должен распределять вычислительные ресурсы по приоритету критичности функций.

риоритеты:
- P0 — Safety Critical;
- P1 — Flight / Navigation / C2;
- P2 — Mission Execution;
- P3 — Situational Awareness;
- P4 — AI / Analysis;
- P5 — Background / Self-Learning.

ри дефиците ресурсов функции более низкого приоритета должны деградировать раньше функций более высокого приоритета.

Функции P5 не должны конкурировать с P0 за критические вычислительные ресурсы.

## Traceability
- [[ARCH-025]]



