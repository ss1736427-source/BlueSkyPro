# ARCH-OPS-025 STATUS

**Status:** IMPLEMENTATION BASELINE — SIL/DECODED-REPLAY SCOPE

Implemented deterministic aggregation of decoded MAVLink telemetry messages into per-vehicle replay snapshots.

## Implemented

- common ArduPilot/PX4 replay path;
- identity/session consistency;
- decoded-message validation gate;
- multi-message telemetry assembly;
- monotonic source/receive timestamp propagation;
- deterministic replay into existing canonical normalization;
- automated CTest coverage.

## Evidence

SIL_DECODED_REPLAY_ONLY

No raw wire parsing, SITL/HIL, real vehicle connectivity or flight verification is claimed.

## Next mandatory slice

Bind a versioned recorded MAVLink stream or concrete external decoder adapter to the ARCH-OPS-024 boundary and execute end-to-end replay evidence.