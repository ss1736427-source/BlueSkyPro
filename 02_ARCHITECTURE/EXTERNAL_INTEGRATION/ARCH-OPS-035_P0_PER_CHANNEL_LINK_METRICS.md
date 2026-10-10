# ARCH-OPS-035 — P0 Per-Channel Link Metrics Runtime

## Objective
Close the executable portion of G2-04 for deterministic per-channel packet-loss/sequence metrics without duplicating C2 selection logic.

## Boundary
MavlinkTransportChannelRuntime → LinkMetricsRuntime → C2LinkRuntimeBridge → C2ChannelManager

### Transport Runtime
Owns channel lifecycle, transport ingress/egress, session binding, and accepted/rejected frame boundary.

### Link Metrics Runtime
Owns per-channel MAVLink sequence continuity, sequence wrap-around, inferred loss, duplicate/out-of-order counters, observation timestamps and age, and packet-loss ratio.

### C2 Link Bridge
Owns translation into ChannelSnapshot and validation of externally supplied link measurements.

### C2 Channel Manager
Owns channel qualification, deterministic selection, and failover eligibility.

## Sequence rule
MAVLink packet sequence is an 8-bit field and wraps at 255. Packet-loss calculation is valid only when observations belong to one channel; redundant channels must not be merged into one sequence stream.

## Safety boundary
No packet-loss threshold, latency threshold, automatic failover trigger, or safety action is introduced by this slice.

## Evidence
SIL_PER_CHANNEL_LINK_METRICS

## Remaining G2-04 work
Actual latency, capacity/bandwidth, integrity/authentication, and metric freshness integration require explicit measurement sources and validation. They are not inferred from transport connectivity alone.
