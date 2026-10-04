---
id: SYS-REQ-079
type: system_requirement
title: Common Fleet Mission State
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-027]]"
---

# SYS-REQ-079 — Common Fleet Mission State

## Requirement

HUB Core должен поддерживать единое актуальное состояние гетерогенной группы .

Состояние должно включать как минимум:
- mission_state
- fleet_state
- UAV_state
- task_state
- capability_state
- communication_state
- health_state
- risk_state

## Traceability
- [[ARCH-027]]



