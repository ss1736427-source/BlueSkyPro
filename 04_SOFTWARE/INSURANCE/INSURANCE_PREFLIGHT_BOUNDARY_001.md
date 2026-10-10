# INSURANCE_PREFLIGHT_BOUNDARY_001 — Deterministic Insurance Preflight Engine

**Status:** implemented — verification pending
**ID:** INS-BOUNDARY-001

## Purpose

Implement the executable deterministic boundary required by SYS-REQ-054 without making insurance an execution authority.

## Authority boundary

REGULATORY RULESET + INSURED_UAV + POLICY + OPERATION
-> INSURANCE PREFLIGHT ENGINE
-> PASS / WARNING / REVIEW / BLOCK
-> INSURANCE PREFLIGHT SNAPSHOT
-> MISSION VALIDATION
-> SAFETY GATE
-> AUTHORIZATION
-> C++ CORE
-> EXECUTION

The engine does not authorize, upload, start, or mutate a mission or UAV state.

## Deterministic controls

- insurance applicability is supplied by a versioned regulatory ruleset;
- minimum liability is calculated from ruleset parameters, not hard-coded provider logic;
- the persistent insurance object is the specific INSURED_UAV;
- policy identity, validity, coverage, limits and conditions are checked;
- applicable remote identification status is checked;
- ambiguous evidence produces REVIEW rather than an invented contractual conclusion;
- mandatory failed conditions produce BLOCK;
- a non-applicable insurance requirement does not create an insurance-only block;
- the snapshot contains the ruleset/policy/UAV decision context and a deterministic integrity digest;
- the snapshot has no mutating public API;
- cryptographic signing remains outside this boundary.

## Decision precedence

BLOCK > REVIEW > WARNING > PASS

WARNING is non-blocking. The final mission readiness gate remains authoritative and may impose additional non-insurance conditions.

## Verification

Primary controlled case: TEST-075.

No capability is considered VERIFIED until controlled CTest execution evidence is recorded.
