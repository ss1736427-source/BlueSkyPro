---
id: SYS-REQ-063
type: system_requirement
title: HUB Health Monitoring
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-025]]"
---

# SYS-REQ-063 — HUB Health Monitoring

## Requirement

HUB должен контролировать:
- CPU;
- memory;
- storage;
- network;
- процессы;
- сервисы;
- очереди сообщений;
- синхронизацию резервных экземпляров;
- внешние интерфейсы;
- внутренние сервисы.

Система должна использовать результаты мониторинга для обнаружения деградации и отказов.

## Traceability
- [[ARCH-025]]



