# ARCH-OPS-024 STATUS

**Status:** IMPLEMENTATION BASELINE — SIL/DECODED-FIXTURE SCOPE

Implemented the concrete common decoder boundary for ArduPilot/PX4 MAVLink telemetry.

## Implemented

- typed decoded message boundary;
- explicit ArduPilot/PX4 dialect identity;
- MAVLink2 reference version;
- system/component identity preservation;
- deterministic rejection of invalid identity, timestamps and decoded messages;
- mapping of HEARTBEAT, GLOBAL_POSITION_INT, ATTITUDE, SYS_STATUS and mission-state fixtures;
- output into the existing MavlinkTelemetrySample only;
- automated CTest coverage.

## Evidence

SIL_DECODED_FIXTURE_ONLY

No raw MAVLink wire parsing or real autopilot connectivity is claimed by this slice.

## Next mandatory slice

Bind actual/recorded ArduPilot and PX4 MAVLink decoder output to the common decoded-message boundary and add deterministic replay/integration evidence.