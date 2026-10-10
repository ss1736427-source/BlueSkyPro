---
id: INS-RISK-002
type: system_design
title: UAV insurance risk source adapters
status: draft
criticality: high
---

# INS-RISK-002 — UAV insurance risk source adapters

## Purpose

Connect existing authoritative telemetry/evidence domains to the persistent `INSURED_UAV` risk vector without allowing source adapters to make insurance, safety, readiness or execution decisions.

## Source domains

- **Battery:** battery identity, state of health, capacity ratio, temperature and cycles.
- **Propulsion:** component identity, operating hours, fault rate, temperature and cycles.
- **C2/GNSS:** C2 loss duration, latency, GNSS deviation and navigation events.
- **Environmental:** flight exposure duration, wind, gusts, temperature, precipitation and adverse-weather exposure.

## Data path

`SOURCE RECORD → SOURCE ADAPTER → RISK OBSERVATION / EXPOSURE → INSURED_UAV RISK VECTOR`

Every accepted record retains UAV identity, source record, method/version, timestamp and evidence validity.

## Authority boundary

Adapters:
- normalize and validate source facts;
- preserve provenance;
- accumulate exposure.

Adapters do not:
- calculate legal insurance applicability;
- determine policy coverage;
- declare an insured event;
- block or authorize flight;
- modify Mission Readiness, Safety Gate, Approval or Execution state;
- produce an opaque overall risk score.

## Future layer

A separate deterministic calculation layer may derive reproducible indicators from these observations. AI may subsequently analyze those indicators and propose actions, but authoritative decisions remain outside the AI/source-adapter layer.
