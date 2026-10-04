---
id: SYS-REQ-072
type: system_requirement
title: Message Addressing and Schema
status: draft
criticality: high
verification_method: inspection
derives_from:
  - "[[ARCH-026]]"
---

# SYS-REQ-072 — Message Addressing and Schema

## Requirement

аждое межмодульное сообщение должно содержать как минимум:
- message_id
- timestamp
- source
- destination
- message_type
- priority
- correlation_id
- schema_version
- payload

Структура сообщений должна быть формализована и версионироваться.

## Traceability
- [[ARCH-026]]



