# ARCH-OPS-026 STATUS

**Status:** IMPLEMENTATION BASELINE — VERSIONED RECORDED-STREAM SCOPE

Implemented a versioned recorded MAVLink stream seam above the concrete decoder boundary.

## Implemented

- versioned BlueSky recorded-stream envelope;
- deterministic text fixture reader;
- explicit ArduPilot/PX4 dialect preservation;
- message identity and source/receive timestamps preserved;
- unsupported message kinds and mismatched fixture versions rejected;
- recorded stream feeds ARCH-OPS-025 replay assembly unchanged;
- end-to-end fixture reaches the existing NormalizedTelemetryState.

## Evidence

SIL_RECORDED_DECODED_REPLAY

The fixture is a BlueSky decoded-message recording representation. It is not claimed to be a raw MAVLink wire capture.

## Architectural boundary

Recorded stream → Decoder boundary → Replay assembler → MavlinkTelemetrySample → NormalizedTelemetryState

No raw MAVLink types are introduced into the canonical operational state.

## Explicit limitation

This slice does not claim raw MAVLink byte parsing, real ArduPilot/PX4 transport, SITL/HIL, physical-UAV operation, flight-test or certification evidence.

## Next mandatory slice

Bind an actual MAVLink decoder/transport adapter or raw recorded MAVLink capture to the existing DecodedMavlinkMessage boundary and preserve the same replay/evidence chain.
