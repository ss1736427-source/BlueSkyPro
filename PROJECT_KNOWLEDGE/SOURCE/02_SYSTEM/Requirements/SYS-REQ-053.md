---
id: SYS-REQ-053
type: system_requirement
title: Привязка страхования к конкретному БПЛА и операции
status: draft
criticality: critical
verification_method: analysis
derives_from:
  - "[[NORM-REQ-004]]"
---

# SYS-REQ-053 — Привязка страхования к конкретному БПЛА и операции

## Requirement

Страховая запись должна иметь постоянным страховым объектом конкретный INSURED_UAV.

Операция/миссия/полет являются контекстом эксплуатации застрахованного БПЛА и должны быть связаны с ним.

Связь должна включать:

- INSURED_UAV;
- Operator / Owner;
- Mission / Flight;
- MTOM;
- Registration / Accounting;
- Operating Area;
- Operating Conditions;
- Configuration;
- Maintenance status;
- Insurance Policy;
- Regulatory Assessment;
- Insurance Preflight Snapshot.

Полёт не должен получать статус Ready, если обязательное страхование отсутствует, истекло или не соответствует применимым требованиям операции.

Если страхование для конкретной операции законодательством не требуется, система не должна блокировать полёт только по причине отсутствия полиса.

## Traceability

- [[NORM-REQ-004]]
- [[INS-ALG-001]]
- [[INS-DATA-001]]
