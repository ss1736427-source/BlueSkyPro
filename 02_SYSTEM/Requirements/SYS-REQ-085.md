---
id: SYS-REQ-085
type: system_requirement
title: Safety-Critical Priority
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-028]]"
---

# SYS-REQ-085 — Safety-Critical Priority

## Requirement

ри конкуренции за ресурсы функции P0 и P1 должны иметь приоритет над P2–P5.

 P0/P1 относятся как минимум safety, flight control, navigation и C2.

изкоприоритетные функции не должны препятствовать выполнению P0/P1.

## Traceability
- [[ARCH-DEC-036]]
- [[ARCH-DEC-016]]
- [[ARCH-DEC-007]]
- [[ARCH-028]]



