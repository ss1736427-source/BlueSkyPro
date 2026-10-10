# ARCH-OPS-035 — Link Metrics Runtime

## Status
IMPLEMENTED — CI pending

## Scope
Per-channel runtime measurement of MAVLink packet-sequence continuity and observation freshness.

## Implemented
- 8-bit MAVLink sequence tracking per transport channel.
- Sequence wrap-around handling.
- Inferred missing packet count from forward sequence gaps.
- Duplicate and out-of-order counters.
- Packet-loss ratio derived from observed and inferred-lost packets.
- First/last observation timestamps.
- Metric age calculation without a hard-coded quality threshold.
- Metrics reset on transport/session reconnect.
- Metrics exposed in MavlinkTransportChannelSnapshot.
- Metrics are updated only after the existing MAVLink decoder/session boundary accepts a frame.
- Deterministic memory and real UDP runtime tests.

## Responsibility boundary
Transport runtime owns channel lifecycle and ingress.
LinkMetricsRuntime owns per-channel metric aggregation.
C2LinkRuntimeBridge translates measurements into the existing C2 contract.
C2ChannelManager remains the authority for channel qualification, selection, and failover eligibility.

## Evidence
Target evidence: SIL_PER_CHANNEL_LINK_METRICS.

## Limitations
- No field, radio, serial, SITL, HIL, physical, or certification evidence.
- No quality thresholds or automatic failover policy are introduced.
- Latency/capacity/authentication measurement is intentionally not fabricated from socket connectivity; those measurements remain an explicit future measurement source.
