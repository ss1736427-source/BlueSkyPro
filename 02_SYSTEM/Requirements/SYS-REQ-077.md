---
id: SYS-REQ-077
type: system_requirement
title: Capability-Based Task Allocation
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-027]]"
---

# SYS-REQ-077 — Capability-Based Task Allocation

## Requirement

Mission Planner должен распределять задачи между  на основании их фактических возможностей и текущего состояния.

ри выборе исполнителя должны учитываться как минимум:
- capability
- availability
- health
- energy
- range
- payload
- communication
- airspace constraints
- risk
- task priority

аспределение не должно основываться исключительно на типе или идентификаторе .

## Traceability
- [[ARCH-027]]



