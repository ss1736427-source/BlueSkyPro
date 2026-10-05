# ARCH-OPS-023 — P0 MAVLink Typed Telemetry Mapping

## Decision

BlueSky keeps NormalizedTelemetryState as the single canonical runtime state. MAVLink-specific telemetry is represented first as a typed decoded sample and then mapped into the canonical state.

## Boundary

MAVLink dialect / decoder
        ↓
MavlinkTelemetrySample
        ↓
MavlinkTelemetryMapping
        ↓
NormalizedTelemetryState
        ↓
Operational Runtime

The mapping layer must not expose MAVLink message IDs or dialect-specific fields to the BlueSky operational core.

## Baseline field groups

| Domain | MAVLink reference | BlueSky mapping |
|---|---|---|
| Identity / presence | HEARTBEAT | provenance + C2/vehicle validity |
| Position / velocity | GLOBAL_POSITION_INT | position + navigation validity |
| Attitude | ATTITUDE | vehicle-state validity |
| Health | HEARTBEAT / SYS_STATUS | vehicle-state validity |
| Energy | battery telemetry / SYS_STATUS | energy-state validity |
| Mission state | mission-state telemetry | mission validity |

MAVLink common message definitions provide the reference semantics for HEARTBEAT, GLOBAL_POSITION_INT, ATTITUDE and SYS_STATUS. The mapping remains independent of a particular ArduPilot or PX4 dialect.

## Replay rule

Replay consumes ordered typed telemetry samples and returns the last state that satisfies the existing canonical usability gate. A delayed or incomplete sample cannot overwrite a previously usable state.

## Non-goals

- No raw MAVLink parser.
- No transport/socket implementation.
- No autopilot-specific mission semantics.
- No claim of real-flight verification.

## Traceability

- G0-06 Vehicle State.
- ARCH-OPS-017 normalized telemetry contract.
- ARCH-OPS-022 vehicle state mapping boundary.
- MAVLink reference integration for ArduPilot/PX4.
