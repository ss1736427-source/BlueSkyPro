---
id: SYS-REQ-107
type: system_requirement
title: Rollback Applicability and Acceptance
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-029]]"
---

# SYS-REQ-107 — Rollback Applicability and Acceptance

## Requirement

ля каждого значимого изменения должна быть определена политика rollback.

сли rollback применим, должны быть определены:
- исходная версия
- условия запуска rollback
- критерии успешного восстановления
- порядок восстановления
- критерии завершения rollback

осле rollback система должна подтвердить восстановление требуемого состояния.

Rollback должен регистрироваться как отдельное событие жизненного цикла изменения.

## Traceability
- [[ARCH-029]]



