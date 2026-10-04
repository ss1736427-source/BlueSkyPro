---
id: SYS-REQ-092
type: system_requirement
title: Redundant HUB Resource Recovery
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-028]]"
---

# SYS-REQ-092 — Redundant HUB Resource Recovery

## Requirement

ри переходе между HUB-A и HUB-B критические сервисы должны получать приоритет восстановления.

P0 и P1 должны восстанавливаться раньше P2–P5.

екритичные сервисы могут запускаться после восстановления критических сервисов.

ереход на резервный HUB не должен приводить к потере критического системного состояния в пределах установленных требований.

## Traceability
- [[ARCH-028]]



