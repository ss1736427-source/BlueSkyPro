# ARCH-OPS-022 — Vehicle State Adapter Mapping

**Status:** IMPLEMENTATION BASELINE — SIL/fixture scope

## Purpose

Close the smallest executable G0-06 slice without replacing ARCH-OPS-017. This layer maps an adapter state boundary or deterministic replay frame into the existing `NormalizedTelemetryState` contract.

## Scope

- preserve vehicle/source identity and timestamps;
- map validity and C2 connection facts;
- classify timestamp freshness conservatively;
- provide deterministic frame-to-normalized-state replay coverage.

## Explicit limitation

The current Universal Autopilot Adapter exposes scalar `NormalizedState` values rather than typed position, navigation, energy and health telemetry. Therefore this slice does **not** claim real ArduPilot/PX4 telemetry coverage. The adapter mapping remains conservative and returns unusable normalized state when the underlying fixture does not expose the required typed facts.

Evidence class: `SIL_FIXTURE_ONLY`.

Next G0-06 slice: concrete ArduPilot/PX4 MAVLink telemetry field mapping plus replay fixtures using typed telemetry frames.
