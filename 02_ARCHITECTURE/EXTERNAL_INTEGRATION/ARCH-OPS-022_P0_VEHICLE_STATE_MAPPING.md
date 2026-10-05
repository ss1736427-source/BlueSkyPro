# ARCH-OPS-022 — P0 Vehicle State Mapping Boundary

## Decision

Use the existing ARCH-OPS-017 `NormalizedTelemetryState` as the sole canonical runtime state boundary. Do not create a second vehicle-state model.

The smallest executable G0-06 slice is a mapping boundary:

`adapter/replay telemetry frame → VehicleStateMapping → NormalizedTelemetryState → runtime`

## Safety semantics

- Missing timestamps produce `Missing`.
- Source timestamps older than reception are `Delayed`; no delayed frame is treated as current.
- Required operational usability remains governed by `normalized_state_usable()`.
- The fixture adapter is not treated as real autopilot telemetry evidence.

## Evidence boundary

This implementation is SIL/fixture-only. It does not establish ArduPilot/PX4 wire-level telemetry correctness, HIL evidence, or representative-UAV verification.

## Traceability

- G0-06 — Vehicle state
- SYS-AUT-003 — Normalize telemetry/health/state
- ARCH-OPS-017 — canonical normalized telemetry state
