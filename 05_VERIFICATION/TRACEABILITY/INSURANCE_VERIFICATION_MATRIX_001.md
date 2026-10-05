---
id: INS-VER-MATRIX-001
type: verification_traceability
title: Insurance verification matrix
status: verified
---

# Insurance Verification Matrix — 001

| Requirement | Verification case | Execution | Evidence | Configuration |
|---|---|---|---|---|
| SYS-REQ-052 | TEST-075 | VERIFIED — CI #687 | Run 35865428007 / job 107195686232 | main @ e67ddcf993eabb4e7fc9fad7f2e898f3a6d04514 |
| SYS-REQ-053 | TEST-075 | VERIFIED — CI #687 | Run 35865428007 / job 107195686232 | main @ e67ddcf993eabb4e7fc9fad7f2e898f3a6d04514 |
| SYS-REQ-054 | TEST-075 | VERIFIED — CI #687 | Run 35865428007 / job 107195686232 | main @ e67ddcf993eabb4e7fc9fad7f2e898f3a6d04514 |
| SYS-REQ-055 | TEST-076 | VERIFIED — CI #687 | Run 35865428007 / job 107195686232 | main @ e67ddcf993eabb4e7fc9fad7f2e898f3a6d04514 |

## Required chain

`NORMATIVE BASIS → REQUIREMENT → TEST CASE → EXECUTION RESULT → EVIDENCE → CONFIGURATION`

The deterministic insurance preflight and INSURED_UAV history/data-lineage boundaries now have recorded CI execution evidence.

## Evidence

- CI workflow: "BlueSky Autopilot Adapter"
- CI run: #687
- workflow run ID: 35865428007
- job: build-and-test
- job ID: 107195686232
- head under test: 06c1cc4c9ac1e8c9779e554cca694ef1a7fc11f7
- merge commit: e67ddcf993eabb4e7fc9fad7f2e898f3a6d04514
- `insurance_preflight_engine_test`: Passed
- `insurance_uav_history_test`: Passed

## Special controls

- regulatory ruleset version must be captured;
- policy version/data must be captured;
- INSURED_UAV identity must be captured;
- decision and snapshot hash must be captured;
- source lineage for derived risk metrics must be preserved.

## Scope boundary

Verification covers the deterministic implementation present in the referenced configuration. It does not certify legal applicability across all jurisdictions, insurer contract interpretation, or future risk-vector/claims extensions.
