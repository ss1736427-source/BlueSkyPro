---
id: SYS-REQ-071
type: system_requirement
title: Message Priority
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-026]]"
---

# SYS-REQ-071 — Message Priority

## Requirement

ежмодульные сообщения должны иметь определённый приоритет.

инимальная шкала:
- P0 — Safety Critical
- P1 — Flight / Navigation / C2
- P2 — Mission Execution
- P3 — Situational Awareness
- P4 — AI / Analysis
- P5 — Background

ри перегрузке коммуникационного или вычислительного контура сообщения более низкого приоритета должны ограничиваться раньше сообщений более высокого приоритета.

P5 не должен конкурировать за критические ресурсы с P0.

## Traceability
- [[ARCH-026]]



