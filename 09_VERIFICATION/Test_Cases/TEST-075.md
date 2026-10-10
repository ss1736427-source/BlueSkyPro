---
id: TEST-075
type: test_case
title: Insurance Preflight Eligibility and Policy Coverage Test
status: verified
verification_method: test
requirements:
  - SYS-REQ-052
  - SYS-REQ-053
  - SYS-REQ-054
---

# TEST-075 — Insurance Preflight Eligibility and Policy Coverage Test

## Objective

Verify that the preflight insurance gate evaluates the specific INSURED_UAV, applicable regulatory ruleset, policy validity, operation coverage, and required liability limit.

## Controlled scenarios

1. Valid policy + covered UAV + covered operation + sufficient limit -> PASS.
2. Expired policy -> BLOCK.
3. Policy belongs to another UAV -> BLOCK.
4. Operation outside policy coverage -> BLOCK or REVIEW according to configured policy rule.
5. Required insurance not applicable -> no insurance-only block.
6. Insufficient liability limit -> BLOCK.
7. Ambiguous coverage condition -> REVIEW.
8. All mandatory conditions valid -> immutable Insurance Preflight Snapshot created and linked to INSURED_UAV and Flight Record.

## Expected evidence

- input policy version;
- regulatory ruleset version;
- UAV identity;
- applicable legal requirement;
- calculated minimum limit;
- policy limit;
- decision;
- snapshot hash;
- evidence references.

## Execution

**VERIFIED — CI Run #687 (2026-09-23), workflow "BlueSky Autopilot Adapter", build-and-test job succeeded.**

The controlled C++ preflight test was compiled and executed by CTest:

- `insurance_preflight_engine_test`: **Passed** (CTest #47/49).
- The test suite covers valid policy, expiry, UAV mismatch, non-applicable insurance, insufficient liability, ambiguous coverage, Remote ID requirement, and stable snapshot digest.
- The readiness-gate implementation consumes the explicit `insurance_applicable` result; its regression coverage is included in the same CI build/test workflow.

Evidence:
- workflow run ID: `35865428007`
- job ID: `107195686232`
- head commit: `06c1cc4c9ac1e8c9779e554cca694ef1a7fc11f7`
- merged to main as `e67ddcf993eabb4e7fc9fad7f2e898f3a6d04514`

Verification conclusion: **PASS / VERIFIED** for the deterministic preflight engine and readiness-gate boundary. Legal applicability and insurer contractual interpretation remain ruleset/provider inputs rather than AI-derived decisions.
