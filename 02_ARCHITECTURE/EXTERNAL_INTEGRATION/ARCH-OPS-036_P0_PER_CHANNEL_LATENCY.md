# ARCH-OPS-036 — P0 per-channel latency measurement

## Gap
G2-04 requires executable link-management metrics. ARCH-OPS-035 established per-channel packet-loss/sequence metrics. This slice adds executable RTT measurement.

## Contract
Latency is measured only for a known outbound TIMESYNC request and a matching accepted response. The response must echo the request ts1; unmatched responses are counted but do not create a measurement.

The measured value is host-observed round-trip time between the request transmission timestamp and accepted response timestamp. It is not presented as one-way propagation delay.

## MAVLink basis
TIMESYNC v2 defines ts1 as the synchronizing component timestamp mirrored by the responder and permits the requester to determine round-trip time. Repeated measurements may be filtered to reduce transient channel/processing effects.

## Authority boundary
C2ChannelManager remains the sole channel qualification/selection/failover authority. ARCH-OPS-036 supplies measurement evidence only and introduces no thresholds or selection policy.

## Evidence
SIL_MAVLINK_TIMESYNC_RTT

No field/radio/serial/SITL/HIL/physical/certification evidence is claimed.
