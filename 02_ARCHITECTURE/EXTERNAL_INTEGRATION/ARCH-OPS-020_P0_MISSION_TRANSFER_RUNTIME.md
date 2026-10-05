# ARCH-OPS-020 — P0 Mission Transfer Runtime

**Status:** IMPLEMENTATION BASELINE

## Decision
Implement the smallest executable G0-04 slice using the existing Mission Interface and Universal Autopilot Adapter boundaries. Do not recreate the Mission Model, Mission Compiler, MAVLink session boundary, or command lifecycle runtime.

## Runtime path

```text
Mission Model
    ↓
Vehicle Capability Gate
    ↓
Universal Autopilot Adapter::compileMission()
    ↓
Connected-State Gate
    ↓
uploadMission()
    ↓
readBackMission()
    ↓
compareMission()
    ↓
VERIFIED / FAILED
```

## Acceptance boundary
A mission is not considered transferred merely because compilation succeeds. The runtime requires upload success, read-back availability, and an equality/integrity result from the adapter comparison boundary.

## Safety rule
Unsupported capability, vehicle/configuration mismatch, disconnected adapter, failed upload, missing read-back, or mission mismatch must produce an explicit rejection/failure outcome. No silent conversion or implicit acceptance is permitted.

## Non-goals
- Real MAVLink Mission Protocol transport.
- ArduPilot/PX4-specific command/item mapping.
- Physical UAV execution.
- HIL/flight-test/certification evidence.
- Partial physical-upload rollback.

## Relation to G0-02/G0-03
This runtime consumes the existing MAVLink session and command lifecycle boundaries where those are available, but does not duplicate either implementation.
