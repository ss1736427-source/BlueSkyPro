---
id: INS-DATA-001
type: system_design
title: Страховая модель данных конкретного БПЛА
status: draft
criticality: high
---

# INS-DATA-001 — Страховая модель данных конкретного БПЛА

## INSURED_UAV

Persistent insurance object:

- aircraft_id
- serial_number
- model
- MTOM
- registration/accounting_number
- owner_id
- operator_id
- remote_id / ERA-GLONASS identity
- current_configuration_id
- configuration_history
- maintenance_history
- flight_history
- exposure_history
- safety_history
- claims_history
- insurance_history

## Risk dimensions

The system shall retain separate dimensions rather than a single opaque risk score:

- technical;
- maintenance;
- battery;
- propulsion;
- C2;
- navigation/GNSS;
- operational;
- environmental;
- compliance;
- pilot/operator-related;
- evidence quality.

## Exposure

The accumulated denominator shall include, as applicable:

- flight count;
- flight hours;
- cycles;
- VLOS/BVLOS exposure;
- operating area exposure;
- altitude exposure;
- weather exposure;
- payload/configuration exposure.

## Event taxonomy

- NORMAL
- ADVISORY
- ANOMALY
- DEVIATION
- INCIDENT
- SERIOUS_INCIDENT
- ACCIDENT

Each event shall include source, timestamp, UAV, flight, phase, location where available, severity, evidence reference, and disposition.

## Data lineage

Every derived insurance metric shall retain:
- source records;
- calculation method;
- rules/model version;
- calculation timestamp;
- evidence references.

A derived metric shall never replace its source evidence.
