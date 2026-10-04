---
id: SYS-REQ-103
type: system_requirement
title: Controlled Model Update and Rollback
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-029]]"
---

# SYS-REQ-103 — Controlled Model Update and Rollback

## Requirement

аждая используемая AI-модель должна иметь идентификатор и версию.

бновление модели должно проходить контролируемую проверку до использования в соответствующем operational loop.

ля изменений, предусматривающих rollback, должна сохраняться возможность возврата к предыдущей версии.

Rollback должен регистрироваться как отдельное событие.

## Traceability
- [[ARCH-029]]



