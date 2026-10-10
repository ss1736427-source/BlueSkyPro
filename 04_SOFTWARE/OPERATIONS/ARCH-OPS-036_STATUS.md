# ARCH-OPS-036 — Per-channel latency runtime

Status: IMPLEMENTATION BASELINE — CI verification in progress

## Scope
Adds per-channel MAVLink round-trip latency measurement using TIMESYNC v2 request/response correlation.

## Runtime behavior
- Records outbound TIMESYNC requests by the transmitted ts1 timestamp.
- Correlates accepted TIMESYNC responses by echoed ts1.
- Measures host-observed round-trip time in milliseconds.
- Exposes probes sent, responses received, unmatched responses, last/min/max RTT, and measurement freshness timestamp.
- Resets latency state on channel reconnect.
- Does not introduce quality thresholds or channel-selection decisions.

## Evidence
SIL_MAVLINK_TIMESYNC_RTT

No field, radio, serial, SITL, HIL, physical, or certification evidence is claimed.
