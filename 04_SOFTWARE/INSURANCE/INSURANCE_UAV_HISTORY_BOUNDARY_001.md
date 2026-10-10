# INSURANCE_UAV_HISTORY_BOUNDARY_001

**Status:** implemented — verification pending

## Purpose

Maintain the persistent insurance history of the specific **INSURED_UAV**. Mission and Flight are operational context; they are not the persistent insurance object.

## Stored history

- completed flight exposure: count, hours, cycles and active configuration;
- maintenance/configuration changes with effective time;
- anomalies, deviations, incidents, serious incidents and accidents;
- derived metrics with source record identifiers, calculation method, method version and calculation timestamp.

## Data-lineage rules

A derived metric is authoritative only when:

1. it belongs to the same INSURED_UAV;
2. it has non-empty source records;
3. calculation method and method version are present;
4. every referenced source flight exists;
5. every referenced source has valid evidence.

Missing or corrupt source evidence cannot silently remain authoritative.

## Authority boundary

This component is a deterministic history/data-lineage service. It does not:

- authorize flight;
- change Mission Readiness;
- change Safety Gate state;
- approve insurance contracts;
- infer legal applicability;
- allow AI to bypass validation or evidence requirements.

## Verification

- SYS-REQ-055
- TEST-076
