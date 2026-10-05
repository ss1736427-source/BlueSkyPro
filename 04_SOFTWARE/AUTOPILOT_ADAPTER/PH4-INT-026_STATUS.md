# PH4-INT-026 — Capability Manifest and Compatibility Gate

**Status:** CLOSED — CI VERIFIED  
**Evidence boundary:** `SIL_FIXTURE_ONLY`  
**Baseline:** `MAVLINK2 / BASELINE-1`

## Purpose

Establish a normalized machine-readable capability manifest and deterministic compatibility gate at the Universal Autopilot Adapter boundary. The gate prevents unsupported protocol/version combinations from entering the operational adapter lifecycle while preserving the existing protocol-independent API.

## Closed scope

- normalized adapter capability manifest;
- adapter, vendor, FCS, vehicle, protocol and protocol-version identity;
- declared command, mission, parameter, telemetry, equipment, failsafe and log capabilities;
- explicit verification status and known limitations;
- compatibility evaluation against required protocol/version and required mission capability;
- deterministic rejection with `INCOMPATIBLE_VERSION` or `CAPABILITY_MISSING` where applicable;
- successful compatibility acceptance for the baseline MAVLink2 fixture;
- EvidenceSession correlation and provenance preservation.

## Safety and boundary

Compatibility evaluation is a readiness gate. It does not execute flight commands and does not change onboard flight-controller authority.

Protocol-specific details remain inside the adapter boundary. The BlueSky core consumes normalized manifest and compatibility results only.

## Verification

The PH4-INT-026 SIL fixture and automated EvidenceSession verification passed repository CI.

CI Run: **#578**  
Workflow: **BlueSky Autopilot Adapter**  
Result: **success**  
Validated steps: repository consistency, Configure, Build, Evidence Recorder, Test.

PR: **#55**  
Merge commit: `5a9ba793c05ed12f5180280b657d7eeb83887085`

## Evidence boundary

Evidence remains classified as `SIL_FIXTURE_ONLY`. No HIL, physical-UAV, flight-test, certification, or real transport evidence is claimed.

## Exit condition

PH4-INT-026 implementation, automated verification, CI and merge are confirmed. The stage is closed.

PH4-INT-027 may now begin.
