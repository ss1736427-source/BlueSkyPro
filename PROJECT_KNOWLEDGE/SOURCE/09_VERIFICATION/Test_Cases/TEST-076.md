---
id: TEST-076
type: test_case
title: UAV Insurance History Accumulation and Data Lineage Test
status: verified
verification_method: test
requirements:
  - SYS-REQ-055
---

# TEST-076 — UAV Insurance History Accumulation and Data Lineage Test

## Objective

Verify that completed flights update the insurance history of the specific INSURED_UAV and that derived metrics remain traceable to source evidence and calculation/ruleset versions.

## Controlled scenarios

1. Complete a normal flight -> flight count and exposure increase.
2. Add a maintenance/configuration change -> history is attached to the UAV and effective period.
3. Add an anomaly/incident -> event appears in UAV safety history.
4. Add a new flight under a different configuration -> configuration-specific history remains distinguishable.
5. Recalculate a metric -> source records, method, version and timestamp are retained.
6. Corrupt/remove source evidence -> derived metric must not silently remain authoritative.

## Expected evidence

- UAV history record;
- exposure counters;
- event records;
- configuration history;
- source references;
- calculation method/version;
- integrity status.

## Execution

**VERIFIED — CI Run #687 (2026-09-23), workflow "BlueSky Autopilot Adapter", build-and-test job succeeded.**

The new deterministic history implementation was compiled and executed by CTest:

- `insurance_uav_history_test`: **Passed** (CTest #49/49).
- Coverage includes exposure accumulation, configuration changes, incident history, configuration-specific flights, authoritative derived metrics with source/method/version lineage, wrong-UAV rejection, missing evidence rejection, and missing lineage rejection.

Evidence:
- workflow run ID: `35865428007`
- job ID: `107195686232`
- head commit: `06c1cc4c9ac1e8c9779e554cca694ef1a7fc11f7`
- merged to main as `e67ddcf993eabb4e7fc9fad7f2e898f3a6d04514`

Verification conclusion: **PASS / VERIFIED** for the implemented deterministic UAV insurance-history and data-lineage boundary.

Scope note: the current implementation represents maintenance-related continuity through configuration changes/events; dedicated maintenance, battery, propulsion, C2/GNSS and environmental exposure records remain subsequent extensions of SYS-REQ-055.
