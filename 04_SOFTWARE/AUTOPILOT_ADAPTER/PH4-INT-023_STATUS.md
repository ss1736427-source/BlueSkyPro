# PH4-INT-023 — Contingency Command Behavior

**Status:** CLOSED — CI VERIFIED  
**Evidence boundary:** `SIL_FIXTURE_ONLY`  
**Baseline:** `MAVLINK2 / BASELINE-1`

## Purpose

Verify the normalized contingency-command contract at the BlueSky autopilot-adapter boundary without assuming that BlueSky replaces onboard flight-controller authority.

## Scope

The SIL verification covers the normalized contingency commands supported by the established contract:

- `HOLD`
- `RTL`
- `LAND`
- `ABORT`

The verification also covers rejection/negative paths for:

- unsupported command;
- invalid lifecycle/state;
- safety rejection;
- link-loss rejection.

## Safety boundary

BlueSky remains supervisory. The onboard flight-control system remains authoritative for stabilization and onboard failsafe behavior. A contingency request from BlueSky is a command request through the adapter, not a transfer of low-level flight-control authority.

Loss of the BlueSky connection is not interpreted as proof that a contingency command has been executed. Command acceptance, execution state, acknowledgement, reason, and timestamps remain correlated and traceable.

## Evidence

The implementation preserves the established EvidenceSession correlation model and produces evidence classified as `SIL_FIXTURE_ONLY`. No HIL, physical-UAV, flight-test, certification, or real MAVLink-transport evidence is claimed by this stage.

Evidence identifiers:

- Run: `TEST-RUN-PH4-INT-023`
- Evidence domain: `EC-03`
- Requirement: `REQ-INT-CONTINGENCY-COMMAND-BEHAVIOR`
- Test method: `TM-PH4-INT-023`
- Test case: `TC-PH4-INT-023`
- Configuration: `CFG-SIL-CONTINGENCY-023`
- Flight record: `FLIGHT-RECORD-PH4-INT-023`
- Mission: `MISSION-023`
- Vehicle: `UAV-CONTINGENCY-023`
- Source: `SRC-07`
- Protocol: `MAVLINK2`
- Protocol version: `BASELINE-1`

## Verification record

- CI workflow: `BlueSky Autopilot Adapter`
- Successful verification run: `#543`
- Verified commit: `d75b8fa5e5f69c1bddd9c8ffade7499f85efdad7`
- Pull request: `#52`
- Merge commit: `a177179e817517f04a43f9e1dd32609387b78b3c`

## Closure rule

PH4-INT-023 is closed only after implementation, automated verification, CI and merge were confirmed. PH4-INT-024 may now begin.
