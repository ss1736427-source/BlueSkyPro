---
id: SYS-REQ-087
type: system_requirement
title: AI Resource Isolation
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-028]]"
---

# SYS-REQ-087 — AI Resource Isolation

## Requirement

AI, машинное обучение, самообучение и аналитические процессы должны выполняться в изолированном ресурсном контуре.

AI-процессы не должны иметь возможности занять ресурсы, зарезервированные для flight control, navigation, C2 или safety.

ри ресурсном конфликте AI и аналитические процессы должны автоматически ограничиваться или приостанавливаться.

## Traceability
- [[ARCH-DEC-035]]
- [[ARCH-DEC-023]]
- [[ARCH-DEC-022]]
- [[ARCH-028]]



