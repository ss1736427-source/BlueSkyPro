# IF-MAVLINK — MAVLink Integration

**Priority:** P0
**Status:** STRUCTURE — TO BE BASELINED

## Scope

MAVLink is treated as an external protocol adapter, not as BlueSky's internal domain model.

## Required areas

- transport: serial / UDP / TCP / radio IP links as applicable
- MAVLink 2 session handling
- HEARTBEAT
- system/component discovery
- AUTOPILOT_VERSION / capabilities
- COMMAND_LONG / COMMAND_INT and COMMAND_ACK
- mission protocol
- parameter protocol
- telemetry subscriptions / message handling
- time synchronization
- status/error handling
- link-loss/recovery
- signing/security where required
- logging and replay

## Architecture

```text
BlueSky Core
     ↓
MAVLink Adapter
     ↓
MAVLink Session
     ↓
Transport
     ↓
Radio / Network / Serial
     ↓
Vehicle Autopilot
```

## Acceptance criterion

The adapter can establish a session, identify the vehicle/component set, exchange supported commands and state, transfer a mission, detect loss/recovery, and expose protocol failures to the BlueSky operational state machine.
