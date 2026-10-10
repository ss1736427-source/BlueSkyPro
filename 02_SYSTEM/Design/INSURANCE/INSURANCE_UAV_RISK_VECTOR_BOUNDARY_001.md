---
id: INS-RISK-001
type: system_design
title: Deterministic risk vector and exposure boundary for INSURED_UAV
status: draft
criticality: high
---

# INS-RISK-001 — INSURED_UAV risk vector boundary

## Purpose

Extend the persistent insurance history of a specific `INSURED_UAV` without collapsing heterogeneous evidence into an opaque risk score.

## Authoritative data classes

- risk observations: technical, maintenance, battery, propulsion, C2, navigation/GNSS, operational, environmental, compliance, pilot/operator, evidence quality;
- exposure observations: flight hours, cycles, BVLOS exposure, altitude exposure, adverse-weather exposure, configuration;
- claims evidence: claim/event linkage and immutable evidence references.

Every observation and exposure must identify the UAV, a source record, and valid evidence. Derived observations additionally retain calculation method and version.

## Deterministic boundary

The component:
- accepts only records belonging to its `INSURED_UAV`;
- rejects missing/invalid evidence and missing lineage;
- rejects duplicate record identifiers;
- accumulates exposure denominators deterministically;
- does not calculate a legal insurance requirement;
- does not decide coverage;
- does not authorize or block flight execution;
- does not mutate Mission Readiness, Safety Gate, Approval, or C++ execution state;
- does not turn observations into an overall risk score.

## Risk-vector principle

Risk remains a vector of independently traceable dimensions. Any future derived metric must preserve source records, method, rules/model version, calculation time, and evidence references.

## Scope boundary

This implementation establishes the deterministic data boundary. Dedicated maintenance, battery/propulsion telemetry adapters, C2/GNSS event adapters, environmental models, insurer-specific claims schemas, and statistical/AI analysis remain separate layers.
