# ARCH-OPS-038 — C2 link measurement integration

Status: IMPLEMENTATION BASELINE — CI verification pending

## Scope

Connect the executable per-channel runtime measurements to the existing C2 link-measurement bridge.

## Behavior

- latency is sourced from the latest accepted MAVLink TIMESYNC RTT when available;
- packet loss is sourced from accepted-frame sequence metrics;
- observed bandwidth is derived from runtime TX/RX throughput and exposed separately as `observed_bandwidth_kbps`;
- the pre-existing `capacity_kbps` field remains an explicitly declared capacity input and is not populated from observed traffic;
- measurement timestamp uses the latest receive timestamp, otherwise transmit timestamp;
- C2 Channel Manager remains responsible for qualification, selection and failover.

This deliberately prevents observed traffic from being mislabeled as physical channel capacity.

## Evidence

SIL_C2_RUNTIME_MEASUREMENT_INTEGRATION

No physical capacity, quality threshold, selection policy, failover execution, or field/radio certification evidence is claimed.
