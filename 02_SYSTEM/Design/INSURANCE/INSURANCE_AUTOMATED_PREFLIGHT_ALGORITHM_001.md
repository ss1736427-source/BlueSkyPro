---
id: INS-ALG-001
type: system_design
title: Автоматизированный страховой допуск БПЛА перед полетом
status: draft
criticality: critical
---

# INS-ALG-001 — Автоматизированный страховой допуск БПЛА перед полетом

## 1. Основной принцип

Страховым объектом в модели BlueSky является **конкретный застрахованный БПЛА (INSURED_UAV)**. Миссия и полет являются событиями эксплуатации этого объекта и не заменяют страховой объект.

BlueSky не принимает решение о страховании вместо страховщика. Система выполняет машиночитаемую проверку применимости страхования, действительности полиса и соответствия конкретной эксплуатации условиям покрытия, а также формирует проверяемую доказательную запись.

## 2. Алгоритм

```
START FLIGHT
  -> IDENTIFY INSURED_UAV
  -> RESOLVE APPLICABLE REGULATORY RULESET
  -> CHECK POLICY
  -> CHECK UAV IDENTITY / REGISTRATION / ACCOUNTING
  -> CHECK ERA-GLONASS / REMOTE IDENTIFICATION, IF APPLICABLE
  -> CHECK TECHNICAL / MAINTENANCE STATUS
  -> CHECK CONFIGURATION
  -> CHECK OPERATOR / PILOT CONDITIONS
  -> CHECK AUTHORIZATION / REGULATORY STATUS
  -> CHECK OPERATION AGAINST POLICY COVERAGE
  -> CHECK OPERATING AREA / CONDITIONS
  -> CHECK WEATHER / UAV / POLICY LIMITS
  -> CHECK REQUIRED LIABILITY LIMIT
  -> CREATE IMMUTABLE INSURANCE PREFLIGHT SNAPSHOT
  -> DECISION: PASS | WARNING | REVIEW | BLOCK
  -> IF PASS: INSURANCE GO
  -> FLIGHT
  -> CAPTURE EVENTS / EVIDENCE
  -> CLOSE FLIGHT
  -> UPDATE UAV INSURANCE HISTORY / RISK PROFILE
```

## 3. Decision semantics

- **PASS** — mandatory insurance and policy conditions are satisfied.
- **WARNING** — condition is noteworthy but does not by itself prevent operation.
- **REVIEW** — automated rules cannot establish coverage with sufficient confidence; human/insurer review required.
- **BLOCK** — mandatory insurance absent/expired/inapplicable, identity mismatch, regulatory or policy condition not satisfied, or another configured blocking condition exists.

Absence of insurance must not block an operation when applicable law establishes that insurance is not required for that operation.

## 4. Required inputs

### Insured UAV
- aircraft ID;
- serial number;
- model;
- MTOM;
- registration/accounting number, as applicable;
- owner/operator;
- ERA-GLONASS / remote identification identity, as applicable;
- current configuration and configuration hash.

### Insurance policy
- policy ID;
- insurer;
- insured party;
- covered UAV;
- effective period;
- liability type;
- insured amount;
- territory;
- covered operation types;
- VLOS/BVLOS conditions, if applicable;
- operational limitations;
- exclusions;
- special conditions.

### Regulatory / operational
- applicable regulatory ruleset version;
- operation type;
- operating area;
- authorization/notification status;
- pilot/operator status;
- maintenance status;
- weather and operating conditions.

## 5. Minimum legal insurance check

For Russian operations, the Regulatory Engine shall calculate the applicable mandatory liability requirement from the current normative ruleset. For Article 131 cases, the calculation shall use MTOM and the legally applicable MROT-based formula; the legal value must be versioned and must not be hard-coded into the insurance provider adapter.

The system shall distinguish:
- legal minimum liability requirement;
- policy contractual limit;
- additional insurer-specific requirements.

## 6. Insurance Preflight Snapshot

Before GO the system shall create an immutable snapshot containing:
- insured UAV identity;
- policy identity and validity;
- regulatory ruleset version;
- applicable legal insurance requirement;
- policy limit;
- configuration;
- maintenance status;
- pilot/operator status;
- operation and area;
- authorization status;
- ERA-GLONASS/remote identification status, if applicable;
- weather/operating conditions;
- all rule results;
- final decision;
- timestamp;
- evidence references;
- snapshot hash.

## 7. Post-flight update

The completed flight shall update the insurance history of the **specific INSURED_UAV**, including:
- flight hours and cycles;
- operational exposure;
- incidents/anomalies/deviations;
- emergency events;
- C2/GNSS events;
- maintenance consequences;
- configuration changes;
- evidence integrity;
- claims-relevant events.

Mission identity remains contextual; UAV insurance history is the persistent insurance object.

## 8. Authority boundary

The insurance algorithm shall not:
- alter authoritative flight state directly;
- declare an accident or insured event solely from an AI inference;
- modify a policy;
- invent coverage conditions;
- replace the insurer's contractual decision.

Rules and evidence are deterministic and versioned. AI may analyze history and provide non-authoritative recommendations.

## 9. Traceability

- NORM-REQ-004
- SYS-REQ-052
- SYS-REQ-053
- INSURED_UAV
- Flight Record
- Configuration
- Maintenance
- Regulatory Engine
- ERA-GLONASS interface
- Insurance Evidence Layer
