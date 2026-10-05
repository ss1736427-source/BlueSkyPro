# ARCH-OPS-037 — P0 per-channel bandwidth / throughput runtime

## Purpose

Close the executable measurement portion of G2-04 for per-channel bandwidth observation without inventing a physical-link capacity estimate.

## Contract

The runtime records:

- transmitted bytes after a transport send succeeds;
- received bytes after the MAVLink session accepts the frame;
- first and last observation timestamps for each direction;
- average observed bytes per second for each direction when the elapsed interval is positive.

The runtime does not:

- claim physical radio capacity from observed traffic;
- generate saturation traffic;
- define link-quality thresholds;
- choose an active channel;
- perform failover;
- replace the C2 Channel Manager.

MAVLink packet serialization defines the actual on-wire frame length; this runtime therefore counts the complete frame bytes handed to or received from the transport boundary rather than only the payload.

## Integration boundary

MavlinkTransportChannelRuntime remains the executable channel/session boundary.

- send() observes bytes only after a successful transport send or deterministic queue handoff.
- receive() observes bytes only after session acceptance.
- reconnect() resets bandwidth metrics together with packet-loss and latency metrics.
- snapshot() exposes the per-channel bandwidth snapshot.

The existing C2 Channel Manager remains responsible for channel qualification, selection and failover eligibility. This slice only supplies a measured per-channel input.

## Evidence

SIL_PER_CHANNEL_OBSERVED_BANDWIDTH

This evidence is deterministic software evidence only. It is not field, radio, serial, SITL, HIL, physical, or certification evidence.
