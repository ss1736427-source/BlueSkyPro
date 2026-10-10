# ARCH-OPS-023 — MAVLink Typed Telemetry Mapping

**Status:** IMPLEMENTATION BASELINE — SIL/REPLAY SCOPE

## Objective

Close the next G0-06 slice without creating a second normalized telemetry model.

## Implemented

- Typed, protocol-facing MavlinkTelemetrySample boundary.
- Explicit identity: vehicle, source, MAVLink system/component IDs.
- Typed position, navigation, attitude, battery, health/heartbeat and mission fields.
- Deterministic mapping into the canonical NormalizedTelemetryState from ARCH-OPS-017.
- Conservative timestamp freshness handling inherited from the existing normalized-state policy.
- Deterministic replay fixture selecting the last usable normalized state.
- Regression tests for valid state, heartbeat loss, incomplete position, replay recovery and missing timestamps.

## MAVLink reference message coverage

The typed sample is aligned to standard/common MAVLink message families used for this baseline:

- HEARTBEAT — identity, system status and mode context.
- GLOBAL_POSITION_INT — filtered global position, relative altitude, ground velocity and heading.
- ATTITUDE — roll/pitch/yaw and angular rates.
- SYS_STATUS / battery telemetry — health and energy validity.
- Mission-state telemetry — represented at the normalized adapter boundary and intentionally not tied to one autopilot dialect.

The implementation does not parse raw MAVLink wire bytes. It consumes a decoded typed sample so that transport/session and dialect-specific decoding remain below the BlueSky normalization boundary.

## Safety rules

- Missing timestamps are Missing.
- Source timestamps older than receive time are Delayed and cannot become Current.
- Missing required position/heartbeat/energy fields cannot be promoted to valid state.
- normalized_state_usable() remains the sole conservative usability gate.

## Evidence class

SIL_REPLAY_FIXTURE_ONLY

This does not constitute real-aircraft telemetry verification.

## Next G0-06 slice

Add concrete ArduPilot/PX4 dialect decoding adapters that populate MavlinkTelemetrySample, followed by integration/replay evidence against real or recorded MAVLink streams.
