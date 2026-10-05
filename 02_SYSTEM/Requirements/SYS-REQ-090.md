---
id: SYS-REQ-090
type: system_requirement
title: Overload Protection
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-028]]"
---

# SYS-REQ-090 — Overload Protection

## Requirement

HUB должен предотвращать каскадную перегрузку сервисов.

ри достижении установленных порогов система должна инициировать управляемое снижение нагрузки.

ерегрузка одного некритичного сервиса не должна приводить к отказу критических сервисов.

## Traceability
- [[ARCH-028]]



