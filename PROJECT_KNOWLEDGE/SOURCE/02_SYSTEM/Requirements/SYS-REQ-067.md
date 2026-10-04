---
id: SYS-REQ-067
type: system_requirement
title: HUB to PILOT Interface
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-026]]"
---

# SYS-REQ-067 — HUB to PILOT Interface

## Requirement

HUB должен обеспечивать стандартизированный интерфейс обмена с модулем PILOT.

HUB должен предоставлять PILOT как минимум:
- UAV State
- Navigation Data
- C2 Data
- Mission Data
- Weather
- Airspace
- Alerts
- Regulatory Constraints
- Configuration Parameters

PILOT должен предоставлять HUB как минимум:
- Flight State
- UAV Telemetry
- Position
- Attitude
- Navigation Status
- C2 Status
- Vehicle Health
- Payload State
- Flight Events
- Faults
- Emergency Events

## Traceability
- [[ARCH-026]]



