---
id: SYS-REQ-088
type: system_requirement
title: Safe Self-Learning
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-028]]"
---

# SYS-REQ-088 — Safe Self-Learning

## Requirement

роцессы самообучения должны выполняться только при наличии достаточного свободного ресурса.

бновление моделей не должно нарушать критические функции системы.

овая модель должна проходить контролируемую проверку до использования в критическом operational loop.

Система должна поддерживать контроль версии и возможность отката модели.

Самообучение не должно автоматически изменять safety-critical поведение без предусмотренного механизма валидации и разрешения.

## Traceability
- [[ARCH-DEC-026]]
- [[ARCH-DEC-025]]
- [[ARCH-DEC-011]]
- [[ARCH-028]]



